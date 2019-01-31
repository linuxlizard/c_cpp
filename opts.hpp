#ifndef OPTS_H
#define OPTS_H

// parse the cli into this struct so the caller can be independent of
// boost/program_options.hpp
struct Args
{
	std::string target;
	std::string sort_by;
	unsigned int verbose;
	bool use_netrc;
};

//std::optional<struct Args args> parse_args(int argc, char *argv[]);

#endif

