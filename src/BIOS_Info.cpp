#include "SysInfoProbe.hpp"

std::optional<Error> SysInfoProbe::GetComputerType() {
    SYSTEM_POWER_CAPABILITIES SystemPowerCapabilities;
    ZeroMemory(&SystemPowerCapabilities, sizeof(SystemPowerCapabilities));
    if (GetPwrCapabilities(&SystemPowerCapabilities) == 0)
		return Error::New("SysInfoProbe::GetComputerType", 1, L"Failed to fetch power capabilities.", GetLastError());

    ComputerType = SystemPowerCapabilities.LidPresent ? LAPTOP : DESKTOP;
    return std::nullopt;
}

std::optional<Error> SysInfoProbe::GetBIOSInfo() {
	const char* FuncName = "SysInfoProbe::GetBIOSInfo";
	if (!bInitialized) return Error::New(FuncName, 1, L"API not initialized.");

	const std::vector<LPCWSTR> Attributes = {
		L"Manufacturer",
		L"Version",
		L"BuildNumber",
		L"SerialNumber"
	};
	bstr_t WQLQuery = WMIMgr.BuildWQLQueryString(L"Win32_BIOS", Attributes);

	auto r = WMIMgr.ExecuteWQLQuery(WQLQuery);
	if (r) {
		r.value().AddNewFunctionToStack(FuncName, 2);
		return r;
	}

	auto [pLocator, pServices, pEnumerator] = WMIMgr.GetData();
	IWbemClassObject* pClassObject = NULL;
	ULONG uReturn = 0;
	VARIANT vtProp = { 0 };

	DEFER{
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

		hr = pClassObject->Get(L"Manufacturer", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 4, L"Failed to get BIOS manufacturer.", hr);
		}
		BIOS.Manufacturer = w2s(vtProp.bstrVal);

		hr = pClassObject->Get(L"Version", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 5, L"Failed to get BIOS version.", hr);
		}
		BIOS.Version = w2s(vtProp.bstrVal);

		/* TODO: fill bios actual version. e.g: f2, f62 etc. */
		/* attempted to get it directly from WMI, but it fails as always. vtProp sets itself to NULL. */

		hr = pClassObject->Get(L"SerialNumber", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 6, L"Failed to get BIOS serial number.", hr);
		}
		BIOS.SerialNumber = w2s(vtProp.bstrVal);
	}

	return std::nullopt;
}