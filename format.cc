/* davep 20181227 ; tinkering with boost::format */
#include <iostream>
#include <boost/format.hpp>
#include <cstdlib>

/* davep 20190328 ; tinkering with fmt */
// http://fmtlib.net/latest/api.html#format
#include "fmt/format.h"

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

// http://fmtlib.net/latest/api.html#format
// "Formatting user-defined types"
namespace fmt {
	template <>
	struct formatter<status> {
	  template <typename ParseContext>
	  constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

	  template <typename FormatContext>
	  auto format(const status &p, FormatContext &ctx) {
		std::string key;
		std::string type_;
		bool plugged;
		std::string reason;
		std::string summary;
		std::tie(key, type_, plugged, reason, summary) = p;

		// didn't know get<type> was a possibility, too
		plugged = std::get<bool>(p);
//		return format_to(ctx.begin(), "{0:>40s} {1:<10s} {2:<7} {3:<10s} {4}", 
//				key, type_, plugged, reason, summary);

		return format_to(ctx.begin(), "{0:>40s} {1:<10s} {2:<7} {3:<10s} {4}", 
				std::get<0>(p), std::get<1>(p), std::get<2>(p), std::get<3>(p), std::get<4>(p));
	  }
	};
}

int main(void)
{
	std::string key;
	std::string type_;
	bool plugged;
	std::string reason;
	std::string summary;

	std::cout << "                          NAME       TYPE    PLUGGED    REASON        SUMMARY\n";
	for (auto iter=status_list.begin() ; iter != status_list.end() ; ++iter ) {
		std::tie(key, type_, plugged, reason, summary) = *iter;
//		std::cout << boost::format("%1% %2% %3% %4% %5%\n") % key % type_ % plugged % reason % summary;
//		std::cout << boost::format("%|s| %|s| %|s| %|s| %|s|\n") % key % type_ % plugged % reason % summary;
//		std::cout << boost::format("%|20s| %|s| %|s| %|s| %|s|\n") % key % type_ % plugged % reason % summary;
		std::cout << boost::format("%|30s| %|10s|    %|-10s| %|-10s|    %|-24s|\n") % key % type_ % (plugged?"true":"false") % reason % summary;
	}

//	std::cout << boost::format("%25s %1% %2% %3% %4%\n") % key % type_ % plugged % reason % summary;

	/* davep 20190328 ; fiddling with fmt (http://fmtlib.net) */
	// http://fmtlib.net/latest/api.html#format
	// http://fmtlib.net/latest/syntax.html#syntax
	std::cout << "\nTrying fmtlib\n";
	std::string message = fmt::format("The answer is {}", 42);
	std::cout << message << "\n";
	fmt::print("Elapsed time: {0:.2f} seconds\n", 1.23);
	fmt::print("Elapsed time: {s:.2f} seconds\n", fmt::arg("s", 1.23));
	std::string smsg = fmt::format("{}\n", status_list.at(0));

	std::cout << "                                    NAME TYPE       PLUGGED REASON     SUMMARY\n";
	for (auto &status : status_list) {
//	for (auto iter=status_list.begin() ; iter != status_list.end() ; ++iter ) {
		std::string smsg = fmt::format("{}\n", status);
//		std::string smsg = fmt::format("{}\n", *iter);
		std::cout << smsg;
	}

	std::string flip = fmt::format("flip {}", u8"(╯°□°）╯︵ ┻━┻");
	std::cout << flip << "\n";

	return EXIT_SUCCESS;
}
