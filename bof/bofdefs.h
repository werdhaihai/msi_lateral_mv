// bofdefs.h
#pragma once
//#include <windows.h>

#ifdef BOF

// KERNEL32
WINBASEAPI DWORD   WINAPI   KERNEL32$GetLastError (VOID);
WINBASEAPI HMODULE WINAPI   KERNEL32$LoadLibraryW (LPCWSTR lpLibFileName);
WINBASEAPI FARPROC WINAPI   KERNEL32$GetProcAddress (HMODULE hModule, LPCSTR lpProcName);
WINBASEAPI BOOL    WINAPI   KERNEL32$FreeLibrary (HMODULE hModule);
WINBASEAPI LPWCH   WINAPI   KERNEL32$GetEnvironmentStringsW();
WINBASEAPI BOOL    WINAPI   KERNEL32$FreeEnvironmentStringsW(LPWCH pEnv);
WINBASEAPI void *  WINAPI   KERNEL32$HeapAlloc (HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
WINBASEAPI HANDLE  WINAPI   KERNEL32$GetProcessHeap();
WINBASEAPI BOOL    WINAPI   KERNEL32$HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);

// OLE32
DECLSPEC_IMPORT HRESULT WINAPI OLE32$CoInitialize (LPVOID pvReserved);
DECLSPEC_IMPORT HRESULT WINAPI OLE32$CoUninitialize (void);
DECLSPEC_IMPORT HRESULT WINAPI OLE32$CoInitializeSecurity(PSECURITY_DESCRIPTOR pSecDesc, LONG cAuthSvc, SOLE_AUTHENTICATION_SERVICE *asAuthSvc, void *pReserved1, DWORD dwAuthnLevel, DWORD dwImpLevel, void *pAuthList, DWORD dwCapabilities, void *pReserved3);
DECLSPEC_IMPORT HRESULT WINAPI OLE32$CoCreateInstanceEx(REFCLSID rClsid, LPUNKNOWN pUnkOuter, DWORD dwClsCtx, COSERVERINFO* pServerInfo, DWORD dwCount, MULTI_QI* pResults);
DECLSPEC_IMPORT HRESULT WINAPI OLE32$CoSetProxyBlanket(IUnknown* pProxy, DWORD dwAuthnSvc, DWORD dwAuthzSvc, OLECHAR* pServerPrincName, DWORD dwAuthnLevel, DWORD dwImpLevel, RPC_AUTH_IDENTITY_HANDLE pAuthInfo, DWORD dwCapabilities);

// SHLWAPI
DECLSPEC_IMPORT LPCWSTR WINAPI SHLWAPI$PathFindFileNameW(LPCWSTR pszPath);
DECLSPEC_IMPORT BOOL    WINAPI SHLWAPI$PathRemoveFileSpecW(LPWSTR pszPath);

// USER32
WINBASEAPI int WINAPI USER32$wsprintfW(LPWSTR  pwszOut, LPCWSTR pwszFmt, ...);

// MSVCRT
// WINBASEAPI void   __cdecl MSVCRT$memset(void *dest, int c, size_t count);
WINBASEAPI size_t __cdecl MSVCRT$wcslen(const wchar_t *_Str);

#else
// C defs
#define KERNEL32$GetLastError               GetLastError
#define KERNEL32$LoadLibraryW               LoadLibraryW
#define KERNEL32$GetProcAddress             GetProcAddress
#define KERNEL32$FreeLibrary                FreeLibrary
#define KERNEL32$GetEnvironmentStringsW     GetEnvironmentStringsW
#define KERNEL32$FreeEnvironmentStringsW    FreeEnvironmentStringsW
#define KERNEL32$HeapAlloc                  HeapAlloc 
#define KERNEL32$GetProcessHeap             GetProcessHeap
#define KERNEL32$HeapFree                   HeapFree

#define OLE32$CoInitialize                  CoInitialize
#define OLE32$CoUninitialize                CoUninitialize
#define OLE32$CoInitializeSecurity          CoInitializeSecurity
#define OLE32$CoCreateInstanceEx            CoCreateInstanceEx
#define OLE32$CoSetProxyBlanket             CoSetProxyBlanket

//#define SHLWAPI$PathFindFileNameW           PathFindFileNameW
#define SHLWAPI$PathRemoveFileSpecW         PathRemoveFileSpecW         

#define USER32$wsprintfW                    wsprintfW

// #define MSVCRT$memset                       memset
#define MSVCRT$wcslen                       wcslen

#define BeaconPrintf(x, y, ...)             printf(y, ##__VA_ARGS__)

#endif