/* davep 20181231 ; parse a .netrc 
 * Going to use .netrc as an option to read username/passwords for cradlepoint
 * router logins.
 * Also an excuse to play with C++ file IO and strings some more.
 * Ooo! And an opportunity to play with <filesystem>
 */


#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <map>
#include <exception>
#include <cstdlib>

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

netrc_map parse_netrc(std::vector<std::string> lines)
{
	netrc_map netrc {};

	std::size_t counter {0};
	std::string machine_name;
	std::string login, password;
	for ( auto line :lines) {
		std::cout << "len=" << line.length() << " line=" << line << "\n";
		if (line.length() == 0) {
			// ignore blank lines
			continue;
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

int main(int argc, char *argv[] )
{
	std::string home = std::getenv("HOME");

	std::filesystem::path path(home);
	path.append(".netrc");
	if ( !std::filesystem::exists(path) ) {
		// try the windows version
		path = home;
		path.append("_netrc");
		if ( !std::filesystem::exists(path) ) {
		}
	}
	if (!std::filesystem::is_regular_file(path)) {
		std::cerr << path << " is not a regular file\n";
		return EXIT_FAILURE;
	}

	// verify permissions (must be owner readable, only)
	// (I think this is what curl, ftp do as well but I'm not sure)
	std::filesystem::perms perm = std::filesystem::status(path).permissions();
//	auto mask = std::filesystem::perms::owner_all;
	if ( (perm & (std::filesystem::perms::others_all | std::filesystem::perms::group_all)) != std::filesystem::perms::none ) {
		std::cerr << path << " has wrong permissions\n";
		return EXIT_FAILURE;
	}

	std::cout << path << "\n";

	std::ifstream infile{path};
	std::vector<std::string> netrc_lines;
	for( std::string line ; getline(infile,line) ; netrc_lines.push_back(line) );

	std::cout << "read " << netrc_lines.size() << " lines\n";
	auto netrc = parse_netrc(netrc_lines);

	auto auth = netrc.at("172.16.22.1");
	std::cout << "username=" << std::get<0>(auth) << " password=" << std::get<1>(auth) << "\n";

	auth = netrc.at("172.16.17.1");
	std::cout << "username=" << std::get<0>(auth) << " password=" << std::get<1>(auth) << "\n";

	auth = netrc.at("172.19.10.119");
	std::cout << "username=" << std::get<0>(auth) << " password=" << std::get<1>(auth) << "\n";

	// parse netrc files from cli 
	for (int i=1 ; i<argc ; i++) {
		std::string f = argv[i];
	}


	return EXIT_SUCCESS;
}
