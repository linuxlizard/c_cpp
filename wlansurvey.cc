#include <iostream>
#include <cstdlib>

#include <boost/format.hpp>
#include <cpprest/http_client.h>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features

static std::optional<json::value> get(http_client& client, uri_builder& builder)
{
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
		return {};
	}

	// extract_json() returns a pplx::task<json::value>
	auto json_response = data_response.extract_json();
	// blocking wait for task to finish
	json_response.wait();
	// return the contents of the task (the template thingy)
	json::value value = json_response.get();
//	std::cout << "value=" << value << std::endl;

	return value;
}

static void object_introspect(json::object& jobj)
{
	// dump the key+value pairs in a json::object
	for (auto fields: jobj) {
		utility::string_t key { fields.first };
		json::value value { fields.second };
		std::cout << key << "=" << value << " type="<< value.type() << "\n";
	}
}

static bool wlansurvey(std::string& target)
{
	if (!web::uri::validate(target)) {
		std::cerr << "invalid target\n";
		return false;
	}

	web::uri target_uri = web::uri{target};

	// TODO get username+password elsewhere
	std::string username { U("admin") };
	std::string password { U("12345") };

	credentials cred{username, password};
	http_client_config config;
	config.set_credentials(cred);
	http_client client(target, config);

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

	builder.clear();
	builder.set_path(U("/api/shallow/status/wlan/radio"));
	std::optional<json::value> get_value = get(client, builder);

	if (!get_value) {
		return EXIT_FAILURE;
	}

	success = get_value.value().at(U("success"));
	data = get_value.value().at(U("data"));
	std::cout << "type=" << data.type() << " data=" << data << "\n"; 
	json::array wlan = data.as_array();
	int num_radios = wlan.size();
	std::cout << "num_radios=" << num_radios << "\n";

	for (int i=0 ; i<num_radios ; i++) { 
		builder.clear();
		builder.set_path(U("/api/status/wlan/radio/" + std::to_string(i) ));
		get_value = get(client, builder);
		if (!get_value) {
			continue;
		}
		success = get_value.value().at(U("success"));
		data = get_value.value().at(U("data"));
		json::object status = data.as_object();;

		for (auto fields: status) {
			utility::string_t key = fields.first;
			value = fields.second;
//			std::cout << key << "=" << value << "\n";
		}

		json::value survey = status.at(U("survey"));
		std::cout << "found " << survey.as_array().size() << " networks\n";
		for (auto network: survey.as_array()) {
			json::object element = network.as_object();
//			object_introspect(element);

			utility::string_t bssid = network.at(U("bssid")).as_string();
			utility::string_t ssid = network.at(U("ssid")).as_string();
			int channel = network.at(U("channel")).as_integer();
//			int frequency = network.at(U("frequency")).as_integer();
			int rssi = network.at(U("rssi")).as_integer();
			utility::string_t seen = network.at(U("seen")).as_string();

			std::cout << boost::format("%|30s|    %|10s| %|5s| %|5s|    %|-20s|\n") % ssid % bssid % rssi % channel % seen;
		}
	}

	return true;
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		return EXIT_FAILURE;
	}

	std::string target{argv[1]};
	std::cout << "target=" << target << "\n";

	wlansurvey(target);

	return EXIT_SUCCESS;
}
