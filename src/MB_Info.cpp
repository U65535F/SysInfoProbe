#include "SysInfoProbe.hpp"

std::optional<Error> SysInfoProbe::GetMotherboardInfo() {
	const char* FuncName = "SysInfoProbe::GetMotherboardInfo";
	if (!bInitialized) return Error::New(FuncName, 1, L"API not initialized.");

	const std::vector<LPCWSTR> Attributes = {
		L"Manufacturer",
		L"Product",
		L"Version",
		L"SerialNumber"
	};
	bstr_t WQLQuery = WMIMgr.BuildWQLQueryString(L"Win32_BaseBoard", Attributes);

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
			return Error::New(FuncName, 4, L"Failed to get mainboard Manufacturer.", hr);
		}
		Mainboard.Manufacturer = w2s(vtProp.bstrVal);

		hr = pClassObject->Get(L"Product", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 5, L"Failed to get mainboard Product name.", hr);
		}
		Mainboard.Name = w2s(vtProp.bstrVal);

		hr = pClassObject->Get(L"Version", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 6, L"Failed to get mainboard Version.", hr);
		}
		Mainboard.Version = w2s(vtProp.bstrVal);

		hr = pClassObject->Get(L"SerialNumber", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 7, L"Failed to get mainboard SerialNumber.", hr);
		}
		Mainboard.SerialNumber = w2s(vtProp.bstrVal);
	}

	return std::nullopt;
}