/* davep 20181224 ; learning how to use boost::program_options */
#include <iostream>
#include <boost/program_options.hpp>
#include <cstdlib>

// https://stackoverflow.com/questions/tagged/boost-program-options

// all the examples/tutorials use 'po = boost::program_options' but 'po' makes
// me think of gettext language files
namespace opt = boost::program_options;

bool parse_args(int argc, char *argv[])
{
	int debug;

	// https://www.boost.org/doc/libs/1_67_0/doc/html/program_options/tutorial.html#id-1.3.31.4.3
	// https://theboostcpplibraries.com/boost.program_options
	opt::options_description desc{"Allowed options"};
	desc.add_options()
		("help,h", "produce help message")
		// https://stackoverflow.com/questions/31921200/how-to-have-an-optional-option-value-in-boost-program-options
		("debug,d", opt::value<int>(&debug)->default_value(0)->implicit_value(1), "set debug level")
		("target,t", opt::value<std::string>()->required(), "target")
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

	std::string target;
	if (argmap.count("target")) {
		target = argmap["target"].as<std::string>();
		std::cout << "target=" << target << "\n";
	}

	std::cout << "debug=" << debug << "\n";

	return true;
}

int main(int argc, char *argv[])
{
	bool flag = parse_args(argc, argv);
	return EXIT_SUCCESS;
}

