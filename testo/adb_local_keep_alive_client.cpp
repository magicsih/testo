#include "adb_local_keep_alive_client.hpp"

void testo::adb::adb_local_keep_alive_client::start()
{
	spdlog::info("adb_local_keep_alive_client connecting to {}", device_id_);
	boost::system::error_code error;
	boost::asio::connect(socket_, resolver_.resolve("localhost", ADB_PORT), error);	

	if (!socket_.is_open())
	{
		spdlog::error("Connect timed out");
		return;
	}

	if (error)
	{
		spdlog::error("Connect error: {}", error.message());
		socket_.close();
		return;
	}

	spdlog::info("adb_local_keep_alive_client connected successfully.");

	make_adb_message_and_write("host:transport:" + device_id_);

	assert(read_adb_header_and_check_okay());

	make_adb_message_and_write("shell:");

	assert(read_adb_header_and_check_okay());
	consume_shell_prefix();
}

void testo::adb::adb_local_keep_alive_client::tap(int x, int y)
{
	std::string msg = "input tap " + std::to_string(x) + " " + std::to_string(y) + "\n";
	write(msg);
	consume_self_command_message(msg);
	consume_shell_prefix();	
}

void testo::adb::adb_local_keep_alive_client::swipe(int x1, int y1, int x2, int y2)
{	
	std::string msg = "input swipe " + std::to_string(x1) + " " + std::to_string(y1) + " " +  std::to_string(x2) + " " + std::to_string(y2) + "\n";
	write(msg);
	consume_self_command_message(msg);
	consume_shell_prefix();
}


std::pair<int,int> testo::adb::adb_local_keep_alive_client::get_wm_size() {
	std::string msg = "wm size\n";
	write(msg);
	consume_self_command_message(msg);

	boost::asio::streambuf response;
	boost::system::error_code error;
	size_t read = boost::asio::read_until(socket_, response, ":/", error);
	handle_socket_error(error);	

	read = boost::asio::read_until(socket_, response, " ", error);
	handle_socket_error(error);

	read = boost::asio::read_until(socket_, response, " ", error);
	handle_socket_error(error);

	std::string s( (std::istreambuf_iterator<char>(&response)), std::istreambuf_iterator<char>() );

	int width = 0, height = 0;

	std::stringstream ss(s);
	std::string to;

	while(std::getline(ss, to, '\n')) {		
		spdlog::trace("get_wm_size {}", to);
		if(width == 0 && height == 0) {
			std::string::size_type loc = to.find("Physical size: ", 0);
			if(loc != std::string::npos) {				
				std::istringstream f(to.substr(15 , to.size() - 15));
				std::string value;
				int pos = 0;
				while(std::getline(f, value,  'x')) {
					if(pos++ == 0){
						width = std::stoi(value);
					} else {
						height = std::stoi(value);
					}
				}
			}
		} else {
			std::string::size_type loc = to.find("Override size: ", 0);
			if(loc != std::string::npos) {
				std::istringstream f(to.substr(15 , to.size() - 15));
				std::string value;
				int pos = 0;
				while(std::getline(f, value,  'x')) {
					if(pos++ == 0){
						width = std::stoi(value);
					} else {
						height = std::stoi(value);
					}
				}
			}
		}
	}

	spdlog::debug("Device screen size {},{}", width, height);	

    return std::make_pair(width,height);
}

size_t testo::adb::adb_local_keep_alive_client::write(const std::string &message)
{
	spdlog::debug("adb_local_keep_alive_client write to adb shell : {}", message);
	
	boost::system::error_code error;
	size_t written = boost::asio::write(socket_, boost::asio::buffer(message), error);	
	if (error)
	{
		spdlog::error("adb_local_keep_alive_client write error: {}", error.message());
		socket_.close();
		throw std::exception("adb_local_keep_alive_client write failed");
	}

	spdlog::debug("adb_local_keep_alive_client written {} bytes", written);	
	return written;
}

size_t testo::adb::adb_local_keep_alive_client::make_adb_message_and_write(const std::string &message)
{
	auto s = testo::adb::adb_util::makeAdbPayload(message);
	return this->write(s);
}

void testo::adb::adb_local_keep_alive_client::stop()
{
	boost::system::error_code ignored_error;
	socket_.close(ignored_error);
}

bool testo::adb::adb_local_keep_alive_client::read_adb_header_and_check_okay()
{
	static std::array<char, 4> header_buffer;

	boost::system::error_code error;
	size_t read = boost::asio::read(socket_, boost::asio::buffer(header_buffer, 4), error);

	if (error)
	{
		spdlog::error("read_adb_header_and_check_okay socket error:{}", error.message());
		return false;
	}

	std::string okay = std::string(header_buffer.data(), read);

	spdlog::debug("read_adb_header_and_check_okay result:{}", okay);	
	if (okay.compare(ADB_MESSAGE_OKAY))
	{
		return false;
	}
	else
	{
		return true;
	}
}

void testo::adb::adb_local_keep_alive_client::consume_shell_prefix()
{
	boost::asio::streambuf response;
	boost::system::error_code error;
	size_t read = boost::asio::read_until(socket_, response, ":/", error);
	handle_socket_error(error);	

	spdlog::debug("read_adb_shell_header {} bytes", read);	

	read = boost::asio::read_until(socket_, response, " ", error);
	handle_socket_error(error);
	spdlog::debug("read_adb_shell_header {} bytes", read);

	read = boost::asio::read_until(socket_, response, " ", error);
	handle_socket_error(error);
	spdlog::debug("read_adb_shell_header {} bytes", read);
}

void testo::adb::adb_local_keep_alive_client::consume_self_command_message(std::string &command) {
	static std::array<char, 128> buf; // this buffer is just useless. so far 128 should be enough.
	boost::system::error_code ec;
	boost::asio::read(socket_, boost::asio::buffer(buf), boost::asio::transfer_exactly(command.size() + 2), ec);
	if (ec)
	{
		spdlog::error("adb_local_keep_alive_client consume_self_command_message error: {}", ec.message());
		socket_.close();
		throw std::exception("adb_local_keep_alive_client consume_self_command_message failed");
	}
}

void testo::adb::adb_local_keep_alive_client::handle_socket_error(const boost::system::error_code &error)
{
	if (error)
	{
		spdlog::error("socket_error:{}", error.message());
		socket_.close();
		throw std::exception("adb_local_keep_alive socket error");
	}
}