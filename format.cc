#include <iostream>
#include <cstdlib>

/* davep 20190328 ; tinkering with fmt */
/* davep 20210716 ; upgrade completely to fmt (no more boost::format) */
// http://fmtlib.net/latest/api.html#format
#include <fmt/format.h>

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
template <> 
struct fmt::formatter<status> 
{
	char presentation = 'S';

	std::string status_fmt { "{0:30s} {1:10s}    {2:10s} {3:10s}    {4:24s}" };

	// Parses format specifications of the form ['f' | 'e'].
	constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
		// [ctx.begin(), ctx.end()) is a character range that contains a part of
		// the format string starting from the format specifications to be parsed,
		// e.g. in
		//
		//   fmt::format("{:f} - point of interest", point{1, 2});
		//
		// the range will contain "f} - point of interest". The formatter should
		// parse specifiers until '}' or the end of the range. In this example
		// the formatter should parse the 'f' specifier and return an iterator
		// pointing to '}'.

		// Parse the presentation format and store it in the formatter:
		auto it = ctx.begin(), end = ctx.end();
		if (it != end && (*it == 'S')) presentation = *it++;
//		if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

		// Check if reached the end of the range:
		if (it != end && *it != '}')
			throw fmt::format_error("invalid format");

		// Return an iterator past the end of the parsed range:
		return it;
	}


	// Formats the point p using the parsed format specification (presentation)
	// stored in this formatter.
	template <typename FormatContext>
	auto format(const status& p, FormatContext& ctx) -> decltype(ctx.out()) {
		// ctx.out() is an output iterator to write to.
		auto [ key, type_, plugged, reason, summary ] = p;
		return fmt::format_to(
				ctx.out(),
				status_fmt, key, type_, (plugged?"true":"false"), reason, summary
				);
	}
};

int main(void)
{
	std::string key;
	std::string type_;
	bool plugged;
	std::string reason;
	std::string summary;

	std::string status_fmt { "{0:30s} {1:10s}    {2:10s} {3:10s}    {4:24s}\n" };
	fmt::print( status_fmt, "NAME", "TYPE", "PLUGGED", "REASON", "SUMMARY" );

	for (auto iter=status_list.begin() ; iter != status_list.end() ; ++iter ) {
		auto [ key, type_, plugged, reason, summary ] = *iter;

		fmt::print(status_fmt, key, type_, (plugged?"true":"false"), reason, summary);
	}

	/* davep 20190328 ; fiddling with fmt (http://fmtlib.net) */
	// http://fmtlib.net/latest/api.html#format
	// http://fmtlib.net/latest/syntax.html#syntax
	std::cout << "\nTrying fmtlib\n";
	std::string message = fmt::format("The answer is {}", 42);
	std::cout << message << "\n";
	fmt::print("Elapsed time: {0:.2f} seconds\n", 1.23);
	fmt::print("Elapsed time: {s:.2f} seconds\n", fmt::arg("s", 1.23));

	fmt::print( status_fmt, "NAME", "TYPE", "PLUGGED", "REASON", "SUMMARY" );
	for (auto &status : status_list) {
		std::string smsg = fmt::format("{:S}\n", status);
		std::cout << smsg;
	}

	// utf8?
	std::string flip = fmt::format("flip {}", u8"(╯°□°）╯︵ ┻━┻");
	std::cout << flip << "\n";

	return EXIT_SUCCESS;
}
