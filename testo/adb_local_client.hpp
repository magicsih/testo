#pragma once

#include "spdlog/spdlog.h"

#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/streambuf.hpp>

#include <functional>
#include <vector>
#include <iostream>

#include "adb_util.hpp"

using boost::asio::ip::tcp;

namespace testo
{
namespace adb
{
class adb_local_client
{
public:
    adb_local_client(boost::asio::io_context &io_context, const std::string &device_id) : io_context_(io_context), socket_(io_context), resolver_(io_context), device_id_(device_id)
    {
    }
    std::vector<char> screenshot();
    /// Android Windows Manager Size
	///	ADB OUTPUT EXAMPLE
	/// Physical size: 1440x2560
	/// Override size: 1080x1920
	/// This function gets Override size if exists, otherwise return physical size.
	/// pair _1 : width
	/// pair _2 : height
	std::pair<int,int> get_wm_size();
private:
    void send_message_to_device(const std::string &message);
    void assert_adb_header_okay();
    size_t write(const std::string &message);
    void read_until_close();
private:
    tcp::socket socket_;
    tcp::resolver resolver_;
    const std::string device_id_;
    boost::asio::io_context &io_context_;
};
} // namespace adb
} // namespace testo