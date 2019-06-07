#include "adb_util.hpp"

std::string testo::adb::adb_util::makeAdbPayload(const std::string & originalPayload)
{	
	std::stringstream ss;	
	//std::cout << originalPayload.length() << std::endl;
	ss << boost::format("%04x") % originalPayload.length() << originalPayload;
	return ss.str();
}
