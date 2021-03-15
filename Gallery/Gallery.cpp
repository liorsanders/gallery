#include <iostream>
#include <string>
#include "MemoryAccess.h"
#include "AlbumManager.h"
#include <Windows.h>

using std::cout;
using std::endl;

#pragma comment(lib, "user32.lib")

void printSystemInfo() {
	SYSTEM_INFO siSysInfo;

	// Copy the hardware information to the SYSTEM_INFO structure. 

	GetSystemInfo(&siSysInfo);

	// Display the contents of the SYSTEM_INFO structure. 

	cout << "Hardware information:" << endl <<
		"  OEM ID: " << siSysInfo.dwOemId << endl <<
		"  Number of processors: " <<
		siSysInfo.dwNumberOfProcessors << endl <<
		"  Page size: " << siSysInfo.dwPageSize << endl <<
		"  Processor type: " << siSysInfo.dwProcessorType << endl <<
		"  Minimum application address: " <<
		siSysInfo.lpMinimumApplicationAddress << endl <<
		"  Maximum application address: " <<
		siSysInfo.lpMaximumApplicationAddress << endl <<
		"  Active processor mask: " <<
		siSysInfo.dwActiveProcessorMask << endl;
}

int getCommandNumberFromUser()
{
	std::string message("\nPlease enter any command(use number): ");
	std::string numericStr("0123456789");
	
	std::cout << message << std::endl;
	std::string input;
	std::getline(std::cin, input);
	
	while (std::cin.fail() || std::cin.eof() || input.find_first_not_of(numericStr) != std::string::npos) {

		std::cout << "Please enter a number only!" << std::endl;

		if (input.find_first_not_of(numericStr) == std::string::npos) {
			std::cin.clear();
		}

		std::cout << std::endl << message << std::endl;
		std::getline(std::cin, input);
	}
	
	return std::atoi(input.c_str());
}

int main(void)
{
	// initialization data access
	MemoryAccess dataAccess;

	// initialize album manager
	AlbumManager* albumManager = new AlbumManager(dataAccess); /*turned albumManager into pointer to make changes by refference*/

	//todo first issue needs to be fixed here

	std::string albumName;
	std::cout << "Welcome to Gallery!" << std::endl;
	std::cout << "===================" << std::endl;

	//part i added:
	cout << "*******************SYSTEM INFORMATION*****************" << endl;
	printSystemInfo();
	cout << "******************************************************" << endl;

	std::cout << "Type " << HELP << " to a list of all supported commands" << std::endl;
	
	do {
		int commandNumber = getCommandNumberFromUser();
		
		try	{
			albumManager->executeCommand(static_cast<CommandType>(commandNumber));
		} catch (std::exception& e) {	
			std::cout << e.what() << std::endl;
		}
	} 
	while (true);

	delete albumManager;
	return 0;
}