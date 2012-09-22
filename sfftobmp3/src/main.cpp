// Main program file
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

#include <cassert>
#include <vector>
#include <iostream>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

extern "C"
{
#define XMD_H
#include <jpeglib.h>
#include <tiffio.h>
}

#include "sfftypes.h"
#include "common.h"

#include "codes.h"
#include "output.h"
#include "decoder.h"
#include "input.h"
#include "cmdline.h"

#include "main.h"

using namespace std;
namespace fs = boost::filesystem;

int main( int argc, char *argv[] )
{
  int   rc = 0;

  fs::path pathInFileName;
  fs::path pathOutFileName;
  fs::path pathOutDirectory;

  CCmdLineProcessor proc(argv, argc);

  int nIdx = 0;
  try
  {
    proc.parseCmdLine();

    if (!proc.getFileCount())
      return 0;

    const string& strOutSpec = proc.getOutSpec();

    bool bStdOut = (strOutSpec == "-");
    bool bQuiet = proc.beQuiet() || bStdOut;

    if (strOutSpec.length() && (strOutSpec != "-")) {
      if (proc.getFileCount() > 1) {
        // More than one input file -> interpret OutSpec as a directory
        if (!fs::exists(strOutSpec)) {           // Create directory, if necessary
          if (!bQuiet) cout << endl << "Creating directory " << strOutSpec.c_str() << "." << endl;
          try {
            fs::create_directories(strOutSpec);
          }
          catch (const std::exception&) {
          }
        }
        // Check that dir exists
        if (!(fs::exists(strOutSpec) && fs::is_directory(strOutSpec))) {
          throw CSimpleException(CSimpleException::err_outdir);
        }
        pathOutDirectory = strOutSpec;
      } else {
        // Exact one input file -> interpret OutSpec as one file
        if (fs::exists(strOutSpec) && fs::is_directory(strOutSpec)) {
          throw CSimpleException(CSimpleException::err_outfileisdir);
        }
        pathOutFileName = strOutSpec;
      }
    }

    while (proc.getNextFile(pathInFileName))
    {
      COutputFilter *pOut = NULL;
      CSffFile      *pInfile = NULL;
      char           acNumber[10];

      try
      {
        fs::path outPath;

        CSffFile *pInfile = new CSffFile(pathInFileName.string());
        int nPageCount = pInfile->GetPageCount();
        if (!bQuiet)
          cout << "File " << pathInFileName.string()
               << " seems to have " << nPageCount << " page(s)." << endl;
        int nFileCountOut = nPageCount;
        time_t modTime = pInfile->GetModificationTime();

        switch (proc.getOutputFormat())
        {
          case CCmdLineProcessor::fmt_bmp:
            pOut = new CBMPFilter(nPageCount);
            break;
          case CCmdLineProcessor::fmt_pbm:
            pOut = new CPBMFilter(nPageCount);
            break;
          case CCmdLineProcessor::fmt_jpeg:
            pOut = new CJPEGFilter(nPageCount, proc.getJpegQuality());
            break;
          case CCmdLineProcessor::fmt_tiff:
            nFileCountOut = 1;  // all pages in one file
            pOut = new CTIFFFilter(nPageCount,
              proc.getTiffCompression(), modTime);
            break;
          case CCmdLineProcessor::fmt_tiff_single_pages:
            pOut = new CTIFFFilter(nPageCount,
              proc.getTiffCompression(), modTime);
            break;
        }

        CFile fileOut;
        if (bStdOut) {
          fileOut.OpenTemp();
          pOut->Init(&fileOut);
        }

        for (int nPage = 0; nPage < nPageCount; nPage++)
        {
          TSFFPage* pPage = pInfile->GetPage(nPage);
          if (pPage->height <= 0) {
            if (!bQuiet) {
              cout << "Skipping page (no lines)." << endl;
            }
            continue;
          }
          if (!bStdOut)
          {
            if (pathOutFileName.string().length()) {
              // A fixed name was given, so use it as a base name
              outPath = pathOutFileName;
              std::string orgExt = fs::extension(outPath);
              if (nFileCountOut > 1) {
                sprintf(acNumber, "_%03d", nPage+1);
                outPath = fs::change_extension(outPath, acNumber);
                if (orgExt.length()) {
                  std::string strTemp = outPath.string();
                  strTemp += orgExt;
                  outPath = fs::path(strTemp);
                }
              }
            } else {
              // Otherwise construct output filename from input filename
              outPath = pathOutDirectory / pathInFileName.leaf();
              if (nFileCountOut > 1) {
                sprintf(acNumber, "_%03d", nPage+1);
                outPath = fs::change_extension(outPath, acNumber);
                std::string strTemp = outPath.string();
                strTemp += pOut->GetExtension();
                outPath = fs::path(strTemp);
              } else {
                outPath = fs::change_extension(outPath, pOut->GetExtension());
              }
            }
            if (!proc.doOverwrite() && !((nPage > 0) && (nFileCountOut == 1)) && fs::exists(outPath)) {
              throw CSimpleException(CSimpleException::err_outfileexists);
            }
          }

          bool bIsLowRes = pInfile->IsLowRes(nPage);
          bool bDoubleLines = !proc.keepVRes() && bIsLowRes;

          if (!bQuiet) {
            if (!((nPage > 0) && (nFileCountOut == 1))) {
              cout << "- Destination File " << outPath.string() << " : " << endl;
            }
            cout << "  Converting page " << nPage+1
               << " (" << pPage->width << "x" << pPage->height << "px / ";
            cout << pPage->dpi << "x" << pPage->lpi << "dpi), ";
            cout << (bIsLowRes ? "LowRes" : "HiRes") << " ..." << endl;
          }

          if (!pInfile->SeekPage(nPage)) {
            throw CSimpleException(CSimpleException::err_corruptfile);
          }

          if (!bStdOut && !((nPage > 0) && (nFileCountOut == 1))) {
            fileOut.Open(outPath.string(), "wb+");
            pOut->Init(&fileOut);
          }

          if (bDoubleLines) {
            pOut->BeginPage(nPage, pPage->width,
              pPage->height*2, pPage->dpi, pPage->lpi*2);
          } else {
            pOut->BeginPage(nPage, pPage->width,
              pPage->height, pPage->dpi, pPage->lpi);
          }

          pOut->BlankLine();

          TSFFRecord rec;

          while (pInfile->GetRecord(rec))
          {
          switch(rec.type)
          {
            case NORMAL :
              pOut->BlankLine();
              if (pInfile->DecodeRecord(rec, pOut->GetBitSink())) {
                pOut->WriteLine();
                if (bDoubleLines) {
                  pOut->WriteLine();
                }
              }
              if (rec.pData != 0) free(rec.pData);
              break;
            case USERINFO :
              // not yet considered
              if (rec.pData != 0) free(rec.pData);
              break;
            case BADLINE :
              pOut->WriteLine();
              if (bDoubleLines) {
                pOut->WriteLine();
              }
              break;
            case WHITESKIP :
              pOut->BlankLine();
              for (int j=0; j < rec.cb; ++j) {
                pOut->WriteLine();
                if (bDoubleLines) {
                  pOut->WriteLine();
                }
              }
              break;
            }
          }
          pOut->EndPage();

          if (!bStdOut) {
            if ((nFileCountOut > 1) || ((nFileCountOut == 1) && (nPage == nPageCount-1))) {
              pOut->Finalize();
              fileOut.Close();
              if (proc.keepDate()) {
                fileOut.SetModificationTime(modTime);
              }
            }
          }
        }

        if (bStdOut) {
          fileOut.DumpToStdOut();
          fileOut.Close();
          if (pOut) {
            pOut->Finalize();
          }
        }

      }
      catch (const std::exception & e) {
        cerr << "ERROR: " << pathInFileName.string() << ": " << e.what() << endl;
        rc = 2;
      }
      catch (CSimpleException e) {
        cerr << "ERROR: " << pathInFileName.string() << ": " << e.what() << endl;
        rc = 2;
      }
      if (pOut) {
        delete pOut;
      }
      if (pInfile) {
        delete pInfile;
      }
      if (!bQuiet) cout << endl;
    }
    if (!bQuiet) cout << "Finished. " << endl << endl;
  }
  catch (const std::exception & e) {
    cerr << "ERROR: " << e.what() << endl;
    rc = 2;
  }
  catch (const CSimpleException& e) {
    cerr << "ERROR: " << e.what() << endl;
    rc = 2;
  }
  return rc;
}
