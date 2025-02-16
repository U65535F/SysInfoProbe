#include "SysInfoProbe.hpp"

std::optional<std::vector<Error>> SysInfoProbe::RetrieveAllData(bool StopOnError) {
	std::vector<Error> errors;
	std::vector<std::optional<Error>(SysInfoProbe::*)()> InfoFunctions{
		 &SysInfoProbe::GetCpuInfo,
		 &SysInfoProbe::GetRamInfo,
		 &SysInfoProbe::GetGpuInfo,
		 &SysInfoProbe::GetMotherboardInfo,
		 &SysInfoProbe::GetBIOSInfo,
		 &SysInfoProbe::GetComputerType,
		 &SysInfoProbe::GetStorageDevices,
		 &SysInfoProbe::GetDisplayInfo,
		 &SysInfoProbe::GetNetworkInterfacesInfo,
		 &SysInfoProbe::GetCDROMInfo,
		 &SysInfoProbe::GetOperatingSystemInfo,
		 &SysInfoProbe::GetSoundInfo
	};

	for (auto& function : InfoFunctions) {
		if (auto r = (this->*function)()) {
			errors.push_back(r.value());

			if (StopOnError) return errors;
		}
	}

	GetUptimeInfo();
	return errors.empty() ? std::nullopt : std::make_optional(errors);
}
