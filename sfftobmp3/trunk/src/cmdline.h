#ifndef __CMDLINE_H__
#define __CMDLINE_H__
//
// Headerfile for command line processor
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
//   None
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

typedef std::vector<boost::filesystem::path> FILEVECTOR;

class CCmdLineProcessor
{
protected:
  const char  *m_pszProgName;
  int          m_nOutputFormat;
  int          m_nTiffCompression;
  int          m_nJpegQuality;
  bool         m_bKeepDate;
  bool         m_bKeepVRes;
  bool         m_bOverwrite;
  bool         m_bQuiet;
  std::string  m_strOutSpec;

  char       **m_argv;
  int          m_argc;

  int          m_nFileIdx;
  FILEVECTOR   m_vFiles;

public:
  enum {
    fmt_jpeg,
    fmt_pbm,
    fmt_bmp,
    fmt_tiff,
    fmt_tiff_single_pages,
    // -- add here
    fmt_unknown
  };

  CCmdLineProcessor(char **argv, int argc);
  ~CCmdLineProcessor();

  void printHelp();
  void printUsage();
  void printVersion();

  void parseCmdLine();  // throw

  int                getFileCount() { return m_vFiles.size(); };
  bool               getNextFile(boost::filesystem::path& strFile);
  const std::string& getOutSpec();

  bool keepDate()           { return m_bKeepDate; };
  bool keepVRes()           { return m_bKeepVRes; };
  bool doOverwrite()        { return m_bOverwrite; };
  bool beQuiet()            { return m_bQuiet; };
  int  getJpegQuality()     { return m_nJpegQuality; };
  int  getTiffCompression() { return m_nTiffCompression; };
  int  getOutputFormat()    { return m_nOutputFormat; };

};

#endif // __CMDLINE_H__
