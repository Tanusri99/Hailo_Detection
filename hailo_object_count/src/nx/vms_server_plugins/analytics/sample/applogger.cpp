#include "applogger.h"
#include <nx/kit/debug.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "base64.h"
#include <cstdlib>

namespace nx {
namespace vms_server_plugins {
namespace analytics {
namespace hailo_object_count {

	//static bool DEBUG_ENABLED=true;


	bool AppLogger::getAppDataPath(std::string &dataPath)
	{
			
#ifdef _WIN32
		dataPath= "C:\\aol\\hailo_objects.dat";
#else
		dataPath="/usr/local/share/aol/hailo_objects.dat";

#endif
		
		return true;
		
	}

	bool AppLogger::getDebugLog(std::string &logPath)
	{
		
#ifdef _WIN32
		logPath = "C:\\aol\\hailo_objects_plugin.log";
#else
		logPath = "/usr/local/share/aol/hailo_objects_plugin.log";

#endif
		return true;
	}
	
	bool AppLogger::getPrintLog(std::string &logPath)
	{

#ifdef _WIN32
		logPath = "C:\\aol\\hailo_objects_plugin.log";
#else
		logPath = "/usr/local/share/aol/hailo_objects_plugin.log";

#endif
		return true;
	}

	bool AppLogger::saveSettings(
		std::string numObjs, 
		std::string expiryDate, 
		std::string numChannels,
		std::string pluginType,
		std::string machineID,
		std::string &message)
	{
		AppLogger::debug("Save Settings");
		try
		{
			std::string appDataFile;
			if(!AppLogger::getAppDataPath(appDataFile))
			{
				AppLogger::debug("Cannot open appdata file: " + appDataFile);
				message="Cannot open appdata file";
			}

			message="success";
			std::string settings = numObjs + "|" + numChannels  + "|" +  expiryDate  + "|" + machineID + "|" + pluginType;
			AppLogger::debug(settings);
			std::string b64 = base64_encode((unsigned char const*)settings.c_str(), settings.size());

			std::ofstream outfile;

			try
			{
				outfile.open(appDataFile);
				outfile << b64; // no new line chars
				outfile.close();

				AppLogger::debug("Saved to " + appDataFile);
			}
			catch (const std::exception&)
			{
				try // backup location in case path does not exist
				{
#ifdef _WIN32

					appDataFile = "C:/temp/hailoobjects.dat";
#else
					appDataFile = "/tmp/hailoobjects.dat";

#endif

					outfile.open(appDataFile);
					outfile << b64; // no new line chars
					outfile.close();

					AppLogger::debug("Saved to " + appDataFile);
				}
				catch (const std::exception&)
				{
					AppLogger::debug("Saved app data failed: " + appDataFile);
				}
			}

		}
		catch(const std::exception& e)
		{
			message = e.what();
			AppLogger::debug("Save error: " + message);
			return false;
		}
		
		return true;
	}
		
	bool AppLogger::loadSettings(
		std::string &numObjs, 
		std::string &expiryDate, 
		std::string &numChannels,
		std::string &pluginType,
		std::string &machineID,
		std::string &message)
	{
		AppLogger::debug("Load Settings");
		try
		{
			std::string appDataFile;
			if(!AppLogger::getAppDataPath(appDataFile))
			{
				AppLogger::debug("Cannot open appdata file: " + appDataFile);
				message="Cannot open appdata file";
			}

			message="success";
			std::string contents;
			std::ifstream infile;

			try
			{
				infile.open(appDataFile);
				if (infile.is_open())
				{

					std::string line;
					while (getline(infile, line))
					{
						contents += line;
					}
					infile.close();
				}
				else
				{
					message = "No file found";
					AppLogger::debug("Load app data failed. No file found: " + appDataFile);
				}
			}
			catch (const std::exception& e)
			{
				std::string err1 = e.what();
				AppLogger::debug(err1);

				try // backup location in case path does not exist
				{
#ifdef _WIN32

					appDataFile = "C:/temp/hailoobjects.dat";
#else
					appDataFile = "/tmp/hailoobjects.dat";

#endif

					infile.open(appDataFile);
					if (infile.is_open())
					{

						std::string line;
						while (getline(infile, line))
						{
							contents += line;
						}
						infile.close();
					}
					else
					{
						message = "No file found";
						AppLogger::debug("Load app data failed. No file found");
						throw std::runtime_error("No data file, hcek permissions");
					}
				}
				catch (const std::exception& e2)
				{
					std::string err2 = e2.what();
					AppLogger::debug(err2);
				}
			}
			

			std::string decrypted = base64_decode(contents);
			AppLogger::debug("Load settings: " + decrypted);

			std::string s = decrypted;
			std::string delimiter = "|";
			size_t pos = s.find(delimiter);
			if(pos == std::string::npos)
			{
				message="App data invalid 1";
				AppLogger::debug("App data invalid 1");
				return false;
			}

			numObjs = s.substr(0, pos);
			AppLogger::debug(numObjs);

			std::string part = s.substr(pos+1);

			size_t pos2 = part.find(delimiter);
			if(pos2 == std::string::npos)
			{
				message="App data invalid 2";
				AppLogger::debug("App data invalid 2");
				return false;
			}

			numChannels = part.substr(0, pos2);
			AppLogger::debug(numChannels);

			std::string part2 = part.substr(pos2+1);

			size_t pos3 = part2.find(delimiter);

			if (pos3 == std::string::npos)
			{
				message = "App data invalid 3";
				AppLogger::debug("App data invalid 3");
				return false;
			}

			expiryDate = part2.substr(0, pos3);
			AppLogger::debug(expiryDate);
			std::string part3 = part2.substr(pos3 + 1);

			size_t pos4 = part3.find(delimiter);

			if (pos4 == std::string::npos)
			{
				message = "App data invalid 4";
				AppLogger::debug("App data invalid 4");
				return false;
			}

			machineID = part3.substr(0, pos4);
			AppLogger::debug(machineID);

			pluginType = part3.substr(pos4 + 1);

			AppLogger::debug(pluginType);

			AppLogger::debug("Loaded > Num Objs: " + numObjs + ", Expiry: " + expiryDate + ", Channels: " + numChannels+ ", MachineID: " + machineID + ", Type: " + pluginType);

		}
		catch(const std::exception& e)
		{
			message = e.what();
			AppLogger::debug("load error: " + message);
			return false;
		}
		
		return true;
	}

	void AppLogger::clear()
	{
		std::string outFile = "/usr/local/share/aol/hailo_objects_plugin.log";
	#ifdef _WIN32
		outFile = "C:\\aol\\hailo_objects_plugin.log";
	#endif

		try
		{
			std::ofstream outStream(outFile, std::ios_base::app);
			outStream << "" << std::endl;
			outStream.close();
		}
		catch (const std::exception&)
		{
#ifdef _WIN32
			outFile = "C:/temp/hailo_objects_plugin.log";
#else
			outFile = "/tmp/hailo_objects_plugin.log";
#endif
			std::ofstream outStream(outFile, std::ios_base::app);
			outStream << "" << std::endl;
			outStream.close();
		}
	}

	void AppLogger::print(std::string output)
	{
		std::string outFile;
		outFile = "/tmp/hailo_objects_plugin.log";
		AppLogger::getPrintLog(outFile);

		try
		{
			std::ofstream outStream(outFile, std::ios_base::app);
			outStream << output << std::endl;
			outStream.close();
		}
		catch (const std::exception&)
		{

		}
		
	}

	void AppLogger::debug(std::string output, bool turnon)
	{		

		std::string outFile;
		outFile = "/tmp/hailo_objects_plugin.log";
		std::ofstream outStream;
		outStream.open(outFile, std::ios_base::app);
		getDebugLog(outFile);

		if (turnon)
		{
			try
			{
				//std::ofstream outStream(outFile, std::ios_base::app);
				outStream << output << std::endl;
				outStream.close();

			}
			catch (const std::exception&)
			{
				try
				{
					// backup log
#ifdef _WIN32
					outFile = "C:/temp/hailo_objects_plugin.log";
#else
					outFile = "/tmp/hailo_objects_plugin.log";
#endif
					std::ofstream outStream2(outFile, std::ios_base::app);
					outStream2 << output << std::endl;
					outStream2.close();
				}
				catch (const std::exception&)
				{


				}
			}

		}
			
		
	}



} // namespace hailo_object_count
} // namespace analytics
} // namespace vms_server_plugins
} // namespace nx
