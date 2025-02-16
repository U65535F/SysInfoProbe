#include "SysInfoProbe.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

constexpr const char* RESET = "\033[0m";
constexpr const char* BOLD = "\033[1m";
constexpr const char* BLUE = "\033[34m";
constexpr const char* CYAN = "\033[36m";
constexpr const char* YELLOW = "\033[33m";

const int LabelWidth = 25;

void PrintSeparator() {
    std::cout << BLUE << std::string(60, '=') << RESET << '\n';
}

void PrintSectionTitle(const std::string& title) {
    PrintSeparator();
    std::cout << BOLD << CYAN << title << RESET << '\n';
    PrintSeparator();
}

void PrintRamInfo(const RAMINFO& ram) {
    PrintSectionTitle("RAM INFORMATION");
    std::cout << std::left << std::setw(LabelWidth) << "Name:" << ram.Name << '\n';
    std::cout << std::setw(LabelWidth) << "Manufacturer:" << ram.Manufacturer << '\n';
    std::cout << std::setw(LabelWidth) << "Model:" << ram.Model << '\n';
    std::cout << std::setw(LabelWidth) << "Memory Type:" << ram.MemoryType << '\n';
    std::cout << std::setw(LabelWidth) << "Form Factor:" << ram.FormFactor << '\n';
    std::cout << std::setw(LabelWidth) << "Serial Number:" << ram.SerialNumber << '\n';

    std::cout << std::fixed << std::setprecision(2);
    std::cout << std::setw(LabelWidth) << "Size (MB):" << ram.SizeInMegabytes << " MB\n";
    std::cout << std::setw(LabelWidth) << "Size (GB):" << ram.SizeInGigabytes << " GB\n";

    std::cout << std::setw(LabelWidth) << "Latency:" << ram.LatencyInNanoseconds << " ns\n";
    std::cout << std::setw(LabelWidth) << "Frequency:" << ram.FrequencyInMHz << " MHz\n";
}

void PrintMBInfo(const MAINBOARDINFO& mb) {
    PrintSectionTitle("MOTHERBOARD INFORMATION");
    std::cout << std::left << std::setw(LabelWidth) << "Manufacturer:" << mb.Manufacturer << '\n';
    std::cout << std::setw(LabelWidth) << "Product:" << mb.Name << '\n';
    std::cout << std::setw(LabelWidth) << "Version:" << mb.Version << '\n';
    std::cout << std::setw(LabelWidth) << "Serial Number:" << mb.SerialNumber << '\n';
}

void PrintGpuInfo(const GPUINFO& gpu) {
    PrintSectionTitle("GPU INFORMATION");
    std::cout << std::left << std::setw(LabelWidth) << "Name:" << gpu.Name << '\n';
    std::cout << std::setw(LabelWidth) << "Manufacturer:" << gpu.Manufacturer << '\n';
    std::cout << std::setw(LabelWidth) << "Driver Version:" << gpu.DriverVersion << '\n';
    std::cout << std::setw(LabelWidth) << "VRAM Size (MB):" << gpu.VRAMSizeInMegabytes << " MB\n";
    std::cout << std::setw(LabelWidth) << "VRAM Size (GB):" << gpu.VRAMSizeInGigabytes << " GB\n";
}

void PrintOSInfo(const OSINFO& os) {
    PrintSectionTitle("OPERATING SYSTEM INFORMATION");
    std::cout << std::left << std::setw(LabelWidth) << "Technical Name:" << os.TechnicalName << '\n';
    std::cout << std::setw(LabelWidth) << "Name:" << os.Name << '\n';
    std::cout << std::setw(LabelWidth) << "Version:" << os.Version << '\n';
    std::cout << std::setw(LabelWidth) << "Build Number:" << os.BuildNumber << '\n';
    std::cout << std::setw(LabelWidth) << "Architecture:" << os.Architecture << '\n';
    std::cout << std::setw(LabelWidth) << "Install Date:" << os.InstallDate << '\n';
}

void PrintCPUInfo(const CPUINFO& cpu) {
    PrintSectionTitle("CPU INFORMATION");
    std::cout << std::left << std::setw(LabelWidth) << "Name:" << cpu.Name << '\n';
    std::cout << std::setw(LabelWidth) << "Manufacturer:" << cpu.Manufacturer << '\n';
    std::cout << std::setw(LabelWidth) << "Cores:" << cpu.CoreCount << '\n';
    std::cout << std::setw(LabelWidth) << "Logical Processors:" << cpu.ThreadCount << '\n';
    std::cout << std::setw(LabelWidth) << "Max Clock Speed:" << cpu.MaxClockSpeed << " MHz\n";

    std::cout << std::setw(LabelWidth) << "L1 Cache:" << cpu.Cache.L1 << " KB\n";
    std::cout << std::setw(LabelWidth) << "L2 Cache:" << cpu.Cache.L2 << " KB\n";
    std::cout << std::setw(LabelWidth) << "L3 Cache:" << cpu.Cache.L3 << " KB\n";

    // Print CPU instructions in a comma-separated list
    std::cout << std::setw(LabelWidth) << "Instructions:";
    for (size_t i = 0; i < cpu.Instructions.size(); ++i) {
        std::cout << cpu.Instructions[i];
        if (i != cpu.Instructions.size() - 1)
            std::cout << ", ";
    }
    std::cout << '\n';
}

void PrintBIOSInfo(const BIOSINFO& bios) {
    PrintSectionTitle("BIOS INFORMATION");
    std::cout << std::left << std::setw(LabelWidth) << "Manufacturer:" << bios.Manufacturer << '\n';
    std::cout << std::setw(LabelWidth) << "Version:" << bios.Version << '\n';
    std::cout << std::setw(LabelWidth) << "Build Number:" << bios.BuildNumber << '\n';
    std::cout << std::setw(LabelWidth) << "Serial Number:" << bios.SerialNumber << '\n';
}

void PrintComputerType(const SysInfoProbe& probe) {
    PrintSectionTitle("COMPUTER TYPE");
    std::cout << "Computer Type: " << (probe.ComputerType == LAPTOP ? "Laptop" : "Desktop") << '\n';
}

void PrintDisplayInfo(const std::vector<DISPLAYINFO>& displays) {
    PrintSectionTitle("DISPLAY INFORMATION");
    for (const auto& display : displays) {
        std::cout << std::left << std::setw(LabelWidth) << "Monitor Name:" << display.MonitorName << '\n';
        std::cout << std::setw(LabelWidth) << "Manufacturer:" << display.MonitorManufacturer << '\n';
        std::cout << std::setw(LabelWidth) << "Screen Width:" << display.ScreenWidth << '\n';
        std::cout << std::setw(LabelWidth) << "Screen Height:" << display.ScreenHeight << '\n';
        std::cout << std::setw(LabelWidth) << "Max Width:" << display.MaxWidthRes << '\n';
        std::cout << std::setw(LabelWidth) << "Max Height:" << display.MaxHeightRes << '\n';
        std::cout << std::setw(LabelWidth) << "Screen Size (inches):" << display.ScreenSizeInch << "\"\n";
        std::cout << std::setw(LabelWidth) << "Refresh Rate (hz):" << display.RefreshRate << '\n';
        PrintSeparator();
    }
}

void PrintUptime(const UPTIMEINFO& uptime) {
    PrintSectionTitle("UPTIME INFORMATION");
    std::cout << "Days: " << uptime.Days << '\n';
    std::cout << "Hours: " << uptime.Hours << '\n';
    std::cout << "Minutes: " << uptime.Minutes << '\n';
    std::cout << "Seconds: " << uptime.Seconds << '\n';
}

void PrintSoundInfo(const SOUNDINFO& sound) {
    PrintSectionTitle("SOUND INFORMATION");
    std::cout << "Name: " << sound.Name << '\n';
}

void PrintCDROMInfo(const std::vector<CDROMINFO>& cdroms) {
    PrintSectionTitle("CDROM INFORMATION");
    for (const auto& cdrom : cdroms) {
        std::cout << "Name: " << cdrom.Name << '\n';
    }
}

void PrintNetworkInterfaceInfo(const std::vector<NETWORKINTERFACEINFO>& network) {
    PrintSectionTitle("NETWORK INTERFACE INFORMATION");
    for (const auto& net : network) {
        std::cout << std::left << std::setw(LabelWidth) << "Name:" << net.Name << '\n';
        std::cout << std::setw(LabelWidth) << "Description:" << net.Description << '\n';
        std::cout << std::setw(LabelWidth) << "Interface Index:" << net.InterfaceIndex << '\n';
        std::cout << std::setw(LabelWidth) << "Interface Type:" << net.InterfaceType << '\n';
        std::cout << std::setw(LabelWidth) << "MAC Address:" << net.MACAddress << '\n';
        std::cout << std::setw(LabelWidth) << "DNS Suffix:" << net.DNSSuffix << '\n';

        std::cout << std::setw(LabelWidth) << "IP Addresses:";
        for (size_t i = 0; i < net.IPAddresses.size(); ++i) {
            std::cout << net.IPAddresses[i];
            if (i != net.IPAddresses.size() - 1)
                std::cout << ", ";
        }
        std::cout << '\n';

        std::cout << std::setw(LabelWidth) << "DNS Addresses:";
        for (size_t i = 0; i < net.DNSAddresses.size(); ++i) {
            std::cout << net.DNSAddresses[i];
            if (i != net.DNSAddresses.size() - 1)
                std::cout << ", ";
        }
        std::cout << '\n';

        std::cout << std::setw(LabelWidth) << "Subnet Masks:";
        for (size_t i = 0; i < net.SubnetMasks.size(); ++i) {
            std::cout << net.SubnetMasks[i];
            if (i != net.SubnetMasks.size() - 1)
                std::cout << ", ";
        }
        std::cout << '\n';
        PrintSeparator();
    }
}

void PrintStorageDevicesInfo(const std::vector<STORAGEDEVICEINFO>& storagedevices) {
    PrintSectionTitle("STORAGE DEVICES INFORMATION");
    for (const auto& storage : storagedevices) {
        std::cout << "Model: " << storage.Model << '\n';
        std::cout << "Manufacturer: " << storage.Manufacturer << '\n';
        std::cout << "Serial Number: " << storage.SerialNumber << '\n';
        std::cout << "Size (MiB): " << storage.SizeInMebibytes << " MB\n";
        std::cout << "Size (GiB): " << storage.SizeInGibibytes << " GB\n";
        PrintSeparator();
    }
}

int Test() {
    SysInfoProbe probe;
    probe.InitializeWMIAPI();
    auto r = probe.RetrieveAllData();
    if (r) {
        std::wcerr << L"Error retrieving information: " << r.value().at(0).Format() << std::endl;
        return 1;
    }

    std::cout << BOLD << YELLOW << "\n*** SYSTEM INFORMATION UTILITY ***\n" << RESET;

    PrintRamInfo(probe.RAM);
    PrintGpuInfo(probe.GPU);
    PrintMBInfo(probe.Mainboard);
    PrintOSInfo(probe.OS);
    PrintCPUInfo(probe.CPU);
    PrintComputerType(probe);
    PrintBIOSInfo(probe.BIOS);
    PrintDisplayInfo(probe.Displays);
    PrintUptime(probe.Uptime);
    PrintSoundInfo(probe.Sound);
    PrintNetworkInterfaceInfo(probe.NetworkInterfaces);
    PrintStorageDevicesInfo(probe.StorageDevices);
    PrintCDROMInfo(probe.CDROMs);

    return 0;
}

int main() {
    return Test();
}
