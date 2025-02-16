/* Info: This file contains all the user-defined types, enumerations and required constants for SysInfoProbe. */
#pragma once
#include <Windows.h>
#include <map>
#include <string>

static std::string RAMFormFactors[24] {
	"Unknown form factor",
	"",
	"SIP",
	"DIP",
	"ZIP",
	"SOJ",
	"Proprietary",
	"SIMM",
	"DIMM",
	"TSOP",
	"PGA",
	"RIMM",
	"SODIMM",
	"SRIMM",
	"SMD",
	"SSMP",
	"QFP",
	"TQFP",
	"SOIC",
	"LCC",
	"PLCC",
	"BGA",
	"FPBGA",
	"LGA"
};

static std::string RAMMemoryTypes[] = {
	"Unknown",
	"Other",
	"DRAM",
	"Synchronous DRAM",
	"Cache DRAM",
	"EDO",
	"EDRAM",
	"VRAM",
	"SRAM",
	"RAM",
	"ROM",
	"Flash",
	"EEPROM",
	"FEPROM",
	"EPROM",
	"CDRAM",
	"3DRAM",
	"SDRAM",
	"SGRAM",
	"RDRAM",
	"DDR",
	"DDR2",
	"DDR2 FB-DIMM",
	"",
	"DDR3",
	"FBD2",
	"DDR4",
	"LPDDR",
	"LPDDR2",
	"LPDDR3",
	"LPDDR4",
	"Logical non-volatile device",
	"HBM",
	"HBM2"
	// TODO: Add a way to check for DDR5
};

// Win32_PhysicalMemory
typedef struct _tag_RAMINFO {
	// Manufacturer + Model
	std::string Name;

	// Aka Vendor; obtained using "Manufacturer" property.
	std::string Manufacturer;

	// TODO: We expect actual name of the RAM but WMI does not provide such a property...
    // The "Name"-property of WMI returns "PhysicalMemory". Obtained by "PartNumber" property;
	std::string Model;

	// Obtained using "MemoryType" property;
	std::string MemoryType;

	// Obtained using "FormFactor" property;
	std::string FormFactor;

	// Obtained using "SerialNumber" property;
	std::string SerialNumber;

	// SizeInMegabytes is obtained by iterating each slot and accumulating "Capacity" property;
	// SizeInGigabytes is obtained by dividing SizeInMegabytes by 1024 
	double SizeInGigabytes = 0.0;
	int SizeInMegabytes = 0;

	// Retrieval latency obtained using "Speed" property; Unsure about its use but seems useful.
	// It seems to be same as the Frequency as RAM.
	int LatencyInNanoseconds = 0;

	// Obtained using "ConfiguredClockSpeed" property.
	int FrequencyInMHz = 0;
} RAMINFO, * PRAMINFO;

// Win32_VideoController
typedef struct _tag_GPUINFO {
	// Obtained using "Name" property;
	std::string Name;

	// Aka Vendor; obtained using "AdapterCompatibility" property.
	std::string Manufacturer;

	// Obtained using "DriverVersion" property.
	std::string DriverVersion;

	// Obtained by dividing the VRAMSizeInGigabytes values by 1024.
	double VRAMSizeInGigabytes = 0.0;

	// Obtained using "AdapterRAM" property.
	int VRAMSizeInMegabytes = 0;
	
	// Obtained using "CurrentRefreshRate" property.
	int RefreshRate = 0;
	// TODO: Add a way of obtaining Frequency, CoreCount and many other properties without using
	// External libraries like OpenCL, CUDA or others.
} GPUINFO, * PGPUINFO;

// Win32_BaseBoard
typedef struct _tag_MAINBOARDINFO {
	// Obtained using "Manufacturer property. "
	std::string Manufacturer;

	// Obtained using "Product" property.
	std::string Name;

	// Obtained using "Version" property.
	std::string Version;

	// Obtained using "SerialNumber" property.
	std::string SerialNumber;
} MAINBOARDINFO, * PMAINBOARDINFO;

// Win32_OperatingSystem
typedef struct _tag_OSINFO {
	// Obtained using "Name" property.
	std::string TechnicalName;

	// Obtained using "Caption" property.
	std::string Name;

	// Obtained using "Version" property.
	std::string Version;

	// Obtained using "BuildNumber" property.
	std::string BuildNumber;

	// Obtained using "OSArchitecture" property.
	std::string Architecture;

	// Obtained using "InstallDate" property.
	std::string InstallDate;
} OSINFO, *POSINFO;

// Win32_PerfFormattedData_Counters_ProcessorInformation
typedef struct _tag_CPUUTILIZATION {
	// Obtained using "PercentProcessorPerformance" property.
	int64_t CurrentClockSpeed = 0;
	// Obtained using "PercentProcessorUtility" property with filter Name=0,_Total
	double CurrentUtilization =  0.0;
	// Both below are obtained using "PercentProcessorUtility" property.
	double ThreadUtilization = 0.0;
	std::vector<double> ThreadsUtilization;
	// Obtained using "PercentProcessorPerformance" property.
	std::vector<int64_t> CurrentClockSpeeds;
} CPUUTILIZATION, *PCPUUTILIZATION;

// Win32_CacheMemory
typedef struct _tag_CPUCACHE {
	// All are obtained using "MaxCacheSize" property.
	// And in kilobytes (KB).
	int L1 = 0;
	int L2 = 0;
	int L3 = 0;
} CPUCACHE, *PCPUCACHE;

// Win32_Processor
typedef struct _tag_CPUINFO {
	// Obtained manually using CPUID instruction.
	std::vector<std::string> Instructions;

	// Obtained using "Name" property.
	std::string Name;
	// Obtained using "Manufacturer" property.
	std::string Manufacturer;
	// Obtained using "NumberOfCores" property.
	int CoreCount = 0;
	// Obtained using "NumberOfLogicalProcessors" property.
	int ThreadCount = 0;
	// Obtained using "MaxClockSpeed" property.
	unsigned int MaxClockSpeed = 0;

	CPUUTILIZATION Utilization;
	CPUCACHE Cache;
} CPUINFO, * PCPUINFO;

// Win32_BIOS
typedef struct _tag_BIOSINFO {
	// Obtained using "Manufacturer" property.
	std::string Manufacturer;
	// Obtained using "Version" property.
	std::string Version;
	// Obtained using "BuildNumber" property; unused as of now
	std::string BuildNumber;
	// Obtained using "SerialNumber" property.
	std::string SerialNumber;
} BIOSINFO, *PBIOSINFO;

// Win32_DesktopMonitor
typedef struct _tag_DISPLAYINFO {
	// Obtained using "Name" property.
	std::string MonitorName;
	// Obtained using "MonitorManufacturer" property.
	std::string MonitorManufacturer;
	// Manually calculated using MaxHorizontalImageSize and MaxVerticalImageSize in `WmiMonitorBasicDisplayParams`.
	double ScreenSizeInch = 0.0;
	// All the below use Win32 API.
	int ScreenWidth = 0;
	int ScreenHeight = 0;
	int MaxWidthRes = 0;
	int MaxHeightRes = 0;
	int RefreshRate = 0;
} DISPLAYINFO, *PDISPLAYINFO;

// Obtained using GetTickCount64.
typedef struct _tag_UPTIMEINFO {
	UINT64 Days = 0;
	UINT64 Hours = 0;
	UINT64 Minutes = 0; 
	UINT64 Seconds = 0;
} UPTIMEINFO, *PUPTIMEINFO;

// Win32_SoundDevice
typedef struct _tag_SOUNDINFO {
	// Obtained using "Caption" property.
	std::string Name;
} SOUNDINFO, *PSOUNDINFO;

// Win32_CDROMDrive
typedef struct _tag_CDROMINFO {
	// Obtained using "Caption" property.
	std::string Name;
} CDROMINFO, *PCDROMINFO;

// Manually retrieved using Win32 network functions.
typedef struct _tag_NETWORKINTERFACEINFO {
	std::string Name;
	std::string Description;
	unsigned int InterfaceIndex = 0;
	unsigned int InterfaceType = 0;
	std::string MACAddress;
	std::string DNSSuffix;
	std::vector<std::string> IPAddresses;
	std::vector<std::string> DNSAddresses;
	std::vector<std::string> SubnetMasks;
} NETWORKINTERFACEINFO, *PNETWORKINTERFACEINFO;

// Win32_DiskDrive
typedef struct _tag_STORAGEDEVICEINFO {
	// Obtained using "Model" property.
	std::string Model;

	// Obtained using "Manufacturer" property.
	std::string Manufacturer;

	// Obtained using "SerialNumber" property.
	std::string SerialNumber;

	// Obtained using size and converted manually.
	DWORD SizeInMebibytes = 0;
	int SizeInGibibytes = 0;
} STORAGEDEVICEINFO, *PSTORAGEDEVICEINFO;

enum COMPUTER_TYPE {
	NONE,
	DESKTOP,
	LAPTOP
};
