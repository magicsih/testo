#include "testo_agent.hpp"

/*
* this function is an entry point.
* to add new javascript function 
* 1. add function pointer to v8::Local<v8::ObjectTemplate> global 
* 2. implement the function pointer as static
*/
void testo::agent::testo_agent::operator()(const std::string &device_id, const std::string &base_script, const std::string &user_script, const std::string &adb_exe_path, const std::string &path_to_apk, const std::string &package_name)
{
    spdlog::info("testo agent {} init...", device_id);
    spdlog::debug("base_script:\n{}", base_script);
    spdlog::debug("user_script:\n{}", user_script);

    //Before script processing

    //1. Install apk
    if(!path_to_apk.empty()) {
        spdlog::info("apk installing... {}", path_to_apk);
        int result = boost::process::system(adb_exe_path, "-s", device_id, "install", "-r", path_to_apk);
        if(result!=0) {
            spdlog::error("failed to install apk!:{}", path_to_apk);
            return;
        }
    }

    //2. Launch the package
    if(!package_name.empty()) {
        spdlog::info("launching {} package", package_name);
        int result = boost::process::system(adb_exe_path, "-s", device_id, "shell", "monkey", "-p", package_name, "-c", "android.intent.category.LAUNCHER", "1");
        if(result!=0) {
            spdlog::error("failed to launch package:{}", package_name);
            return;
        }
    }

    // 3. Components initialize
    boost::asio::io_context io_context;
    local_client_ = new testo::adb::adb_local_client(io_context, device_id);

    keepalive_local_client_ = new testo::adb::adb_local_keep_alive_client(io_context, device_id);
    keepalive_local_client_->start();
    std::pair<int,int> screen_size = keepalive_local_client_->get_wm_size();

    screen_width_ = screen_size.first;
    screen_height_ = screen_size.second;

    spdlog::info("DEVICE SCREEN SIZE ({},{})", screen_width_, screen_height_);	

    image_loader_ = new testo::image::image_loader(io_context);

    // 4. V8 Engine initialize
    v8::V8::InitializeICUDefaultLocation(device_id.c_str());
    v8::V8::InitializeExternalStartupData(device_id.c_str());

    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();

    v8::Isolate *isolate = v8::Isolate::New(create_params);
    {
        v8::Locker locker(isolate);
        isolate->Enter();
        v8::Isolate::Scope isolate_scope(isolate);

        // Create a stack-allocated handle scope.
        v8::HandleScope handle_scope(isolate);

        // Create a new context.
        v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
        v8::Local<v8::External> self = v8::External::New(isolate, (void *)this);

        global->Set(
            v8::String::NewFromUtf8(isolate, "print", v8::NewStringType::kNormal).ToLocalChecked(),
            v8::FunctionTemplate::New(isolate, testo::agent::testo_agent::script_print, self));

        global->Set(
            v8::String::NewFromUtf8(isolate, "tap", v8::NewStringType::kNormal).ToLocalChecked(),
            v8::FunctionTemplate::New(isolate, testo::agent::testo_agent::script_tap, self));

        global->Set(
            v8::String::NewFromUtf8(isolate, "swipe", v8::NewStringType::kNormal).ToLocalChecked(),
            v8::FunctionTemplate::New(isolate, testo::agent::testo_agent::script_swipe, self));

        global->Set(
            v8::String::NewFromUtf8(isolate, "matchImage", v8::NewStringType::kNormal).ToLocalChecked(),
            v8::FunctionTemplate::New(isolate, testo::agent::testo_agent::script_match_image, self));

        global->Set(
            v8::String::NewFromUtf8(isolate, "waitUntilCondition", v8::NewStringType::kNormal).ToLocalChecked(),
            v8::FunctionTemplate::New(isolate, testo::agent::testo_agent::script_wait_until_condition, self));

        global->Set(
            v8::String::NewFromUtf8(isolate, "tapImage", v8::NewStringType::kNormal).ToLocalChecked(),
            v8::FunctionTemplate::New(isolate, testo::agent::testo_agent::script_tap_image, self));

        v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global);

        // Enter the context for compiling and running the hello world script.
        v8::Context::Scope context_scope(context);
        {
            v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, (base_script + user_script).c_str(), v8::NewStringType::kNormal).ToLocalChecked();
            v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();
            spdlog::info("Javascript compile finished.");

            // Run the script to get the result.
            v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
        }
        isolate->Exit();
    }

    keepalive_local_client_->stop();
    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();

    delete create_params.array_buffer_allocator;
    delete local_client_;
    delete keepalive_local_client_;
}

void testo::agent::testo_agent::script_print(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    bool first = true;
    for (int i = 0; i < args.Length(); i++)
    {
        v8::HandleScope handle_scope(args.GetIsolate());
        if (first)
        {
            first = false;
        }
        v8::String::Utf8Value str(args.GetIsolate(), args[i]);
        const char *cstr = ToCString(str);
        spdlog::info("[js - print] {}", cstr);
    }
}

void testo::agent::testo_agent::script_tap(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    v8::Local<v8::External> ext = args.Data().As<v8::External>();
    testo::agent::testo_agent *self = static_cast<testo::agent::testo_agent *>(ext->Value());
    int x = args[0]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromMaybe(0);
    int y = args[1]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromMaybe(0);
    spdlog::info("[js - tap] {},{}", x, y);
    self->keepalive_local_client_->tap(x, y);
}

void testo::agent::testo_agent::script_swipe(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    v8::Local<v8::External> ext = args.Data().As<v8::External>();
    testo::agent::testo_agent *self = static_cast<testo::agent::testo_agent *>(ext->Value());
    int x1 = args[0]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromMaybe(0);
    int y1 = args[1]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromMaybe(0);

    int x2 = args[2]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromMaybe(0);
    int y2 = args[3]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromMaybe(0);
    spdlog::info("[js - swipe] {},{}->{},{}", x1, y1, x2, y2);
    self->keepalive_local_client_->swipe(x1, y1, x2, y2);
}

void testo::agent::testo_agent::script_match_image(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    v8::Isolate *isolate = args.GetIsolate();

    if(args.Length() < 1 || !args[0]->IsObject()) {
        isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Error: [MatchImage] Image object expected")));
        return;
    }

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> obj = args[0]->ToObject(context).ToLocalChecked();
    v8::Local<v8::Array> props = obj->GetOwnPropertyNames(context).ToLocalChecked();

    v8::Local<v8::Value> local_key_name = props->Get(0);
    v8::Local<v8::Value> local_key_path = props->Get(1);

    v8::Local<v8::Value> local_value_name = obj->Get(context, local_key_name).ToLocalChecked();
    v8::Local<v8::Value> local_value_path = obj->Get(context, local_key_path).ToLocalChecked();

    std::string templ_name = *v8::String::Utf8Value(isolate, local_value_name);
    std::string templ_path = *v8::String::Utf8Value(isolate, local_value_path);

    v8::Local<v8::External> ext = args.Data().As<v8::External>();
    testo::agent::testo_agent *self = static_cast<testo::agent::testo_agent *>(ext->Value());

    auto data = self->local_client_->screenshot();

    std::vector<char> templ_img_data = self->image_loader_->load_image_from_remote(templ_path);

    testo::image::image_recognizer image_recognizer;
    bool match_image_result = image_recognizer.match_image(data, templ_img_data);
    spdlog::info("[js - matchImage] template image name: {} path : {} match_result:{}", templ_name, templ_path, match_image_result);

    args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(), match_image_result));
}

void testo::agent::testo_agent::script_wait_until_condition(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    v8::Isolate *isolate = args.GetIsolate();

    if(args.Length() < 2 || !args[0]->IsFunction() || !args[1]->IsNumber()) {
        isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Error: [WaitUntilCondition] Check your parameter")));
        return;
    }

    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Handle<v8::Function> func = v8::Handle<v8::Function>::Cast(args[0]);

    int32_t timeout = args[1]->Int32Value(context).ToChecked();
    spdlog::info("[js - waitUntilCondition] timeout:{}", timeout);  

    bool result = false;
    
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    do {
         v8::Local<v8::Value> function_call_result = func->Call(context, v8::Number::New(isolate, 0), 0, NULL).ToLocalChecked();    
    
        if(!function_call_result->IsBoolean()) {
            isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Error: [WaitUntilCondition] Condition function should return boolean (true/false)")));
            return;
        }
        result = function_call_result->BooleanValue(isolate);
        spdlog::info("[js - waitUntilCondition] result:{}", result);
    } while(!result && (std::chrono::steady_clock::now() - start < std::chrono::seconds(timeout)));
}

void testo::agent::testo_agent::script_tap_image(const v8::FunctionCallbackInfo<v8::Value> &args)
{
    v8::Isolate *isolate = args.GetIsolate();

    if(args.Length() < 1 || !args[0]->IsObject()) {
        isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, "Error: [TapImage] Image object expected")));
        return;
    }

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> obj = args[0]->ToObject(context).ToLocalChecked();
    v8::Local<v8::Array> props = obj->GetOwnPropertyNames(context).ToLocalChecked();

    v8::Local<v8::Value> local_key_name = props->Get(0);
    v8::Local<v8::Value> local_key_path = props->Get(1);

    v8::Local<v8::Value> local_value_name = obj->Get(context, local_key_name).ToLocalChecked();
    v8::Local<v8::Value> local_value_path = obj->Get(context, local_key_path).ToLocalChecked();

    std::string templ_name = *v8::String::Utf8Value(isolate, local_value_name);
    std::string templ_path = *v8::String::Utf8Value(isolate, local_value_path);

    v8::Local<v8::External> ext = args.Data().As<v8::External>();
    testo::agent::testo_agent *self = static_cast<testo::agent::testo_agent *>(ext->Value());

    auto data = self->local_client_->screenshot();

    std::vector<char> templ_img_data = self->image_loader_->load_image_from_remote(templ_path);

    testo::image::image_recognizer image_recognizer;
    std::pair<int,int> match_loc = image_recognizer.find_image_and_get_center_position(data, templ_img_data);

    spdlog::info("[js - tapImage] template image name: {} path : {} tap:({},{})", templ_name, templ_path, match_loc.first, match_loc.second);

    if(match_loc.first > 0 || match_loc.second > 0){
        self->keepalive_local_client_->tap(match_loc.first, match_loc.second);
    } else {
        spdlog::warn("[js - tapImage] can't find image to tap");
    }
}

const char *testo::agent::testo_agent::ToCString(const v8::String::Utf8Value &value)
{
    return *value ? *value : "<string conversion failed>";
}