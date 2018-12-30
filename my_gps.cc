/* davep 20181230 ; tinkering with large config settings via cpprestsdk by
 * poking at cradlepoint router gps 
 *
 * work in progress
 */

#include <iostream>
#include <cstdlib>

#include <boost/format.hpp>
#include <cpprest/http_client.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features

// TODO get username+password from cli, env var, etc
std::string username { U("admin") };
std::string password { U("00000000") };
//std::string password { U("12345") };

#if 0
// connection field JSON
[
    {
        "_id_": "00000000-cb35-39e3-bc26-fffd7b4f4c4a",
        "client": {
            "destination": "server",
            "num_sentences": 1000,
            "port": 4242,
            "server": "iguana.example.com",
            "time_interval": {
                "enabled": false,
                "end_time": "5:00 PM",
                "start_time": "9:00 AM"
            },
            "useudp": false
        },
        "distance_interval_meters": 0,
        "enabled": true,
        "interval": 10,
        "language": "taip",
        "name": "dave1",
        "nmea": {
            "include_id": true,
            "prepend_id": false,
            "provide_gga": true,
            "provide_rmc": true,
            "provide_vtg": true
        },
        "stationary_distance_threshold_meters": 20,
        "stationary_movement_event_threshold_seconds": 0,
        "stationary_time_interval_seconds": 0,
        "taip": {
            "include_cr_lf_enabled": false,
            "provide_al": true,
            "provide_cp": true,
            "provide_id": false,
            "provide_ln": true,
            "provide_pv": true,
            "report_msg_checksum_enabled": true,
            "vehicle_id_reporting_enabled": true
        }
    }
]

#endif

void object_introspect(json::object& jobj)
{
	// dump the key+value pairs in a json::object
	for (auto fields: jobj) {
		utility::string_t key { fields.first };
		json::value value { fields.second };
		std::cout << key << "=" << value << " type="<< value.type() << "\n";
	}
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

	boost::uuids::uuid _id_ { boost::uuids::random_generator()() };
	std::cout << boost::uuids::to_string(_id_) << "\n";

	json::value gps = json::value::object();
	BOOST_ASSERT_MSG(gps.is_object(), "failed" );
	gps["enabled"] = json::value(false);
	gps["enable_gps_keepalive"] = json::value(false);
	gps["taip_vehicle_id"] = json::value("DAVE");

	object_introspect(gps.as_object());
	std::cout << "gps=" << gps.serialize() << "\n";

	utility::ostringstream_t buf;
	buf << "data=" << gps.serialize() ;

	credentials cred{username, password};
	http_client_config config;
	config.set_credentials(cred);
	http_client client(target, config);

	std::string path { "/api/config/system/gps" };
	http::uri uri = http::uri_builder(target).append_path(path).to_uri();

	// have to manually make a request so can change Content-Type
	http_request request(methods::PUT);
	request.set_request_uri(uri);
	request.set_body(buf.str());
	request.headers()["Content-Type"] = "application/x-www-form-urlencoded";
	std::cout << "request=" << request.to_string() << "\n";
	http_response response = client.request(request).get();
	json::value response_json = response.extract_json().get();
	std::cout << "response value=" << response_json << std::endl;

	return EXIT_SUCCESS;
}
