/* davep 20181231 ; parse a .netrc 
 * Going to use .netrc as an option to read username/passwords for cradlepoint
 * router logins.
 * Also an excuse to play with C++ file IO and strings some more.
 * Ooo! And an opportunity to play with <filesystem>
 *
 * References:
 * man netrc(5)
 * https://docs.python.org/3/library/netrc.html
 *
 */


#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <exception>
#include <cstdlib>
//#include <boost/filesystem.hpp>  // boost::filesystem migrated into std
#include <boost/algorithm/string.hpp>

// https://en.cppreference.com/w/cpp/feature_test
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

class parse_error : public std::exception
{
public:
	parse_error (std::string& failure) noexcept
	{
		this->line_failed = failure;
	}
	
	virtual const char* what() const throw()
	{ 
		return this->line_failed.c_str();
	}

private:
	std::string line_failed;
};

// key: machine's value
// value: pair of login,password
using netrc_map  = std::map<std::string, std::pair<std::string,std::string>> ;

netrc_map parse_netrc(std::istream& infile)
{
	netrc_map netrc {};

	std::string line;
	std::size_t counter {0};
	std::string machine_name;
	std::string login, password;

	std::vector<std::string> fields;

	while( getline(infile,line) ) {
		// bless you, boost
		boost::trim(line);

		std::cout << "len=" << line.length() << " line=" << line << "\n";
		if (line.length() == 0) {
			// ignore blank lines
			continue;
		}

		// split preserves empty fields
		boost::split(fields, line, boost::algorithm::is_space());
		// so remove blank fields
		fields.erase(
			std::remove_if(
				fields.begin(), 
				fields.end(), 
				[](std::string& s)->bool { return s.length() == 0; }), 
			fields.end());
		for (auto s: fields) {
			std::cout << "field=" << s << "\n";
		}

		std::size_t pos = line.find_first_of(" \t");
		if (pos == std::string::npos) {
			// invalid line
			throw parse_error(line);
		}

		std::string key = line.substr(0,pos);
		std::cout << "key=\"" << key << "\"\n";

		std::size_t value_pos = line.find_first_not_of(" \t", pos);
		std::string value = line.substr(value_pos, line.length()-value_pos+1);
		std::cout << "value=\"" << value << "\"\n";

		if (key.compare("machine") == 0) {
			if (counter > 0) {
				// we have hit a new machine block so save the current
				std::cout << "save machine=" << machine_name << " login=" << login << " password=" << password << "\n";
				netrc[machine_name] = std::make_pair(login, password);
			}
			// start over
			machine_name = std::move(value);
			login.clear();
			password.clear();
			counter++;
		}
		else if (key == "login") {
			login = std::move(value);
		}
		else if (key == "password") {
			password = std::move(value);
		}
		else {
			throw parse_error(line);
		}

	}
	// save final parse
	netrc[machine_name] = std::make_pair(login, password);

	return netrc;
}

void test(void)
{
	std::string testbuf {
		// C++ raw string literal F T W
R"#(machine 172.16.17.1
login admin
password 12345

machine 172.16.22.1
login admin
password 12345

machine 172.19.10.119
login admin
password 00000000

	machine 		foo.bar.baz
	login	dave
	password 		this is invalid because embedded spaces
)#"
	};

	std::stringstream infile{testbuf};

	auto netrc = parse_netrc(infile);

	std::array<std::string,3> tests { "172.16.22.1", "172.16.17.1", "172.19.10.119" };
	for (auto s : tests) {
		try {
			auto auth = netrc.at(s);
			std::cout << "username=" << std::get<0>(auth) << " password=" << std::get<1>(auth) << "\n";
		} catch (std::out_of_range& err ) {
			std::cerr << "machine " << s << " not found\n";
		};
	}

}

int main(int argc, char *argv[] )
{
	test();

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
	if (!fs::is_regular_file(path)) {
		std::cerr << path << " is not a regular file\n";
		return EXIT_FAILURE;
	}

	// verify permissions (must be owner readable, only)
	// (I think this is what curl, ftp do as well but I'm not sure)
	fs::perms perm = fs::status(path).permissions();
	if ( (perm & (fs::perms::others_all | fs::perms::group_all)) != fs::perms::none ) {
		std::cerr << path << " has wrong permissions\n";
		return EXIT_FAILURE;
	}

	std::cout << path << "\n";

	// load file into vector of string
	std::ifstream infile{path};
//	std::vector<std::string> netrc_lines;
//	for( std::string line ; getline(infile,line) ; netrc_lines.push_back(line) );
//	std::cout << "read " << netrc_lines.size() << " lines\n";

	auto netrc = parse_netrc(infile);

	std::array<std::string,3> tests { "172.16.22.1", "172.16.17.1", "172.19.10.119" };
	for (auto s : tests) {
		try {
			auto auth = netrc.at(s);
			std::cout << "username=" << std::get<0>(auth) << " password=" << std::get<1>(auth) << "\n";
		} catch (std::out_of_range& err ) {
			std::cerr << "machine " << s << " not found\n";
		};
	}

	// parse netrc files from cli 
	for (int i=1 ; i<argc ; i++) {
		std::string f = argv[i];
	}


	return EXIT_SUCCESS;
}
