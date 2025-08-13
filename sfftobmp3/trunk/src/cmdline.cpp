// Command line processor
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

#include <vector>
#include <cassert>
#include <cstdio>
#include <iostream>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/version.hpp>

#include <tiff.h>

#include "sfftypes.h"
#include "common.h"
#include "getopt.h"

#include "cmdline.h"

extern "C"
{
#define XMD_H
#include <jpeglib.h>
#include <tiffio.h>
}

using namespace std;
namespace fs = boost::filesystem;

//-------------------------------------------------------------

#define VERSION "3.1.4"
#define COPYRIGHT "1998-2012 Peter Schaefer-Hutter and contributors"

//-------------------------------------------------------------

CCmdLineProcessor::CCmdLineProcessor(char **argv, int argc)
{
  m_argv = argv;
  m_argc = argc;
  m_pszProgName      = argv[0];
  m_nOutputFormat    = fmt_unknown;
  m_nTiffCompression = COMPRESSION_CCITTFAX3;
  m_nJpegQuality     = 40;
  m_nFileIdx         = 0;
  m_bKeepDate = m_bKeepVRes = m_bOverwrite = m_bQuiet = false;
}

//-------------------------------------------------------------

CCmdLineProcessor::~CCmdLineProcessor()
{
  FILEVECTOR::iterator it;
  m_vFiles.clear();
}

//-------------------------------------------------------------

void CCmdLineProcessor::printHelp()
{
  cout << endl << "Usage: " << m_pszProgName << " [options] INFILE1 [INFILE2 ..] [-o OUTSPEC]" << endl << endl;
  cout << "Options:" << endl;
  cout << "-h or -help       Show this message and exit" << endl;
  cout << "-v or -version    Show version and exit" << endl;
  cout << "-b or -bmp        Output is one monochrome BMP file for each fax page" << endl;
  cout << "-p or -pbm        Output is one Portable Bitmap file for each fax page" << endl;
  cout << "-j or -jpg        Output is one JPEG file for each fax page" << endl;
  cout << "-jNUM or -jpg=NUM Use jpeg quality of NUM percent (1..99)" << endl;
  cout << "-T or -tifs       Output is one single-page TIFF file for each fax page" << endl;
  cout << "-t or -tif        Output is one multi-paged TIFF file containing all pages" << endl;
  cout << "-r or -keepres    Inhibit line doubling for low-res faxes" << endl;
  cout << "-d or -keepdate   Keep date and time of input file for the output file(s)" << endl;
  cout << "-q or -quiet      No messages except errors" << endl << endl;
  cout << "The OUTSPEC is interpreted as:" << endl;
  cout << " - a filename if only one input file is given." << endl;
  cout << " - a directory name if more than one input file is given." << endl << endl;
  cout << "If OUTSPEC is omitted, the name of the output files will be derived from the input" << endl;
  cout << "files and created in the same directory." << endl << endl;
  cout << "Output on stdout is available for multipaged TIFF output only (option \"-t\")." << endl;
  cout << "Use \"-\" as output filename in this case." << endl << endl;
  cout << "In case of TIFF output, you can specify the compression by using additional" << endl;
  cout << "specifiers: " << endl;
  cout << "    -tr resp. -Tr : CCITT modified Huffman RLE" << endl;
  cout << "    -t4 resp. -T4 : CCITT Fax Class 4 compression" << endl << endl;
  cout << "The default is CCITT Fax Class 3 compression." << endl << endl;
}

//-------------------------------------------------------------

void CCmdLineProcessor::printUsage()
{
  cout << "Summary: " << m_pszProgName
       << " [-help] [-version] [options] INFILE OUTSPEC"
       << endl;
}

//-------------------------------------------------------------

void CCmdLineProcessor::printVersion()
{
  cout << endl << "SffToBmp Version " << VERSION << endl;
  cout << "Copyright (C) " << COPYRIGHT << endl;
  cout << "This is free software, and you are welcome to redistribute it under" << endl;
  cout << "the terms of the enclosed license. See the file COPYING for details." << endl;
  cout << "If you haven't got this file, please contact peter.schaefer@gmx.de." << endl;
  cout << "This program comes with ABSOLUTELY NO WARRANTY without even the implied" << endl;
  cout << "warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << endl << endl;
  cout << "Contributors:" << endl;
  cout << "Ulf Zibis <ulf.zibis@gmx.de>" << endl;
  cout << "Gernot Hillier <ghillie@suse.de>" << endl << endl;
  cout << "Library versions:" << endl << endl;
  cout << TIFFGetVersion() << endl << endl;
  cout << "Independent JPEG Group's CJPEG, version "
	<< JPEG_LIB_VERSION / 10 << "." << JPEG_LIB_VERSION % 10 << endl << endl;
  cout << "Boost libraries, version: "
	<< BOOST_VERSION / 100000 << "."
	<< BOOST_VERSION / 100 % 1000 << "."
	<< BOOST_VERSION % 100 << endl;
}

void CCmdLineProcessor::parseCmdLine()
{
  // short options string (one colon: req arg, two colon: opt arg)
  const char *shortopts = "vhbpT::t::frdj::o:q";
  // long options list
  struct option longopts[] =
  {
    // name,        has_arg,           flag, val       longind
    { "version",    no_argument,       0,    'v' }, //       0
    { "help",       no_argument,       0,    'h' }, //       1
    { "bmp",        no_argument,       0,    'b' }, //       2
    { "pbm",        no_argument,       0,    'p' }, //       3
    { "jpg",        optional_argument, 0,    'j' }, //       4
    { "tifs",       optional_argument, 0,    'T' }, //       5
    { "tif",        optional_argument, 0,    't' }, //       6
    { "force-overwrite", no_argument,  0,    'f' }, //       7
    { "keepres",    no_argument,       0,    'r' }, //       8
    { "keepdate",   no_argument,       0,    'd' }, //       9
    { "out",        required_argument, 0,    'o' }, //      10
    { "quiet",      no_argument,       0,    'q' }, //      11
    // end-of-list marker
    { 0, 0, 0, 0 }
  };
  // long option list index
  int longind = 0;
  int opt;

  m_strOutSpec = "";

  // parse all options from the command line
  while ((opt = getopt_long_only(m_argc, m_argv, shortopts, longopts, &longind)) != -1)
  {
    switch (opt)
    {
    case 'v': // -version
      printVersion();
      return;
    case 'h': // -help
      printHelp();
      return;
    case 'j': // -jpg[=NUM]
      if (m_nOutputFormat != fmt_unknown) {
        throw CSimpleException(CSimpleException::err_toomuchformats);
      }
      if (optarg) {
        // we use this while trying to parse a numeric argument
        char ignored;
        int val;
        if (sscanf(optarg, "%d%c", &val, &ignored) != 1) {
          cerr << m_pszProgName << ": jpeg quality is not a number" << endl;
          throw CSimpleException(CSimpleException::err_cmdline);
        }
        if ((val <= 0) || (val >= 100)) {
          cerr << m_pszProgName << ": jpeg quality out of range (1..99)" << endl;
          throw CSimpleException(CSimpleException::err_cmdline);
        }
        m_nJpegQuality = val;
      }
      m_nOutputFormat = fmt_jpeg;
      break;
    case 'b':
      if (m_nOutputFormat != fmt_unknown) {
        throw CSimpleException(CSimpleException::err_toomuchformats);
      }
      m_nOutputFormat = fmt_bmp;
      break;
    case 'p':
      if (m_nOutputFormat != fmt_unknown) {
        throw CSimpleException(CSimpleException::err_toomuchformats);
      }
      m_nOutputFormat = fmt_pbm;
      break;
    case 't': // -tif[=r|4]
    case 'T': // -tifs[=r|4]
      if (m_nOutputFormat != fmt_unknown) {
        throw CSimpleException(CSimpleException::err_toomuchformats);
      }
      m_nOutputFormat = (opt == 't') ? fmt_tiff : fmt_tiff_single_pages;
      if (optarg) {
        // we use this while trying to parse the compression
        switch (optarg[0]) {
          case 'r': // CCITT modified Huffman RLE
            m_nTiffCompression = COMPRESSION_CCITTRLE;
            break;
          case '4': // CCITT Fax 4
            m_nTiffCompression = COMPRESSION_CCITTFAX4;
            break;
        }
      }
      break;
    case 'f':
      m_bOverwrite = true;
      break;
    case 'r':
      m_bKeepVRes = true;
      break;
    case 'd':
      m_bKeepDate = true;
      break;
    case 'o':
      m_strOutSpec = optarg;
      break;
    case 'q':
      m_bQuiet = true;
      break;
    case '?': // getopt_long_only noticed an error
      printUsage();
      throw CSimpleException(CSimpleException::err_cmdline);
    default: // something unexpected has happened
      printUsage();
      throw CSimpleException(CSimpleException::err_cmdline);
    }
  }
  if (m_nOutputFormat == fmt_unknown) {
    throw CSimpleException(CSimpleException::err_noformat);
  }
  if ((m_strOutSpec == "-") && (m_nOutputFormat != fmt_tiff)) {
    throw CSimpleException(CSimpleException::err_stdoutnotallowed);
  }

#ifdef WIN32
  WIN32_FIND_DATA fd;
  fs::path fp = fs::path(m_argv[optind], fs::native);
  fp.remove_filename();
  HANDLE hFind = ::FindFirstFile(m_argv[optind], &fd);
  if (hFind == INVALID_HANDLE_VALUE) {
    throw CSimpleException( CSimpleException::err_openfile);
  }
  do {
    if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      fs::path fp1 = fp;
      fp1 /= fs::path(fd.cFileName, fs::native);
      m_vFiles.push_back( fp1  );
    }
  } while (::FindNextFile(hFind, &fd));
  ::FindClose(hFind);
#else
  int n = optind;
  while (n < m_argc) {
#if !defined(BOOST_FILESYSTEM_VERSION) || BOOST_FILESYSTEM_VERSION == 2
    m_vFiles.push_back( fs::path(m_argv[n], fs::native) );
#else
    m_vFiles.push_back( fs::path(m_argv[n]) );
#endif
    ++n;
  }
#endif

  if ((m_strOutSpec == "-") && (getFileCount() > 1)) {
    throw CSimpleException(CSimpleException::err_stdoutonlyonefile);
  }

}

bool CCmdLineProcessor::getNextFile(fs::path& strFile)
{
  bool rc = false;
  fs::path fname;
  if (m_nFileIdx < getFileCount()) {
    strFile = m_vFiles[m_nFileIdx];
    ++m_nFileIdx;
    rc = true;
  }
  return rc;
}

const std::string& CCmdLineProcessor::getOutSpec()
{
  return m_strOutSpec;
}
