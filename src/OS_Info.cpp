#include "SysInfoProbe.hpp"
#include <chrono>

std::string SysInfoProbe::_FormatWMIDateTime(const std::string& WMIDateTime) {
	int Year = std::stoi(WMIDateTime.substr(0, 4));
	int Month = std::stoi(WMIDateTime.substr(4, 2));
	int Day = std::stoi(WMIDateTime.substr(6, 2));
	int Hour = std::stoi(WMIDateTime.substr(8, 2));
	int Minutes = std::stoi(WMIDateTime.substr(10, 2));
	int Seconds = std::stoi(WMIDateTime.substr(12, 2));

	int tzMinutes = 0;
	bool tzPlus = false;
	std::size_t tzSignPlus = WMIDateTime.find('+');
	std::size_t tzSignMinus = WMIDateTime.find('-');

	if (tzSignPlus != std::string::npos) {
		tzMinutes = std::stoi(WMIDateTime.substr(tzSignPlus + 1));
		tzPlus = true;
	}
	else if (tzSignMinus != std::string::npos) {
		tzMinutes = std::stoi(WMIDateTime.substr(tzSignMinus + 1));
		tzPlus = false;
	}

	int tzHours = tzMinutes / 60;
	int tzRemainderMinutes = tzMinutes % 60;

	std::ostringstream TimezoneStream;
	TimezoneStream << "UTC " << (tzPlus ? "+" : "-")
		<< std::setw(2) << std::setfill('0') << tzHours << ":"
		<< std::setw(2) << std::setfill('0') << tzRemainderMinutes;
	std::string Timezone = TimezoneStream.str();

	std::string AmPm = "AM";
	if (Hour >= 12) {
		AmPm = "PM";
		if (Hour > 12) Hour -= 12; 
	}
	else if (Hour == 0) Hour = 12;

	return std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02} {} {}", Year, Month, Day, Hour, Minutes, Seconds, AmPm, Timezone);
}

std::optional<Error> SysInfoProbe::GetOperatingSystemInfo() {
	const char* FuncName = "SysInfoProbe::GetOperatingSystemInfo";
	if (!bInitialized) return Error::New(FuncName, 1, L"API not initialized.");
	const std::vector<LPCWSTR> Attributes = {
		L"Name",
		L"Caption",
		L"Version",
		L"BuildNumber",
		L"OSArchitecture",
		L"InstallDate"
	};
	bstr_t WQLQuery = WMIMgr.BuildWQLQueryString(L"Win32_OperatingSystem", Attributes);
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
		hr = pClassObject->Get(L"Name", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 4, L"Failed to get Name property or property is empty.", hr);
		}
		OS.TechnicalName = w2s(vtProp.bstrVal);

		hr = pClassObject->Get(L"Caption", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 5, L"Failed to get Caption property or property is empty.", hr);
		}
		OS.Name = w2s(vtProp.bstrVal);

		hr = pClassObject->Get(L"Version", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 6, L"Failed to get Version property or property is empty.", hr);
		}
		OS.Version = w2s(vtProp.bstrVal);

		hr = pClassObject->Get(L"BuildNumber", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 7, L"Failed to get BuildNumber property or property is empty.", hr);
		}
		OS.BuildNumber = w2s(vtProp.bstrVal);

		hr = pClassObject->Get(L"OSArchitecture", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 8, L"Failed to get OSArchitecture property or property is empty.", hr);
		}
		OS.Architecture = w2s(vtProp.bstrVal);

		hr = pClassObject->Get(L"InstallDate", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 9, L"Failed to get InstallDate property or property is empty.", hr);
		}
		std::string InstallDateWMI = w2s(vtProp.bstrVal);
		OS.InstallDate = _FormatWMIDateTime(InstallDateWMI);
	}

	return std::nullopt;
}