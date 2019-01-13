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

//#include <boost/filesystem.hpp>  // boost::filesystem migrated into std
#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/indexed.hpp>

#include "netrc.hpp"

#define DEBUG

netrc_map netrc_parse(std::istream& infile)
{
	netrc_map netrc {};

	std::cout << __func__ << " " << __LINE__ << "\n";
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
	std::size_t line_counter {0};
	while( getline(infile,line) ) {
		line_counter++;
		std::cout << __func__ << " " << __LINE__ << "\n";
		std::cout << "len=" << line.length() << " line=\"" << line << "\"\n";

		// bless you, boost
		std::cout << __func__ << " " << __LINE__ << "\n";
		boost::trim(line);

		if (line.length() == 0 || line[0] == '#') {
			// ignore blank lines
			// ignore comment lines
			std::cout << "drop blank//comment line\n";
			continue;
		}
		std::cout << __func__ << " " << __LINE__ << "\n";

//		std::cout << "len=" << line.length() << " line=\"" << line << "\"\n";

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
		std::cout << __func__ << " " << __LINE__ << "\n";

		for (size_t i=0 ; i<fields.size() ; i++ ) {
			std::cout << "size=" << fields.size() << " i=" << i << " field=" << fields[i] << " line#=" << line_counter << "\n";
			if (fields[i] == "machine" || fields[i] == "default") {
				if (machine_counter > 0) {
					// new machine block so save previous
					netrc[machine_name] = std::make_tuple(login, password, account);
					std::cout << "save machine=" << machine_name << " login=" << login << " account=" << account << " password=" << password << "\n";
				}
				if (fields[i] == "machine") {
					if (i+1 >= fields.size() ) {
						throw parse_error(line);
					}
					machine_name = std::move(fields.at(++i));
				}
				else {
					machine_name = "default";
				}
				login.clear();
				password.clear();
				account.clear();
				machine_counter++;
				continue;
			}
			// at this point we must have at least two tokens
			if (i+1 >= fields.size() ) {
				throw parse_error(line);
			}

			if (fields[i] == "login") {
				login = std::move(fields.at(++i));
			}
			else if (fields[i] == "account") {
				account = std::move(fields.at(++i));
			}
			else if (fields[i] == "password") {
				password = std::move(fields.at(++i));
			}
			else {
				throw parse_error(line);
			}
		}
	}

	// save final parse
	if ( ! (machine_name.empty() || login.empty() || password.empty())  ) {
		netrc[machine_name] = std::make_tuple(login, password, account);
		std::cout << "save machine=" << machine_name << " login=" << login << " account=" << account << " password=" << password << "\n";
	}
	std::cout << __func__ << " " << __LINE__ << "\n";

	return netrc;
}

netrc_map netrc_parse_file( const std::string& infilename)
{
	fs::path path {infilename};

	if (!fs::is_regular_file(path)) {
		std::clog << path << " is not a regular file\n";
		throw file_error(path);
	}

	// verify permissions (must be owner readable, only)
	// (I think this is what curl, ftp do as well but I'm not sure)
	fs::perms perm = fs::status(path).permissions();
	if ( (perm & (fs::perms::others_all | fs::perms::group_all)) != fs::perms::none ) {
		std::clog << path << " has wrong permissions\n";
		throw file_error(path);
	}

//	std::cout << infilename << "\n";

	// load file into vector of string
	std::ifstream infile{path};
//	std::vector<std::string> netrc_lines;
//	for( std::string line ; getline(infile,line) ; netrc_lines.push_back(line) );
//	std::cout << "read " << netrc_lines.size() << " lines\n";

	return netrc_parse(infile);
}

