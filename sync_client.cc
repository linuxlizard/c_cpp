//https://www.boost.org/doc/libs/1_49_0/doc/html/boost_asio/example/http/client/sync_client.cpp
//
// sync_client.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

/* davep 20210616 ; adding auth; adding base64; adding json */
// https://github.com/ReneNyffenegger/cpp-base64
// https://github.com/nlohmann/json
// gcc -g -Wall -o sync_client sync_client.cc -std=c++17 -I../cpp-base64 -I../n-json/include ../cpp-base64/base64-17.o -lstdc++ -lpthread

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>

#include "base64.h"

#include <nlohmann/json.hpp>

// for convenience
 using json = nlohmann::json;

using boost::asio::ip::tcp;

// https://www.boost.org/community/error_handling.html
struct NoPasswordException : std::runtime_error
{
	NoPasswordException() : std::runtime_error("Missing Password") { } 
};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cout << "Usage: sync_client <server> <path>\n";
      std::cout << "Example:\n";
      std::cout << "  sync_client www.boost.org /LICENSE_1_0.txt\n";
      return 1;
    }

    boost::asio::io_service io_service;

    // Get a list of endpoints corresponding to the server name.
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(argv[1], "http");
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    // Try each endpoint until we successfully establish a connection.
    tcp::socket socket(io_service);
    boost::asio::connect(socket, endpoint_iterator);

    // Form the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.
    boost::asio::streambuf request;
    std::ostream request_stream(&request);
    request_stream << "GET " << argv[2] << " HTTP/1.0\r\n";
    request_stream << "Host: " << argv[1] << "\r\n";
    request_stream << "Accept: */*\r\n";

	/* davep 20210615 ; attempt http auth */
	const char *pw = getenv("CP_PASSWORD");
	if (!pw) {
		throw NoPasswordException();
	}
	std::string password = pw;
	std::string username { "admin" };
	std::string upw = username + ":" + password;
	std::string auth = "Basic " + base64_encode(upw);
	std::cout << "auth=" << auth << "\n";
	request_stream << "Authorization: " + auth + "\r\n";

    request_stream << "Connection: close\r\n\r\n";

    // Send the request.
    boost::asio::write(socket, request);

    // Read the response status line. The response streambuf will automatically
    // grow to accommodate the entire line. The growth may be limited by passing
    // a maximum size to the streambuf constructor.
    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\r\n");

    // Check that response is OK.
    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline(response_stream, status_message);
    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
    {
      std::cout << "Invalid response\n";
      return 1;
    }

    // Read the response headers, which are terminated by a blank line.
    boost::asio::read_until(socket, response, "\r\n\r\n");

    // Process the response headers.
    std::string header;
    while (std::getline(response_stream, header) && header != "\r")
      std::cout << header << "\n";
    std::cout << "\n";

	std::string buf;
    // Write whatever content we already have to output.
//    if (response.size() > 0)
//      std::cout << &response;

	// read into string
	response_stream >> buf;
	std::cout << "buf=" << buf << "\n";

    // Read until EOF, writing data to output as we go.
    boost::system::error_code error;
    while (boost::asio::read(socket, response,
          boost::asio::transfer_at_least(1), error)) {
//      std::cout << &response;
	}

    if (status_code != 200)
    {
      std::cout << "Response returned with status code " << status_code << "\n";
      return 1;
    }

    if (error != boost::asio::error::eof)
      throw boost::system::system_error(error);
  }
  catch (std::exception& e)
  {
    std::cout << "Exception: " << e.what() << "\n";
  }

  return 0;
}
