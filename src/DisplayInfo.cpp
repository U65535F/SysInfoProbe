#include "SysInfoProbe.hpp"

std::optional<Error> SysInfoProbe::_GetRealMonitorSize() {
	const char* FuncName = "SysInfoProbe::_GetRealMonitorSize";
	if (!bInitialized) return Error::New(FuncName, 1, L"API not initialized.");
	const std::vector<LPCWSTR> Attributes = {
		L"MaxHorizontalImageSize",
		L"MaxVerticalImageSize",
	};
	WMIMgr.Cleanup();
	auto r = WMIMgr.InitializeAPI(L"ROOT\\WMI");
	if (r) {
		WMIMgr.Cleanup();
		r.value().AddNewFunctionToStack(FuncName, 2);
		return r;
	}

	bstr_t WQLQuery = WMIMgr.BuildWQLQueryString(L"WmiMonitorBasicDisplayParams", Attributes);
	r = WMIMgr.ExecuteWQLQuery(WQLQuery);
	if (r) {
		WMIMgr.Cleanup();
		r.value().AddNewFunctionToStack(FuncName, 3);
		return r;
	}

	auto [pLocator, pServices, pEnumerator] = WMIMgr.GetData();
	IWbemClassObject* pClassObject = NULL;
	ULONG uReturn = 0;
	VARIANT vtProp = { 0 };
	int index = 0;

	DEFER{
		VariantClear(&vtProp);
		if (pClassObject) pClassObject->Release();
		WMIMgr.Cleanup();
	};

	while (pEnumerator) {
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pClassObject, &uReturn);
		if (FAILED(hr)) {
			return Error::New(FuncName, 4, L"Failed to fetch next WMI object.", hr);
		}
		if (uReturn == 0) break;
		VariantInit(&vtProp);

		hr = pClassObject->Get(L"MaxHorizontalImageSize", 0, &vtProp, 0, 0);
		if (FAILED(hr) || (vtProp.vt != VT_UINT && vtProp.vt != VT_I4 && vtProp.vt != VT_UI1)) {
			return Error::New(FuncName, 5, L"Failed to get MaxHorizontalImageSize property or property is empty.", hr);
		}
		unsigned int HorizontalSizeCm = vtProp.uintVal;

		hr = pClassObject->Get(L"MaxVerticalImageSize", 0, &vtProp, 0, 0);
		if (FAILED(hr) || (vtProp.vt != VT_UINT && vtProp.vt != VT_I4 && vtProp.vt != VT_UI1)) {
			return Error::New(FuncName, 6, L"Failed to get MaxVerticalImageSize property or property is empty.", hr);
		}
		unsigned int VerticalSizeCm = vtProp.uintVal;

		double HorizontalSizeInches = HorizontalSizeCm * 0.393701;
		double VerticalSizeInches = VerticalSizeCm * 0.393701;
		double DiagonalInches = std::sqrt(HorizontalSizeInches * HorizontalSizeInches + VerticalSizeInches * VerticalSizeInches);
		
		if (Displays.size() < index + 1) {
			Displays.resize(index + 1);
		}
		
		Displays[index].ScreenSizeInch = DiagonalInches;
		index++;
	}

	return std::nullopt;
}

std::optional<Error> SysInfoProbe::GetDisplayInfo() {
	const char* FuncName = "SysInfoProbe::GetDisplayInfo";
	if (!bInitialized) return Error::New(FuncName, 1, L"API not initialized.");
	const std::vector<LPCWSTR> Attributes = {
		L"Name",
		L"MonitorManufacturer"
	};

	bstr_t WQLQuery = WMIMgr.BuildWQLQueryString(L"Win32_DesktopMonitor", Attributes);
	auto r = WMIMgr.ExecuteWQLQuery(WQLQuery);
	if (r) {
		r.value().AddNewFunctionToStack(FuncName, 2);
		return r;
	}

	auto [pLocator, pServices, pEnumerator] = WMIMgr.GetData();
	IWbemClassObject* pClassObject = NULL;
	ULONG uReturn = 0;
	VARIANT vtProp = { 0 };
	DISPLAYINFO CurrentDisplay;

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
		CurrentDisplay.MonitorName = trim(w2s(vtProp.bstrVal));

		hr = pClassObject->Get(L"MonitorManufacturer", 0, &vtProp, 0, 0);
		if (FAILED(hr) || vtProp.vt != VT_BSTR || vtProp.bstrVal == NULL) {
			return Error::New(FuncName, 5, L"Failed to get MonitorManufacturer property or property is empty.", hr);
		}
		CurrentDisplay.MonitorManufacturer = trim(w2s(vtProp.bstrVal));
		Displays.push_back(CurrentDisplay);
	}

	DISPLAY_DEVICE DisplayDevice = { sizeof(DisplayDevice) };
	int DeviceIndex = 0, index = 0;

	while (EnumDisplayDevicesW(NULL, DeviceIndex, &DisplayDevice, 0))
	{
		if (!(DisplayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE))
		{
			DeviceIndex++;
			continue;
		}

		DEVMODE DeviceMode = { sizeof(DeviceMode) };
		if (!EnumDisplaySettingsW(DisplayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &DeviceMode))
		{
			DeviceIndex++;
			continue;
		}

		DEVMODE MaxDeviceMode = { sizeof(MaxDeviceMode) };

		DWORD MaxWidth = 0, MaxHeight = 0;
		int ModeIndex = 0;
		while (EnumDisplaySettingsW(DisplayDevice.DeviceName, ModeIndex, &MaxDeviceMode))
		{
			if (MaxDeviceMode.dmPelsWidth * MaxDeviceMode.dmPelsHeight > MaxWidth * MaxHeight)
			{
				MaxWidth = MaxDeviceMode.dmPelsWidth;
				MaxHeight = MaxDeviceMode.dmPelsHeight;
			}
			ModeIndex++;
		}

		Displays[index].RefreshRate = DeviceMode.dmDisplayFrequency;
		Displays[index].ScreenWidth = DeviceMode.dmPelsWidth;
		Displays[index].MaxWidthRes = MaxWidth;
		Displays[index].MaxHeightRes = MaxHeight;
		Displays[index].ScreenHeight = DeviceMode.dmPelsHeight;
		DeviceIndex++;
		index++;
	}
	
	r = _GetRealMonitorSize();
	auto r2 = WMIMgr.InitializeAPI();
	if (r2) {
		r2.value().AddNewFunctionToStack(FuncName, 6);
	}
	else if (r) {
		r.value().AddNewFunctionToStack(FuncName, 7);
		return r;
	}

	return std::nullopt;
}