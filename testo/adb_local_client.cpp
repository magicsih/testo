#include "adb_local_client.hpp"

// void testo::adb::adb_local_client::tap(unsigned short x, unsigned short y) {
// 	spdlog::debug("adb_local_client tap {} {}", x, y);
// 	send_message_to_device("shell:input tap " + std::to_string(x) + " " + std::to_string(y));
//     assert_adb_header_okay();
//     read_until_close();        
// }

std::pair<int,int> testo::adb::adb_local_client::get_wm_size() {
    spdlog::debug("adb_local_client get_wm_size");

    send_message_to_device("shell::wm size");
    assert_adb_header_okay();

    std::string wm_size_result;
    boost::system::error_code error;    
    boost::asio::read_until(socket_, boost::asio::dynamic_buffer(wm_size_result), "\n", error);

    std::cout << wm_size_result << std::endl;

    boost::asio::read_until(socket_, boost::asio::dynamic_buffer(wm_size_result), "\n", error);

     std::cout << wm_size_result << std::endl;

    return std::make_pair(1,1);
}

std::vector<char> testo::adb::adb_local_client::screenshot() {
    spdlog::debug("adb_local_client screenshot");

    send_message_to_device("shell:screencap -p");
    assert_adb_header_okay();

    boost::system::error_code error;
    boost::asio::streambuf response;
    
    while (boost::asio::read(socket_, response, boost::asio::transfer_at_least(1), error));
    std::vector<char> target(response.size());
	boost::asio::buffer_copy(boost::asio::buffer(target), response.data());
    spdlog::debug("screenshot read {} bytes", response.size());
    return target;
}

void testo::adb::adb_local_client::read_until_close() {
    boost::system::error_code error;
    boost::asio::streambuf response;
    while (boost::asio::read(socket_, response,boost::asio::transfer_at_least(1), error)) {
      std::cout << &response;
    }
}

void testo::adb::adb_local_client::send_message_to_device(const std::string& message)
{
    auto endpoints = resolver_.resolve("localhost", ADB_PORT);

    boost::system::error_code error;
    boost::asio::connect(socket_, endpoints.begin(), endpoints.end(), error);

    if (error)
    {
        spdlog::error("adb_local_client connect error:{}", error.message());
        throw std::system_error(error);
    }

    auto host_transport_device = "host:transport:" + device_id_;
    write(host_transport_device);   

    assert_adb_header_okay();
    
    write(message);    
}

size_t testo::adb::adb_local_client::write(const std::string &message)
{
    auto s = testo::adb::adb_util::makeAdbPayload(message);
    spdlog::trace("adb_local_client write... {}", s);

    boost::system::error_code error;
    size_t written = boost::asio::write(socket_, boost::asio::buffer(s), error);

    if (error)
    {
        spdlog::error("adb_local_client write error: {} payload:{}", error.message(), s);
        socket_.close();
        throw std::exception("adb_local_client write failed");
    }

    return written;
}

void testo::adb::adb_local_client::assert_adb_header_okay()
{
    static std::array<char, 4> header_buffer;

    boost::system::error_code error;
    size_t read = boost::asio::read(socket_, boost::asio::buffer(header_buffer, 4), error);

    if (error)
    {
        spdlog::error("read_adb_header_and_check_okay socket error:{}", error.message());
        assert(false);
    }

    std::string okay = std::string(header_buffer.data(), read);

    spdlog::debug("read_adb_header_and_check_okay result:{}", okay);    

    assert(okay.compare(ADB_MESSAGE_OKAY) == 0);
}