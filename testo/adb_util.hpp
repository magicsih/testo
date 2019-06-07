#pragma once

#include <iostream>
#include <sstream>

#include <boost/format.hpp>

#define ADB_PORT "5037"
#define ADB_MESSAGE_OKAY "OKAY"

namespace testo {
	namespace adb {
		class adb_util {
		public:			
			static std::string makeAdbPayload(const std::string& originalPayload);
		};
	}
}