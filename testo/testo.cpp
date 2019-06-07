#include "testo.hpp"

using namespace std;

inline void initialize_spd_log(spdlog::level::level_enum log_level)
{
	spdlog::set_level(log_level);		
	spdlog::set_pattern("[%H:%M:%S %z] [thread %^%t%$] [%^%l%$] %v");
}

static std::string read_file(const wchar_t *file_name)
{
	std::ifstream ifs(file_name);
	std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
	return content;
}

int main(int argc, char **argv)
{
	namespace po = boost::program_options;

	// Define program options
	po::options_description desc("Allowed options");
	desc.add_options()
			("help,h", "this help message")
			("script,s", po::value<string>()->required(), "path to your script file")
			("device,d", po::value<string>()->required(), "an android device id (to get an id, use adb.exe command like this <adb.exe devices>")			
			("adb,a", po::value<string>(),  "<optional> path to adb.exe (including directory and executable name. eg. /usr/local/bin/adb, C:/Android/nox_adb.exe). if it's not specified, this program will try to run adb.exe in current directory passion. (it can be pointing to path for customized adb like nox_adb")
			("apk,k", po::value<string>(),"<optional> path to apk file. if it's specified, this program will try to install and run it")
			("package,p", po::value<string>(), "<optional> package name to start before script processing")
			("debug", "<optional> print logs in debug verbosity")
			//("devices", po::value<string>()->multitoken(), "(NO SUPPORT YET) comma separated android device ids to run scripts simultaneously.")
	;
	
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);

	if (vm.count("help"))
	{
		std::cout << desc << std::endl;
		return 1;
	}

	if (!vm.count("script") || !vm.count("device"))
	{
		std::cout << "This app loads script sources from a javascript file. the scripts may contain how to control an android device. the script engine is powered by V8." <<std::endl;
		std::cout << desc << std::endl;
		std::cout << "Note : This program require adb.exe file to communicate with android devices. you can download it from https://developer.android.com/studio/command-line/adb" << std::endl;
		return 1;
	}

	if(vm.count("debug")) 
	{
		initialize_spd_log(spdlog::level::debug);
	} else {
		initialize_spd_log(spdlog::level::info);
	}	

	auto script_file_path = boost::filesystem::absolute(boost::filesystem::path(vm["script"].as<string>()), boost::filesystem::current_path());
	auto inhouse_script_file_path = boost::filesystem::absolute(boost::filesystem::path("testo.js"), boost::filesystem::current_path());
		
	spdlog::info("testo is initializing...");
	spdlog::info("your script file: {}", script_file_path);
	spdlog::info("embedded script file : {}", inhouse_script_file_path);

	//ADB EXECUTABLE PATH CHECK
	std::string adb_exe_path;
	if(vm.count("adb")) {		
		if(!boost::filesystem::exists(vm["adb"].as<string>())) {
			std::cerr << "Failed to find adb from " << vm["adb"].as<string>() << std::endl;
			return 1;
		}
		adb_exe_path = boost::filesystem::absolute(boost::filesystem::path(vm["adb"].as<string>()), boost::filesystem::current_path()).string();
	} else {
		adb_exe_path = "adb.exe";
	}
	spdlog::info("adb executable file path :{}", adb_exe_path);

	//APK FILE CHECK
	std::string apk_file_path;
	if(vm.count("apk")) {
		if(!boost::filesystem::exists(vm["apk"].as<string>())) {
			std::cerr << "Failed to find apk from " << vm["apk"].as<string>() << std::endl;
			return 1;
		}
		apk_file_path = boost::filesystem::absolute(boost::filesystem::path(vm["apk"].as<string>()), boost::filesystem::current_path()).string();
		spdlog::info("apk file path :{}", apk_file_path);
	}

	std::string package_name;
	if(vm.count("package")) {
		package_name = vm["package"].as<string>();
		spdlog::info("package name : {}", package_name);
	}
	
	//TARGET DEVICES
	spdlog::info("target device id: {}", vm["device"].as<string>());	
		
	std::vector<std::string> devices;
	boost::split(devices, vm["device"].as<string>(), boost::is_any_of(","), boost::token_compress_on);

	spdlog::info("trying to execute adb daemon process...");

	int result = boost::process::system(adb_exe_path + " devices");

	if (result != 0)
	{
		spdlog::error("failed to execute adb daemon process");
		return 1;
	}

	spdlog::info("adb daemon process is now running...");

	testo::adb::adb_host_client host_client;
	host_client.loadDeviceList();
	

	//TODO : ADD DEVICE ID VALIDATION LOGIC HERE (USE adb_host_client CLASS)

	std::string base_script = read_file(inhouse_script_file_path.c_str());
	std::string user_script = read_file(script_file_path.c_str());

	vector<thread> agents;

	for (auto const &device : devices)
	{			
		agents.push_back(thread(testo::agent::testo_agent(), device, base_script, user_script, adb_exe_path, apk_file_path, package_name));
	}

	for (auto &agent : agents)
	{
		agent.join();
	}

	spdlog::info("testo is terminating...");
	return 0;
}