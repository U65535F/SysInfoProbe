#include "SysInfoProbe.hpp"

std::optional<Error> SysInfoProbe::GetNetworkInterfacesInfo() {
	const char* FuncName = "SysInfoProbe::GetNetworkInterfacesInfo";
	if (!bInitialized) return Error::New(FuncName, 1, L"API not initialized.");

    ULONG ulRequiredBufferSize = 0;
    DWORD dwRetVal = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, NULL, &ulRequiredBufferSize);
    if (dwRetVal != ERROR_BUFFER_OVERFLOW) {
		return Error::New(FuncName, 2, L"Failed to get buffer size.");
    }

    std::unique_ptr<BYTE[]> Buffer(new BYTE[ulRequiredBufferSize]);
    PIP_ADAPTER_ADDRESSES pAdapterAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(Buffer.get());

    dwRetVal = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAdapterAddresses, &ulRequiredBufferSize);
    if (dwRetVal != NO_ERROR) {
		return Error::New(FuncName, 3, L"Failed to get adapter addresses.");
    }

    for (PIP_ADAPTER_ADDRESSES pAdapter = pAdapterAddresses; pAdapter != nullptr; pAdapter = pAdapter->Next) {
        NETWORKINTERFACEINFO CurrentInterfaceInfo;

        if (pAdapter->FriendlyName) 
            CurrentInterfaceInfo.Name = w2s(pAdapter->FriendlyName);
        

        if (pAdapter->Description)
            CurrentInterfaceInfo.Description = w2s(pAdapter->Description);
        
        CurrentInterfaceInfo.InterfaceIndex = pAdapter->IfIndex;
        CurrentInterfaceInfo.InterfaceType = pAdapter->IfType;

        if (pAdapter->PhysicalAddressLength > 0) {
            std::ostringstream oss;
            for (ULONG i = 0; i < pAdapter->PhysicalAddressLength; i++) {
                if (i != 0) oss << "-";
                oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(pAdapter->PhysicalAddress[i]);
            }
            CurrentInterfaceInfo.MACAddress = oss.str();
        }

        if (pAdapter->DnsSuffix)
            CurrentInterfaceInfo.DNSSuffix = w2s(pAdapter->DnsSuffix);
        
        for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pAdapter->FirstUnicastAddress; pUnicast != nullptr; pUnicast = pUnicast->Next) {

            char addressBuffer[INET6_ADDRSTRLEN] = { 0 };
            sockaddr* sa = pUnicast->Address.lpSockaddr;

            if (sa->sa_family == AF_INET) {
                // IPv4 address.
                sockaddr_in* sa_in = reinterpret_cast<sockaddr_in*>(sa);
                inet_ntop(AF_INET, &(sa_in->sin_addr), addressBuffer, sizeof(addressBuffer));
                CurrentInterfaceInfo.IPAddresses.push_back(addressBuffer);

                // Compute subnet mask using OnLinkPrefixLength.
                unsigned int Prefix = pUnicast->OnLinkPrefixLength; // e.g., 24 for 255.255.255.0
                unsigned int Mask = (Prefix == 0) ? 0 : (0xFFFFFFFF << (32 - Prefix));
                std::ostringstream MaskStream;
                MaskStream << ((Mask >> 24) & 0xFF) << "."
                            << ((Mask >> 16) & 0xFF) << "."
                            << ((Mask >> 8) & 0xFF) << "."
                            << (Mask & 0xFF);
                CurrentInterfaceInfo.SubnetMasks.push_back(MaskStream.str());
            }
            else if (sa->sa_family == AF_INET6) {
                // IPv6 address
                sockaddr_in6* sa_in6 = reinterpret_cast<sockaddr_in6*>(sa);
                inet_ntop(AF_INET6, &(sa_in6->sin6_addr), addressBuffer, sizeof(addressBuffer));
                CurrentInterfaceInfo.IPAddresses.push_back(addressBuffer);

                // the subnet is typically represented as the prefix length.
                std::ostringstream oss;
                oss << static_cast<int>(pUnicast->OnLinkPrefixLength);
                CurrentInterfaceInfo.SubnetMasks.push_back(oss.str()); // e.g., "64"
            }
        }

        for (PIP_ADAPTER_DNS_SERVER_ADDRESS pDNS = pAdapter->FirstDnsServerAddress; pDNS != nullptr; pDNS = pDNS->Next) {
            char DNSBuffer[INET6_ADDRSTRLEN] = { 0 };
            sockaddr* sa = pDNS->Address.lpSockaddr;
            if (sa->sa_family == AF_INET) {
                sockaddr_in* sa_in = reinterpret_cast<sockaddr_in*>(sa);
                inet_ntop(AF_INET, &(sa_in->sin_addr), DNSBuffer, sizeof(DNSBuffer));
            }
            else if (sa->sa_family == AF_INET6) {
                sockaddr_in6* sa_in6 = reinterpret_cast<sockaddr_in6*>(sa);
                inet_ntop(AF_INET6, &(sa_in6->sin6_addr), DNSBuffer, sizeof(DNSBuffer));
            }
            CurrentInterfaceInfo.DNSAddresses.push_back(DNSBuffer);
        }
        NetworkInterfaces.push_back(CurrentInterfaceInfo);
    }

	return std::nullopt;
}