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
 * davep 20180705 ; Tired of Python's problems. Let's try my getting CP router WAN status with C++
 *
 */
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <thread>
#include <chrono>
#include <boost/program_options.hpp>
#include <cstdlib>
#include <regex>
#include <boost/format.hpp>
#include <boost/algorithm/string/join.hpp>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

// all the examples/tutorials use 'po = boost::program_options' but 'po' makes
// me think of gettext language files
namespace opt = boost::program_options;

struct args {
	std::string target;
	int debug;
};

bool parse_uri(const std::string& uri)
{
	/* davep 20180818 ; playing with RFC3986 */
	std::smatch what;
//	std::cmatch what;

	// https://tools.ietf.org/html/rfc3986#page-50
	std::regex uri_regex("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?",
			std::regex::extended);
//	std::regex uri_regex("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?",
//			std::regex::extended);

	if ( !std::regex_match(uri, what, uri_regex)) {
		std::cerr << "no match\n";
		return false;
	}

	std::string match_s = what.str();
	std::cout << "Match! " << match_s << "\n";

	std::cout << "Found " << what.size() << " matches\n";

	for (auto iter=what.begin() ; iter != what.end() ; iter++ ) {
		std::cout << *iter << "\n";
	}
	return true;
}

bool parse_args(int argc, char *argv[], struct args& args)
{
	// https://stackoverflow.com/questions/tagged/boost-program-options

	// https://www.boost.org/doc/libs/1_67_0/doc/html/program_options/tutorial.html#id-1.3.31.4.3
	// https://theboostcpplibraries.com/boost.program_options
	opt::options_description desc{"Allowed options"};
	desc.add_options()
		("help,h", "produce help message")
		// https://stackoverflow.com/questions/31921200/how-to-have-an-optional-option-value-in-boost-program-options
		("debug,d", opt::value<int>(&args.debug)->default_value(0)->implicit_value(1), "set debug level")
		("target,t", opt::value<std::string>()->required(), "target url e.g. http://192.168.0.1")
	;
	opt::variables_map argmap;
	try {
		opt::store(opt::parse_command_line(argc, argv, desc), argmap);
	}
	catch (const opt::error &err) {
		std::cerr << err.what() << "\n";
		return false;
	}

	// https://stackoverflow.com/questions/5395503/required-and-optional-arguments-using-boost-library-program-options
	if (argmap.count("help")) {
		std::cout << desc << std::endl;
		return false;
	}

	try {
		opt::notify(argmap);    
	}
	catch (const opt::error &err) {
		std::cerr << err.what() << "\n";
		return false;
	}

	args.target = argmap["target"].as<std::string>();

	/* final step is verifying the validity of the URL */
	return parse_uri(args.target);
}

int main(int argc, char* argv[])
{
	struct args args;

	if (!parse_args(argc, argv, args) ) {
		return EXIT_FAILURE;
	}

	std::cout << "target=" << args.target << "\n";
	std::cout << "debug=" << args.debug << "\n";

	// some defaults
	std::string scheme = U("https");
	std::string username = U("admin");
	std::string password = U("12345");
	int http_port = 443;
	std::string host;

	if (!web::uri::validate(args.target)) {
		std::cerr << "invalid target\n";
		return EXIT_FAILURE;
	}

	web::uri target_uri = web::uri(args.target);

	//	std::map<utility::string_t, utility::string_t> target_fields = target_uri.split_query(args.target);
	auto target_fields = target_uri.split_query(args.target);
	for (auto iter=target_fields.begin() ; iter != target_fields.end() ; iter++ ) {
		std::cout << "target_fields " << (*iter).first << "\n";
	}

	// std::vector<utility::string_t>
	auto target_vec = target_uri.split_path(args.target);
	for (auto iter=target_vec.begin() ; iter != target_vec.end() ; iter++ ) {
		std::cout << "target_vec " << *iter << "\n";
	}

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
		// blocking wait for the http
		response.wait();
		data_response = response.get(); 
	}
	catch (web::http::http_exception & err) {
		std::cerr << "Error! " << err.what() << std::endl;
		std::cerr << "Error! " << err.error_code() << std::endl;
		return EXIT_FAILURE;
	}

//	web::http::http_headers headers = data_response.headers();
//	std::cout << data_response.to_string() << std::endl;

	// extract_json() returns a pplx::task<json::value>
	auto json_response = data_response.extract_json();
	// blocking wait for task to finish
	json_response.wait();
	// return the contents of the task (the template thingy)
	json::value value = json_response.get();
	std::cout << "value=" << value << std::endl;

	// pull out the contents the router sends back
	json::value success = value.at(U("success"));
	json::value data = value.at(U("data"));

	// so are we connected or what?
	std::string connection_state = data.as_string();
	std::cout << "router is " << connection_state << "\n";

	// start a new get
	builder.clear();
	builder.set_path(U("/api/status/wan/devices"));
//	std::cout << builder.to_string() << std::endl;

	try {
		auto response = client.request(methods::GET, builder.to_string());
		response.wait();
		data_response = response.get(); 
	}
	catch (web::http::http_exception & err) {
		std::cerr << "Error! " << err.what() << std::endl;
		std::cerr << "Error! " << err.error_code() << std::endl;
		return EXIT_FAILURE;
	}
//	// try polling out of curiousity
//	while( ! response.is_done()) {
//		std::cout << response.is_done() << std::endl;
//		// https://stackoverflow.com/questions/4184468/sleep-for-milliseconds
//		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//	}

//	headers = data_response.headers();
//	std::cout << data_response.to_string() << std::endl;

	// another task to extract the json
	json_response = data_response.extract_json();
	json_response.wait();
	value = json_response.get();
	success = value.at(U("success"));
	std::cout << "success=" << success << std::endl;
	data = value.at(U("data"));
//	std::cout << "data=" << data << " is array=" << data.is_array() << std::endl;

	// for use with std::optional<json::value>.value_or()
	json::value no_value(U("n/a"));

	// parse the response
	json::object status_obj = data.as_object();
	for (auto ptr = status_obj.begin() ; ptr != status_obj.end() ; ptr++) {
		utility::string_t key = ptr->first;
		value = ptr->second;
		std::cout << "key=" << key << std::endl;
		json::value config = value.at(U("config"));
		json::value status = value.at(U("status"));
		json::value info = value.at(U("info"));
		json::array connectors = value.at(U("connectors")).as_array();
		for (auto conn = connectors.begin() ; conn != connectors.end() ; conn++) {
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

//		std::cout << rssi.value_or(no_value) << "\n";
//		std::cout << "type=" << type_ << " plugged=" << plugged << "\n";
		std::cout << "type=" << type_ << " plugged=" << plugged << " rssi=" << rssi.value_or(no_value) << "\n";
	}

	std::cout << "router is " << connection_state << "\n";
	boost::format formatter("%|30s| %|10s|    %|-10s| %|-10s|    %|-24s|\n");
	std::cout << "                          NAME       TYPE    PLUGGED    REASON        SUMMARY\n";
	for (auto &ptr : status_obj ) {
		utility::string_t key = ptr.first;
		value = ptr.second;

		json::value config = value.at(U("config"));
		json::value status = value.at(U("status"));
		json::value info = value.at(U("info"));

		// 'at' will throw if key doesn't exist
		std::string type_ = info.at(U("type")).as_string();
		bool  plugged = status.at(U("plugged")).as_bool();
		std::string reason = status.at(U("reason")).as_string();
		std::string summary = status.at(U("summary")).as_string();
		std::cout << formatter % key % type_ % (plugged?"true":"false") % reason % summary;
//		std::cout << boost::format("%|30s| %|10s|    %|-10s| %|-10s|    %|-24s|\n") % key % type_ % (plugged?"true":"false") % reason % summary;
	}

	for (auto &ptr : status_obj ) {
		utility::string_t key = ptr.first;
		value = ptr.second;

//		json::value config = value.at(U("config"));
//		json::value status = value.at(U("status"));
//		json::value info = value.at(U("info"));

		json::array connectors = value.at(U("connectors")).as_array();
		std::cout << "\nconnectors for " << key << "\n";
		std::cout << "                          NAME  STATE         EXCEPTION TIMEOUT\n";
		for (auto& conn : connectors ) {
			// name state exception timeout 
			json::object conn_obj = conn.as_object();

			for (auto& fields : conn_obj) {
				utility::string_t key = fields.first;
				value = fields.second;
//				std::cout << key << "=" << value << "\n";
			}

			// can I do something like python's string.join with an iterators cross the connectors' keys?
			// starting simple, make an array, then directly join
			std::vector<std::string> connector_keys;
			for (auto& fields : conn_obj) {
				utility::string_t key = fields.first;
				value = fields.second;
				connector_keys.push_back(key);
			}
			std::string all_keys = boost::algorithm::join(connector_keys, ",");
//			std::cout << "all_keys=" << all_keys << "\n";

//			std::string all_keys2 = boost::algorithm::join(conn_obj.begin(), ",");
//			std::cout << "all_keys=" << all_keys2 << "\n";

			std::string name = conn_obj.at(U("name")).as_string();
			json::value state = conn_obj.at(U("state"));
			json::value exception = conn_obj.at(U("exception"));
			// timeout may or may not exist
			std::optional<json::value> timeout;
			try {
				timeout = conn_obj.at(U("timeout"));
			}
			catch ( json::json_exception& err ) {
				// pass
			}


//			std::cout << "name=" << name << "\n";
//			std::cout << "state=" << state << "\n";
//			std::cout << "exception=" << exception << "\n";
//			std::cout << timeout << "\n";
			std::cout << boost::format("%|30s| %|10s|    %|-10s| %|-10s|\n") % name % state % exception % timeout.value_or(no_value);
		}
	}

	// tinkering with boost assert
	BOOST_ASSERT_MSG(true,"hello, world");

	return 0;
}
