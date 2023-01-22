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
 */

#include <windows.h>

/*****************************************************************************
*                                                                            *
* MylstrcmpW( )                                                              *
*                                                                            *
*****************************************************************************/
int PASCAL MylstrcmpW(LPWSTR lp0, LPWSTR lp1)
{
    while(*lp0 && *lp1 && (*lp0 == *lp1)) {
                lp0++;
                lp1++;
        }
        return (*lp0 - *lp1);
}

/*****************************************************************************
*                                                                            *
* MylstrcpyW( )                                                              *
*                                                                            *
*****************************************************************************/
int PASCAL MylstrcpyW(LPWSTR lp0, LPWSTR lp1)
{
    int n = 0;

    while(*lp1)
    {
        *lp0 = *lp1;
        lp0++;
        lp1++;
        n++;
    }
    *lp0 = *lp1;
    return n;
}

/*****************************************************************************
*                                                                            *
* MylstrcpynW( )                                                             *
*                                                                            *
*****************************************************************************/
LPWSTR PASCAL MylstrcpynW(LPWSTR lp0, LPWSTR lp1, int nCount)
{
    int n;
    for (n = 0; *lp1 && n < nCount - 1; *lp0++ = *lp1++, n++)
               ;
    *lp0 = L'\0';
    return lp0;
}
