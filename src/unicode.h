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

/* for Unicode SAENARU */
typedef LPTSTR            LPMYSTR;
typedef TCHAR             MYCHAR;
#define MYTEXT(x)         TEXT(x)
#define Mylstrlen(x)      lstrlen(x)
#define Mylstrcpy(x, y)   MylstrcpyW((x), (y))
#define Mylstrcmp(x, y)   MylstrcmpW((x), (y))
#define MyCharPrev(x, y)  MyCharPrevW((x), (y))
#define MyCharPrev(x, y)  MyCharPrevW((x), (y))
#define MyHanCharPrev(x, y)  MyHanCharPrevW((x), (y))
#define MyCharNext(x)     MyCharNextW(x)
#include <string.h>
#define Mystrtok          wcstok
#define Mystrchr          wcschr
#define Mylstrcpyn        lstrcpyn
#define MyTextOut         TextOut
#define MyGetTextExtentPoint  GetTextExtentPoint
#define LPMYIMEMENUITEMINFO LPIMEMENUITEMINFO
#define MyImmRequestMessage ImmRequestMessage
#define MyOutputDebugString OutputDebugString

int PASCAL MylstrcmpW(LPWSTR lp0, LPWSTR lp1);
int PASCAL MylstrcpyW(LPWSTR lp0, LPWSTR lp1);
LPWSTR PASCAL MylstrcpynW(LPWSTR lp0, LPWSTR lp1, int nCount);
