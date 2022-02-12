#include <iostream>
#include <cassert>

#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include "json.hpp"

using json = nlohmann::json;

int main(void)
{
        uint8_t num8;
        uint16_t num16;
        uint32_t num32;
        uint64_t num64;
        num8 = num16 = num32 = num64 = 0;

        for (size_t i=0 ; i<8 ; i++) {
                json v;
                num8 = 1u<<i;
                v["num8"] = num8;
                std::cout << std::dec << (unsigned int)num8 << " " << std::hex << (unsigned int)num8 << " " << v.dump() << "\n";
        }

        for (size_t i=0 ; i<16 ; i++) {
                json v;
                num16 = 1u<<i;
                v["num16"] = num16;
                std::cout << std::dec << num16 << " 0x" << std::hex << num16 << " " << v.dump() << "\n";
        }

        for (size_t i=0 ; i<32 ; i++) {
                json v;
                num32 = 1u<<i;
                v["num32"] = num32;
                std::cout << std::dec << num32 << " 0x" << std::hex << num32 << " " << v.dump() << "\n";
        }

        for (size_t i=0 ; i<32 ; i++) {
                json v;
                num32 = 1ul<<i;
                v["num32"] = num32;
                std::cout << std::dec << num32 << " 0x" << std::hex << num32 << " " << v.dump() << "\n";
        }

        for (size_t i=0 ; i<64 ; i++) {
                json v;
                num64 = 1ull<<i;
                v["num64"] = num64;
                std::cout << std::dec << num64 << " 0x" << std::hex << num64 << " " << v.dump() << "\n";
        }


        return 0;
}
