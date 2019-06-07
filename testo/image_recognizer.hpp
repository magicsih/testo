#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

#include <vector>
#include <utility> 

#include "spdlog/spdlog.h"


namespace testo
{
namespace image
{
class image_recognizer
{
	const int match_method = 5;

public:
	bool crop_and_match_image(const std::vector<char> &original, int x, int y, int width, int height, const std::vector<char> &templ, const double threshhold = 0.8);
	bool match_image(const std::vector<char> &original, const std::vector<char> &templ, const double threshhold = 0.8);
	bool match_image(const std::string &original_img_name, const std::string &templ_img_name, const double threshhold = 0.8);
	std::pair<int, int> find_image_and_get_center_position(const std::vector<char> &original, const std::vector<char> &templ, const double threshhold = 0.8);
private:
	bool match_image(cv::Mat &original, cv::Mat &templ, const double threshhold = 0.8);
	cv::Point find_templ_image(cv::Mat &original, cv::Mat &templ, const double threshhold = 0.8);
};
} // namespace image
} // namespace testo