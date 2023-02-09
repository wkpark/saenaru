/*
 * This file is part of Saenaru.
 *
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
 */

#include <windows.h>
#include "hangul.h"
#include "unicode.h"
#include "debug.h"

#define MEMALLOC(x)      HeapAlloc(GetProcessHeap(), 0, (x))
#define MEMFREE(x)       HeapFree(GetProcessHeap(), 0, (x))

#define SAENARU_KEYBOARD_2SET 2
#define SAENARU_KEYBOARD_3SET 3
#define SAENARU_KEYBOARD_RAW  4

#define SAENARU_COMPOSE_DEFAULT  0
#define SAENARU_COMPOSE_2SET     2
#define SAENARU_COMPOSE_3SET     0
#define SAENARU_COMPOSE_AHN      3
#define SAENARU_COMPOSE_YET      4

BOOL GetRegKeyHandle(LPCTSTR lpszSubKey, HKEY *hKey);
long PASCAL GetRegMultiStringValue(LPCTSTR lpszSubKey, LPCTSTR lpszKey, LPTSTR lpString);

static
DWORD string_to_hex(TCHAR* p)
{
    DWORD ret = 0;
    UINT remain = 0;

    if (*p == 'U')
        p++;

    while (*p != '\0') {
        if (*p >= '0' && *p <= '9')
            remain = *p - '0';
        else if (*p >= 'a' && *p <= 'f')
            remain = *p - 'a' + 10;
        else if (*p >= 'A' && *p <= 'F')
            remain = *p - 'A' + 10;
        else
            return 0;

        ret = ret * 16 + remain;
        p++;
    }
    return ret;
}

UINT
load_keyboard_map_from_reg(LPCTSTR lpszKeyboard, UINT nKeyboard, WCHAR *keyboard_map)
{
    int i;
    WCHAR *line, *p, *saved_position;
    WCHAR *save;
    TCHAR buf[256];
    //FILE* file;
    DWORD key, value;
    UINT type, ctype;
    INT sz, len;
    WCHAR name[256];
    //LPTSTR kbuf=NULL;
    LPTSTR kbuf=NULL;
    WCHAR _map[94];

    WCHAR achValue[256]; 

    //NabiKeyboardMap *keyboard_map;
    //
    if (lpszKeyboard == NULL && nKeyboard <10)
    {
        DWORD cchValue = 256;
        DWORD retCode;
        HKEY hKey;

        if (!GetRegKeyHandle(TEXT("\\Keyboard"), &hKey))
            return 0;

        achValue[0] = '\0'; 
        retCode = RegEnumValue(hKey, nKeyboard, 
            achValue, 
            &cchValue, 
            NULL, 
            NULL,
            NULL,
            NULL);

        if (retCode != ERROR_SUCCESS ) 
        { 
            MyDebugPrint((TEXT("(%d) %s\n"), nKeyboard, achValue));
            return 0;
        }
        lpszKeyboard = (LPCTSTR) &achValue;
    }

    sz= GetRegMultiStringValue(TEXT("\\Keyboard"),lpszKeyboard,NULL);
    if (sz <= 0) {
            MyDebugPrint((TEXT("Saenaru: Keyboard not found\n")));
            return 0;
    }

    MyDebugPrint((TEXT("Saenaru: reg size %d\n"), sz));
    kbuf=(LPTSTR) MEMALLOC(sz);
    //
    if (kbuf == (LPTSTR)NULL) {
            MyDebugPrint((TEXT("Saenaru: Can't read keyboard registry\n")));
            return 0;
    }

    GetRegMultiStringValue(TEXT("\\Keyboard"),lpszKeyboard,kbuf);
    //keyboard_map = g_malloc(sizeof(NabiKeyboardMap));

    /* init */
    type = SAENARU_KEYBOARD_3SET;
    ctype = SAENARU_COMPOSE_DEFAULT;
    //keyboard_map->filename = g_strdup(filename);
    //name = NULL;
    //MyDebugPrint((TEXT("Saenaru: %s\n"),kbuf));

    for (i = 0; i < 94; i++)
            _map[i] = i + '!';
            //_map[i] = 0;

    for (line = Mystrtok(kbuf, TEXT("\0"), &save);
         ;
         line = Mystrtok(saved_position, TEXT("\0"), &save)) {
        len=Mylstrlen(line);
        saved_position=line+len+1;

        //MyDebugPrint((TEXT("Saenaru: %s:%d\n"),line,len));
        if (len==0) break;

        p = Mystrtok(line, TEXT(" \t\0"), &save);
        MyDebugPrint((TEXT("tok: %s\n"),p));
        /* comment */
        if (p == NULL || p[0] == '#')
            continue;

        if (Mylstrcmp(p, TEXT("Name:")) == 0) {
            p = Mystrtok(NULL, TEXT(" \t\0"), &save);
            if (p == NULL)
                continue;
            //name = g_strdup(p);
            continue;
        } else if (Mylstrcmp(p, TEXT("Compose:")) == 0) {
            // Set default compose map
            p = Mystrtok(NULL, TEXT(" \t\0"), &save);
            if (p == NULL)
                continue;
            if (Mylstrcmp(p, TEXT("yet")) == 0) {
                ctype = SAENARU_COMPOSE_YET;
            } else if (Mylstrcmp(p, TEXT("full")) == 0) {
                ctype = SAENARU_COMPOSE_YET;
            } else if (Mylstrcmp(p, TEXT("2set")) == 0) {
                ctype = SAENARU_COMPOSE_2SET;
            } else if (Mylstrcmp(p, TEXT("3set")) == 0) {
                ctype = SAENARU_COMPOSE_3SET;
            } else if (Mylstrcmp(p, TEXT("ahn")) == 0) {
                ctype = SAENARU_COMPOSE_AHN;
            }
            //name = g_strdup(p);
            continue;
        } else if (Mylstrcmp(p, TEXT("Type2")) == 0) {
            type = SAENARU_KEYBOARD_2SET;
        } else if (Mylstrcmp(p, TEXT("Type3")) == 0) {
            type = SAENARU_KEYBOARD_3SET;
        } else {
            key = string_to_hex(p);
            if (key == 0)
                continue;

            p = Mystrtok(NULL, TEXT(" \t"), &save);
            if (p == NULL)
                continue;
            value = string_to_hex(p);
            if (value == 0)
                continue;

            if (key < '!' || key > '~')
                continue;

            MyDebugPrint((TEXT("REG %x=%x\n"),(DWORD)key,(DWORD)value));

            _map[key - '!'] = (WCHAR)value;
        }
    }
    MEMFREE(kbuf);

    if (keyboard_map != NULL)
        for (i=0;i<94;i++) keyboard_map[i]=_map[i];        

    //if (name == NULL)
    //        name = g_path_get_basename(keyboard_map->filename);

    return type | (ctype << 8);
}

#define SAENARU_AUTOMATA_NONE 1
#define SAENARU_AUTOMATA_2SET 2
#define SAENARU_AUTOMATA_3SET 3
#define SAENARU_AUTOMATA_RAW  4
UINT
load_compose_map_from_reg(LPCTSTR lpszCompose, UINT nCompose, HangulCompose *compose_map)
{
    WCHAR *line, *p, *saved_position;
    WCHAR *save;
    TCHAR buf[256];
    //FILE* file;
    UINT key1, key2;
    DWORD value;
    //NabiComposeItem *citem;
    UINT map_size;
    //GSList *list = NULL;
    INT sz, len;
    WCHAR name[256];
    //LPTSTR kbuf=NULL;
    LPTSTR kbuf=NULL;
    UINT _key[256];
    WCHAR _code[256];
    UINT id=0,i;
    UINT type=SAENARU_AUTOMATA_NONE;

    WCHAR achValue[256];

    if (lpszCompose == NULL && nCompose <10)
    {
        DWORD cchValue = 256;
        DWORD retCode;
        HKEY hKey;

        if (!GetRegKeyHandle(TEXT("\\Compose"), &hKey))
            return 0;

        achValue[0] = '\0'; 
        retCode = RegEnumValue(hKey, nCompose, 
            achValue, 
            &cchValue, 
            NULL, 
            NULL,
            NULL,
            NULL);

        if (retCode != ERROR_SUCCESS ) 
        { 
            MyDebugPrint((TEXT("(%d) %s\n"), nCompose, achValue));
            return 0;
        }
        lpszCompose = (LPCTSTR) &achValue;
    }

    sz= GetRegMultiStringValue(TEXT("\\Compose"),lpszCompose,NULL);
    if (sz <= 0) {
            MyDebugPrint((TEXT("Saenaru: ComposeMap not found\n")));
            return 0;
    }

    MyDebugPrint((TEXT("Saenaru: reg size %d\n"), sz));
    kbuf=(LPTSTR) MEMALLOC(sz);
    //
    if (kbuf == (LPTSTR)NULL) {
            MyDebugPrint((TEXT("Saenaru: Can't read compose map registry\n")));
            return 0;
    }

    GetRegMultiStringValue(TEXT("\\Compose"),lpszCompose,kbuf);
    /* init */
    //compose_map->name = NULL;
    //compose_map->map = NULL;
    //compose_map->size = 0;

    for (line = Mystrtok(kbuf, TEXT("\0"), &save);id<256
	 ;
	 line = Mystrtok(saved_position, TEXT("\0"), &save)) {
        len=Mylstrlen(line);
        saved_position=line+len+1;

        if (len==0) break;

	p = Mystrtok(line, TEXT(" \t\0"), &save);
        MyDebugPrint((TEXT("tok: %s\n"),p));
	/* comment */
	if (p == NULL || p[0] == '#')
	    continue;

	if (Mylstrcmp(p, TEXT("Name:")) == 0) {
	    p = Mystrtok(NULL, TEXT(" \t\0"), &save);
	    if (p == NULL)
		continue;
	    //compose_map->name = g_strdup(p);
	    continue;
        } else if (Mylstrcmp(p, TEXT("Type0")) == 0) {
            type = SAENARU_AUTOMATA_RAW;
	    //compose_map->name = g_strdup(p);
	    continue;
	} else {
	    key1 = string_to_hex(p);
	    if (key1 == 0)
		continue;

	    p = Mystrtok(NULL, TEXT(" \t"), &save);
	    if (p == NULL)
		continue;
	    key2 = string_to_hex(p);
	    if (key2 == 0)
		continue;

	    p = Mystrtok(NULL, TEXT(" \t"), &save);
	    if (p == NULL)
		continue;
	    value = string_to_hex(p);
	    if (value == 0)
		continue;

	    _key[id] = key1 << 16 | key2;
	    _code[id] = (WCHAR)value;
            id++;
	}
    }
    MEMFREE(kbuf);
    map_size=(UINT)id;

    //if (compose_map == NULL) {
	/* on error free the list */
//	while (list != NULL) {
//	    g_free(list->data);
//	    list = list->next;
//	}
//	g_slist_free(list);
//
//	return FALSE;
//    }

    /* sort compose map */
//    list = g_slist_reverse(list);
//    list = g_slist_sort(list, compose_item_compare);

    /* move data to map */
//    map_size = g_slist_length(list);
//    compose_map->map = (NabiComposeItem**)
//		g_malloc(map_size * sizeof(NabiComposeItem*));
    for (i = 0; i < map_size; i++) {
	compose_map[i].key = _key[i];
	compose_map[i].code = _code[i];
    }
    compose_map[map_size].key=(UINT)-1;

    /* free the list */
    //g_slist_free(list);

    return type;
}

/*
 * ex: ts=8 sts=4 sw=4 et
 */
