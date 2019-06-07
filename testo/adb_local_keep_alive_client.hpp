#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

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
#include <utility>
#include <iterator>

#include "adb_util.hpp"

using boost::asio::steady_timer;
using boost::asio::ip::tcp;
using std::placeholders::_1;
using std::placeholders::_2;

namespace testo
{
namespace adb
{
class adb_local_keep_alive_client
{
public:
	adb_local_keep_alive_client(boost::asio::io_context &io_context, const std::string &device_id)
		: io_context_(io_context), socket_(io_context), resolver_(io_context), device_id_(device_id)
	{
	}

	void start();

	void stop();

	void tap(int x, int y);
	void swipe(int x1, int y1, int x2, int y2);

	/// Android Windows Manager Size
	///	ADB OUTPUT EXAMPLE
	/// Physical size: 1440x2560
	/// Override size: 1080x1920
	/// This function gets Override size if exists, otherwise return physical size.
	/// pair _1 : width
	/// pair _2 : height
	std::pair<int,int> get_wm_size();

private:
	bool read_adb_header_and_check_okay();	
	void consume_shell_prefix();
	void consume_self_command_message(std::string &command);
	size_t make_adb_message_and_write(const std::string &message);
	size_t write(const std::string &message);
	void handle_socket_error(const boost::system::error_code&);

private:
	std::string device_id_;
	tcp::socket socket_;
	tcp::resolver resolver_;
	boost::asio::io_context &io_context_;
};
} // namespace adb
} // namespace testo