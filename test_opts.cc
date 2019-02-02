#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstdlib>
#include <boost/algorithm/string.hpp>

#define BOOST_TEST_MODULE test_opts
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "opts.hpp"

BOOST_AUTO_TEST_CASE(test_zero) 
{
	// this is my boost unit_test sanity check to verify I at least built
	// correctly
	int i {0};
	BOOST_REQUIRE(i==0);
}

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

BOOST_AUTO_TEST_CASE(test_help) 
{
	const char* const argv[] = {
		"test_opts", "-h"
	};

	auto parsed = parse_args(ARRAY_SIZE(argv), argv);
	BOOST_REQUIRE(!parsed);
}

BOOST_AUTO_TEST_CASE(test_simple) 
{
	const char* const argv[] = {
		"test_opts",
		"-n", 
		"-s", "ssid", 
		"-t", "http://192.168.0.1",
		"-v", "42",
	};

	BOOST_TEST_CHECKPOINT("parse_args");
	auto parsed = parse_args(ARRAY_SIZE(argv), argv);

	BOOST_TEST_CHECKPOINT("verify parsed");
	BOOST_REQUIRE(parsed);
	auto const opts = parsed.value();

	BOOST_TEST_CHECKPOINT("check target");
	BOOST_REQUIRE(!opts.target.empty());
	BOOST_REQUIRE(opts.target == "http://192.168.0.1");

	BOOST_TEST_CHECKPOINT("check sort_by");
	BOOST_REQUIRE(!opts.sort_by.empty());
	BOOST_REQUIRE(opts.sort_by == "ssid");

	BOOST_TEST_CHECKPOINT("expect opts.verbose == " << 42);
	BOOST_REQUIRE(opts.verbose == 42);
}
