#include "SysInfoProbe.hpp"

std::optional<Error> SysInfoProbe::RefreshCPUUtilizations() {
	const char* FuncName = "SysInfoProbe::RefreshCPUUtilizations";
	if (!bInitialized) return Error::New(FuncName, 1, L"API not initialized.");
	const std::vector<LPCWSTR> Attributes = {
		L"PercentProcessorPerformance",
		L"PercentProcessorUtility",
	};
	bstr_t WQLQuery = WMIMgr.BuildWQLQueryString(L"Win32_PerfFormattedData_Counters_ProcessorInformation", Attributes);
	auto r = WMIMgr.ExecuteWQLQuery(WQLQuery);
	if (r) {
		r.value().AddNewFunctionToStack(FuncName, 2);
		return r;
	}

	auto [pLocator, pServices, pEnumerator] = WMIMgr.GetData();
	IWbemClassObject* pClassObject = NULL;
	ULONG uReturn = 0;
	VARIANT vtProp = { 0 };
	
	DEFER {
		VariantClear(&vtProp);
		if (pClassObject) pClassObject->Release();
	};

	while (pEnumerator) {
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pClassObject, &uReturn);
		if (FAILED(hr)) {
			return Error::New(FuncName, 3, L"Failed to fetch next WMI object.", hr);
		}
		if (uReturn == 0) break;
		VariantInit(&vtProp);
		
	}

	/* TODO: add actual code. */
	return std::nullopt;
}

std::optional<Error> SysInfoProbe::_GetCPUCache() {
	const char* FuncName = "SysInfoProbe::_GetCPUCache";
	DWORD dwBufferSize = 0;
	GetLogicalProcessorInformationEx(RelationCache, nullptr, &dwBufferSize);

	if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
		return Error::New(FuncName, 1, L"Failed to retrieve buffer size.", GetLastError());
	}

	std::vector<BYTE> Buffer(dwBufferSize);
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX pSysLogicalProcessorInformationInfo = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(Buffer.data());

	if (!GetLogicalProcessorInformationEx(RelationCache, pSysLogicalProcessorInformationInfo, &dwBufferSize)) {
		return Error::New(FuncName, 2, L"Failed to retrieve RelationCache information.", GetLastError());
	}

	size_t szOffset = 0;
	while (szOffset < dwBufferSize) {
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX pSysLogicalProcessorInfoEntry = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(Buffer.data() + szOffset);
		szOffset += pSysLogicalProcessorInfoEntry->Size;
		if (pSysLogicalProcessorInfoEntry->Relationship != RelationCache) continue;

		switch (pSysLogicalProcessorInfoEntry->Cache.Level) {
		case 1: CPU.Cache.L1 = pSysLogicalProcessorInfoEntry->Cache.CacheSize / 1024; break;
		case 2: CPU.Cache.L2 = pSysLogicalProcessorInfoEntry->Cache.CacheSize / 1024; break;
		case 3: CPU.Cache.L3 = pSysLogicalProcessorInfoEntry->Cache.CacheSize / 1024; break;
		}
	}

	return std::nullopt;
}

// MSFT has a pretty neat example on https://learn.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex?view=msvc-170
// On using the cpuid function and for fetching all CPU instructions including the very obscure ones (which this doesn't have).
void SysInfoProbe::_GetCPUInstructions() {
	std::vector<std::string> Instructions;
	int CpuInfo[4] = { 0 };

	// Basic features (EAX=1)
	__cpuid(CpuInfo, 1);
	int ecx = CpuInfo[2];
	int edx = CpuInfo[3];

	if (edx & (1 << 23)) Instructions.push_back("MMX");
	if (edx & (1 << 25)) Instructions.push_back("SSE");
	if (edx & (1 << 26)) Instructions.push_back("SSE2");
	if (ecx & (1 << 0)) Instructions.push_back("SSE3");
	if (ecx & (1 << 9)) Instructions.push_back("SSSE3");
	if (ecx & (1 << 19)) Instructions.push_back("SSE4.1");
	if (ecx & (1 << 20)) Instructions.push_back("SSE4.2");
	if (ecx & (1 << 28)) Instructions.push_back("AVX");
	if (ecx & (1 << 25)) Instructions.push_back("AES");
	if (ecx & (1 << 5)) Instructions.push_back("VT-x");

	// Extended features (EAX=0x80000001)
	__cpuid(CpuInfo, 0x80000001);
	int extEcx = CpuInfo[2];
	int extEdx = CpuInfo[3];

	if (extEdx & (1 << 29)) Instructions.push_back("EM64T");
	if (extEcx & (1 << 5)) Instructions.push_back("LZCNT");
	if (extEcx & (1 << 8)) Instructions.push_back("PREFETCHW");

	// AVX2, BMI1, BMI2, etc. (EAX=7)
	__cpuid(CpuInfo, 7);
	int ebx = CpuInfo[1];
	if (ebx & (1 << 5)) Instructions.push_back("AVX2");
	if (ebx & (1 << 3)) Instructions.push_back("BMI1");
	if (ebx & (1 << 8)) Instructions.push_back("BMI2");
	if (ebx & (1 << 18)) Instructions.push_back("RDSEED");
	if (ebx & (1 << 29)) Instructions.push_back("SHA");

	CPU.Instructions = Instructions;
}

std::optional<Error> SysInfoProbe::GetCpuInfo() {
	const char* FuncName = "SysInfoProbe::GetCpuInfo";
	if (!bInitialized) return Error::New(FuncName, 1, L"API not initialized.");
	const std::vector<LPCWSTR> Attributes = {
		L"Name",
		L"Manufacturer",
		L"NumberOfCores",
		L"NumberOfLogicalProcessors",
		L"MaxClockSpeed",
	};
	bstr_t WQLQuery = WMIMgr.BuildWQLQueryString(L"Win32_Processor", Attributes);
	auto r = WMIMgr.ExecuteWQLQuery(WQLQuery);
	if (r) {
		r.value().AddNewFunctionToStack(FuncName, 2);
		return r;
	}

	auto [pLocator, pServices, pEnumerator] = WMIMgr.GetData();
	IWbemClassObject* pClassObject = NULL;
	ULONG uReturn = 0;
	VARIANT vtProp = { 0 };

	DEFER {
		VariantClear(&vtProp);
		if (pClassObject) pClassObject->Release();
	};

	while (pEnumerator) {
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pClassObject, &uReturn);
		if (FAILED(hr)) {
			return Error::New(FuncName, 3, L"Failed to fetch next WMI object.", hr);
		}
		if (uReturn == 0) break;
		VariantInit(&vtProp);

		hr = pClassObject->Get(L"Name", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 4, L"Failed to get Name property or property is empty.", hr);
		}
		CPU.Name = trim(w2s(vtProp.bstrVal));

		hr = pClassObject->Get(L"Manufacturer", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 5, L"Failed to get Manufacturer property or property is empty.", hr);
		}
		CPU.Manufacturer = trim(w2s(vtProp.bstrVal));

		hr = pClassObject->Get(L"NumberOfCores", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_I4) {
			return Error::New(FuncName, 6, L"Failed to get NumberOfCores property or property is empty.", hr);
		}
		CPU.CoreCount = vtProp.intVal;

		hr = pClassObject->Get(L"NumberOfLogicalProcessors", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_I4) {
			return Error::New(FuncName, 7, L"Failed to get NumberOfLogicalProcessors property or property is empty.", hr);
		}
		CPU.ThreadCount = vtProp.intVal;

		hr = pClassObject->Get(L"MaxClockSpeed", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_I4) {
			return Error::New(FuncName, 8, L"Failed to get MaxClockSpeed property or property is empty.", hr);
		}
		CPU.MaxClockSpeed = vtProp.intVal;
	}

	_GetCPUInstructions();
	r = _GetCPUCache();
	if (r) {
		r.value().AddNewFunctionToStack(FuncName, 9);
		return r;
	}
	r = RefreshCPUUtilizations();
	if (r) {
		r.value().AddNewFunctionToStack(FuncName, 10);
		return r;
	}

	return std::nullopt;
}
