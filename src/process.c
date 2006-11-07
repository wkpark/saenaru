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
 * $Saenaru: saenaru/src/process.c,v 1.6 2006/10/05 04:53:52 wkpark Exp $
 */

/**********************************************************************/
/* These tables are for ImeProcessKey(). And IMEProcessKey function   */
/* refer this table and deside the VKey is needed for SAENARU or not. */
/*                                                                    */
/**********************************************************************/
#include "windows.h"


BYTE bNoComp[] = {      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,        // 10-1F
                        // VK_HANJA = 0x19
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 20-2F
                        1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,        // 30-3F
                        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,        // 40-4F
                        1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,        // 50-5F
                        //1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,        // 60-6F
                        0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,        // 60-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 70-7F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 80-8F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 90-9F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // A0-AF
                        //0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // B0-BF
                        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,        // B0-BF
                        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // C0-CF
                        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,        // D0-DF
                        //0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // D0-DF
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // E0-EF
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};       // F0-FF

BYTE bNoCompCtl[] = {   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 10-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 20-0F
                        0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,        // 30-0F
                        //0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 30-0F
                        0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,        // 40-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 50-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 60-0F
                        0,0,1,0,0,1,1,1,1,0,0,0,0,0,0,0,        // 70-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 80-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 90-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // A0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // B0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // C0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // D0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // E0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};       // F0-0F

BYTE bNoCompSht[] = {   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 10-0F
                        // 0x19 == VK_HANJA
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 20-0F
                        1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,        // 30-0F
                        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,        // 40-0F
                        1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,        // 50-0F
                        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,        // 60-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 70-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 80-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 90-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // A0-0F
                        //0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // B0-0F
                        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,        // B0-BF
                        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // C0-0F
                        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,        // D0-DF
                        //0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // D0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // E0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};       // F0-0F
        
BYTE bNoCompAlt[] = {   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 10-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 20-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 30-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 40-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 50-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 60-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 70-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 80-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 90-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // A0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // B0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // C0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // D0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // E0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};       // F0-0F

BYTE bComp[] = {        0,0,0,1,0,0,0,0,1,1,0,0,0,1,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,        // 10-0F
                        // 0x19 == VK_HANJA
                        //0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,        // 10-0F
                        1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,        // 20-0F
                        1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,        // 30-0F
                        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,        // 40-0F
                        1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,        // 50-0F
                        //1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,        // 60-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,        // 60-0F
                        0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,        // 70-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 80-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 90-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // A0-0F
                        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,        // B0-0F
                        //0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // B0-0F
                        //0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // C0-0F
                        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // C0-0F
                        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,        // D0-DF
                        //0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // D0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // E0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};       // F0-0F

BYTE bCompCtl[] = {     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 10-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 20-0F
                        0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,        // 30-0F
                        // VK_SPACE == 0x39
                        //0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 30-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 40-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 50-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 60-0F
                        //0,0,1,0,0,1,1,1,1,0,0,0,0,0,0,0,        // 70-0F
                        0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,        // 70-0F
                        // VK_F3 == 114 == 0x72
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 80-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 90-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // A0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // B0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // C0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // D0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // E0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};       // F0-0F

BYTE bCompSht[] = {     0,0,0,1,0,0,0,0,1,1,0,0,0,1,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,        // 10-0F
                        1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,        // 20-0F
                        1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,        // 30-0F
                        0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,        // 40-0F
                        1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,        // 50-0F
                        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,        // 60-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 70-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 80-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 90-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // A0-0F
                        0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,        // B0-0F
                        //0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // B0-0F
                        //0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // C0-0F
                        1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // C0-0F
                        0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,        // D0-DF
                        //0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // D0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // E0-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};       // F0-0F

BYTE bCompAlt[] = {     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,        // 00-0F
                        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};        // 00-0F

/*
 * ex: ts=8 sts=4 sw=4 et
 */
