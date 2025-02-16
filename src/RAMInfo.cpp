#include "SysInfoProbe.hpp"

std::optional<Error> SysInfoProbe::GetRamInfo() {
	const char* FuncName = "SysInfoProbe::GetRamInfo";
	if (!bInitialized) return Error::New(FuncName, 1, L"API not initialized.");
	const std::vector<LPCWSTR> Attributes = {
		L"Capacity",
		L"Manufacturer",
		L"PartNumber",
		L"SerialNumber",
		L"ConfiguredClockSpeed",
		L"FormFactor",
		L"MemoryType",
		L"Speed"
	};
	bstr_t WQLQuery = WMIMgr.BuildWQLQueryString(L"Win32_PhysicalMemory", Attributes);
	auto r = WMIMgr.ExecuteWQLQuery(WQLQuery);
	if (r) {
		r.value().AddNewFunctionToStack(FuncName, 2);
		return r;
	} 

	auto [pLocator, pServices, pEnumerator] = WMIMgr.GetData();
	IWbemClassObject* pClassObject = NULL;
	ULONG uReturn = 0;
	VARIANT vtProp = { 0 };

	/* TODO: potential memory leak; please revisit this. */
	/* possibility of improper variant handling. */
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

		hr = pClassObject->Get(L"Capacity", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 4, L"Failed to get Capacity property or property is empty.", hr);
		}

		unsigned long long llRamBytes = std::stoll(vtProp.bstrVal);
		double RamSizeInMegabytes = llRamBytes / pow(1024, 2);  // BYTES -> KB -> MB = BYTES / 1024 * 1024 
		RAM.SizeInMegabytes = static_cast<int>(RamSizeInMegabytes);
		RAM.SizeInGigabytes = RAM.SizeInMegabytes / 1024;

		hr = pClassObject->Get(L"Manufacturer", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 5, L"Failed to get Manufacturer property or property is empty.", hr);
		}
		RAM.Manufacturer = trim(w2s(vtProp.bstrVal));

		hr = pClassObject->Get(L"PartNumber", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 6, L"Failed to get PartNumber property or property is empty.", hr);
		}
		RAM.Model = trim(w2s(vtProp.bstrVal));
		RAM.Name = trim(std::string(RAM.Manufacturer + " " + RAM.Model));

		hr = pClassObject->Get(L"SerialNumber", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 7, L"Failed to get SerialNumber property or property is empty.", hr);
		}
		RAM.SerialNumber = w2s(vtProp.bstrVal);

		hr = pClassObject->Get(L"ConfiguredClockSpeed", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_I4) {
			return Error::New(FuncName, 8, L"Failed to get ConfiguredClockSpeed property or property is empty.", hr);
		}
		RAM.FrequencyInMHz = vtProp.intVal;

		hr = pClassObject->Get(L"FormFactor", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_I4) {
			return Error::New(FuncName, 9, L"Failed to get FormFactor property or property is empty.", hr);
		}
		RAM.FormFactor = RAMFormFactors[vtProp.intVal];

		hr = pClassObject->Get(L"MemoryType", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_I4) {
			return Error::New(FuncName, 10, L"Failed to get MemoryType property or property is empty.", hr);
		}
		RAM.MemoryType = RAMMemoryTypes[vtProp.intVal];

		hr = pClassObject->Get(L"Speed", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_I4) {
			return Error::New(FuncName, 11, L"Failed to get Speed property or property is empty.", hr);
		}
		RAM.LatencyInNanoseconds = vtProp.intVal;
	}

	return std::nullopt;
}