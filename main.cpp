//#include "lic_manager.cpp"
//
//int main()
//{
//	auto mgr = LicManager(LIC_FILES_NAME, true, 60);
//	mgr.LicenceData.Enabled = true;
//	mgr.LicenceData.RemainingTimeUnit = 3;
//	mgr.Write();
//	mgr.Poll();
//	return 0;
//}
//


#include "lic_manager.cpp"
#include<iostream>

#define LIC_NO_COPY "No valid licence copies are available." 


#include <unistd.h>
#include <termios.h>
#include <climits>
using namespace std;

char getch(bool _print = true)
{
	char buf = 0;
	struct termios old = { 0 };
	fflush(stdout);
	if (tcgetattr(0, &old) < 0)
		perror("tcsetattr()");
	old.c_lflag &= ~ICANON;
	old.c_lflag &= ~ECHO;
	old.c_cc[VMIN] = 1;
	old.c_cc[VTIME] = 0;
	if (tcsetattr(0, TCSANOW, &old) < 0)
		perror("tcsetattr ICANON");
	if (read(0, &buf, 1) < 0)
		perror("read()");
	old.c_lflag |= ICANON;
	old.c_lflag |= ECHO;
	if (tcsetattr(0, TCSADRAIN, &old) < 0)
		perror("tcsetattr ~ICANON");
	if (_print)
		printf("Selection -> %c\n", buf);
	return buf;
}


int main()
{
	auto mgr = LicManager(LIC_FILES_NAME, true);
	mgr.LicenceData.Enabled = true;
	bool done = false;
	while (!done)
	{

		cout << endl << "**Lic Manager**" << endl;
		cout << endl << "1. Create" << endl << "2. Extend" << endl << "3. Disable" << endl << "4. Enable" << endl << "5. Delete" << endl << "6. Display" << endl << "7. Exit" << endl << endl;

		switch (getch())
		{
		case '1':
		{
			int delta;
			string sDelta;
			cout << "Enter total licencing days!." << endl;
			cin >> sDelta;
			delta = atoi(sDelta.c_str());
			if (delta <= 0)
			{
				cout << "Invalid range, try again!." << endl;
			}
			else
			{
				mgr.LicenceData.RemainingTimeUnit = delta * 24;
				cout << "Succesfully created " << mgr.Write() << " licence copies for " << mgr.LicenceData.RemainingTimeUnit << " hours." << endl;
			}
			break;
		}
		case '2':
		{
			if (mgr.Read())
			{
				cout << "Remaining units -> " << mgr.LicenceData.RemainingTimeUnit << endl << "Enter (hours) +ve to extend and -ve to reduce." << endl;

				int delta;
				string sDelta;
				cin >> sDelta;
				delta = atoi(sDelta.c_str());
				if (delta == 0)
				{
					cout << "Invalid range, try again!." << endl;
				}
				else
				{
					auto old = mgr.LicenceData.RemainingTimeUnit;
					mgr.LicenceData.RemainingTimeUnit += delta;
					cout << mgr.Write() << " licence copies are adjusted from " << old << " hours to " << mgr.LicenceData.RemainingTimeUnit << " hours succesfully!." << endl;
				}
			}
			else
			{
				cout << LIC_NO_COPY << endl;
			}
			break;
		}
		case '3':
		{
			if (mgr.Read())
			{
				mgr.LicenceData.Enabled = false;
				cout << mgr.Write() << " licence copies are disabled succesfully!." << endl;
			}
			else
			{
				cout << LIC_NO_COPY << endl;
			}
			break;
		}
		case '4':
		{
			if (mgr.Read())
			{
				mgr.LicenceData.Enabled = true;
				cout << mgr.Write() << "  licence copies are enabled succesfully!." << endl;
			}
			else
			{
				cout << LIC_NO_COPY << endl;
			}
			break;
		}
		case '5':
		{
			cout << mgr.Delete() << " licence copies are deleted succesfully!." << endl;
			break;
		}
		case '6':
		{
			if (mgr.Read())
			{
				cout << mgr.LicenceData.toString();
			}
			else
			{
				cout << LIC_NO_COPY << endl;
			}
			break;
		}
		case '7':
		{
			exit(0);
		}
		default:
			cout << " Invalid selection!, try again." << endl;
		}

	}

	mgr.Write();
	return 0;
}

