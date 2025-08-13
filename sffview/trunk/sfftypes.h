#ifndef __SFFTYPES_H__
#define __SFFTYPES_H__
//
// This file is part of sffview, a program to view structured fax files (sff)
//
// Copyright (C) 1998-2012 Peter Schaefer-Hutter and contributors ("THE AUTHORS")
//
// Permission to use, copy, modify, distribute, and sell this software and
// its documentation for any purpose is hereby granted without fee.
//
// THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
// EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
// WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
//
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL,
// INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY
// DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY
// THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE
// OR PERFORMANCE OF THIS SOFTWARE.
//
// Contributor(s): 
//   None
//
// You can contact the original author by email at peter.schaefer@gmx.de.
//
// I'm always pleased to hear that somebody is actually using my software.
// If you can manage it, e-mail me a quick notice. Thanks!
// 
/*-RCS-Info----------------------------------------------------

 $Id: sfftypes.h,v 1.2 2008/03/21 13:47:02 pschaefer Exp $

---RCS-Info--------------------------------------------------*/

#if !defined(__WINDOWS__) && (defined(_WINDOWS) || defined(_Windows))
#define __WINDOWS__
#endif
#if !defined(__WIN32__) && (defined(_WIN32) || defined(WIN32))
#define __WIN32__
#endif
#if defined(__WIN32__) || defined(__WINDOWS__)
#include <windows.h>
#ifdef __WIN32__
DECLARE_HANDLE(uhandle_t);	/* Win32 file handle */
#else
typedef	HFILE uhandle_t;	/* Windows file handle */
#endif
#else
typedef	void* uhandle_t;	/* client data handle */
#endif

typedef unsigned char  sff_byte;
typedef unsigned short sff_word;
typedef unsigned int   sff_dword;

#ifndef _MAX_PATH
#define _MAX_PATH 256
#endif

#endif // __SFFTYPES_H__
