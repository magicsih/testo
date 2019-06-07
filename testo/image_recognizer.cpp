#include "image_recognizer.hpp"

using namespace cv;
using namespace std;

bool testo::image::image_recognizer::match_image(cv::Mat &original, cv::Mat &templ, const double threshhold) 
{
	if (original.empty() || templ.empty())
	{		
		spdlog::error("Can't read one of images");
		return false;
	}

	cv::Mat result;

	int result_cols = original.cols - templ.cols + 1;
	int result_rows = original.rows - templ.rows + 1;
	result.create(result_rows, result_cols, CV_32FC1);

	cv::matchTemplate(original, templ, result, match_method);

	double minVal, maxVal;
	Point minLoc, maxLoc;
	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

	spdlog::debug("match_image min:{} max:{} threshhold:{}", minVal, maxVal, threshhold);

	// Point matchLoc;	
	// if (match_method == TM_SQDIFF || match_method == TM_SQDIFF_NORMED)
	// {
	// 	matchLoc = minLoc;
	// }
	// else
	// {
	// 	matchLoc = maxLoc;
	// }

	return maxVal >= threshhold;
}

cv::Point testo::image::image_recognizer::find_templ_image(cv::Mat &original, cv::Mat &templ, const double threshhold) {
	if (original.empty() || templ.empty())
	{		
		spdlog::error("Can't read one of images");
		//TODO throw exception
	}

	cv::Mat result;

	int result_cols = original.cols - templ.cols + 1;
	int result_rows = original.rows - templ.rows + 1;
	result.create(result_rows, result_cols, CV_32FC1);

	cv::matchTemplate(original, templ, result, match_method);

	double minVal, maxVal;
	Point minLoc, maxLoc;
	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

	spdlog::info("match image min:{} max:{} threshhold:{}", minVal, maxVal, threshhold);

	Point matchLoc;	

	if(maxVal >= threshhold) {
		if (match_method == TM_SQDIFF || match_method == TM_SQDIFF_NORMED)
		{
			matchLoc = minLoc;
		}
		else
		{
			matchLoc = maxLoc;
		}
	}

	return matchLoc;
}

std::pair<int,int> testo::image::image_recognizer::find_image_and_get_center_position(const std::vector<char> &original, const std::vector<char> &templ, const double threshhold) {
	cv::Mat original_image_data(original, true);
	cv::Mat original_image(cv::imdecode(original_image_data, 1));
	cv::Mat template_image_data(templ, true);
	cv::Mat template_image(cv::imdecode(template_image_data, 1));

	int additional_rows = template_image.rows / 2;
	int additional_cols = template_image.cols / 2;

	Point match_loc = find_templ_image(original_image, template_image, threshhold);
	spdlog::debug("find_image_location: ({},{})", match_loc.x , match_loc.y);
	
	if(match_loc.x == 0 && match_loc.y == 0) {
		additional_rows = 0;
		additional_cols = 0;
	}

	return std::make_pair(match_loc.x + additional_cols, match_loc.y + additional_rows);
}

bool testo::image::image_recognizer::match_image(const std::string& original_img_name, const std::string& templ_img_name, const double threshhold)
{
	cv::Mat img = imread(original_img_name, IMREAD_COLOR);
	cv::Mat templ = imread(templ_img_name, IMREAD_COLOR);
	return match_image(img, templ, threshhold);
}

bool testo::image::image_recognizer::match_image(const std::vector<char> &original, const std::vector<char> &templ, const double threshhold) 
{
	cv::Mat original_image_data(original, true);
	cv::Mat original_image(cv::imdecode(original_image_data, 1));
	cv::Mat template_image_data(templ, true);
	cv::Mat template_image(cv::imdecode(template_image_data, 1));
	
	return match_image(original_image, template_image);
}

bool testo::image::image_recognizer::crop_and_match_image(const std::vector<char> &original, int x, int y, int width, int height, const std::vector<char> &templ, const double threshhold) 
{
	cv::Mat original_image_data(original, true);
	cv::Mat original_image(cv::imdecode(original_image_data, 1));
	cv::Mat cropped_original_image = original_image(cv::Rect(x, y, width, height));

	cv::Mat template_image_data(templ, true);
	cv::Mat template_image(cv::imdecode(template_image_data, 1));

	return match_image(cropped_original_image, template_image);
}