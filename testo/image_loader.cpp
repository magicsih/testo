#include "image_loader.hpp"

std::vector<char> testo::image::image_loader::load_image_from_remote(const std::string &protocol, const std::string &host, const std::string &path) 
{
	spdlog::debug("load_image_from_remote - protocol:{} host:{} path:{}", protocol, host, path);
    boost::asio::ip::tcp::resolver::results_type endpoints = resolver_.resolve(host, protocol);	
    boost::asio::connect(socket_, endpoints);

    boost::asio::streambuf request;
	std::ostream request_stream(&request);

    request_stream << "GET " << path << " HTTP/1.0\r\n";
    request_stream << "Host: " << host << "\r\n";
    request_stream << "Accept: */*\r\n";
	request_stream << "Connection: close\r\n\r\n";

    boost::asio::write(socket_, request);

	boost::asio::streambuf response;
	boost::asio::read_until(socket_, response, "\r\n");

	// Check that response is OK.
	std::istream response_stream(&response);
	std::string http_version;
	response_stream >> http_version;
	unsigned int status_code;
	response_stream >> status_code;
	std::string status_message;
	std::getline(response_stream, status_message);

	if (!response_stream || http_version.substr(0, 5) != "HTTP/")
	{		
        spdlog::error("Invalid response");
		throw std::exception("Invalud response");
	}

	if (status_code != 200)
	{		
        spdlog::error("Response returned with status code {}", status_code);
		throw std::exception("Response returned with status code " + status_code);
	}

	// Read the response headers, which are terminated by a blank line.
	boost::asio::read_until(socket_, response, "\r\n\r\n");

	// Process the response headers.
	std::string header;	
	spdlog::debug("HEADER BEGIN =================");
	while (std::getline(response_stream, header) && header != "\r")		
		spdlog::debug("{}", header);
	spdlog::debug("HEADER E N D =================");

	boost::system::error_code error;

	boost::asio::read(socket_, response, boost::asio::transfer_all(), error);	
	if (error != boost::asio::error::eof)
		throw boost::system::system_error(error);

	std::vector<char> target(response.size());
    boost::asio::buffer_copy(boost::asio::buffer(target), response.data());

    return target;
}

std::vector<char> testo::image::image_loader::load_image_from_remote(const std::string &full_uri) {
	boost::regex ex("(http|https)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)\\x3f?([^ #]*)#?([^ ]*)");
    boost::cmatch what;  
    if(boost::regex_match(full_uri.c_str(), what, ex)) 
    {
        std::string protocol = std::string(what[1].first, what[1].second);
        std::string host   = std::string(what[2].first, what[2].second);
        std::string port     = std::string(what[3].first, what[3].second);
        std::string path     = std::string(what[4].first, what[4].second);
        std::string query    = std::string(what[5].first, what[5].second);

		//TODO why don't you cache images within load_image_from_remote
		return load_image_from_remote(protocol, host, path + ((query.empty()) ? "" : "?" + query));
    } else{                
		throw std::invalid_argument("an argument should be http or https");
    }
}