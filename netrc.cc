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
#include <set>
//#include <boost/filesystem.hpp>  // boost::filesystem migrated into std
#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/indexed.hpp>

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
// value: tuple of login,password,account
// (account is optional)
using netrc_map  = std::map<std::string, std::tuple<std::string, std::string, std::string>> ;

netrc_map parse_netrc(std::istream& infile)
{
	netrc_map netrc {};

	std::string line;
	std::size_t machine_counter {0};
	std::string machine_name;
	std::string login, password, account;

	std::vector<std::string> fields;

	// https://www.gnu.org/software/inetutils/manual/html_node/The-_002enetrc-file.html
	//
	// keywords: 
	// machine default login password account macdef
	std::set<std::string> keywords { "machine", "default", "login", "password", "account", "macdef" };

	// FIXME spaces can't be in values (e.g., passwords); will need to switch to a different
	// parsing technique to support
	while( getline(infile,line) ) {
		// bless you, boost
		boost::trim(line);

		if (line.length() == 0 || line[0] == '#') {
			// ignore blank lines
			// ignore comment lines
			continue;
		}

		std::cout << "len=" << line.length() << " line=\"" << line << "\"\n";

		// split preserves empty fields
		boost::split(fields, line, boost::algorithm::is_space());
		// so remove blank fields
		fields.erase(
			std::remove_if(
				fields.begin(), 
				fields.end(), 
				[](std::string& s)->bool { return s.length() == 0; }), 
			fields.end());

		// output index+value like python enumerate()
		for (auto s: fields | boost::adaptors::indexed(0) ) {
			std::cout << "index=" << s.index() << " field=" << s.value() << "\n";
		}

		for (size_t i=0 ; i<fields.size() ; i++ ) {
			if (fields[i] == "machine" || fields[i] == "default") {
				if (machine_counter > 0) {
					// new machine block so save previous
					netrc[machine_name] = std::make_tuple(login, password, account);
					std::cout << "save machine=" << machine_name << " login=" << login << " account=" << account << " password=" << password << "\n";
				}
				if (fields[i] == "machine") {
					i++;
					machine_name = std::move(fields[i]);
				}
				else {
					machine_name = "default";
				}
				login.clear();
				password.clear();
				account.clear();
				machine_counter++;
			}
			else if (fields[i] == "login") {
				login = std::move(fields[++i]);
			}
			else if (fields[i] == "account") {
				account = std::move(fields[++i]);
			}
			else if (fields[i] == "password") {
				password = std::move(fields[++i]);
			}
			else {
				throw parse_error(line);
			}

#if 0
			else {
				auto search = keywords.find(fields[i]);
				if (search != keywords.end()) {
					std::cout << "keyword=" << *search << " value=" << fields[i+1] << "\n";
				}
				else {
					throw parse_error(line);
				}
				i++;
//				value = fields[i];
			}
#endif
		}
	}

	// save final parse
	if ( ! (machine_name.empty() || login.empty() || password.empty())  ) {
		netrc[machine_name] = std::make_tuple(login, password, account);
		std::cout << "save machine=" << machine_name << " login=" << login << " account=" << account << " password=" << password << "\n";
	}

	return netrc;
}

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
	auto netrc = parse_netrc(infile);
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

	verify(netrc);

	// parse netrc files from cli 
	for (int i=1 ; i<argc ; i++) {
		std::string f = argv[i];
	}

	return EXIT_SUCCESS;
}
