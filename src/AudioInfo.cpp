#include "SysInfoProbe.hpp"

std::optional<Error> SysInfoProbe::GetSoundInfo() {
	const char* FuncName = "SysInfoProbe::GetSoundInfo";
	if (!bInitialized) return Error::New(FuncName, 1, L"API not initialized.");
	const std::vector<LPCWSTR> Attributes = {
		L"Caption"
	};

	bstr_t WQLQuery = WMIMgr.BuildWQLQueryString(L"Win32_SoundDevice", Attributes);
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

		hr = pClassObject->Get(L"Caption", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 4, L"Failed to get audio Caption property or property is empty.", hr);
		}
		Sound.Name = trim(w2s(vtProp.bstrVal));
	}

	return std::nullopt;
}