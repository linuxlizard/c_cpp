#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <regex>
#include <functional>

// davep 20180816 ; playing with C++ regex

/*
 * >>> import keyword
 * >>> keyword.kwlist
 * ['False', 'None', 'True', 'and', 'as', 'assert', 'break', 'class', 'continue',
 * 'def', 'del', 'elif', 'else', 'except', 'finally', 'for', 'from', 'global',
 * 'if', 'import', 'in', 'is', 'lambda', 'nonlocal', 'not', 'or', 'pass',
 * 'raise', 'return', 'try', 'while', 'with', 'yield']
 */
std::array<std::string,33> python_keywords {"False", "None", "True", "and",
	"as", "assert", "break", "class", "continue", "def", "del", "elif", "else",
	"except", "finally", "for", "from", "global", "if", "import", "in", "is",
	"lambda", "nonlocal", "not", "or", "pass", "raise", "return", "try",
	"while", "with", "yield"};

//static const boost::regex e("(\\d{4}[- ]){3}\\d{4}");
// https://en.cppreference.com/w/cpp/regex
//static const std::regex word_regex("([a-zA-Z_][a-zA-Z_0-9]*)");

// boost::regex reportedly faster than std::regex
static const boost::regex word_regex("([a-zA-Z_][a-zA-Z_0-9]*)");

void load_file(std::string& infilename, std::map<std::string, int> &wordmap)
{
	std::ifstream infile(infilename);
	std::string line;

	// TODO instead of reading line-by-line 
	// https://stackoverflow.com/questions/116038/what-is-the-best-way-to-read-an-entire-file-into-a-stdstring-in-c

	std::cout << "reading from " << infilename << "\n";
	int counter = 0;
	int char_counter = 0;

	// read entire file into single string; from:
	// https://github.com/fenbf/articles/blob/master/cpp17/searchers/searchers.cpp
	std::stringstream strstream;
	strstream << infile.rdbuf();
	std::string allfile;
	allfile = strstream.str();
	auto start = allfile.begin();
	auto end = allfile.end();
	auto words_begin = boost::sregex_iterator(start, end, word_regex);
	auto words_end = boost::sregex_iterator();

	for (auto i = words_begin; i != words_end ; ++i ) {
		wordmap[i->str()] += 1;
	}
	return;


	while(std::getline(infile, line)) {
		counter += 1;
		char_counter += line.length();
		auto start = line.begin();
		auto end = line.end();

		// https://en.cppreference.com/w/cpp/regex
//		auto words_begin = std::sregex_iterator(start, end, word_regex);
//		auto words_end = std::sregex_iterator();
		auto words_begin = boost::sregex_iterator(start, end, word_regex);
		auto words_end = boost::sregex_iterator();

		for (auto i = words_begin; i != words_end ; ++i ) {
//			std::smatch match = *i;
//			std::cout << match.str() << "\n";

			wordmap[i->str()] += 1;
//			wordmap[match.str()] += 1;
		}

		// https://www.boost.org/doc/libs/1_68_0/libs/regex/doc/html/boost_regex/ref/regex_search.html
//		std::match_results<std::string::const_iterator> what;
//		while (boost::regex::regex_search(start, end, word)) {
//		}

	}
	std::cout << "lines=" << counter << " chars=" << char_counter << "\n";
}

int main(int argc, char *argv[])
{
	std::string arg;

	std::map<std::string, int> wordmap;
	for (int i=1 ; i<argc ; i++ ) {
		arg = argv[i];
//		std::cout << arg << "\n";
#if 0
		// playing with boost::filesystem
		if (!boost::filesystem::exists(arg)) {
			std::cerr << arg << " does not exist\n";
			continue;
		}
		if (!boost::filesystem::is_regular_file(arg)) {
			std::cerr << arg << " is not a valid file\n";
			continue;
		}
#endif
		load_file(arg, wordmap);
	}
	std::cout << "found " << wordmap.size() << " words\n";
	std::cout << "\"self\" occurs " << wordmap["self"] << " times\n";

	// iterate over map showing the counts
	// https://en.cppreference.com/w/cpp/container/map/begin
//	for (auto it = wordmap.cbegin(); it != wordmap.cend(); ++it) {
//		std::cout << it->first << "=" << it->second << '\n';
//	}

	// create an array of string+count
	typedef std::pair<std::string, int> word_count_t ;
	std::vector<word_count_t> word_counts ;
	word_counts.reserve(wordmap.size());
	for (auto it = wordmap.cbegin(); it != wordmap.cend(); ++it) {
		word_counts.push_back( {it->first, it->second} );
	}
	std::cout << "size=" << word_counts.size() << " capacity=" << word_counts.capacity() << "\n";

	// sort to find the most popular words
	// https://stackoverflow.com/questions/279854/how-do-i-sort-a-vector-of-pairs-based-on-the-second-element-of-the-pair

	std::sort(word_counts.begin(), word_counts.end(), 
			[](word_count_t &a, word_count_t &b) {
				return a.second > b.second;
				});

	for (int i=0 ; i<20 ; i++ ) {
		std::cout << word_counts[i].first << " " << word_counts[i].second << "\n";
	}


}
