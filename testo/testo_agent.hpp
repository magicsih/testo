#pragma once

#include <thread>
#include <fstream>
#include <chrono>
#include <utility>

#include <boost/asio/io_service.hpp>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>

#include "image_loader.hpp"
#include "image_recognizer.hpp"

#include "adb_host_client.hpp"
#include "adb_local_keep_alive_client.hpp"
#include "adb_local_client.hpp"

#include "v8.h"
#include "libplatform/libplatform.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace testo
{
namespace agent
{
class testo_agent
{
public:
    testo_agent()
    {
    }
    void operator()(const std::string &device_id, const std::string &base_script, const std::string &adb_exe_path,
                    const std::string &user_script, const std::string &path_to_apk, const std::string &package_name);

private:
    testo::adb::adb_local_client *local_client_ = nullptr;
    testo::adb::adb_local_keep_alive_client *keepalive_local_client_ = nullptr;
    testo::image::image_loader *image_loader_ = nullptr;

    int screen_width_ = 0;
    int screen_height_ = 0;    

    // Binding Javascript function to C++
    static void script_print(const v8::FunctionCallbackInfo<v8::Value> &args);
    static void script_tap(const v8::FunctionCallbackInfo<v8::Value> &args);
    static void script_swipe(const v8::FunctionCallbackInfo<v8::Value> &args);
    static void script_match_image(const v8::FunctionCallbackInfo<v8::Value> &args);
    static void script_wait_until_condition(const v8::FunctionCallbackInfo<v8::Value> &args);
    static void script_tap_image(const v8::FunctionCallbackInfo<v8::Value> &args);

    static const char *ToCString(const v8::String::Utf8Value &value);
};
} // namespace agent
} // namespace testo