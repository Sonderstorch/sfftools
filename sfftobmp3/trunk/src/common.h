#ifndef __COMMON_H__
#define __COMMON_H__
//
// Headerfile for support classes
//
// This file is part of sfftobmp, a program to convert
// structured fax files (sff) to windows bitmap files (bmp),
// portable bitmap graphics (pbm), tagged image file format (tiff)
// or JPEG (jpg).
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
//    Ulf Zibis <ulf.zibis@gmx.de> (Prepared for inheritance,
//       destructor handling, preserving filetime, CWildcardTranslator)
//
// You can contact the original author by email at peter.schaefer@gmx.de.
//
// I'm always pleased to hear that somebody is actually using my software.
// If you can manage it, e-mail me a quick notice. Thanks!
//
/*-RCS-Info----------------------------------------------------

 $Id$

---RCS-Info--------------------------------------------------*/

#include <boost/filesystem/path.hpp>

class CSimpleException
{
public:
  enum {
    err_invalidfile = 0,
    err_corruptfile,
    err_lastpageread,
    err_notsupported,
    err_openfile,
    err_closedfile,
    err_findPath,
    err_nowhitestart,
    err_noblackcode,
    err_noblackterm,
    err_nowhiteterm,
    err_invalidversion,
    err_unknowncoding,
    err_toomuchformats,
    err_cmdline,
    err_noformat,
    err_outfileexists,
    err_outfileisdir,
    err_outdir,
    err_stdoutnotallowed,
    err_stdoutonlyonefile,
    // insert here
    err_count
  };

  unsigned m_nError;

  CSimpleException(const int nError) :
    m_nError(nError) { };

  const std::string& what() const;

protected:
    static const std::string m_aReasons[err_count+1];
};

//-----------------------------------------------------------------

class CFile
{
protected:
  FILE                   *m_hFile;
  int                     m_nFileNo;
  boost::filesystem::path m_strPath;

public:
  enum seek_offset {
    sk_from_start,
    sk_current
  };

  CFile() : m_hFile(NULL), m_nFileNo(0) { /* sonst nix */ };
  CFile(const std::string& strPath);
    // throw CSimpleException

  virtual ~CFile();

  FILE       *GetFP() { return m_hFile; };
  int         GetFN() { return m_nFileNo; };
  const char *GetFileName() { return m_strPath.string().c_str(); };
  sff_byte    GetC();
  sff_dword   Tell();
  bool        Eof();
  void        Seek(int pos, CFile::seek_offset dir);
  void        Read(void *pTarget, int nLen);
  void        Write(void *pSource, int nLen);

  void        DumpToStdOut();  // throw CSimpleException

  virtual void Open(const std::string& strPath, const char *pszMode);
    // throw CSimpleException
  virtual void OpenTemp();
    // throw CSimpleException
  virtual void Close();
    // throw CSimpleException

  time_t GetModificationTime();
  virtual void SetModificationTime(const time_t &modtime);
};

#endif // __COMMON_H__
