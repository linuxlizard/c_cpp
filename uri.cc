#include <iostream>
#include <regex>

bool parse_uri(const std::string& uri)
{
	/* davep 20180818 ; playing with RFC3986 */
	std::smatch what;
//	std::cmatch what;

	// https://tools.ietf.org/html/rfc3986#page-50
	std::regex uri_regex("^(([^:/?#]+):)?(//([^/?#]*))?",
			std::regex::extended);
//	std::regex uri_regex("^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?",
//			std::regex::extended);

	if ( ! std::regex_match(uri, what, uri_regex)) {
		return false;
	}

	std::string match_s = what.str();
	std::cout << "Match! " << match_s << "\n";

	std::cout << "Found " << what.size() << " matches\n";

	auto iter = what.begin();
	for (auto iter=what.begin() ; iter < what.end() ; iter++ ) {
		std::cout << *iter << "\n";
	}


	return true;
}


int main(int argc, char *argv[])
{
	std::string uri;

	for (int i=1 ; i<argc ; i++ ) {
		uri.assign(argv[i]);
		std::cout << "testing \"" << uri << "\"...\n";
		parse_uri(uri);
	}

	return EXIT_SUCCESS;
}

