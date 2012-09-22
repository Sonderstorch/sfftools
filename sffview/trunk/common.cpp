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

 $Id: common.cpp,v 1.4 2009/04/28 19:07:10 pschaefer Exp $

---RCS-Info--------------------------------------------------*/

#include "sfftypes.h"
#include "common.h"
#include "errno.h"

#include <time.h>
#if defined(_MSC_VER)
#include <sys/utime.h>
#else
#include <utime.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#include <cassert>
#include <cstdio>
#include <iostream>

using namespace std;

#ifdef _MSC_VER
#define fileno _fileno
#endif

//-----------------------------------------------------------------

const std::string CSimpleException::m_aReasons[err_count+1] =
{
//  err_invalidfile
    "Not a valid sff file.",
//    err_corruptfile,
    "File seems corrupt. Reading abandoned.",
//    err_lastpageread,
    "Last page read.",
//    err_notsupported,
    "Operation not supported.",
//    err_openfile,
    "Can't open file.",
//    err_closedfile,
    "Operation on closed file.",
//    err_findPath,
    "Path not found.",
//    err_nowhitestart,
    "Line doesn't begin with white code.",
//    err_noblackcode,
    "White code not followed by black code.",
//    err_noblackterm,
    "Black MUC not followed by black TERM.",
//    err_nowhiteterm,
    "White MUC not followed by white TERM.",
//    err_invalidversion,
    "OOps. Don't know how to handle this Fileversion.",
//    err_unknowncoding,
    "Oh my dear. Don't know how to handle this encoding.",
//    err_toomuchformats,
    "Please specify only one output format.",
//    err_cmdline,
    "Error in commandline.",
//    err_noformat,
    "No output format specified.",
//    err_outfileexists,
    "Output file already exists, use -f to force overwrite.",
//    err_outfileisdir,
    "Given output file is directory.",
//    err_outdir,
    "Cannot create output directory.",
//
// ------------------- INSERT HERE
//
//    err_count
    "Unknown error."
};

const string& CSimpleException::what() const
{
  if (m_nError < err_count) {
    return m_aReasons[m_nError];
  }
  return m_aReasons[err_count];
}

//-----------------------------------------------------------------

CFile::CFile(const std::string& strPath) :
  m_hFile(NULL)
{
  Open( strPath, "rb");
}

CFile::~CFile()
{
  Close();
}

void CFile::Open(const std::string& strPath, const char *pszMode)
{
  m_strPath = strPath;
  if (( m_hFile ) || !( m_hFile = fopen(strPath.c_str(), pszMode)))
    throw CSimpleException(CSimpleException::err_openfile);
  m_nFileNo = fileno( m_hFile);
}

void CFile::Close()
{
  if (m_hFile) {
    fclose(m_hFile);
  }
  m_hFile = NULL;
}

time_t CFile::GetModificationTime()
{
  struct stat buf;
  if (::fstat( m_nFileNo, &buf ) == 0)
    return buf.st_mtime ;
  else
    throw CSimpleException(CSimpleException::err_notsupported);
}

void CFile::SetModificationTime(const time_t &modtime)
{
  time_t ltime;
  time( &ltime);
  utimbuf filetime;
  filetime.actime = ltime;
  filetime.modtime = modtime;
  if (::utime( m_strPath.c_str(), &filetime) != 0) {
    cerr << "CFile::SetModificationTime(): ErrorNr.: " << errno << endl;
  }
}

sff_byte CFile::GetC()
{
  return ::fgetc(m_hFile);
}

void CFile::Read(void *pTarget, int nLen)
{
  size_t rc = ::fread(pTarget, 1, nLen, m_hFile);
}

void CFile::Write(void *pSource, int nLen)
{
  ::fwrite(pSource, nLen, 1, m_hFile);
}

sff_dword CFile::Tell()
{
  return ::ftell(m_hFile);
}

void CFile::Seek(int pos, CFile::seek_offset dir)
{
  int whence;
  if (dir == sk_from_start) {
    whence = SEEK_SET;
  } else if (dir == sk_current) {
    whence = SEEK_CUR;
  } else {
    return;
  }
  ::fseek(m_hFile, pos, whence);
}

bool CFile::Eof()
{
  return (feof(m_hFile) != 0);
}
