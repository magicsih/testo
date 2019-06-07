#pragma once

#include "spdlog/spdlog.h"

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/regex.hpp>

namespace testo
{
namespace image
{
class image_loader
{
public:
    image_loader(boost::asio::io_context &io_context) : io_context_(io_context), resolver_(io_context), socket_(io_context)
    {
    }
    std::vector<char> load_image_from_remote(const std::string &protocol, const std::string &host, const std::string &path);
    std::vector<char> load_image_from_remote(const std::string &full_uri);

private:
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::io_context &io_context_;
};
} // namespace image
} // namespace testo