#include "WMIMgr.hpp"

std::optional<Error> WMIManager::InitializeAPI(LPCWSTR Namespace) {
    if (bInitialized) {
        return Error::New("WMIManager::InitializeAPI", -1, L"API already initialized");
    }

    HRESULT hRes = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hRes)) {
        return Error::New("WMIManager::InitializeAPI", -2, L"Failed to initialize COM", hRes);
    }

    hRes = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    if (FAILED(hRes)) {
        CoUninitialize();
        return Error::New("WMIManager::InitializeAPI", -3, L"Failed to initialize security", hRes);
    }

    hRes = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLocator);
    if (FAILED(hRes)) {
        CoUninitialize();
        return Error::New("WMIManager::InitializeAPI", -4, L"Failed to create IWbemLocator object", hRes);
    }

    hRes = pLocator->ConnectServer(_bstr_t(Namespace), NULL, NULL, 0, NULL, 0, 0, &pServices);
    if (FAILED(hRes)) {
        Cleanup();
        return Error::New("WMIManager::InitializeAPI", -5, L"Failed to connect to WMI namespace", hRes);
    }

    hRes = CoSetProxyBlanket(pServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
    if (FAILED(hRes)) {
        Cleanup();
        return Error::New("WMIManager::InitializeAPI", -6, L"Failed to set proxy blanket", hRes);
    }

    bInitialized = true;
    return std::nullopt;
}

std::optional<Error> WMIManager::ExecuteWQLQuery(BSTR WQLQuery) {
    if (!bInitialized) {
        return Error::New("WMIManager::ExecuteWQLQuery", -1, L"API not initialized");
    }

    if (!WQLQuery) {
        return Error::New("WMIManager::ExecuteWQLQuery", -2, L"Invalid WQL query");
    }

    if (pEnumerator) {
        pEnumerator->Release();
        pEnumerator = nullptr;
    }
    HRESULT hRes = pServices->ExecQuery(bstr_t(L"WQL"), WQLQuery, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
    if (FAILED(hRes)) {
        return Error::New("WMIManager::ExecuteWQLQuery", -3, L"WQL Query Failed", hRes);
    }

    return std::nullopt;
}

bstr_t WMIManager::BuildWQLQueryString(LPCWSTR WMIClass, const std::vector<LPCWSTR>&WMIAttributes, LPCWSTR WhereClause) {
    if (!WMIClass || WMIAttributes.empty()) return bstr_t(L"");

    std::wstring query = L"SELECT ";
    for (size_t i = 0; i < WMIAttributes.size(); ++i) {
        query += WMIAttributes[i];
        if (i < WMIAttributes.size() - 1) {
            query += L", ";
        }
    }

    query += L" FROM ";
    query += WMIClass;

    if (WhereClause) {
        query += L" WHERE ";
        query += WhereClause;
    }

    return bstr_t(query.c_str());
}

std::tuple<IWbemLocator*, IWbemServices*, IEnumWbemClassObject*> WMIManager::GetData() {
    return std::make_tuple(pLocator, pServices, pEnumerator);
}

void WMIManager::Cleanup() {
    if (pEnumerator) {
        pEnumerator->Release();
        pEnumerator = nullptr;
    }

    if (pServices) {
        pServices->Release();
        pServices = nullptr;
    }

    if (pLocator) {
        pLocator->Release();
        pLocator = nullptr;
    }

    CoUninitialize();
    bInitialized = false;
}