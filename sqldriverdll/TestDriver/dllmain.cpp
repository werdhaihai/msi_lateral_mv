#include <windows.h>
#include <stdio.h>
#include <lmcons.h>
#include <sddl.h>

#ifndef INSTAPI
#define INSTAPI __stdcall
#endif

void WriteToLog(const char* message) {
    FILE* logFile;
    if (fopen_s(&logFile, "C:\\Users\\domainadmin\\Desktop\\MSI_Output.log", "a") == 0) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        fprintf(logFile, "[%04d-%02d-%02d %02d:%02d:%02d] %s\n",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, message);
        fclose(logFile);
    }
}

void LogUserContext() {
    char logBuffer[2048];
    HANDLE hToken = NULL;
    DWORD dwSize = 0;

    // Get current username
    char username[UNLEN + 1];
    DWORD usernameSize = sizeof(username);
    if (GetUserNameA(username, &usernameSize)) {
        snprintf(logBuffer, sizeof(logBuffer), "Username: %s", username);
        WriteToLog(logBuffer);
    }

    // Get process token
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {

        // Get token user SID
        GetTokenInformation(hToken, TokenUser, NULL, 0, &dwSize);
        if (dwSize > 0) {
            TOKEN_USER* pTokenUser = (TOKEN_USER*)malloc(dwSize);
            if (pTokenUser && GetTokenInformation(hToken, TokenUser, pTokenUser, dwSize, &dwSize)) {
                LPSTR sidString = NULL;
                if (ConvertSidToStringSidA(pTokenUser->User.Sid, &sidString)) {
                    snprintf(logBuffer, sizeof(logBuffer), "User SID: %s", sidString);
                    WriteToLog(logBuffer);
                    LocalFree(sidString);
                }
            }
            if (pTokenUser) free(pTokenUser);
        }

        // Get token elevation info
        TOKEN_ELEVATION elevation;
        dwSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &elevation, dwSize, &dwSize)) {
            snprintf(logBuffer, sizeof(logBuffer), "Token Elevated: %s", elevation.TokenIsElevated ? "Yes" : "No");
            WriteToLog(logBuffer);
        }

        // Get logon session info
        TOKEN_STATISTICS tokenStats;
        dwSize = sizeof(TOKEN_STATISTICS);
        if (GetTokenInformation(hToken, TokenStatistics, &tokenStats, dwSize, &dwSize)) {
            snprintf(logBuffer, sizeof(logBuffer), "Logon Session ID: %08X-%08X",
                tokenStats.AuthenticationId.HighPart, tokenStats.AuthenticationId.LowPart);
            WriteToLog(logBuffer);
        }

        CloseHandle(hToken);
    }

    // Get process ID and parent process ID
    DWORD pid = GetCurrentProcessId();
    snprintf(logBuffer, sizeof(logBuffer), "Process ID: %d", pid);
    WriteToLog(logBuffer);

    // Get process integrity level
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &dwSize);
        if (dwSize > 0) {
            TOKEN_MANDATORY_LABEL* pTIL = (TOKEN_MANDATORY_LABEL*)malloc(dwSize);
            if (pTIL && GetTokenInformation(hToken, TokenIntegrityLevel, pTIL, dwSize, &dwSize)) {
                DWORD dwIntegrityLevel = *GetSidSubAuthority(pTIL->Label.Sid,
                    (DWORD)(UCHAR)(*GetSidSubAuthorityCount(pTIL->Label.Sid) - 1));

                const char* integrityLevel = "Unknown";
                if (dwIntegrityLevel >= SECURITY_MANDATORY_HIGH_RID) integrityLevel = "High";
                else if (dwIntegrityLevel >= SECURITY_MANDATORY_MEDIUM_RID) integrityLevel = "Medium";
                else if (dwIntegrityLevel >= SECURITY_MANDATORY_LOW_RID) integrityLevel = "Low";
                else integrityLevel = "Untrusted";

                snprintf(logBuffer, sizeof(logBuffer), "Integrity Level: %s (%08X)", integrityLevel, dwIntegrityLevel);
                WriteToLog(logBuffer);
            }
            if (pTIL) free(pTIL);
        }
        CloseHandle(hToken);
    }
}



BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) BOOL INSTAPI ConfigDriver(
    HWND hwndParent,
    WORD fRequest,
    LPCSTR lpszDriver,
    LPCSTR lpszArgs,
    LPSTR lpszMsg,
    WORD cbMsgMax,
    WORD * pcbMsgOut)
{
    WriteToLog("ConfigDriver Called");
    LogUserContext();
    return TRUE;
}
