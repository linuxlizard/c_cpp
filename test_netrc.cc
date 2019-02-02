#include <iostream>
#include <fstream>
#include <string>
#include <map>
//#include <exception>
#include <cstdlib>
#include <boost/algorithm/string.hpp>

#define BOOST_TEST_MODULE test_netrc
#define BOOST_TEST_DYN_LINK
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

BOOST_AUTO_TEST_CASE(test_zero) {
	int i {0};
	BOOST_REQUIRE(i==0);
}

BOOST_AUTO_TEST_CASE(test_one) {
	int i {1};
	BOOST_REQUIRE(i==1);
}

void verify(netrc_map& netrc)
{
	std::array<std::string,4> tests { "172.16.22.1", "172.16.17.1", "172.19.10.119", "default" };
	for (auto s : tests) {
		// running inside test code so want the exception to propogate
		auto auth = netrc.at(s);
		std::cout << s << " username=" << std::get<0>(auth) << " password=" << std::get<1>(auth) << "\n";
//		try {
//			auto auth = netrc.at(s);
//			std::cout << s << " username=" << std::get<0>(auth) << " password=" << std::get<1>(auth) << "\n";
//		} catch (std::out_of_range& err ) {
//			std::cerr << "machine " << s << " not found\n";
//		};
	}
}

BOOST_AUTO_TEST_CASE(test_simple)
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

BOOST_AUTO_TEST_CASE(test_only_default)
{
	std::string testbuf {
R"#(default login admin password hythloday@example.com

)#"};

	std::stringstream infile{testbuf};
	auto netrc = netrc_parse(infile);

	auto default_ = netrc.at("default");
	auto [a,b,c] = default_;
	BOOST_REQUIRE(a=="admin");
	BOOST_REQUIRE(b=="hythloday@example.com");

	auto [x,y,z] = netrc.at("default");
}

BOOST_AUTO_TEST_CASE(test_whitespace)
{
	std::string testbuf {
R"#(
#
	# should not see this machine
	# comment	machine ignore.example.com login foo password foo

		machine 172.16.17.1 login admin 
		password 12345 

# can have entire thing on one line
# oh, and comments, too
machine 192.168.0.1 login admin password 12345

	machine 			172.16.22.1				
	login admin
account foobarbaz
			password 12345

machine 172.19.10.119
login admin
password 00000000

default login admin password hythloday@example.com
)#"
	};

	std::stringstream infile{testbuf};

#if 1
	// tinkering with stringstream
	std::vector<std::string> netrc_lines;
	for( std::string line ; getline(infile,line) ; netrc_lines.push_back(line) );
	std::cout << "read " << netrc_lines.size() << " lines\n";
	// seems both these lines are required in order to re-read the stream
	infile.clear();
	infile.seekg(std::ios_base::beg);

	// re-read the stream
	netrc_lines.clear();
	for( std::string line ; getline(infile,line) ; netrc_lines.push_back(line) );
	std::cout << "read " << netrc_lines.size() << " lines\n";
	BOOST_TEST(netrc_lines.size() == 22);

	infile.clear();
	infile.seekg(std::ios_base::beg);
#endif
	auto netrc = netrc_parse(infile);
	verify(netrc);

	// verify not see the 'ignore.example.com'
	BOOST_REQUIRE_THROW( netrc.at("ignore.example.com"), std::out_of_range);
}

class NetrcFile
{
public:
	fs::path path;
	std::ofstream outfile;
	int fd;

	NetrcFile() : path(fs::temp_directory_path() / "netrctestXXXXXX")
	{ 
		char filename[FILENAME_MAX+1] = {};

		strncpy(filename, path.native().c_str(), FILENAME_MAX);
		fd = mkstemp(filename);
		if (fd < 0) {
			std::clog << "unable to create temp file=" << path << " errno=" 
				<< errno << " error=\"" << strerror(errno) << "\n";
			throw file_error(path);
		}

		path = filename;
		outfile.open(path);
		std::cout << "NetrcFile " << path << "\n";
	};

	~NetrcFile() 
	{
		std::cout << "~NetrcFile " << path << "\n";
		close(fd);
		outfile.close();
		bool success = fs::remove(path);
		if (!success) {
			// can't throw in a destuctor so we'll just complain loudly
			std::cerr << "failed to remove the file=" << path << std::endl;
		}
	};
};


class NetrcFileDefault : public NetrcFile
{
public:
	NetrcFileDefault()
	{
		outfile << "default login admin password hythloday@example.com\n" << std::endl;
	};
};

BOOST_FIXTURE_TEST_CASE(test_file, NetrcFile)
{
	outfile << "default login admin password hythloday@example.com\n" << std::endl;

	// file should be created with 0600 so change to incorrect permissions
	// davep 20190113 ; perms::add_perms doesn't seem to exist yet so big hammer set the perms
	fs::permissions(path, fs::perms::owner_read|fs::perms::owner_write|fs::perms::group_read|fs::perms::group_write);
//	fs::permissions(path, fs::perms::add_perms|fs::perms::group_read|fs::perms::group_write);

	// verify failure if permissions are wrong
	BOOST_REQUIRE_THROW( netrc_parse_file(path.native()), file_error);

	fs::permissions(path, fs::perms::owner_read|fs::perms::owner_write);

	auto netrc = netrc_parse_file(path.native());
	auto [a,b,c] = netrc.at("default");
	std::cout << a << " " << b << " " << c << "\n";
	BOOST_REQUIRE(a=="admin");
	BOOST_REQUIRE(b=="hythloday@example.com");
	BOOST_REQUIRE(c.empty());
}

BOOST_FIXTURE_TEST_CASE(test_file_with_fixture, NetrcFile)
{
	outfile << "machine 192.168.0.1 login admin password 12345\n" << std::endl;

	auto netrc = netrc_parse_file(path.native());
	auto [a,b,c] = netrc.at("192.168.0.1");
	BOOST_REQUIRE(a=="admin");
	BOOST_REQUIRE(b=="12345");
	BOOST_REQUIRE(c.empty());
}

BOOST_FIXTURE_TEST_CASE(test_file_with_default_fixture, NetrcFileDefault)
{
	auto netrc = netrc_parse_file(path.native());
	auto [a,b,c] = netrc.at("default");
	BOOST_REQUIRE(a=="admin");
	BOOST_REQUIRE(b=="hythloday@example.com");
	BOOST_REQUIRE(c.empty());
}

BOOST_FIXTURE_TEST_CASE(test_file_with_macdef, NetrcFileDefault)
{
	outfile << "machine 192.168.0.1 login admin password 12345\n" << std::endl;
	outfile << "macdef init\nfoo\nbar\nbaz\n\n" << std::endl;

	auto netrc = netrc_parse_file(path.native());
	auto [login,password,account] = netrc.at("default");
	BOOST_REQUIRE(login=="admin");
	BOOST_REQUIRE(password=="hythloday@example.com");
	BOOST_REQUIRE(account.empty());

	std::tie(login,password,account) = netrc.at("192.168.0.1");
	BOOST_REQUIRE(login=="admin");
	BOOST_REQUIRE(password=="12345");
	BOOST_REQUIRE(account.empty());
}

BOOST_FIXTURE_TEST_CASE(test_file_with_macdef_2, NetrcFileDefault)
{
	outfile << "macdef init\nfoo\nbar\nbaz\n\n" << std::endl;
	outfile << "machine 192.168.0.1 login admin password 12345\n" << std::endl;
	outfile << "default login admin password hythloday@example.com\n" << std::endl;

	auto netrc = netrc_parse_file(path.native());
	auto [login,password,account] = netrc.at("default");
	BOOST_REQUIRE(login=="admin");
	BOOST_REQUIRE(password=="hythloday@example.com");
	BOOST_REQUIRE(account.empty());

	std::tie(login,password,account) = netrc.at("192.168.0.1");
	BOOST_REQUIRE(login=="admin");
	BOOST_REQUIRE(password=="12345");
	BOOST_REQUIRE(account.empty());
}

BOOST_FIXTURE_TEST_CASE(test_file_with_macdef_3, NetrcFile)
{
	outfile << "machine 192.168.0.1 login admin password 12345\n";
//	outfile << "macdef init\nfoo\nbar\nbaz\n\n";
	outfile << "default login admin password 12345\n";
	outfile << "macdef init\nfoo\nbar\nbaz" << std::endl;

	auto netrc = netrc_parse_file(path.native());
	auto [login,password,account] = netrc.at("default");
	BOOST_REQUIRE(login=="admin");
	BOOST_REQUIRE(password=="12345");
	BOOST_REQUIRE(account.empty());

	std::tie(login,password,account) = netrc.at("192.168.0.1");
	BOOST_REQUIRE(login=="admin");
	BOOST_REQUIRE(password=="12345");
	BOOST_REQUIRE(account.empty());
}

BOOST_AUTO_TEST_CASE(test_home_netrc)
{
	// look for a $HOME/.netrc and parse it if it exists
	std::string home = std::getenv("HOME");
	fs::path path(home);
	path.append(".netrc");
	if ( !fs::exists(path) ) {
		// try the windows version
		path = home;
		path.append("_netrc");
		if ( !fs::exists(path) ) {
			std::cerr << "no .netrc found\n"; 
			return;
		}
	}

	auto netrc = netrc_parse_file(path.native());

	netrc.at("default");

//	verify(netrc);
//
//	// parse netrc files from cli 
//	for (int i=1 ; i<argc ; i++) {
//		std::string f = argv[i];
//	}
//
//	return EXIT_SUCCESS;
}

