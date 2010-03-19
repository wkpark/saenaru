/****************************** Module Header ******************************\
* Module Name: Colemak.H
*
* keyboard layout header
*
* Copyright (c) 2006 Shai Coleman colemak.com. Public Domain
*
* Various defines for use by keyboard input code.
*
* History:
*  - change KBD_TYPE definition by wkpark@kldp.org 2010/03/13
*  - created by KBDTOOL v3.40 Sat Mar 13 01:25:57 2010
*
\***************************************************************************/

/*
 * kbd type can be controlled by cl command-line argument
 */
#ifndef KBD_TYPE
/*
 * KO 101 Hangul Keyboard Type A
 */
#define KBD_TYPE 10
/*
 * KO 101 Hangul Keyboard Type B
 */
//#define KBD_TYPE 11

/*
 * KO 101 Hangul Keyboard Type C
 */
//#define KBD_TYPE 12
//
/*
 * KO 103/106 Hangul Keyboard
 */
//#define KBD_TYPE 13
#endif

/*
* Include the basis of all keyboard table values
*/
#include "kbd.h"
/***************************************************************************\
* The table below defines the virtual keys for various keyboard types where
* the keyboard differ from the US keyboard.
*
* _EQ() : all keyboard types have the same virtual key for this scancode
* _NE() : different virtual keys for this scancode, depending on kbd type
*
*     +------+ +----------+----------+----------+----------+----------+----------+
*     | Scan | |    kbd   |    kbd   |    kbd   |    kbd   |    kbd   |    kbd   |
*     | code | |   type 1 |   type 2 |   type 3 |   type 4 |   type 5 |   type 6 |
\****+-------+_+----------+----------+----------+----------+----------+----------+*/

#undef  T12
 #define T12 _EQ(                                        'F'                      )
#undef  T13
 #define T13 _EQ(                                        'P'                      )
#undef  T14
 #define T14 _EQ(                                        'G'                      )
#undef  T15
 #define T15 _EQ(                                        'J'                      )
#undef  T16
 #define T16 _EQ(                                        'L'                      )
#undef  T17
 #define T17 _EQ(                                        'U'                      )
#undef  T18
 #define T18 _EQ(                                        'Y'                      )
#undef  T19
 #define T19 _EQ(                                      OEM_1                      )
#undef  T1F
 #define T1F _EQ(                                        'R'                      )
#undef  T20
 #define T20 _EQ(                                        'S'                      )
#undef  T21
 #define T21 _EQ(                                        'T'                      )
#undef  T22
 #define T22 _EQ(                                        'D'                      )
#undef  T24
 #define T24 _EQ(                                        'N'                      )
#undef  T25
 #define T25 _EQ(                                        'E'                      )
#undef  T26
 #define T26 _EQ(                                        'I'                      )
#undef  T27
 #define T27 _EQ(                                        'O'                      )
#undef  T31
 #define T31 _EQ(                                        'K'                      )

#undef T3A
 #define T3A _EQ(                                       BACK                      )
#undef X3A
 #define X3A _EQ(                                       BACK                      )

#if ( KBD_TYPE == 10 )
 #undef T5C
  #define T5C _EQ(                                       RMENU                     )
 #undef X5C
  #define X5C _EQ(                                       RMENU                     )
#endif
