/* davep 20181224 ; learning how to use boost::program_options */
/* davep 20190131 ; TODO need to clean up this file; lots of extraneous stuff I
 * added while learning program_options 
 */

#include <iostream>
#include <typeinfo>
#include <numeric>
#include <cstdlib>
#include <optional>
#include <boost/program_options.hpp>
//#include <boost/optional.hpp>

#include "opts.hpp"

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

	// taking apart how program_options/src/cmdline.cpp  parse_short_option() works
	std::string name = s.substr(0,2);
	std::string adjacent = s.substr(2);
	std::cout << "name=" << name << " adjacent=" << adjacent << "\n";

	return std::make_pair(std::string(), std::string());
}

std::optional<struct Args> parse_args(int argc, const char * const argv[])
{
	int verbose;

	// https://www.boost.org/doc/libs/1_67_0/doc/html/program_options/tutorial.html#id-1.3.31.4.3
	// https://theboostcpplibraries.com/boost.program_options
//	opt::options_description desc{"Allowed options"};
	opt::options_description desc{"Allowed options", 90}; // 90 default line length
	desc.add_options()
		("help,h", "produce help message")
		// want only -n with no arg and no long option
		(",n", "use $HOME/.netrc")
		("sort,s", opt::value<std::string>()->value_name("sort-by"), "sort by: ssid, bssid, rssi, channel, mode, security")
		("target,t", opt::value<std::string>()->required()->value_name("url"), "target")
		// want multiple -v to increase verbosity
		// https://stackoverflow.com/questions/31921200/how-to-have-an-optional-option-value-in-boost-program-options
//		("verbose,v", "set verbosity level")
		("verbose,v", opt::value<int>(&verbose)->multitoken()->default_value(0)->implicit_value(1)->composing()->value_name("level"), "set verbosity (debug) level")
//		("verbose,v", opt::value<int>(&verbose)->default_value(0)->implicit_value(1)->value_name("level"), "set verbose level")
	;

//	sort_choices = ("ssid", "bssid", "rssi", "channel", "mode", "security")
//	std::cout << desc << "\n";

	// no options, display help then quit
	if (argc == 1) {
		std::cout << desc << std::endl;
		return {};
	}

	opt::variables_map varmap;
	try {
		// learning from custom_syntax.hpp
		auto parsed = opt::command_line_parser(argc, argv).options(desc).extra_parser(verbose_parser).run();
		// run() returns basic_parsed_options with field .options which is a vector<option>

		// testing for const auto& ; make sure I understand how to get a
		// reference and I'm not making a copy of 'option'
//		for (int i=0 ; i<parsed.options.size() ; i++ ) {
//			std::cout << "ptr=" << &parsed.options[i] << "\n";
//		}
		// debug dump everything found
		for (const auto& o : parsed.options) {
			// 'o' should be a basic_option
			std::cout << "key=" << o.string_key << "\n";
			std::cout << "values len=" << o.value.size() << "\n";
			std::cout << "typeid=" << typeid(o).name() << "\n";
			std::cout << "unregistered=" << o.unregistered << "\n";
			std::cout << "original_tokens=" << o.original_tokens.size() << "\n";
//			std::cout << "ptr=" << &a << "\n\n";
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
//						std::cout << "ptr=" << &option << "\n";

						// find options with a value e.g., -v 99 and accumulate
						// those values as well.  So "--verbose 99 -v" would be
						// a final value of 100.  Because I'm weird.
						if (flag && !option.value.empty()) {
							std::cout << "found " << option.value.size() << " values\n";
							std::cout << "value=" << option.value.at(0) << "\n";

							verbose_value_counter += std::accumulate(
									std::cbegin(option.value), 
									std::cend(option.value),
									0,
									[](const int& n, const std::string& s) { return std::stoi(s) + n; });
							// problem: -v -v 9 == 11, not 10 as intuitively
							// expected because the -v 9  shouldn't count the
							// "-v" of "-v 9" as a value. So fool the count_if.
							flag = false;
						}
						return flag; 
					}
				);

		// TODO a better idea: just sum up the -v/--verbose with and without a
		// value. If there is no value, default value is 1.
#if 0
		verbose_count = std::accumulate(
								std::cbegin(option.value), 
								std::cend(option.value),
								0,
								[&option](const int& n, const std::string& s) { 
									if (s == "verbose") {
										if (option.value.empty()) {
											return 1 + n;
										} else {
											return std::stoi(s) + n; 
										}
									}
								}
							);
#endif

		std::cout << "verbose_count=" << verbose_count << " verbose_value_counter=" << verbose_value_counter << "\n";
		// sum of all the --verbose/-v flags + values
		verbose_count += verbose_value_counter;

		// store() will complain about how I'm using -v/--verbose so remove
		// them from the parsed results.
		parsed.options.erase(
			std::remove_if(
				parsed.options.begin(), 
				parsed.options.end(), 
				[](const auto& option)->bool { return option.string_key == "verbose"; }), 
			parsed.options.end());
		std::cout << "verbose_count=" << verbose_count << "\n";

		// TODO can I do the count+erase in one step?

		// can I add my weird-o verbosity to the varmap?
//		std::vector<std::string> value {std::to_string(verbose_count) };
//		opt::option v { "verbose",  value};
//		opt::option v { "verbose", {std::to_string(verbose_count)} };
//		parsed.options.push_back(v);

		// can I add it with emplace_back() ?
		// Tripping over initializer_list confusion.
		// Not sure if this is doing emplace or move (???)
		parsed.options.emplace_back(
				std::string("verbose"), std::vector<std::string>{std::to_string(verbose_count)}
//				std::string("verbose"), std::vector({std::to_string(verbose_count)})
			);

		opt::store(parsed, varmap);

//		opt::store(opt::command_line_parser(argc, argv).options(desc).extra_parser(verbose_parser)
//				.run(), varmap);

//		opt::store(opt::parse_command_line(argc, argv, desc), varmap);
	}
	catch (const opt::error &err) {
		std::cerr << err.what() << "\n";
		return {};
	}

	// https://stackoverflow.com/questions/5395503/required-and-optional-arguments-using-boost-library-program-options
	if (varmap.count("help")) {
		std::cout << desc << std::endl;
		return {};
	}

	try {
		opt::notify(varmap);    
	}
	catch (const opt::error &err) {
		std::cerr << err.what() << "\n";
		return {};
	}

	// peek at the varmap which should be a descendent of std::map
	std::cout << "varmap size=" << varmap.size() << "\n";
	for (const auto& v: varmap) {
		std::cout << "typeid=" << typeid(v).name() << "\n";
		auto [a,b] = v;
		std::cout << "typeid=" << typeid(a).name() << "\n";
		std::cout << "typeid=" << typeid(b).name() << "\n";
		std::cout << "a=" << a << "\n";
//		std::cout << "b=" << b.string_key << "\n";
	}

	// convert the opt::variables_map to my struct Opts
	struct Args opts {};
	std::cout << opts << "\n";

	if (varmap.count("target")) {
		opts.target = varmap["target"].as<std::string>();
	}
	std::cout << opts << "\n";

	if (varmap.count("sort")) {
		opts.sort_by = varmap["sort"].as<std::string>();
	}

	std::cout << "final: verbose=" << varmap.count("verbose") << "\n";
	if (varmap.count("verbose")) {
		std::cout << "varmap verbose=" << varmap["verbose"].as<int>() << "\n";
		opts.verbose = varmap["verbose"].as<int>();
	}

	// for some reason, without the long option, the short option is stored
	// with the dash
	auto use_netrc = varmap["-n"];
	std::cout << "use_netrc count=" << varmap.count("-n") 
		<< " empty=" << use_netrc.empty() 
		<< " defaulted=" << use_netrc.defaulted() << "\n";
	opts.use_netrc = varmap.count("-n") > 0;

	std::cout << opts << "\n";

	return opts;
}

int old_main(int argc, char *argv[])
{
	auto parsed = parse_args(argc, argv);

	if (!parsed) {
		return EXIT_FAILURE;
	}

	auto const opts = parsed.value();
	std::cout << "target=" << opts.target << "\n";

	if (opts.sort_by.empty()) {
		std::cout << "no default sort\n";
	}
	else {
		std::cout << "sort by " << opts.sort_by << "\n";
	}

	return EXIT_SUCCESS;
}

