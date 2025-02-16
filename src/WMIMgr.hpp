#pragma once
#include "Errors.hpp"       // Better error handling
#include <Windows.h>        // Windows API
#include <iostream>         // Basic C++ I/O
#include <optional>         // For std::optional
#include <comdef.h>         // For _bstr_t
#include <Wbemidl.h>        // For WMI and WQL query execution

#pragma comment(lib, "WbemUUID.lib") // Required to use Wbemidl.h

class WMIManager {
private:
    IWbemLocator* pLocator;
    IWbemServices* pServices;
    IEnumWbemClassObject* pEnumerator;
    bool bInitialized;

public:
    WMIManager() : 
        pLocator(nullptr),
        pServices(nullptr),
        pEnumerator(nullptr),
        bInitialized(false) {}

    ~WMIManager() { Cleanup(); }

    std::optional<Error> InitializeAPI(LPCWSTR Namespace = L"ROOT\\CIMV2");
    std::optional<Error> ExecuteWQLQuery(BSTR WQLQuery);
    bstr_t BuildWQLQueryString(LPCWSTR WMIClass, const std::vector<LPCWSTR>& WMIAttributes, LPCWSTR WhereClause = nullptr);
    std::tuple<IWbemLocator*, IWbemServices*, IEnumWbemClassObject*> GetData();
    void Cleanup();
};
