/* davep 20190103 ; learn some std::sort so can better display wlansurvey.cc results */
#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>
//#include <execution>
#include <ctime>

// http://fmtlib.net/latest/api.html#format
#include <fmt/format.h>

using survey = std::tuple<std::string, std::string, int, int, int, std::string>;

std::vector<survey> data {
	std::make_tuple("Gouda", "70:4d:7b:11:5b:7c",-23, 149, 5745, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER2200-b15-5g", "00:30:44:16:7b:17",-40, 157, 5785, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AP22-7cc-5g", "00:30:44:24:27:ce",-39, 149, 5745, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("Oranjestad", "ac:a3:1e:03:c6:70",-41, 52, 5260, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("CP-CORP", "24:f2:7f:a3:40:10",-67, 48, 5240, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER2200-96e-5g", "00:30:44:2c:59:70",-40, 161, 5805, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("IBR900-405-5g", "00:30:44:36:94:07",-65, 36, 5180, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("Vampire", "00:30:44:22:2f:38",-37, 36, 5180, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("CP-Guest", "24:f2:7f:a3:40:11",-66, 48, 5240, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("AER3100-5a0-5g", "00:30:44:29:65:a2",-69, 48, 5240, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER2200-438-5g", "00:30:44:2f:64:3a",-74, 157, 5785, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("AER2200-f72-5g", "00:30:44:2d:0f:74",-74, 48, 5240, "Thu Jan  3 15:10:16 2019"),
	std::make_tuple("AER2200-c3c-5g", "00:30:44:2d:ac:3e",-70, 153, 5765, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("CP-Engineering", "00:30:44:24:64:42",-31, 60, 5300, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("Krypton", "00:30:44:16:7b:1d",-36, 153, 5765, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER1600-8ec-5g", "00:30:44:16:78:ee",-39, 48, 5240, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AP22-74c-5g", "00:30:44:24:27:4e",-38, 36, 5180, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("IBR1700-ebd-5g", "00:30:44:2c:fe:c0",-52, 149, 5745, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER2200-7db-5g", "00:30:44:2e:57:dd",-53, 157, 5785, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("Golden Public", "06:30:44:2f:51:21",-55, 149, 5745, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("Monitor-analytics", "00:30:44:2d:7e:84",-56, 153, 5765, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER1600-6f5-5g", "00:30:44:21:16:f7",-43, 44, 5220, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("2100-791-5g", "00:30:44:19:67:93",-63, 157, 5785, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("Ken CP-07", "00:30:44:15:74:b8",-65, 36, 5180, "Thu Jan  3 15:10:16 2019"),
	std::make_tuple("AER3100-fb2-5g", "00:30:44:1f:3f:b4",-69, 157, 5785, "Thu Jan  3 15:12:16 2019"),
//	std::make_tuple("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", "00:30:44:29:5c:2d",-71, 161, 5805, "Thu Jan  3 15:11:16 2019"),
	std::make_tuple("NYCT-WiFi-5G", "00:30:44:2d:40:32",-64, 157, 5785, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("CP-GuestTest", "24:f2:7f:a3:40:12",-66, 48, 5240, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("Ken-1600-Cameras", "02:30:44:21:ad:65",-61, 36, 5180, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER1600-c81-5g", "00:30:44:1f:dc:83",-70, 44, 5220, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("IBR1700-aee-5g", "00:30:44:16:7a:f1",-56, 149, 5745, "Thu Jan  3 15:10:16 2019"),
	std::make_tuple("AER1600-cd2-5g", "00:30:44:1f:dc:d4",-71, 48, 5240, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("AER1600-a1a-5g", "00:30:44:16:7a:1c",-69, 157, 5785, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER1600-7c3-5g", "00:30:44:26:67:c5",-64, 44, 5220, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("2100-ca7-5g", "00:30:44:29:2c:a9",-64, 44, 5220, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("CP-Guest", "24:f2:7f:a3:74:71",-69, 36, 5180, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("Eberle_5G", "6c:3b:6b:c9:a2:26",-74, 149, 5745, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER1600-83c-5g", "00:30:44:28:58:3e",-70, 48, 5240, "Thu Jan  3 15:10:16 2019"),
	std::make_tuple("AER3100-34b-5g", "00:30:44:1d:83:4d",-73, 36, 5180, "Thu Jan  3 15:10:16 2019"),
	std::make_tuple("IBR1100-CASH5g", "00:30:44:26:27:db",-78, 36, 5180, "Thu Jan  3 15:09:57 2019"),
	std::make_tuple("NETGEAR64-5G", "dc:ef:09:a9:5c:cc",-81, 153, 5765, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("RailRoad-Public", "00:30:44:2d:33:e7",-57, 149, 5745, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("AER1600-dbf-5g", "00:30:44:1f:dd:c1",-60, 48, 5240, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("CP-GuestTest", "04:bd:88:73:5b:32",-68, 161, 5805, "Thu Jan  3 15:10:16 2019"),
	std::make_tuple("AER2200-abc-5g", "00:30:44:2f:6a:be",-74, 161, 5805, "Thu Jan  3 15:09:57 2019"),
	std::make_tuple("AER1600-19f-5g", "00:30:44:21:c1:a1",-74, 48, 5240, "Thu Jan  3 15:10:16 2019"),
	std::make_tuple("IBR900-b86-5g", "00:30:44:22:3b:88",-37, 36, 5180, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("CP-GuestTest", "24:f2:7f:a3:74:72",-69, 36, 5180, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER1600-f1e-5g", "00:30:44:27:cf:20",-70, 48, 5240, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("IBR900-105-5g", "00:30:44:36:51:07",-76, 153, 5765, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("2100-f00-5g", "00:30:44:18:5f:02",-81, 157, 5785, "Thu Jan  3 15:09:57 2019"),
	std::make_tuple("scottdev_public", "02:30:44:29:5c:2e",-70, 161, 5805, "Thu Jan  3 15:11:16 2019"),
	std::make_tuple("Cloud4Wi", "02:30:44:18:e7:69",-82, 48, 5240, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("CP-CORP", "04:bd:88:73:5b:30",-68, 161, 5805, "Thu Jan  3 15:11:16 2019"),
	std::make_tuple("IBR1100-314-5g", "00:30:44:1b:63:16",-74, 157, 5785, "Thu Jan  3 15:11:16 2019"),
	std::make_tuple("AER3100-1e5-5g", "00:30:44:1e:c1:e7",-74, 153, 5765, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("CP-Guest", "ac:a3:1e:da:c8:71",-76, 161, 5805, "Thu Jan  3 15:09:57 2019"),
	std::make_tuple("Eberle_Guest_5G", "6e:3b:6b:c9:a2:26",-74, 149, 5745, "Thu Jan  3 15:10:16 2019"),
	std::make_tuple("CP-Guest", "24:f2:7f:a3:38:71",-78, 149, 5745, "Thu Jan  3 15:09:57 2019"),
	std::make_tuple("CP-CORP", "24:f2:7f:a3:4f:10",-78, 36, 5180, "Thu Jan  3 15:09:57 2019"),
	std::make_tuple("CP-CORP", "24:f2:7f:a3:36:50",-50, 40, 5200, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("CP-Guest", "24:f2:7f:a3:36:51",-51, 40, 5200, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER1600-5f2-5g", "00:30:44:2a:35:f4",-68, 40, 5200, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("AER1600-a96-5g", "00:30:44:26:6a:98",-68, 40, 5200, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("2100-4c2-5g", "00:30:44:1e:14:c4",-69, 40, 5200, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("CP-CORP", "24:f2:7f:a3:4d:d0",-71, 40, 5200, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("Cradlepoint Booth", "00:30:44:22:05:12",-72, 36, 5180, "Thu Jan  3 15:11:16 2019"),
	std::make_tuple("AP22-600", "00:30:44:25:56:02",-72, 36, 5180, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("AER1600-390-5g", "00:30:44:1e:e3:92",-74, 40, 5200, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("AER2200-497-5g", "00:30:44:2f:44:99",-75, 48, 5240, "Thu Jan  3 15:10:57 2019"),
	std::make_tuple("Jakes2100", "00:30:44:16:c7:f3",-75, 157, 5785, "Thu Jan  3 15:11:16 2019"),
	std::make_tuple("Jakes1700", "00:30:44:2c:ff:50",-65, 149, 5745, "Thu Jan  3 15:09:57 2019"),
	std::make_tuple("AER1600-f27-5g", "00:30:44:27:cf:29",-72, 40, 5200, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("AER1600-76d-5g", "00:30:44:28:57:6f",-72, 48, 5240, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("AER1600-13f-5g", "00:30:44:27:01:41",-73, 48, 5240, "Thu Jan  3 15:10:57 2019"),
	std::make_tuple("AER1600-a38-5g", "00:30:44:16:7a:3a",-76, 40, 5200, "Thu Jan  3 15:10:16 2019"),
	std::make_tuple("AER1600-f24-5g", "00:30:44:1f:df:26",-76, 40, 5200, "Thu Jan  3 15:10:16 2019"),
	std::make_tuple("AER2200-b79-5g", "00:30:44:2d:ab:7b",-75, 157, 5785, "Thu Jan  3 15:10:57 2019"),
	std::make_tuple("PVcorp", "00:30:44:1f:7b:8d",-76, 40, 5200, "Thu Jan  3 15:09:57 2019"),
	std::make_tuple("AER1600-99e-5g", "00:30:44:28:a9:a0",-76, 40, 5200, "Thu Jan  3 15:09:57 2019"),
	std::make_tuple("AER1600-692-5g", "00:30:44:1e:66:94",-76, 40, 5200, "Thu Jan  3 15:10:16 2019"),
	std::make_tuple("CP-GuestTest", "24:f2:7f:a3:38:72",-77, 149, 5745, "Thu Jan  3 15:09:57 2019"),
	std::make_tuple("AER3100-5d6-5g", "00:30:44:1f:45:d8",-78, 40, 5200, "Thu Jan  3 15:10:16 2019"),
	std::make_tuple("CP-CORP", "24:f2:7f:a3:38:70",-77, 149, 5745, "Thu Jan  3 15:09:57 2019"),
	std::make_tuple("CP-GuestTest", "24:f2:7f:a3:4f:12",-77, 36, 5180, "Thu Jan  3 15:09:57 2019"),
	std::make_tuple("IBR900-b7a-5g", "00:30:44:22:3b:7c",-78, 36, 5180, "Thu Jan  3 15:09:57 2019"),
	std::make_tuple("bcprivate", "f0:25:72:ca:b9:0f",-80, 64, 5320, "Thu Jan  3 15:11:16 2019"),
	std::make_tuple("CP-Guest", "ac:a3:1e:da:c9:91",-81, 48, 5240, "Thu Jan  3 15:10:16 2019"),
	std::make_tuple("nmfield", "c8:b5:ad:6b:d8:91",-81, 52, 5260, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("bcpublic", "70:7d:b9:2b:0d:2e",-82, 56, 5280, "Thu Jan  3 15:10:57 2019"),
	std::make_tuple("AER1600-d9e-5g", "00:30:44:2c:9d:a0",-84, 48, 5240, "Thu Jan  3 15:10:16 2019"),
	std::make_tuple("PLTE-CP1", "00:30:44:31:06:46",-90, 36, 5180, "Thu Jan  3 15:11:16 2019"),
	std::make_tuple("AER3100-5f0-5g", "00:30:44:1e:c5:f2",-64, 40, 5200, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("PERF-513-5g", "00:30:44:22:05:15",-67, 40, 5200, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("CP-GuestTest", "24:f2:7f:a3:4d:d2",-70, 40, 5200, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("bcpublic", "38:90:a5:08:ed:2e",-74, 52, 5260, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("AER1600-072-5g", "00:30:44:2e:40:74",-75, 40, 5200, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("__SecDashTest", "00:30:44:1f:7a:d9",-76, 48, 5240, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("RailRoad-Private", "00:30:44:2d:33:e6",-77, 36, 5180, "Thu Jan  3 15:10:16 2019"),
	std::make_tuple("CP-GuestTest", "ac:a3:1e:f8:f1:12",-78, 36, 5180, "Thu Jan  3 15:10:16 2019"),
	std::make_tuple("CP-CORP", "ac:a3:1e:f8:f1:10",-78, 36, 5180, "Thu Jan  3 15:10:16 2019"),
	std::make_tuple("NM Guest", "c8:b5:ad:6b:d8:90",-81, 52, 5260, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("CP-GuestTest", "04:bd:88:62:76:72",-81, 48, 5240, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("bcpublic", "38:90:a5:32:bb:ce",-82, 52, 5260, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("bcpublic", "70:7d:b9:01:ca:ce",-82, 52, 5260, "Thu Jan  3 15:10:57 2019"),
	std::make_tuple("bcprivate", "70:7d:b9:28:2b:ef",-82, 60, 5300, "Thu Jan  3 15:10:57 2019"),
	std::make_tuple("IBR1700-f95-5g-r2", "00:30:44:2c:ff:98",-43, 165, 5825, "Thu Jan  3 15:11:16 2019"),
	std::make_tuple("NYCT-Yard-5g", "02:30:44:27:00:f4",-64, 40, 5200, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("AER1600-836-5g", "00:30:44:28:58:38",-69, 44, 5220, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("CP_Axon", "00:30:44:22:5c:1f",-67, 44, 5220, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("AER2200-Marketing", "00:30:44:2d:0f:ac",-72, 161, 5805, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("AER1600-c74-5g", "00:30:44:2a:1c:76",-77, 48, 5240, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("AER3100-WWAN Test", "00:30:44:1e:c8:0b",-79, 165, 5825, "Thu Jan  3 15:11:16 2019"),
	std::make_tuple("AER1600-PM", "00:30:44:1e:c8:fb",-72, 44, 5220, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER1600-a90-5g", "00:30:44:26:6a:92",-73, 44, 5220, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("_Drumpf.5", "00:30:44:1d:1c:e8",-75, 44, 5220, "Thu Jan  3 15:11:57 2019"),
	std::make_tuple("2100-0a7-5g", "00:30:44:18:60:a9",-79, 161, 5805, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER3100-160-5g", "00:30:44:1d:61:62",-43, 153, 5765, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER3100-8d4-5g", "02:30:44:16:78:d7",-43, 149, 5745, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER2200-a6f-5g", "00:30:44:33:ca:71",-47, 153, 5765, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("2100-38d-5g", "00:30:44:19:23:8f",-47, 149, 5745, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("CP-GuestTest", "24:f2:7f:a3:36:52",-49, 40, 5200, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("IBR1700-aea-5g", "00:30:44:16:7a:ed",-54, 44, 5220, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("MV-radio3-noncrit", "00:30:44:2d:36:0b",-57, 149, 5745, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("Golden-Yard-5g", "00:30:44:27:00:f3",-63, 40, 5200, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER3100-b1f-5g", "00:30:44:1e:bb:21",-67, 44, 5220, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER1600-f1b-5g", "00:30:44:27:cf:1d",-68, 44, 5220, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AP22-154", "00:30:44:24:61:56",-67, 36, 5180, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("Driver-WiFi-5G", "00:30:44:23:20:ae",-70, 44, 5220, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("cocotests5point0", "00:30:44:1e:c1:4e",-73, 161, 5805, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AP22-976", "00:30:44:24:c9:78",-74, 36, 5180, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER1600-90b-5g", "00:30:44:1e:c9:0d",-75, 44, 5220, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("2100-d3a-5g", "00:30:44:17:7d:3c",-76, 40, 5200, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("AER1600-686-5g", "00:30:44:28:a6:88",-79, 157, 5785, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("CP-CORP", "24:f2:7f:a3:74:70",-70, 36, 5180, "Thu Jan  3 15:12:16 2019"),
	std::make_tuple("MV-radio2-crit", "00:30:44:2d:36:0a",-70, 36, 5180, "Thu Jan  3 15:12:16 2019"),
	// this should throw a std::invalid_argument
//	std::make_tuple("MV-radio2-crit", "00:30:44:2d:36:0a",-70, 36, 5180, "hi dave")
};

static void print_data(const char *title)
{
	std::string fmt = fmt::format("{0:30s}    {1:10s} {2:5s} {3:5s} {4:5s}   {5:20s}\n");
	std::cout << "sorted by " << title << "\n";
	std::for_each(data.begin(), data.end(), [&fmt](survey a) {
		std::string ssid, bssid, seen;
		int rssi, channel, frequency;
		std::tie(ssid, bssid, rssi, channel, frequency, seen) = a;
		fmt::print(fmt, ssid, bssid, rssi, channel, frequency, seen);
//		std::cout << std::get<0>(a) << "\n";
	});
}

int main(void)
{
	std::cout << "test data len=" << data.size() << "\n";
	int counter { 0 };

	// convert the string date/timestamp into integer for sorting
	std::vector<time_t> timestamp;
	timestamp.resize(data.size(), 0);
	std::transform(data.begin(), data.end(), timestamp.begin(), [](survey a)->time_t {
		struct tm tm {};
		std::string& timestring = std::get<5>(a);
		char *p = strptime(timestring.c_str(), "%a %b %d %H:%M:%S %Y", &tm);
		if (!p) {
			// parse error
			std::clog << "parse error of timestring=" << timestring << "\n";
			throw std::invalid_argument(timestring);
		}

		return mktime(&tm);
	});

	std::cout << timestamp.at(0) << "\n";
	std::for_each(timestamp.begin(), timestamp.end(), [](int a) {
		std::cout << a << "\n";
	});

	std::for_each(data.begin(), data.end(), [](survey a) {
		std::cout << std::get<0>(a) << "\n";
	});

	auto sort_ssid = [&counter](survey a, survey b)->bool { 
		std::cout << counter << " " << std::get<0>(a) << " vs " << std::get<0>(b) << "\n";
		counter++;
		bool val = std::get<0>(a).compare(std::get<0>(b)) < 0;
		std::cout << "val=" << val << "\n";
		return val;
	};

	auto sort_signal_strength = [](survey a, survey b)->bool {
		// usually negative so sort descending to put higher SS at top
		return std::get<2>(a) > std::get<2>(b);
	};

	auto sort_channel = [](survey a, survey b)->bool {
		return std::get<3>(a) < std::get<3>(b);
	};

	std::sort(data.begin(), data.end(), sort_channel);
	print_data("channel");

	std::sort(data.begin(), data.end(), sort_signal_strength);
	print_data("signal strength");

	std::sort(data.begin(), data.end(), sort_ssid);
	print_data("SSID");

	return 0;
}
