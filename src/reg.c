/*
 * This file is part of Saenaru.
 *
 * Copyright (c) 1990-1998 Microsoft Corporation.
 * Copyright (c) 2003 Hye-Shik Chang <perky@i18n.org>.
 * Copyright (c) 2003 Won-Kyu Park <wkpark@kldp.org>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Saenaru: saenaru/src/reg.c,v 1.5 2004/10/10 10:59:18 wkpark Exp $
 */

#include <windows.h>
#include "immdev.h"
#include "saenaru.h"

#if   UNICODE
const LPTSTR g_szRegInfoPath = TEXT("software\\OpenHangulProject\\Saenaru");
#else
const LPTSTR g_szRegInfoPath = TEXT("software\\OpenHangulProject\\Saenaru");
#endif

#ifdef DEBUG
int DebugPrint(LPCTSTR lpszFormat, ...)
{
    int nCount;
    TCHAR szMsg[1024];

    va_list marker;
    va_start(marker, lpszFormat);
    nCount = wvsprintf(szMsg, lpszFormat, marker);
    va_end(marker);
    OutputDebugString(szMsg);
    return nCount;
}
#endif

DWORD PASCAL GetDwordFromSetting(LPTSTR lpszKey)
{
    HKEY hkey;
    DWORD dwRegType, dwData, dwDataSize, dwRet;

    dwData = 0;
    dwDataSize=sizeof(DWORD);
    if (RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegInfoPath, 0, KEY_READ, &hkey) != ERROR_SUCCESS &&
        RegOpenKeyEx (HKEY_LOCAL_MACHINE, g_szRegInfoPath, 0, KEY_READ, &hkey) != ERROR_SUCCESS)
        return 0;

    dwRet = RegQueryValueEx(hkey, lpszKey, NULL, &dwRegType, (LPBYTE)&dwData, &dwDataSize);
    RegCloseKey(hkey);

    MyDebugPrint((TEXT("Getting: %s=%#8.8x: dwRet=%#8.8x\n"), lpszKey, dwData, dwRet));
    return dwData;
}

long    PASCAL
GetRegMultiStringValue (
    register LPCTSTR        lpszSubKey,
    register LPCTSTR        lpszKey,
    register LPTSTR         lpString)
{
    HKEY    hKey ;
    DWORD    dwRegType, dwData, dwDataSize, dwRet ;
    TCHAR    szRegInfoPath [MAX_PATH] ;
    dwData        = 0 ;
    /* */
    lstrcpy (szRegInfoPath, g_szRegInfoPath) ;
    if (lpszSubKey)
        lstrcat (szRegInfoPath, lpszSubKey) ;

        MyDebugPrint((TEXT("Getting: %s: path=%s\n"), lpszKey, szRegInfoPath));
    if (RegOpenKeyEx (HKEY_CURRENT_USER, szRegInfoPath, 0, KEY_READ, &hKey) != ERROR_SUCCESS &&
        RegOpenKeyEx (HKEY_LOCAL_MACHINE, szRegInfoPath, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return    -1 ;
    dwRet    = RegQueryValueEx (hKey, lpszKey, NULL, &dwRegType, (LPBYTE)lpString, &dwDataSize) ;
    RegCloseKey (hKey) ;

    if (dwRet != ERROR_SUCCESS || dwRegType != REG_MULTI_SZ)
        return    -1 ;
        MyDebugPrint((TEXT("OK %s\n"), lpszKey));
    return    (long)dwDataSize ;
}

BOOL GetRegKeyHandle(LPCTSTR lpszSubKey, HKEY *hKey)
{ 
    TCHAR szRegInfoPath [MAX_PATH] ;

    lstrcpy (szRegInfoPath, g_szRegInfoPath) ;
    if (lpszSubKey)
        lstrcat (szRegInfoPath, lpszSubKey) ;

    if (RegOpenKeyEx (HKEY_CURRENT_USER, szRegInfoPath, 0, KEY_READ, hKey) != ERROR_SUCCESS &&
        RegOpenKeyEx (HKEY_LOCAL_MACHINE, szRegInfoPath, 0, KEY_READ, hKey) != ERROR_SUCCESS) {
        MyDebugPrint((TEXT(">>FAIL: path=%s\n"), szRegInfoPath));
        return FALSE;
    }
    MyDebugPrint((TEXT(">>OK: path=%s\n"), szRegInfoPath));
    return TRUE;
}

void GetRegKeyList(LPCTSTR lpszSubKey)
{ 
    HKEY    hKey ;
    TCHAR    szRegInfoPath [MAX_PATH] ;

    lstrcpy (szRegInfoPath, g_szRegInfoPath) ;
    if (lpszSubKey)
        lstrcat (szRegInfoPath, lpszSubKey) ;

        MyDebugPrint((TEXT("Getting: path=%s\n"), szRegInfoPath));
    if (RegOpenKeyEx (HKEY_CURRENT_USER, szRegInfoPath, 0, KEY_READ, &hKey) != ERROR_SUCCESS &&
        RegOpenKeyEx (HKEY_LOCAL_MACHINE, szRegInfoPath, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return;
    {
/* from MSDN document */
#define MAX_KEY_LENGTH 260
#define MAX_VALUE_NAME 16383
    WCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
    DWORD    cbName;                   // size of name string 
    CHAR     achClass[MAX_PATH] = "";  // buffer for class name 
    DWORD    cchClassName = MAX_PATH;  // size of class string 
    DWORD    cSubKeys=0;               // number of subkeys 
    DWORD    cbMaxSubKey;              // longest subkey size 
    DWORD    cchMaxClass;              // longest class string 
    DWORD    cValues;              // number of values for key 
    DWORD    cchMaxValue;          // longest value name 
    DWORD    cbMaxValueData;       // longest value data 
    DWORD    cbSecurityDescriptor; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 
 
    DWORD i, retCode; 
 
    WCHAR  achValue[MAX_VALUE_NAME]; 
    DWORD cchValue = MAX_VALUE_NAME; 
 
    // Get the class name and the value count. 
    retCode = RegQueryInfoKey(
        hKey,                    // key handle 
        (LPWSTR)achClass,                // buffer for class name 
        &cchClassName,           // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        &cbMaxSubKey,            // longest subkey size 
        &cchMaxClass,            // longest class string 
        &cValues,                // number of values for this key 
        &cchMaxValue,            // longest value name 
        &cbMaxValueData,         // longest value data 
        &cbSecurityDescriptor,   // security descriptor 
        &ftLastWriteTime);       // last write time 

#if 0 
    // Enumerate the subkeys, until RegEnumKeyEx fails.
    
    if (cSubKeys)
    {
        MyDebugPrint((TEXT("Number of subkeys: %d\n"), cSubKeys));

        for (i=0; i<cSubKeys; i++) 
        { 
            cbName = MAX_KEY_LENGTH;
            retCode = RegEnumKeyEx(hKey, i,
                     achKey, 
                     &cbName, 
                     NULL, 
                     NULL, 
                     NULL, 
                     &ftLastWriteTime); 
            if (retCode == ERROR_SUCCESS) 
            {
                MyDebugPrint((TEXT("(%d) %s\n"), i+1, achKey));
            }
        }
    }
#endif
 
    // Enumerate the key values. 

    if (cValues) 
    {
        MyDebugPrint((TEXT("\nNumber of values: %d\n"), cValues));

        for (i=0, retCode=ERROR_SUCCESS; i<cValues; i++) 
        { 
            cchValue = MAX_VALUE_NAME; 
            achValue[0] = '\0'; 
            retCode = RegEnumValue(hKey, i, 
                achValue, 
                &cchValue, 
                NULL, 
                NULL,
                NULL,
                NULL);
 
            if (retCode == ERROR_SUCCESS ) 
            { 
                MyDebugPrint((TEXT("(%d) %s\n"), i+1, achValue)); 
            } 
        }
    }
    }
}


long    PASCAL
GetRegStringValue (
    register LPCTSTR        lpszSubKey,
    register LPCTSTR        lpszKey,
    register LPTSTR            lpString)
{
    HKEY    hKey ;
    DWORD    dwRegType, dwData, dwDataSize, dwRet ;
    TCHAR    szRegInfoPath [MAX_PATH] ;
    dwData        = 0 ;
    /* レジストリの検索パスを設定する。*/
    lstrcpy (szRegInfoPath, g_szRegInfoPath) ;
    if (lpszSubKey)
        lstrcat (szRegInfoPath, lpszSubKey) ;

    MyDebugPrint((TEXT("GetRegString: %s: path=%s\n"), lpszKey, szRegInfoPath));

    if (RegOpenKeyEx (HKEY_CURRENT_USER, szRegInfoPath, 0, KEY_READ, &hKey) != ERROR_SUCCESS &&
        RegOpenKeyEx (HKEY_LOCAL_MACHINE, szRegInfoPath, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return    -1 ;
    dwRet    = RegQueryValueEx (hKey, lpszKey, NULL, &dwRegType, (LPBYTE)lpString, &dwDataSize) ;
    RegCloseKey (hKey) ;
    if (dwRet != ERROR_SUCCESS || dwRegType != REG_SZ)
        return    -1 ;
    return    (long)dwDataSize ;
}

BOOL    PASCAL
SetRegStringValue (
    register LPCTSTR        lpszSubKey,
    register LPCTSTR        lpszKey,
    register LPTSTR            lpString)
{
    HKEY    hKey ;
    DWORD    dwDisposition ;
    DWORD    dwDataSize, dwRet ;
    TCHAR    szRegInfoPath [MAX_PATH] ;
    /* レジストリのパスを設定する。*/
    lstrcpy (szRegInfoPath, g_szRegInfoPath) ;
    if (lpszSubKey)
        lstrcat (szRegInfoPath, lpszSubKey) ;
    /* レジストリを開く。無ければ作成する。*/
    if (RegCreateKeyEx (HKEY_CURRENT_USER, szRegInfoPath, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &hKey, &dwDisposition) != ERROR_SUCCESS)
        return    FALSE ;
    /* 値を設定する。*/
    dwRet    = RegSetValueEx (hKey, lpszKey, 0, REG_SZ, (CONST BYTE *)lpString, lstrlen (lpString) * sizeof (TCHAR)) ;
    RegCloseKey (hKey) ;
    return    (dwRet == ERROR_SUCCESS) ;
}


void SetDwordToSetting(LPCTSTR lpszKey, DWORD dwFlag)
{
    HKEY hkey;
    DWORD dwDataSize, dwRet;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegInfoPath, 0, KEY_WRITE, &hkey)) {
        dwRet = RegSetValueEx(hkey, lpszKey, 0, REG_DWORD, (CONST BYTE *) &dwFlag, sizeof(DWORD));
        RegCloseKey(hkey);
    }
    MyDebugPrint((TEXT("Setting: %s=%#8.8x: dwRet=%#8.8x\n"), lpszKey, dwFlag, dwRet));
}

void PASCAL SetGlobalFlags()
{
    DWORD tmp;
#ifdef DEBUG
    dwLogFlag = GetDwordFromSetting(TEXT("LogFlag"));
    dwDebugFlag = GetDwordFromSetting(TEXT("DebugFlag"));
#endif

    dwOptionFlag = GetDwordFromSetting(TEXT("OptionFlag"));
    dwLayoutFlag = GetDwordFromSetting(TEXT("LayoutFlag"));

    // get ScanCode based setting.
    dwScanCodeBased = GetDwordFromSetting(TEXT("ScanCodeBased"));

    // HangulToggle Key
    dwToggleKey = GetDwordFromSetting(TEXT("HangulToggle"));

    tmp = GetDwordFromSetting(TEXT("ImeFlag"));
    if (tmp) {
        dwImeFlag = tmp;
    }
}

#ifdef DEBUG
void PASCAL ImeLog(DWORD dwFlag, LPTSTR lpStr)
{
    TCHAR szBuf[80];

    if (dwFlag & dwLogFlag)
    {
        if (dwDebugFlag & DEBF_THREADID)
        {
            DWORD dwThreadId = GetCurrentThreadId();
            wsprintf(szBuf, TEXT("ThreadID = %X "), dwThreadId);
            OutputDebugString(szBuf);
        }

        OutputDebugString(lpStr);
        OutputDebugString(TEXT("\r\n"));
    }
}
#endif //DEBUG

/*
 * ex: ts=8 sts=4 sw=4 et
 */
