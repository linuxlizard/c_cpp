#pragma once

#include <iostream>
#include <ostream>
#include <optional>

// parse the cli into this struct so the caller can be independent of
// boost/program_options.hpp
struct Args
{
public:
	std::string target;
	std::string sort_by;
	unsigned int verbose;
	bool use_netrc;

	friend std::ostream& operator<<(std::ostream& stream, const struct Args& args) { 
		stream << "target=" << args.target << " netrc=" << args.use_netrc;
		return stream; 
	};
};

std::optional<Args> parse_args(int argc, const char * const argv[]);

