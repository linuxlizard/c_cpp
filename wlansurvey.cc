/* davep 20181230 ; poke at cradlepoint router wifi survey : work in progress */
#include <iostream>
#include <cstdlib>
#include <optional>

// http://fmtlib.net/latest/api.html#format
#include <fmt/format.h>

#include <cpprest/http_client.h>

#include "netrc.hpp"

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features

// TODO get username+password from cli, env var, etc
std::string username { U("admin") };
//std::string password { U("00000000") };
std::string password { U("12345") };

static std::optional<json::value> get(http_client& client, http::uri& uri)
{
	web::http::http_response data_response; 
	try {
		// blocking wait for the http
		data_response = client.request(methods::GET, uri.to_string()).get(); 
	}
	catch (web::http::http_exception & err) {
		std::cerr << "Error! " << err.what() << std::endl;
		std::cerr << "Error! " << err.error_code() << std::endl;
		return {};
	}

	// extract_json() returns a pplx::task<json::value>
	// blocking wait for task to finish
	// return the contents of the task (the template thingy)
	json::value value = data_response.extract_json().get();
//	std::cout << "value=" << value << std::endl;

	return value;
}

void object_introspect(json::object& jobj)
{
	// dump the key+value pairs in a json::object
	for (auto fields: jobj) {
		utility::string_t key { fields.first };
		json::value value { fields.second };
		std::cout << key << "=" << value << " type="<< value.type() << "\n";
	}
}

int channel_to_frequency(int f)
{
//	# https://en.wikipedia.org/wiki/List_of_WLAN_channels
	if (f >= 36)
		return f*5 + 5000;
	else if (f < 14)
		return ((f-1)*5) +2412;
	else if (f == 14)
		return 2484;
	else
		return 0;
}

void check_netrc(std::string host, std::string& username_, std::string& password_)
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
			username_ = std::move(netrc_login);
			password_ = std::move(netrc_password);
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

bool wlansurvey(std::string& target)
{
	web::uri target_uri = web::uri{target};

	// read netrc before trying higher priority methods
	check_netrc(target_uri.host(), username, password);

	const char * p = std::getenv("CP_PASSWORD");
	if (p) {
		password.assign(p);
	}

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
	http::uri uri = builder.to_uri();
	std::optional<json::value> get_value = get(client, uri);

	if (!get_value) {
		return EXIT_FAILURE;
	}

	success = get_value.value().at(U("success"));
	data = get_value.value().at(U("data"));
	std::cout << "type=" << data.type() << " data=" << data << "\n"; 
	/* davep 20190117 ; single radio products return a dict. Multiple radio
	 * products return an array of dicts. 
	 */
	int num_radios;
	if (data.type() == 3) {
		num_radios = 1;
	}
	else {
		json::array wlan = data.as_array();
		num_radios = wlan.size();
	}
	std::cout << "num_radios=" << num_radios << "\n";

	for (int i=0 ; i<num_radios ; i++) { 
		builder.clear();
		builder.set_path(U("/api/status/wlan/radio/" + std::to_string(i) ));
		uri = builder.to_uri();
		get_value = get(client, uri);
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
			int frequency = channel_to_frequency(channel);
			int rssi = network.at(U("rssi")).as_integer();
			utility::string_t seen = network.at(U("seen")).as_string();

			fmt::print("{0:36s} {1:10s} {2:3d} {3:4d} {4:5d}   {5:20s}\n", 
					ssid, bssid, rssi, channel, frequency, seen);
		}
	}

	return true;
}

void trigger_scan(std::string& target)
{
	std::cout << target << "\n";

	// let's learn how to PUT 
	std::string path {"/api/control/wlan/radio/0"};

	http::uri uri = http::uri_builder(target).append_path(path).to_uri();
//	http::uri uri = http::uri(target);

	credentials cred{username, password};
	http_client_config config;
	config.set_credentials(cred);
	http_client client(target, config);

	// maketh the json (stupid to go through all this for something as simple
	// as {"survey":True} but I'm learning how to use the library 
//	json::value value {true};
//	json::object obj;
//	obj["survey"] = value;

	utility::string_t k {"survey"};
	json::value v {true};

	// this seems awfully complicated
	std::vector fields {std::make_pair(k,v)};
	json::value value =  json::value::object(fields);
	object_introspect(value.as_object());
	utility::string_t s = value.serialize();
	std::cout << s << "\n";

	// this might be simpler
	value = json::value::parse("{\"survey\":true}");
	object_introspect(value.as_object());
	std::cout << value.serialize() << "\n";

	// our firmware wants payload URL encoded 
	// my python also sends Content-Type: application/x-www-form-urlencoded
	//
	// (using BlackJackClient.cpp as example on building this string)
	utility::ostringstream_t buf;
	buf << "data=" << value.serialize() ;
//	buf << "data={\"survey\":true}" ;
//	buf << "data=%7B%22survey%22%3A+true%7D";

	http_request request(methods::PUT);
	request.set_request_uri(uri);
	request.set_body(buf.str());
	request.headers()["Content-Type"] = "application/x-www-form-urlencoded";
	std::cout << "request=" << request.to_string() << "\n";
	http_response response = client.request(request).get();
//	http_response response = client.request(methods::PUT, path, buf.str()).get();
	json::value response_json = response.extract_json().get();
	std::cout << "value=" << response_json << std::endl;
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		return EXIT_FAILURE;
	}

	std::string target{argv[1]};
	std::cout << "target=" << target << "\n";

	if (!web::uri::validate(target)) {
		std::cerr << "invalid target\n";
		return EXIT_FAILURE;
	}

//	trigger_scan(target);
	wlansurvey(target);

	return EXIT_SUCCESS;
}
