#ifndef _COMMON_LIC_HPP_
#define _COMMON_LIC_HPP_

#include <fstream>
#include <string>
#include <sys/stat.h>
#include <ctime>

#define LIC_FILES_NAME "data/fip,raw/tag,lil/ack,con"

#define LIC_COMMON_KEY "&*(345vbmhVVfgcj!@#24"

#define LIC_COMMON_KEY "&*(345vbmhVVfgcj!@#24"

#define LIC_POLL_DELAY_MIN 60

#define LIC_POLL_DELAY_HOUR LIC_POLL_DELAY_MIN*LIC_POLL_DELAY_MIN


using namespace std;

class Utils
{
public:
	static string TimeToString(time_t& _t)
	{
		ostringstream os;
		os << _t;
		return os.str();
	}

	static bool StringToTime(string const& _str, time_t& _res)
	{
		bool res = false;
		istringstream ss(_str);
		ss >> _res;
		if (!ss.fail())
		{
			res = true;
		}
		return res;
	}

	static string EDcrypt(string _msg, string _key)
	{
		// Make sure the key is at least as long as the message
		string tmp(_key);
		while (_key.size() < _msg.size())
			_key += tmp;

		// And now for the encryption part
		for (size_t i = 0; i < _msg.size(); ++i)
			_msg[i] ^= _key[i];
		return _msg;
	}

	static bool RemoveFile(string const& _filePath)
	{
		return remove(_filePath.c_str()) == 0;
	}

	static bool CreateDirectoryTreeIfNotExists(string const& _filePath, bool createFile = true)
	{
		bool res = true;

		string fileName = _filePath;
		auto pos = fileName.rfind('/');
		if (string::npos != pos)
		{
			string dirPath = fileName;
			dirPath.erase(pos, fileName.size());
			size_t pos = 0;
			string dir;

			if (dirPath[dirPath.size() - 1] != '/') {
				// force trailing / so we can handle everything in loop
				dirPath += '/';
			}

			while ((pos = dirPath.find_first_of('/', pos)) != string::npos) {
				dir = dirPath.substr(0, pos++);
				if (dir.size() == 0) continue; // if leading / first time is 0 length
				if (mkdir(dir.c_str(), 0755) && errno != EEXIST) {
					res = false;
					break;
				}
			}
		}

		if (res && createFile && !IsFileExists(fileName))
		{
			ofstream file(fileName.c_str());
			file.close();
		}
		return res;
	}

	static bool IsFileExists(string const& _fileName)
	{
		ifstream infile(_fileName.c_str());
		return infile.good();
	}
};


#endif
