// comstuff.c
#include <Windows.h>
#include "beacon.h"
#include "bofdefs.h"
#include "comstuff.h"

static const GUID IID_IUnknownInternal = {0x00000000, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
static const GUID IID_IClassFactoryInternal =  {0x00000001, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};

HRESULT SetupInterfaceAuth(IUnknown* iface, COAUTHINFO* pAuthInfo)
{
    WCHAR szPrincipalName[512] = { 0 };
    OLECHAR* pServerPrincName = NULL;

    if (pAuthInfo->pAuthIdentityData) {
        COAUTHIDENTITY* pAuthId = (COAUTHIDENTITY*)pAuthInfo->pAuthIdentityData;
        PCWSTR domain = (PCWSTR)pAuthId->Domain;
        PCWSTR user = (PCWSTR)pAuthId->User;

        BeaconPrintf(CALLBACK_OUTPUT, "[+] Setting up CoSetProxyBlanket with user: %ls\\%ls\n", domain ? domain : L"(null)", user ? user : L"(null)");

        if (domain && user) {
            USER32$wsprintfW(szPrincipalName, L"%ls\\%ls", domain, user);
            pServerPrincName = szPrincipalName;
        }
    }
    else {
        BeaconPrintf(CALLBACK_OUTPUT, "[-] No auth identity provided, using NULL principal.\n");
    }

    HRESULT hr = OLE32$CoSetProxyBlanket(
        iface,
        pAuthInfo->dwAuthnSvc,
        pAuthInfo->dwAuthzSvc,
        pServerPrincName,
        pAuthInfo->dwAuthnLevel,
        pAuthInfo->dwImpersonationLevel,
        pAuthInfo->pAuthIdentityData,
        EOAC_DEFAULT
    );

    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "[!] CoSetProxyBlanket  0x%08X\n", hr);
    }
    return hr;
}

HRESULT SetupAuthOnParentIUnknownCastToIID(IUnknown* pAnyIface, COAUTHINFO* pAuthInfo, IUnknown** ppCastedOutput, const IID* riid)
{
    IUnknown* pUnknown = NULL;
    BeaconPrintf(CALLBACK_OUTPUT, "[-] Starting SetupAuthOnParentIUnknownCastToIID\n");

    HRESULT hr = SetupInterfaceAuth(pAnyIface, pAuthInfo);
    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "[!] SetupInterfaceAuth (original iface) Failed 0x%08X\n", hr);
        return hr;
    }

    BeaconPrintf(CALLBACK_OUTPUT, "[-] pAnyIface = %p\n", pAnyIface);
    BeaconPrintf(CALLBACK_OUTPUT, "[-] pAnyIface->lpVtbl = %p\n", pAnyIface ? pAnyIface->lpVtbl : NULL);
    BeaconPrintf(CALLBACK_OUTPUT, "[-] QueryInterface function ptr = %p\n", 
             (pAnyIface && pAnyIface->lpVtbl) ? pAnyIface->lpVtbl->QueryInterface : NULL);
   
    BeaconPrintf(CALLBACK_OUTPUT, "[-] Querying for IID_IUnknown...\n");
    hr = pAnyIface->lpVtbl->QueryInterface(pAnyIface, &IID_IUnknownInternal, (void**)&pUnknown);

    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "[!] QueryInterface for IID_IUnknown failed 0x%08X\n", hr);
        return hr;
    }

    BeaconPrintf(CALLBACK_OUTPUT, "[-] Setting up auth on base IUnknown...\n");
    hr = SetupInterfaceAuth(pUnknown, pAuthInfo);
    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "[!] SetupInterfaceAuth (underlying IUnknown) 0x%08X\n", hr);
        pUnknown->lpVtbl->Release(pUnknown);
        return hr;
    }

    BeaconPrintf(CALLBACK_OUTPUT, "[-] Querying base IUnknown for target MsiServerIID...\n");
    hr = pUnknown->lpVtbl->QueryInterface(pUnknown, riid, (void**)ppCastedOutput);
    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "[!] QueryInterface for target IID 0x%08X\n", hr);
        pUnknown->lpVtbl->Release(pUnknown);
        return hr;
    }

    pUnknown->lpVtbl->Release(pUnknown);
    BeaconPrintf(CALLBACK_OUTPUT, "[+] Successfully obtained authenticated interface.\n");
    return SetupInterfaceAuth(*ppCastedOutput, pAuthInfo);
}

IUnknown* CreateObjectFromDllFactory(HMODULE hDll, CLSID clsidFactory) {
    BeaconPrintf(CALLBACK_OUTPUT, "[-] Attempting to create COM object via Dll factory...\n");

    IClassFactory* factory;
    
    IUnknown* requestedObject = NULL;

    DllGetClassObjectFn DllGetClassObject = (DllGetClassObjectFn)KERNEL32$GetProcAddress(hDll, "DllGetClassObject");

    if (!DllGetClassObject) {
        BeaconPrintf(CALLBACK_ERROR, "[!] Failed locating DllGetClassObject on hDll");
        return NULL;
    }
    BeaconPrintf(CALLBACK_OUTPUT, "[+] Located DllGetClassObject at address: 0x%p\n", DllGetClassObject);

    HRESULT hr = DllGetClassObject(&clsidFactory, &IID_IClassFactoryInternal, (PVOID*)&factory);
    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "[!] Failed to get com factory0x%08X\n", hr);
        return NULL;
    }

    hr = factory->lpVtbl->CreateInstance(factory, NULL, &clsidFactory, (PVOID*)&requestedObject);
    if (FAILED(hr)) {
        BeaconPrintf(CALLBACK_ERROR, "[!] CreateInstance failed: HRESULT = 0x%08X, GetLastError = %lu\n", hr, KERNEL32$GetLastError());
        factory->lpVtbl->Release(factory);
        return NULL;
    }
    
    factory->lpVtbl->Release(factory);
    return requestedObject;
}

