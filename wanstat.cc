/*
 * https://github.com/Microsoft/cpprestsdk/wiki/Getting-Started-Tutorial
 * https://github.com/Microsoft/cpprestsdk/wiki/Programming-with-Tasks
 * https://github.com/Microsoft/cpprestsdk/wiki/JSON
 *
 * g++ -g -Wall -o bing -std=c++14 bing.cc -lstdc++ -lcrypto -lssl -lcpprest -lboost_system
 *
 * dnf install cpprest-devel boost-devel
 * sudo apt-get install libcpprest-dev  # seems to drag along the rest of the dependencies
 *
 * davep 20180705 ;
 */
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <thread>
#include <chrono>
#include <boost/program_options.hpp>
#include <cstdlib>
//#include <boost/regex.hpp>
#include <regex>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

namespace po = boost::program_options;

void parse_uri(const std::string& uri)
{
	/* davep 20180818 ; playing with RFC3986 */
	std::cmatch what;

	// https://tools.ietf.org/html/rfc3986#page-50
	std::regex uri_regex("^[^:/?#]+://",
			std::regex::extended);
//	std::regex uri_regex("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?",
//			std::regex::extended);

	// interesting that regex_match() doesn't take std::string but wants a c_string
	if (std::regex_match(uri.c_str(), what, uri_regex)) {
		std::cout << "Match!\n";
	}

}

int main(int argc, char* argv[])
{
	// https://www.boost.org/doc/libs/1_67_0/doc/html/program_options/tutorial.html#id-1.3.31.4.3
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("debug", po::value<int>(), "set debug level")
		("target", po::value<std::string>(), "target")
	;
	po::variables_map vm;        
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);    

	if (vm.count("help")) {
		std::cout << desc << std::endl;
		return 1;
	}

	parse_uri(vm["target"].as<std::string>());
	return 0;

	// some defaults
	std::string scheme = U("http");
	std::string username = U("admin");
	std::string password = U("12345");
	int http_port = 80;
	std::string host;


	if (vm.count("target")) {
		std::string target_str = vm["target"].as< std::string >();
		std::cout << "target=" << target_str << "\n";
		if (!web::uri::validate(target_str)) {
			std::cerr << "invalid target\n";
			return EXIT_FAILURE;
		}

		auto target_uri = web::uri(target_str);
		auto target_fields = target_uri.split_query(target_str);
		auto target_vec = target_uri.split_path(target_str);

		scheme = target_uri.scheme();
		std::cout << "scheme=" << scheme << "\n";
		host = target_uri.host();
		std::cout << "host=" << target_uri.host() << "\n";
		std::cout << "user_info=" << target_uri.user_info() << "\n";
		std::cout << "uri=" << target_uri.to_string() << "\n";
		http_port = target_uri.port();
		std::cout << "http_port=" << http_port << "\n";

		std::cout << "is_authority=" << target_uri.is_authority() << "\n";

		// check for password in the URI
		std::string user_info = target_uri.user_info();
		auto pos = user_info.find(':');
		if (pos) {
			// maybe has a password
			// TODO
		}
		else {
			username = user_info;
		}

	}

	// password no longer supported in URI 
	// https://tools.ietf.org/html/rfc3986
	// Use of the format "user:password" in the userinfo field is
	// deprecated.  Applications should not render as clear text any data
	// after the first colon (":") character found within a userinfo
	// subcomponent unless the data after the colon is the empty string
	// (indicating no password).
	const char * p = std::getenv("CP_PASSWORD");
	if (p) {
		password.assign(p);
	}

	credentials cred(username, password);
//	credentials cred(U("admin"), U("00000000"));
	http_client_config config;
	config.set_credentials(cred);

	uri_builder uri;
	uri.set_scheme(scheme);
	uri.set_host(host);
	uri.set_port(http_port);
	http_client client(uri.to_uri(), config);
//	http_client client(U("http://localhost:10000/"), config);
//	http_client client(U("http://admin:00000000@localhost:10000/"));

	uri_builder builder(U("/api/status/wan/connection_state"));
	web::http::http_response data_response; 
	try {
		auto response = client.request(methods::GET, builder.to_string());
		response.wait();
		data_response = response.get(); 
	}
	catch (web::http::http_exception & err) {
		std::cerr << "Error! " << err.what() << std::endl;
		return EXIT_FAILURE;
	}

	web::http::http_headers headers = data_response.headers();
	std::cout << data_response.to_string() << std::endl;
	auto json_response = data_response.extract_json();
	// blocking wait for response
	// XXX why do I have to .wait() to get data I already have locally?
	// something to do with the async core of this library?
	json_response.wait();
	json::value obj = json_response.get();
	std::cout << "obj=" << obj << std::endl;
	json::value success = obj.at(U("success"));
	json::value data = obj.at(U("data"));

	builder.clear();
	builder.set_path(U("/api/status/wan/devices"));
	std::cout << builder.to_string() << std::endl;

	auto response = client.request(methods::GET, builder.to_string());

	response.wait();
//	// try polling out of curiousity
//	while( ! response.is_done()) {
//		std::cout << response.is_done() << std::endl;
//		// https://stackoverflow.com/questions/4184468/sleep-for-milliseconds
//		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//	}

	data_response = response.get(); 
	headers = data_response.headers();

//	std::cout << data_response.to_string() << std::endl;

	json_response = data_response.extract_json();
	json_response.wait();
	obj = json_response.get();
//	std::cout << "obj=" << obj << std::endl;

//	json::value obj = json::value::parse(data_response.to_string());
//	utility::stringstream_t ss1;
//	ss1 << data_response.to_string();
//	json::value obj = json::value::parse(ss1);
//	json::value obj = json::value::parse(U("{\"success\": true, \"data\": \"connected\"}"));
	success = obj.at(U("success"));
	std::cout << "success=" << success << std::endl;
	data = obj.at(U("data"));
//	std::cout << "data=" << data << " is array=" << data.is_array() << std::endl;

	// parse the response
	json::object obj2 = data.as_object();
	for (auto ptr = obj2.begin() ; ptr < obj2.end() ; ptr++) {
		utility::string_t key = ptr->first;
		json::value value = ptr->second;
		std::cout << "key=" << key << std::endl;
		json::value config = value.at(U("config"));
		json::value status = value.at(U("status"));
		json::value info = value.at(U("info"));
		json::array connectors = value.at(U("connectors")).as_array();
		for (auto conn = connectors.begin() ; conn < connectors.end() ; conn++) {
			std::cout << "conn=" << *conn << std::endl;
		}

		std::optional<json::value> rssi;
		try {
			rssi = info.at(U("rssi"));
		} 
		catch ( json::json_exception& err ) {
			// pass
		}

		json::value type_ = info.at(U("type"));
		json::value plugged = status.at(U("plugged"));

		json::value no_value(U("n/a"));

		std::cout << rssi.value_or(no_value) << "\n";
//		std::cout << "type=" << type_ << " plugged=" << plugged << "\n";
		std::cout << "type=" << type_ << " plugged=" << plugged << " rssi=" << rssi.value_or(no_value) << "\n";
	}

//	json::array data_array = data.as_array();

//	json::object::object data2 = obj.at(U("data"));

//	for (auto ptr = data_array.begin() ; ptr < data_array.end() ; ptr++) {
////		std::cout << "ptr=" << *ptr << std::endl;
//		bool connected = ptr->at(U("connected")).as_bool();
//		
//	}

	BOOST_ASSERT_MSG(true,"hello, world");

	return 0;
}
