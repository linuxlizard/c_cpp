// davep 20190408 walk a filesystem, find duplicate filenames
// Learning exercise for std::filesystem std::regex std::map
//
// gcc -g -Wall -o find-dup-names find-dup-names.cc -std=c++17 -lstdc++ -lstdc++fs
//
#include <iostream>
#include <filesystem>
#include <map>
#include <regex>

namespace fs = std::filesystem;

namespace {
	// https://en.cppreference.com/w/cpp/regex/basic_regex
	std::regex src_file { "[a-zA-Z_]+\\.c" };
}

int main()
{
	std::map<std::string, std::string> pathmap;
	std::map<std::string, std::string>::iterator iter;
	bool flag;

	for (auto& f : fs::recursive_directory_iterator(".")) {
		if (! f.is_regular_file()) {
			continue;
		}
		fs::path path = f.path();

		std::cout << path << "\n";
		// iterate over the components in the path
		for (auto& p : path) {
			std::cout << "p=" << p << "\n";
		}

		std::string filename = path.filename();
		std::cout << filename << "\n";

		if (!std::regex_match(filename, src_file)) {
			continue;
		}
		std::cout << "regex match\n";

		std::tie(iter, flag) = pathmap.emplace(filename, path);
		if (!flag) {
			std::cerr << "duplicate filename found " << filename << " at " << iter->second << "\n";
			throw filename;
		}
	}

	return 0;
}

