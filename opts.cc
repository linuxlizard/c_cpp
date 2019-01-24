/* davep 20181224 ; learning how to use boost::program_options */
#include <iostream>
#include <typeinfo>
#include <numeric>
#include <cstdlib>
#include <boost/program_options.hpp>
#include <boost/optional.hpp>


// https://stackoverflow.com/questions/tagged/boost-program-options

// all the examples/tutorials use 'po = boost::program_options' but 'po' makes
// me think of gettext language files
namespace opt = boost::program_options;

std::pair<std::string, std::string> verbose_parser(const std::string& s)
{
	std::cout << verbose_parser << " s=" << s << "\n";
//	if (s.find("-v") == 0) {
//		return std::make_pair( std::string("-v"), std::string("1"));
//	}
//
	return std::make_pair(std::string(), std::string());
}

bool parse_args(int argc, char *argv[])
{
	int verbose;

	// https://www.boost.org/doc/libs/1_67_0/doc/html/program_options/tutorial.html#id-1.3.31.4.3
	// https://theboostcpplibraries.com/boost.program_options
//	opt::options_description desc{"Allowed options"};
	opt::options_description desc{"Allowed options", 90}; // 90 default line length
	desc.add_options()
		("help,h", "produce help message")
		// https://stackoverflow.com/questions/31921200/how-to-have-an-optional-option-value-in-boost-program-options
		(",n", "use $HOME/.netrc")
		("sort,s", opt::value<std::string>()->value_name("sortby"), "sort by: ssid, bssid, rssi, channel, mode, security")
		("target,t", opt::value<std::string>()->required()->value_name("url"), "target")
		// want multiple -v to increase verbosity
//		("verbose,v", "set verbosity level")
		("verbose,v", opt::value<int>(&verbose)->multitoken()->default_value(0)->implicit_value(1)->composing()->value_name("level"), "set verbosity level")
//		("verbose,v", opt::value<int>(&verbose)->default_value(0)->implicit_value(1)->value_name("level"), "set verbose level")
	;

//	sort_choices = ("ssid", "bssid", "rssi", "channel", "mode", "security")
//	std::cout << desc << "\n";

	// no options, display help then quit
	if (argc == 1) {
		std::cout << desc << std::endl;
		return false;
	}

	opt::variables_map varmap;
	try {
		// learning from custom_syntax.hpp
		auto parsed = opt::command_line_parser(argc, argv).options(desc).extra_parser(verbose_parser).run();
		// run() returns basic_parsed_options with field .options which is a vector<option>

//		for (auto a=std::cbegin(parsed.options) ; a != std::cend(parsed.options) ; ++a) {
//			std::cout << "key=" << a->string_key << "\n";
//			std::cout << "typeid=" << typeid(*a).name() << "\n";
//		}
		// testing for const auto& ; make sure I understand how to get a
		// reference and I'm not making a copy of 'option'
		for (int i=0 ; i<parsed.options.size() ; i++ ) {
			std::cout << "ptr=" << &parsed.options[i] << "\n";
		}
		for (const auto& a : parsed.options) {
			std::cout << "key=" << a.string_key << "\n";
			std::cout << "typeid=" << typeid(a).name() << "\n";
			std::cout << "ptr=" << &a << "\n";
		}

		int verbose_value_counter {0};

		// I want a specific behavior for the -v/--verbose flag. The more 'v',
		// the higher the verbosity level. This doesn't seem to be a feature
		// built into program_options so I'll fake it.
		//
		// Count the number of times -v/--verbose appears in the parsed command
		// line.
		int verbose_count = std::count_if( 
					std::cbegin(parsed.options), 
					std::cend(parsed.options), 
					[&verbose_value_counter](const auto& option)->bool { 
						bool flag = option.string_key == "verbose"; 
						std::cout << "ptr=" << &option << "\n";

						if (flag && !option.value.empty()) {
							std::cout << "found " << option.value.size() << " values\n";
							std::cout << "value=" << option.value.at(0) << "\n";

							// find options with a value e.g., -v 99 and
							// accumulate those values as well.  So 
							// "--verbose 99 -v" would be a final value of 100.
							// Because I'm weird.
							//
							verbose_value_counter += std::accumulate(
									std::cbegin(option.value), 
									std::cend(option.value),
									0,
									[](const int& n, const std::string& s) { return std::stoi(s) + n; });
						}
						return flag; 
					}
				);

		std::cout << "verbose_count=" << verbose_count << " verbose_value_counter=" << verbose_value_counter << "\n";

		// store() will complain about how I'm using -v/--verbose so remove
		// them from the parsed results.
		parsed.options.erase(
			std::remove_if(
				parsed.options.begin(), 
				parsed.options.end(), 
				[](auto& option)->bool { return option.string_key == "verbose"; }), 
			parsed.options.end());
		std::cout << "verbose_count=" << verbose_count << "\n";

		opt::store(parsed, varmap);

		// TODO can I add my verbosity to the varmap?

//		opt::store(opt::command_line_parser(argc, argv).options(desc).extra_parser(verbose_parser)
//				.run(), varmap);

//		opt::store(opt::parse_command_line(argc, argv, desc), varmap);
	}
	catch (const opt::error &err) {
		std::cerr << err.what() << "\n";
		return false;
	}

	// https://stackoverflow.com/questions/5395503/required-and-optional-arguments-using-boost-library-program-options
	if (varmap.count("help")) {
		std::cout << desc << std::endl;
		return false;
	}

	std::cout << "verbose=" << varmap.count("verbose") << "\n";

	try {
		opt::notify(varmap);    
	}
	catch (const opt::error &err) {
		std::cerr << err.what() << "\n";
		return false;
	}

	std::string target;
	if (varmap.count("target")) {
		target = varmap["target"].as<std::string>();
		std::cout << "target=" << target << "\n";
	}

//	std::cout << "verbose=" << verbose << "\n";
//	std::cout << "n=" << varmap["n"] << "\n";
	auto use_netrc = varmap["n"];
	std::cout << "empty=" << use_netrc.empty() << " defaulted=" << use_netrc.defaulted() << "\n";
	auto value = use_netrc.value();

//	auto use_netrc2 = desc.find("n", false);
//	std::cout << "key=" << use_netrc2.key("n") << "\n";

	return true;
}

int main(int argc, char *argv[])
{
	bool flag = parse_args(argc, argv);
	return EXIT_SUCCESS;
}

