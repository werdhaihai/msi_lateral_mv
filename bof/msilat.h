// msilat.h
#pragma once

#include <windows.h>
#include <unknwn.h>

#define COOKIE_SIZE 16

// GUID/IID definitions
GUID CLSID_MsiServer                = { 0x000c101c,0x0000,0x0000,{0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46} };
IID  IID_IMsiServer                 = { 0x000c101c,0x0000,0x0000,{0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46} };
GUID CLSID_MSIRemoteApi             = { 0x000c1035,0x0000,0x0000,{0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46} };
IID  IID_IMsiRemoteAPI              = { 0x000C1033,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46} };
GUID CLSID_IMsiCustomAction         = { 0x000c1025,0x0000,0x0000,{0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46} };
IID  IID_IMsiCustomAction           = { 0x000c1025,0x0000,0x0000,{0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46} };
IID  IID_IMsiConfigurationManager   = { 0x000C101B,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46} };

// Forwards
typedef struct IMsiMessage IMsiMessage;
typedef struct IMsiRecord IMsiRecord;

// typedefs
typedef wchar_t ICHAR;

// MSI Enums 
typedef enum iesEnum {
    iesNoAction = 0,  
    iesSuccess = 1,  
    iesUserExit = 2,  
    iesFailure = 3,  
    iesSuspend = 4,  
    iesFinished = 5,
    iesWrongState = 6,
    iesBadActionData = 7,
    iesInstallRunning = 8,
    iesNextEnum
} iesEnum;

typedef enum ireEnum {
    ireProductCode,
    irePackagePath,
    ireSubStorage,
    ireDatabaseHandle,
    ireInstallFinalize,
} ireEnum;

typedef enum iioEnum {
    iioUpgrade = 0x00000001,
    iioChild = 0x00000002,
    iioDisablePlatformValidation = 0x00000004,
    iioEndDialog = 0x00000008,
    iioCreatingAdvertiseScript = 0x00000010,
    iioDisableRollback = 0x00000020,
    iioMustAccessInstallerKey = 0x00000040,
    iioReinstallModePackage = 0x00000080,
    iioClientEngine = 0x00000100,
    iioSimulateX86 = 0x00000200,
    iioSimulateIA64 = 0x00000400,
    iioRestrictedEngine = 0x00000800,
    iioSimulateAMD64 = 0x00001000,
    iioPatchApplication = 0x00002000,
} iioEnum;

typedef enum isrcEnum {
    isrcNet = 0,
    isrcMedia = 1,
    isrcURL = 2,
} isrcEnum;

typedef enum icacCustomActionContext {
    icacFirst = 0,
    icac32Impersonated = 0,
    icac64Impersonated = 1,
    icac32Elevated = 2,
    icac64Elevated = 3,
    icacNext = 4,
} icacCustomActionContext;

//
// IMsiRemoteAPI
//

typedef struct IMsiRemoteAPI IMsiRemoteAPI;

typedef struct IMsiRemoteAPIVtbl {
    HRESULT (*QueryInterface)(IMsiRemoteAPI* This, REFIID riid, void** ppvObject);
    ULONG (*AddRef)(IMsiRemoteAPI* This);
    ULONG (*Release)(IMsiRemoteAPI* This);
} IMsiRemoteAPIVtbl;

typedef struct IMsiRemoteAPI {
    IMsiRemoteAPIVtbl* lpVtbl;
} IMsiRemoteAPI;

//
// IMsiCustomAction
//
typedef struct IMsiCustomAction IMsiCustomAction;

typedef struct IMsiCustomActionVtbl {
    HRESULT (*QueryInterface)(IMsiCustomAction* This, REFIID riid, void** ppvObject);
    ULONG (*AddRef)(IMsiCustomAction* This);
    ULONG (*Release)(IMsiCustomAction* This);
    HRESULT (*PrepareDLLCustomAction)(IMsiCustomAction* This, const ICHAR* szActionName,
        const ICHAR* szPath, const ICHAR* szEntryPoint, unsigned long hmsi,
        boolean fDebugBreak, boolean fAppCompat, const GUID* pguidAppCompatDB,
        const GUID* pguidAppCompatID, DWORD* dwThreadId);
    HRESULT (*RunDLLCustomAction)(IMsiCustomAction* This, DWORD dwThreadId, unsigned long* pulRet);
    HRESULT (*FinishDLLCustomAction)(IMsiCustomAction* This, DWORD dwThreadId);
    HRESULT (*RunScriptAction)(IMsiCustomAction* This, int icaType, IDispatch* piEngine, const ICHAR* szSource,
        const ICHAR* szTarget, LANGID iLangId, int* iScriptResult, int* pcb, char** pchRecord);
    HRESULT (*QueryPathOfRegTypeLib)(IMsiCustomAction* This, REFGUID guid, unsigned short wVerMajor, unsigned short wVerMinor,
        LCID lcid, OLECHAR* lpszPathName, int cchPath);
    HRESULT (*ProcessTypeLibrary)(IMsiCustomAction* This, const OLECHAR* szLibID, LCID lcidLocale,
        const OLECHAR* szTypeLib, const OLECHAR* szHelpPath, int fRemove, int* fInfoMismatch);
    HRESULT (*SQLInstallDriverEx)(IMsiCustomAction* This, int cDrvLen, const ICHAR* szDriver,
        const ICHAR* szPathIn, ICHAR* szPathOut, WORD cbPathOutMax,
        WORD* pcbPathOut, WORD fRequest, DWORD* pdwUsageCount, int* rawReturnCode);
    HRESULT (*SQLConfigDriver)(IMsiCustomAction* This, WORD fRequest, const ICHAR* szDriver,
        const ICHAR* szArgs, ICHAR* szMsg, WORD cbMsgMax, WORD* pcbMsgOut, int* fnResult);
    HRESULT (*SQLRemoveDriver)(IMsiCustomAction* This, const ICHAR* szDriver, int fRemoveDSN, DWORD* pdwUsageCount);
    HRESULT (*SQLInstallTranslatorEx)(IMsiCustomAction* This, int cTransLen, const ICHAR* szTranslator,
        const ICHAR* szPathIn, ICHAR* szPathOut, WORD cbPathOutMax,
        WORD* pcbPathOut, WORD fRequest, DWORD* pdwUsageCount, int* rawReturnCode);
    HRESULT (*SQLRemoveTranslator)(IMsiCustomAction* This, const ICHAR* szTranslator, DWORD* pdwUsageCount, int* rawReturnCode);
    HRESULT (*SQLConfigDataSource)(IMsiCustomAction* This, WORD fRequest, const ICHAR* szDriver,
        const ICHAR* szAttributes, DWORD cbAttrSize, int* rawReturnCode);
    HRESULT (*SQLInstallDriverManager)(IMsiCustomAction* This, ICHAR* szPath, WORD cbPathMax, WORD* pcbPathOut);
    HRESULT (*SQLRemoveDriverManager)(IMsiCustomAction* This, DWORD* pdwUsageCount);
    HRESULT (*SQLInstallerError)(IMsiCustomAction* This, WORD iError, DWORD* pfErrorCode,
        ICHAR* szErrorMsg, WORD cbErrorMsgMax, WORD* pcbErrorMsg);
    HRESULT (*URTMakeFusionFullPath)(IMsiCustomAction* This, USHORT const*, USHORT*, ULONG, int*);
    HRESULT (*URTCarryingNDP)(IMsiCustomAction* This, int);
    HRESULT (*URTUnloadFusionBinaries)(IMsiCustomAction* This);
    HRESULT (*URTAddAssemblyInstallComponent)(IMsiCustomAction* This, wchar_t const* UserDefinedGuid1, wchar_t const* UserDefinedGuid2, wchar_t const* UserDefinedName);
    HRESULT (*URTIsAssemblyInstalled)(IMsiCustomAction* This, USHORT const*, USHORT const*, int*, int*, char**);
    HRESULT (*URTProvideGlobalAssembly)(IMsiCustomAction* This, wchar_t const* AsmName, DWORD InstallMode, wchar_t* AsmPath);
    HRESULT (*URTCommitAssemblies)(IMsiCustomAction* This, wchar_t const* UserDefinedGuid1, int* pInt, char** pStr);
    HRESULT (*URTUninstallAssembly)(IMsiCustomAction* This, USHORT const*, USHORT const*, int*, char**);
    HRESULT (*URTGetAssemblyCacheItem)(IMsiCustomAction* This, wchar_t const* UserDefinedGuid1, wchar_t const* UserDefinedGuid2, ULONG zeroed, int* pInt, char** pStr);
    HRESULT (*URTCreateAssemblyFileStream)(IMsiCustomAction* This, wchar_t const* FileName, int Format);
    HRESULT (*URTWriteAssemblyBits)(IMsiCustomAction* This, const char* pv, ULONG cb, ULONG* pcbWritten);
    HRESULT (*URTCommitAssemblyStream)(IMsiCustomAction* This);
    HRESULT (*URTGetFusionPath)(IMsiCustomAction* This, USHORT const*, int, USHORT*, ULONG, ULONG*, USHORT*, ULONG, int*);
    HRESULT (*URTAreAssembliesEqual)(IMsiCustomAction* This, USHORT const*, USHORT const*, int*, int*, char**);
    HRESULT (*URTQueryAssembly)(IMsiCustomAction* This, ICHAR const*, USHORT const*, ULONG, int*, char**);
    HRESULT (*LoadEmbeddedDLL)(IMsiCustomAction* This, ICHAR const* path, BOOL debug);
    HRESULT (*CallInitDLL)(IMsiCustomAction* This, ULONG intVar, PVOID pVar, ULONG* pInt, ULONG* pExportReturnCode);
    HRESULT (*CallMessageDLL)(IMsiCustomAction* This, UINT, ULONG, ULONG*);
    HRESULT (*CallShutdownDLL)(IMsiCustomAction* This, ULONG*);
    HRESULT (*UnloadEmbeddedDLL)(IMsiCustomAction* This);
    HRESULT (*SetNewClientProcess)(IMsiCustomAction* This, ULONG, IMsiRemoteAPI*);
    HRESULT (*SetRemoteAPI)(IMsiCustomAction* This, IMsiRemoteAPI* pIMsiRemoteAPI);
} IMsiCustomActionVtbl;

typedef struct IMsiCustomAction {
    IMsiCustomActionVtbl* lpVtbl;
} IMsiCustomAction;


//
// IMsiConfigurationManager
//

typedef struct IMsiConfigurationManager IMsiConfigurationManager;

typedef struct IMsiConfigurationManagerVtbl {
    HRESULT (*QueryInterface)(IMsiConfigurationManager* This, REFIID riid, void** ppvObject);
    ULONG (*AddRef)(IMsiConfigurationManager* This);
    ULONG (*Release)(IMsiConfigurationManager* This);
    iesEnum (*InstallFinalize)(IMsiConfigurationManager* This, iesEnum iesState, void* riMessage, boolean fUserChangedDuringInstall);
    IMsiRecord* (*SetLastUsedSource)(IMsiConfigurationManager* This, const ICHAR* szProductCode, const wchar_t* szPath, boolean fAddToList, boolean fPatch);
    boolean (*Reboot)(IMsiConfigurationManager* This);
    int (*DoInstall)(IMsiConfigurationManager* This, ireEnum ireProductCode, const ICHAR* szProduct, const ICHAR* szAction,
                     const ICHAR* szCommandLine, const ICHAR* szLogFile, int iLogMode, boolean fFlushEachLine, 
                     IMsiMessage* riMessage, iioEnum iioOptions, ULONG, HWND*, IMsiRecord*);
    HRESULT (*IsServiceInstalling)(IMsiConfigurationManager* This);
    IMsiRecord* (*RegisterUser)(IMsiConfigurationManager* This, const ICHAR* szProductCode, const ICHAR* szUserName, const ICHAR* szCompany, const ICHAR* szProductID);
    IMsiRecord* (*RemoveRunOnceEntry)(IMsiConfigurationManager* This, const ICHAR* szEntry);
    boolean (*CleanupTempPackages)(IMsiConfigurationManager* This, IMsiMessage* riMessage, BOOL flag);
    HRESULT (*SourceListClearByType)(IMsiConfigurationManager* This, const ICHAR* szProductCode, const ICHAR*, isrcEnum isrcType);
    HRESULT (*SourceListAddSource)(IMsiConfigurationManager* This, const ICHAR* szProductCode, const ICHAR* szUserName, isrcEnum isrcType, const ICHAR* szSource);
    HRESULT (*SourceListClearLastUsed)(IMsiConfigurationManager* This, const ICHAR* szProductCode, const ICHAR* szUserName);
    HRESULT (*RegisterCustomActionServer)(IMsiConfigurationManager* This, icacCustomActionContext* picacContext, const char* rgchCookie, const int cbCookie, 
                                        IMsiCustomAction* piCustomAction, unsigned long* dwProcessId, IMsiRemoteAPI** piRemoteAPI, DWORD* dwPrivileges);
    HRESULT (*CreateCustomActionServer)(IMsiConfigurationManager* This, const icacCustomActionContext icacContext, const unsigned long dwProcessId, 
                                       IMsiRemoteAPI* piRemoteAPI, const WCHAR* pvEnvironment, DWORD cchEnvironment, DWORD dwPrivileges, 
                                       char* rgchCookie, int* cbCookie, IMsiCustomAction** piCustomAction, unsigned long* dwServerProcessId, BOOL unkFalse);
} IMsiConfigurationManagerVtbl;

typedef struct IMsiConfigurationManager {
    IMsiConfigurationManagerVtbl* lpVtbl;
} IMsiConfigurationManager;



