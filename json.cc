#include <iostream>
#include <fstream>
#include <memory>
#include <json/json.h>

int main(void)
{
	Json::Value value;
//	Json::String s;
//	JSONCPP_STRING s;

	std::fstream infile;
	infile.open("radio.json", std::fstream::in);

	Json::CharReaderBuilder builder;
	builder["collectComments"] = false;
	JSONCPP_STRING errs;
	bool ok = parseFromStream(builder, infile, &value, &errs);
//	bool ok = parseFromStream(builder, std::cin, &value, &errs);

	if (!ok) {
		std::terminate();
	}
	std::cout << "ok!\n";

	Json::StreamWriterBuilder writer_builder;
	writer_builder["indentation"] = " ";
	std::unique_ptr<Json::StreamWriter> writer(writer_builder.newStreamWriter());
	writer->write(value, &std::cout);
	std::cout << "\n";

	Json::Value radio_status;
	radio_status["status"] = value;
	writer->write(radio_status, &std::cout);
	std::cout << "\n";

	std::cout << "num_radios=" << radio_status["status"].size() << "\n";
	Json::Value radio0 { radio_status["status"][0] };

	for (auto radio : radio_status["status"]) {
		std::cout << "num_channels=" << radio["channel_list"].size() << "\n";
		for (auto ch : radio["channel_list"]) {
			std::cout << "channel=" << ch << "\n";
		}
	}

	infile.close();
	infile.open("wlan.json", std::fstream::in);
	if (!infile.is_open()){
		std::terminate();
	}

	Json::Value wlan_config;
	ok = parseFromStream(builder, infile, &wlan_config, &errs);
	if (!ok) {
		std::terminate();
	}
	writer->write(wlan_config, &std::cout);
	std::cout << "\n";

	std::cout << "num_radios=" << wlan_config["data"]["radio"].size() << "\n";
	for (auto radio : wlan_config["data"]["radio"]) {
		for (auto bss : radio["bss"]) {
			std::cout << "ssid=" << bss["ssid"] << "\n";
		}
	}

	std::string s { wlan_config.toStyledString() };
	std::cout << "s=" << s << "\n";

	Json::Value bss;
	bss["ssid"] = "dave1";
	for (int i=0 ; i<10 ; i++) {
		bss["channel"][i] = i;
	}
	s  = bss.toStyledString();
	std::cout << "s=" << s << "\n";
	bss["channel"].clear();

	for (int i=0 ; i<10 ; i++) {
		bss["channel"].append(i);
	}
	s  = bss.toStyledString();
	std::cout << "s=" << s << "\n";

//	bss["channels"] = [0,1,2,3,4,5,6,7,8];

	Json::Value arr_value {Json::arrayValue};
	std::cout << "array is" << (arr_value.empty()?" ":" not ") << "empty \n";
	arr_value[0] = 42;
	std::cout << "array is" << (arr_value.empty()?" ":" not ") << "empty \n";

	return 0;
}

