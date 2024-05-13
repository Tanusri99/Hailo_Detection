#pragma once
#include <iostream>
#include <fstream>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace hailo_lpr {

	class AppLogger
	{
	public:
		static void clear();
		static void print(std::string output);
		static void debug(std::string output, bool turnon = true);

		// use to save and load settings, not saved if not in NX manifest
		static bool getAppDataPath(std::string &dataPath);
		static bool getDebugLog(std::string &logPath);
		static bool getPrintLog(std::string &logPath);


		static bool saveSettings(
			std::string numObjs, 
			std::string expiryDate, 
			std::string numChannels,
			std::string pluginType,
			std::string machineID,
			std::string &message);

		static bool loadSettings(
			std::string &numObjs, 
			std::string &expiryDate, 
			std::string &numChannels,
			std::string &pluginType,
			std::string &machineID,
			std::string &message);
	};

} // namespace hailo_lpr
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
