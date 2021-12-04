//https://www.boost.org/doc/libs/master/libs/beast/example/http/client/sync/http_client_sync.cpp
//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: HTTP client, synchronous
//
//------------------------------------------------------------------------------

//[example_http_client

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

/* davep 20210616 ; adding base64 */
//#include "base64.h"

#include "netrc.hpp"

/* davep 20210730 ; new base64 library via vcpkg */
// https://renenyffenegger.ch/notes/development/Base64/Encoding-and-decoding-base-64-with-cpp/
#include "cpp-base64/base64.h"

//#include "n-json.h"
#include <nlohmann/json.hpp>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

// https://www.boost.org/community/error_handling.html
struct NoPasswordException : std::runtime_error
{
	NoPasswordException() : std::runtime_error("Missing Password") { } 
};

struct TransactionFailedException : std::runtime_error
{
	TransactionFailedException() : std::runtime_error("api call failed") { }
	TransactionFailedException(const char* errmsg) : std::runtime_error(errmsg) { }
};

void check_netrc(std::string host, std::string& username, std::string& password)
{
	netrc_map netrc;

	try {
		netrc = netrc_parse_default_file();
	}
	catch (file_error& err) {
		return;
	}

	while(1) {
		try {
			auto [netrc_login, netrc_password, netrc_account] = netrc.at(host);
			username = std::move(netrc_login);
			password = std::move(netrc_password);
			return;
		}
		catch (file_error& err) {

		}
		catch (std::out_of_range & err) {
			std::clog << "netrc has no entry for " << host << "\n";
		}
		if (host == "default") {
			return;
		}
		host = "default";
	}
}

// Performs an HTTP GET and prints the response
int main(int argc, char** argv)
{
    try
    {
        // Check command line arguments.
        if(argc != 4 && argc != 5)
        {
            std::cerr <<
                "Usage: http-client-sync <host> <port> <target> [<HTTP version: 1.0 or 1.1(default)>]\n" <<
                "Example:\n" <<
                "    http-client-sync www.example.com 80 /\n" <<
                "    http-client-sync www.example.com 80 / 1.0\n";
            return EXIT_FAILURE;
        }
        auto const host = argv[1];
        auto const port = argv[2];
        auto const target = argv[3];
        int version = argc == 5 && !std::strcmp("1.0", argv[4]) ? 10 : 11;

        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

        // Make the connection on the IP address we get from a lookup
        stream.connect(results);

        // Set up an HTTP GET request message
        http::request<http::string_body> req{http::verb::get, target, version};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

	std::string username { "admin" };
	std::string password;

	// read netrc before trying higher priority methods
	check_netrc(target, username, password);

	/* davep 20210615 ; attempt http auth */
	const char *pw = getenv("CP_PASSWORD");
	if (pw) {
		password.assign(pw);
	}
	if ( password.empty() ) {
		throw NoPasswordException();
	}
//	std::string password = pw;
//	std::string username { "admin" };
	std::string upw = username + ":" + password;
	std::string auth = "Basic " + base64_encode(upw);
//	std::cout << "auth=" << auth << "\n";
	req.set(http::field::authorization, auth);

        // Send the HTTP request to the remote host
        http::write(stream, req);

        // This buffer is used for reading and must be persisted
        beast::flat_buffer buffer;

		// davep 20210617 ; change body to std::string to make getting data for
		// json parsing easier
        // Declare a container to hold the response
        http::response<http::string_body> res;
//        http::response<http::dynamic_body> res;

        // Receive the HTTP response
        http::read(stream, buffer, res);

#if 0
        // Write the message to standard out
//        std::cout << res << std::endl;
		std::cout << "result=" << res.result() << "\n";
		std::cout << "result_int=" << res.result_int() << "\n";
		std::cout << "reason=" << res.reason() << "\n";
//		std::string buf;
//		std::cout << "body=" << res.body() << "\n";
#endif

//		std::string s = boost::beast::buffers_to_string(res.body().data());
		std::string s = res.body().data();
//		std::cout << "s=" << s << "\n";

//		nlohmann::json j2 = nlohmann::json::parse(res.begin(), res.end());

		nlohmann::json j = nlohmann::json::parse(s);
		if (j["success"] != true) {
			auto err_json = j.dump();
			std::cout << s << "\n";
			std::cout << err_json << "\n";
			throw TransactionFailedException();
		}
		if ( j["data"] == nullptr) {
			throw TransactionFailedException("No data");
		}

		std::cout << j.dump() << "\n";

#if 0
		for (auto const& node : j["data"]) {
			std::cout << node.dump() << "\n";
//			std::cout << node[1] << "\n";
		}
#endif

        // Gracefully close the socket
        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes
        // so don't bother reporting it.
        //
        if(ec && ec != beast::errc::not_connected)
            throw beast::system_error{ec};

        // If we get here then the connection is closed gracefully
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

//]
