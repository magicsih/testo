#include "adb_host_client.hpp"

std::string testo::adb::adb_host_client::send(const std::string& message) {

	spdlog::debug("Sending...");

	auto s = testo::adb::adb_util::makeAdbPayload("host:" + message);
	spdlog::info("Send {}", s);

	boost::system::error_code error;
	boost::asio::write(socket_, boost::asio::buffer(s.c_str(),s.length()), error);

	if (error)
		throw std::system_error(error);


	spdlog::debug("Sent successfully");

	std::array<char, 4> okay_receive_buffer;

	size_t n = boost::asio::read(socket_, boost::asio::buffer(okay_receive_buffer, 4), error);

	// Determine whether the read completed successfully.
	if (error)
		throw std::system_error(error);

	spdlog::debug("Read successfully {} bytes", n);
	
	std::string okay(okay_receive_buffer.data(), n);
	spdlog::debug("Result Header:{}", okay);

	if (okay.compare(ADB_MESSAGE_OKAY)) {
		throw std::exception("ADB RETURNS FAIL");
	}

	std::string receive_buffer;

	boost::asio::read(socket_, boost::asio::dynamic_buffer(receive_buffer), error);
	
	
	spdlog::debug("Read successfully {} bytes", n);

	std::string result(receive_buffer.substr(0, n - 1));
	
	spdlog::info("Receive:{}", result);

	return result;
}

std::string testo::adb::adb_host_client::loadDeviceList()
{
	return this->send("devices-l");
}