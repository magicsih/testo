#pragma once

#include "spdlog/spdlog.h"

#include <boost/asio/buffer.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio/write.hpp>
#include <boost/array.hpp>

#include <iostream>

#include "adb_util.hpp"

using boost::asio::ip::tcp;

namespace testo {
	namespace adb {
		class adb_host_client
		{		
		public:
			adb_host_client()			
			{
				auto endpoints = tcp::resolver(io_context_).resolve("localhost", ADB_PORT);
				boost::system::error_code error;
				boost::asio::async_connect(socket_, endpoints,
					[&](const boost::system::error_code& result_error,
						const tcp::endpoint&)
				{
					error = result_error;
				});

				if (error)
					throw std::system_error(error);
			}

			std::string send(const std::string& command);
			std::string loadDeviceList();

		private:
			boost::asio::io_context io_context_;
			tcp::socket socket_{ io_context_ };
		};
	}
}