#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>

//using namespace cv;

int main()
{
	std::string ssid = u8"(╯°□°）╯︵ ┻━┻";
	std::cout << ssid << "\n";

	std::string ssid2 = "(╯°□°）╯︵ ┻━┻";
	std::cout << ssid2 << "\n";

	std::string filename  = "zen.jpg";
	cv::Mat img = cv::imread(filename);
}

