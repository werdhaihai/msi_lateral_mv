// comstuff.h
#pragma once
#include <windows.h>  
#include <unknwn.h>


typedef HRESULT(*DllGetClassObjectFn)(
    const CLSID* rclsid,
    const IID*,
    LPVOID* ppv
    );

HRESULT SetupInterfaceAuth(IUnknown* iface, COAUTHINFO* pAuthInfo);
HRESULT SetupAuthOnParentIUnknownCastToIID(IUnknown* pAnyIface, COAUTHINFO* pAuthInfo, IUnknown** ppCastedOutput, const IID* riid);
IUnknown* CreateObjectFromDllFactory(HMODULE hdll, CLSID clsidFactory);