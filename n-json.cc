/* davep 20210616 ;  */
// gcc -g -Wall -c -o n-json.o n-json.cc -std=c++17 -I../n-json/include 
#include <string>
#include <nlohmann/json.hpp>

#include "n-json.h"

using json = nlohmann::json;

json parse(std::string &s)
{
	return json::parse(s);
}

