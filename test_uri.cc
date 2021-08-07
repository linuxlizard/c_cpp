/* davep 20210806 ; split uri into a library and a test file */

#include <iostream>
#include <array>
#include <boost/assert.hpp>

#include "uri.h"

int main(int argc, char *argv[])
{
	std::string uri;

	std::array<std::string,10> tests {
		"http://",
		"ftp://",
		"sftp://",
		"http://hythloday@www.utopia.com:1516",
		"https://www.infradead.org/~tgr/libnl/",
		"https://tools.ietf.org/html/rfc3986#page-50",
		"https://www.google.com/search?client=firefox-b-1&q=std%3A%3Aarray",
		"127.0.0.1",

		// https://tools.ietf.org/html/rfc3986#page-35
		"http://a/b/c/d;p?q",

		// https://tools.ietf.org/html/rfc3986#page-45
		"ftp://cnn.example.com&story=breaking_news@10.0.0.1/top_story.htm",
	};

	std::array<std::string,1> fail_tests {
		// username:password is deprecated so this should fail
		"ftp://anonymous:complaints@whitehouse.gov/pub/taxreturns/",
	};

	/* these should succeed */
	for (auto test_iter = tests.begin(); test_iter < tests.end() ; test_iter++ ) {
		std::cout << "testing \"" << *test_iter << "\"...\n";
		parse_uri(*test_iter);
	}

	/* these should fail */
	/* FIXME username:password should fail but doesn't */
	for (auto test_iter = fail_tests.begin(); test_iter < fail_tests.end() ; test_iter++ ) {
		std::cout << "testing \"" << *test_iter << "\"...\n";
		parse_uri(*test_iter);
	}

	/* also parse any from the command line */
	for (int i=1 ; i<argc ; i++ ) {
		uri.assign(argv[i]);
		std::cout << "testing \"" << uri << "\"...\n";
		bool flag = parse_uri(uri);
		BOOST_ASSERT_MSG(flag, "u failed");
	}

	return EXIT_SUCCESS;
}


