#include "SysInfoProbe.hpp"

std::optional<Error> SysInfoProbe::GetStorageDevices() {
	const char* FuncName = "SysInfoProbe::GetStorageDevices";
	if (!bInitialized) return Error::New(FuncName, 1, L"API not initialized.");
	const std::vector<LPCWSTR> Attributes = {
		L"Model",
		L"Manufacturer",
		L"SerialNumber",
		L"Size"
	};
	bstr_t WQLQuery = WMIMgr.BuildWQLQueryString(L"Win32_DiskDrive", Attributes);
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

		hr = pClassObject->Get(L"Model", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 4, L"Failed to get Model property or property is empty.", hr);
		}
		STORAGEDEVICEINFO CurrentDeviceInfo;
		CurrentDeviceInfo.Model = trim(w2s(vtProp.bstrVal));

		hr = pClassObject->Get(L"Manufacturer", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 5, L"Failed to get Manufacturer property or property is empty.", hr);
		}
		CurrentDeviceInfo.Manufacturer = trim(w2s(vtProp.bstrVal));

		hr = pClassObject->Get(L"SerialNumber", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 6, L"Failed to get SerialNumber property or property is empty.", hr);
		}
		CurrentDeviceInfo.SerialNumber = trim(w2s(vtProp.bstrVal));

		hr = pClassObject->Get(L"Size", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 7, L"Failed to get Size property or property is empty.", hr);
		}

		DWORD64 SizeInBytes = std::stoll(vtProp.bstrVal);
		CurrentDeviceInfo.SizeInMebibytes = static_cast<DWORD>(SizeInBytes / (1024ULL * 1024ULL));
		CurrentDeviceInfo.SizeInGibibytes = CurrentDeviceInfo.SizeInMebibytes / 1024;
		StorageDevices.push_back(CurrentDeviceInfo);
	}
	return std::nullopt;
}