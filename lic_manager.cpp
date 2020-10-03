#ifndef _LIC_MANAGER_CPP_
#define _LIC_MANAGER_CPP_

#include <string>
#include <sstream>
#include <iostream>
#include <ctime>
#include <list>
#include <thread>
#include "common_lic.hpp"
#define DEBUG

using namespace std;

class LicData
{

private:

	static const int LineNo = 4;

	//read api's

	bool read1CreatedTime(string const& _ctimeLine)
	{
		bool res = false;
		if (Utils::StringToTime(_ctimeLine, CreatedTime))
			res = true;
		return res;
	}

	bool read2Enabled(string const& _enabledLine)
	{
		Enabled = atoi(_enabledLine.c_str());
		return true;
	}

	bool read3RemainingHours(string const& _rmLine)
	{
		RemainingTimeUnit = atol(_rmLine.c_str());
		return true;
	}

	bool read4LastUpdatedTime(string const& _lutimeLine)
	{
		bool res = false;
		if (Utils::StringToTime(_lutimeLine, LastUpdatedTime))
			res = true;
		return res;
	}

	//write api's
	void chainWrite(stringstream& _ss)
	{
		write1CreatedTime(_ss);
		write2Enabled(_ss);
		write3RemainingHours(_ss);
		write4LastUpdatedTime(_ss);
	}

	void write1CreatedTime(stringstream& _ss)
	{
		_ss << Utils::TimeToString(CreatedTime) << endl;
	}

	void write2Enabled(stringstream& _ss)
	{
		_ss << Enabled << endl;
	}

	void write3RemainingHours(stringstream& _ss)
	{
		_ss << RemainingTimeUnit << endl;
	}

	void write4LastUpdatedTime(stringstream& _ss)
	{
		_ss << Utils::TimeToString(LastUpdatedTime) << endl;
	}

public:

	time_t CreatedTime;					//Line 1
	bool Enabled = false;				//Line 2
	long RemainingTimeUnit = 0;			//Line 3
	time_t LastUpdatedTime;				//Line 4

	string toString()
	{
		stringstream ss;
		ss << endl << "CreatedTime -> " << ctime(&CreatedTime);
		ss << "Enabled -> " << (Enabled ? "True" : "False") << endl;
		ss << "RemainingTimeUnit -> " << RemainingTimeUnit << endl;
		ss << "LastUpdatedTime -> " << ctime(&LastUpdatedTime) << endl;
		return ss.str();
	}

	bool read(string _fileName)
	{
		ifstream file(_fileName.c_str());
		if (!file.good())
			return false;

		bool res = true;
		list<string> lines;
		string cipherData, plainData, curLine;

		while (getline(file, curLine))
			cipherData += curLine;

		file.close();

		plainData = Utils::EDcrypt(cipherData, LIC_COMMON_KEY);

		stringstream ss(plainData);
		while (ss.good())
		{
			getline(ss, curLine, '\n');
			if (curLine.size() > 0)
				lines.push_back(curLine);
		}
		auto actualLineCount = lines.size();
		if (actualLineCount != LineNo)
			return false;

		int lineNo = 0;
		for (auto line : lines)
		{
			if (!res)
				break;

			++lineNo;

			switch (lineNo)
			{
			case 1:
			{
				res = read1CreatedTime(line);
				break;
			}
			case 2:
			{
				res = read2Enabled(line);
				break;
			}
			case 3:
			{
				res = read3RemainingHours(line);
				break;
			}
			case 4:
			{
				res = read4LastUpdatedTime(line);
				break;
			}
			}
		}
		return res && lineNo == LineNo;
	}

	bool write(string _fileName, bool _create = true)
	{
		bool res = false;
		ofstream outFile;
		outFile.open(_fileName.c_str(), ofstream::out | ofstream::trunc);
		bool bad = !outFile.good();
		if (bad)
		{
			if (_create && !Utils::CreateDirectoryTreeIfNotExists(_fileName))
				return res;
			else
				outFile.open(_fileName.c_str());
		}

		bad = !outFile.good();

		if (bad)
			return res;

		stringstream ss;

		chainWrite(ss);

		auto cipherData = Utils::EDcrypt(ss.str(), LIC_COMMON_KEY);

		ss.str("");

		ss << cipherData;

		outFile << ss.rdbuf();

		outFile.close();

		return true;

	}


};

class LicManager
{
private:
	list<string> _fileNames;

	void Validate()
	{
		if (LicenceData.RemainingTimeUnit <= 0)
			Exit();
		time_t now;
		time(&now);
		if (LicenceData.LastUpdatedTime > now || LicenceData.CreatedTime > now)
			Exit();
	}

	void Exit()
	{
		cout << "LicenceData expired / Manipulated!";
		exit(0);
	}

public:
	const int PollDelay;
	LicData LicenceData;

	LicManager(string __fileNames = LIC_FILES_NAME, bool _create = false, int _pollDelay = LIC_POLL_DELAY_HOUR) :PollDelay(_pollDelay), LicenceData()
	{
		time(&LicenceData.LastUpdatedTime);
		time(&LicenceData.CreatedTime);
		bool res = false;
		stringstream ss(__fileNames);
		while (ss.good())
		{
			string substr;
			getline(ss, substr, ',');
			if (_create)
			{
				if (Utils::CreateDirectoryTreeIfNotExists(substr))
					_fileNames.push_back(substr);
			}
			else
			{
				res = res || Utils::IsFileExists(substr);
				_fileNames.push_back(substr);
			}
		}

		if (_create)
		{
			if (_fileNames.size() == 0)
				Exit();
		}
		else if (!res)
		{
			Exit();
		}


	}

	void printFiles()
	{
		for (auto const& entry : _fileNames)
		{
			cout << entry << endl;
		}
	}

	bool Read()
	{
		bool res = false;
		for (auto const& fileName : _fileNames)
		{
			if (LicenceData.read(fileName))
			{
				res = true;
				break;
			}
		}
		return res;
	}

	int Delete()
	{
		int res = -1;
		for (auto const& fileName : _fileNames)
		{
			if (Utils::RemoveFile(fileName))
				res++;
		}
		return res + 1;
	}

	int Write()
	{
		int res = -1;
		for (auto const& fileName : _fileNames)
		{
			if (LicenceData.write(fileName))
			{
				++res;
			}
		}
		return res + 1;
	}


	void Poll()
	{
		for (;;)
		{
			if (Read())
			{
				if (LicenceData.Enabled)
				{
#ifdef DEBUG
					cout << LicenceData.toString();
#endif
					Validate();
					this_thread::sleep_for(chrono::seconds(PollDelay));
					LicenceData.RemainingTimeUnit--;
					time(&LicenceData.LastUpdatedTime);
					Write();
				}
				else
				{
#ifdef DEBUG
					cout << "Licence disabled!";
#endif
					this_thread::sleep_for(chrono::seconds(PollDelay));
				}
			}
			else
			{
				Exit();
			}
		}
	}


};

#endif