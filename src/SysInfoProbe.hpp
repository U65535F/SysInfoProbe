#pragma once
#include <WinSock2.h>		// For network information
#include <WS2tcpip.h>		// For network information
#include <iphlpapi.h>		// For network information
#include "Errors.hpp"		// For error handling
#include "Defer.hpp"		// For golang-styled defer keyword
#include "WMIMgr.hpp"		// For WMI API
#include "Utils.hpp"		// For small utility functions
#include "SysInfoTypes.hpp"	// User-defined types for system information
#include <intrin.h>			// For CPUID instruction
#include <PowerBase.h>		// For GetPwrCapabilities function
#include <sstream>			// For std::ostringstream used in network information
#include <chrono>			// For setw and setfill
#pragma comment(lib, "PowrProf.lib")	// Required to use PowerBase.h
#pragma comment(lib, "ws2_32.lib")		// Required for network information
#pragma comment(lib, "IPHlpApi.lib")	// Required for network information

class SysInfoProbe {
public:
	CPUINFO CPU;
	MAINBOARDINFO Mainboard;
	RAMINFO RAM;
	GPUINFO GPU;
	OSINFO OS;
	BIOSINFO BIOS;
	UPTIMEINFO Uptime;
	SOUNDINFO Sound;

	std::vector<STORAGEDEVICEINFO> StorageDevices;
	std::vector<NETWORKINTERFACEINFO> NetworkInterfaces;
	std::vector<CDROMINFO> CDROMs;
	std::vector<DISPLAYINFO> Displays;
	COMPUTER_TYPE ComputerType = NONE;

	std::optional<Error> GetCpuInfo();
	std::optional<Error> GetRamInfo();
	std::optional<Error> GetGpuInfo();
	std::optional<Error> GetMotherboardInfo();
	std::optional<Error> GetBIOSInfo();
	std::optional<Error> GetComputerType();

	std::optional<Error> GetStorageDevices();
	std::optional<Error> GetDisplayInfo();
	std::optional<Error> GetNetworkInterfacesInfo();
	std::optional<Error> GetCDROMInfo();
	std::optional<Error> GetOperatingSystemInfo();
	std::optional<Error> GetSoundInfo();
	void GetUptimeInfo();
	std::optional<std::vector<Error>> RetrieveAllData(bool StopOnError = false);

	std::optional<Error> RefreshCPUUtilizations();
	std::optional<Error> RefreshFreeRAM() { return std::nullopt; };

	std::optional<Error> InitializeWMIAPI() {
		auto r = WMIMgr.InitializeAPI();
		if (!r) bInitialized = TRUE;
		return r;
	};

private:
	BOOL bInitialized = FALSE;
	WMIManager WMIMgr;

	/* Private Information Retrieval Functions */
	/* - CPU */
	void _GetCPUInstructions();
	std::optional<Error> _GetCPUCache();

	/* - Mainboard */
	std::string _FormatWMIDateTime(const std::string& WMIDateTime);

	/* - Monitor/Display */
	std::optional<Error> _GetRealMonitorSize();
};
