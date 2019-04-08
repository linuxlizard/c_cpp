#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <numeric>

namespace {
	const char hex_ascii[] ="0123456789abcdef";
}

int main()
{
//	std::vector<uint8_t> input { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };
	std::vector<uint8_t> input(256);

	std::iota(input.begin(), input.end(), 0);

	std::stringstream ss;
	std::ostreambuf_iterator<char> out_ss(ss);

	std::for_each(input.cbegin(), input.cend(), 
			[&out_ss](char c) { 
				*out_ss++ = hex_ascii[(c>>4)&0x0f]; 
				*out_ss++ = hex_ascii[c&0x0f]; 
			});

	std::cout << "len=" << ss.str().length() << "\n";
	std::cout << ss.str() << "\n";
}

