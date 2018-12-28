/* davep 20181227 ; tinkering with boost::format */
#include <iostream>
#include <boost/format.hpp>
#include <cstdlib>

typedef std::tuple<std::string, std::string, bool, std::string, std::string> status;

// fiddling with Uniform Initialization
status foo 
	{ "mdm-90981e7f", "mdm",        true,    "Failback",   "available"};

std::array<status,4> status_list {
	std::make_tuple("mdm-90981e7f", "mdm",        true,    "Failback",   "available"),
	std::make_tuple("mdm-90c5e536", "mdm",        true,    "Plugged",    "configure error"),
	std::make_tuple("ethernet-wan", "ethernet",   false,   "Unplugged",  "unplugged"),
	std::make_tuple("wwan-70:4d:7b:11:5b:7c", "wwan", true,    "Failback",   "connected")
};

int main(void)
{
	std::string key;
	std::string type_;
	bool plugged;
	std::string reason;
	std::string summary;

	std::cout << "                          NAME       TYPE    PLUGGED    REASON        SUMMARY\n";
	for (auto iter=status_list.begin() ; iter != status_list.end() ; iter++ ) {
		std::tie(key, type_, plugged, reason, summary) = *iter;
//		std::cout << boost::format("%1% %2% %3% %4% %5%\n") % key % type_ % plugged % reason % summary;
//		std::cout << boost::format("%|s| %|s| %|s| %|s| %|s|\n") % key % type_ % plugged % reason % summary;
//		std::cout << boost::format("%|20s| %|s| %|s| %|s| %|s|\n") % key % type_ % plugged % reason % summary;
		std::cout << boost::format("%|30s| %|10s|    %|-10s| %|-10s|    %|-24s|\n") % key % type_ % (plugged?"true":"false") % reason % summary;
	}

//	std::cout << boost::format("%25s %1% %2% %3% %4%\n") % key % type_ % plugged % reason % summary;

	return EXIT_SUCCESS;
}
