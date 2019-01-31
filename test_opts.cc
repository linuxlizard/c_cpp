#include <iostream>
#include <fstream>
#include <string>
#include <map>
//#include <exception>
#include <cstdlib>
#include <boost/algorithm/string.hpp>

#define BOOST_TEST_MODULE test_opts
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "opts.hpp"

BOOST_AUTO_TEST_CASE(test_zero) {
	int i {0};
	BOOST_REQUIRE(i==0);
}

