#include <iostream>
#include <fstream>
#include <string>
#include <map>
//#include <exception>
#include <cstdlib>
#include <boost/algorithm/string.hpp>
//#include <boost/range/adaptor/indexed.hpp>

#define BOOST_TEST_MODULE TestNetrc
#include <boost/test/unit_test.hpp>

#include "netrc.hpp"

#ifdef __has_include
#  if __has_include(<filesystem>)
#    include <filesystem>  // gcc8 (Fedora29+)
     namespace fs = std::filesystem;
#  elif __has_include(<experimental/filesystem>)
#    include <experimental/filesystem> // gcc7 (Ubuntu 18.04)
     namespace fs = std::experimental::filesystem;
#  endif
#else
#error no __has_include
#endif

void verify(netrc_map& netrc)
{
	std::array<std::string,4> tests { "172.16.22.1", "172.16.17.1", "172.19.10.119", "default" };
	for (auto s : tests) {
		try {
			auto auth = netrc.at(s);
			std::cout << s << " username=" << std::get<0>(auth) << " password=" << std::get<1>(auth) << "\n";
		} catch (std::out_of_range& err ) {
			std::cerr << "machine " << s << " not found\n";
		};
	}
}

void test_simple(void)
{
	std::string testbuf {
		// C++ raw string literal F T W
R"#(machine 172.16.17.1
login admin
password 12345

# can have entire thing on one line
# oh, and comments, too
machine 192.168.0.1 login admin password 12345

machine 172.16.22.1
login admin
password 12345

machine 172.19.10.119
login admin
password 00000000

default login admin password hythloday@example.com

# TODO put fail tests cases into own string
# weird whitespace
#	machine 		foo.bar.baz
#	login	dave
#	password 		this is invalid because embedded spaces
)#"
	};

	std::stringstream infile{testbuf};
	auto netrc = netrc_parse(infile);
	verify(netrc);
}

int main(int argc, char *argv[] )
{
	test_simple();
//	return 0;

	std::string home = std::getenv("HOME");
	fs::path path(home);
	path.append(".netrc");
	if ( !fs::exists(path) ) {
		// try the windows version
		path = home;
		path.append("_netrc");
		if ( !fs::exists(path) ) {
			std::cerr << "no .netrc found\n"; 
			return EXIT_FAILURE;
		}
	}

	auto netrc = netrc_parse_file(path.native());
	verify(netrc);

	// parse netrc files from cli 
	for (int i=1 ; i<argc ; i++) {
		std::string f = argv[i];
	}

	return EXIT_SUCCESS;
}

