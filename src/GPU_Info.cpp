#include "SysInfoProbe.hpp"

std::optional<Error> SysInfoProbe::GetGpuInfo() {
	const char* FuncName = "SysInfoProbe::GetGpuInfo";
	if (!bInitialized) return Error::New(FuncName, 1, L"API not initialized.");
	
	const std::vector<LPCWSTR> Attributes = {
		L"Name",
		L"AdapterCompatibility",
		L"DriverVersion",
		L"CurrentRefreshRate",
		L"AdapterRAM"
	};
	bstr_t WQLQuery = WMIMgr.BuildWQLQueryString(L"Win32_VideoController", Attributes);

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
			return Error::New(FuncName, 4, L"Failed to get GPU name.", hr);
		}
		GPU.Name = w2s(vtProp.bstrVal);

		hr = pClassObject->Get(L"AdapterCompatibility", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 5, L"Failed to get manufacturer name.", hr);
		}
		GPU.Manufacturer = w2s(vtProp.bstrVal);

		hr = pClassObject->Get(L"DriverVersion", 0, &vtProp, 0, 0); 
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 6, L"Failed to get driver version.", hr);
		}
		GPU.DriverVersion = w2s(vtProp.bstrVal);

		hr = pClassObject->Get(L"CurrentRefreshRate", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_I4) {
			return Error::New(FuncName, 7, L"Failed to get refresh rate.", hr);
		}
		GPU.RefreshRate = vtProp.intVal;

		hr = pClassObject->Get(L"AdapterRam", 0, &vtProp, 0, 0);
		if (FAILED(hr) || (vtProp.vt != VT_I4)) {
			return Error::New(FuncName, 8, L"Failed to get VRAM size.", hr);
		}

		// CAUTION - 4GB is the limit.
		// std::cout << vtProp.uintVal << std::endl;
		GPU.VRAMSizeInMegabytes = static_cast<int>(vtProp.uintVal / pow(1024, 2));  // BYTES -> KB -> MB = BYTES / 1024 * 1024 
		GPU.VRAMSizeInGigabytes = GPU.VRAMSizeInMegabytes / 1024;

		// TODO: Implement a much safer method to fetch VRAM - 
		// https://stackoverflow.com/questions/68274009/wmi-win32-videocontroller-ram-4gb-limit
		// https://stackoverflow.com/a/74552200 - Direct Link to Answer
	}
	
	return std::nullopt;
}