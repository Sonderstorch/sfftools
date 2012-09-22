// Input classes
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
//   Ulf Zibis <ulf.zibis@gmx.de> (resolution preserving, RVS COM resolutions,
//                                 inheritance from CFile)
//
// You can contact the original author by email at peter.schaefer@gmx.de.
//
// I'm always pleased to hear that somebody is actually using my software.
// If you can manage it, e-mail me a quick notice. Thanks!
//
/*-RCS-Info----------------------------------------------------

 $Id: input.cpp,v 1.6 2008/09/13 13:06:26 pschaefer Exp $

---RCS-Info--------------------------------------------------*/

#include <cstring>
#include <vector>
#include <iostream>

#include "sfftypes.h"
#include "common.h"

extern "C"
{
#define XMD_H
#include <jpeglib.h>
#include <tiffio.h>
}

#include "codes.h"
#include "output.h"
#include "decoder.h"
#include "input.h"

using namespace std;

//-Constants-------------------------------------------------------

sff_byte CSffFile::m_SFFID[4] = { 0x53, 0x66, 0x66, 0x66 };

//-Types-----------------------------------------------------------

typedef enum {
  NEED_MAGIC,
  NEED_PAGESTART,
  NEED_PAGEHEADER,
  NEED_RECORD,
  LAST_PAGE
} TScannerState;

//-----------------------------------------------------------------

CSffFile::CSffFile(const std::string& strFileName) :
  CFile(strFileName)
{
  ScanFile();
}

CSffFile::~CSffFile()
{
  PAGEVECTOR::iterator it = m_acPages.begin();
  for (; it != m_acPages.end(); it++) {
    delete (*it);
  }
}

void CSffFile::ScanFile()
{
  TSFFFileHeader dh;
  TSFFPageHeader ph;
  sff_byte       b1 = 0, b2 = 0;
  sff_word       w;
  int            nLineCount = 0;
  int            fuzz = 0;
  TSFFPage      *pPage;

  if (Eof())
    return;

  TScannerState state = NEED_MAGIC;
  do {
    switch (state) {
    case NEED_MAGIC :
      for (fuzz = 0; fuzz < 2048; ++fuzz) {
        Seek(fuzz, CFile::sk_from_start);
        Read(&dh, sizeof(dh));
        if (Eof())
          throw CSimpleException(CSimpleException::err_invalidfile);
        if (::memcmp(&dh.sff_id, &m_SFFID, sizeof(m_SFFID)) == 0)
          break;
      }
      if (::memcmp(&dh.sff_id, &m_SFFID, sizeof(m_SFFID)) != 0)
        throw CSimpleException(CSimpleException::err_invalidfile);
      if (dh.version > 1)
        throw CSimpleException(CSimpleException::err_invalidversion);
      if (fuzz>0) {
        cerr << "NOTE: File starts with " << fuzz << " bytes of garbage." << endl;
        dh.first_page += fuzz;
      }
      Seek(dh.first_page, CFile::sk_from_start);
      state = NEED_PAGESTART;
      break;
    case NEED_PAGESTART :
      b1 = GetC(); // record header (0xFE fuer beginning of page)
      if (Eof())
        throw CSimpleException(CSimpleException::err_corruptfile);
      if (b1 != 0xFE)
        throw CSimpleException(CSimpleException::err_corruptfile);
      b1 = GetC(); // record length (usually 0x10)
      if (b1 == 0)
        state = LAST_PAGE;
      else
        state = NEED_PAGEHEADER;
      break;
    case NEED_PAGEHEADER :
      Read(&ph, sizeof(TSFFPageHeader));
      if (Eof())
        throw CSimpleException(CSimpleException::err_corruptfile);
      if (ph.coding > 0)
        throw CSimpleException(CSimpleException::err_unknowncoding);
      Seek(b1 - sizeof(TSFFPageHeader), CFile::sk_current); // skip user data
      pPage = new TSFFPage;
      pPage->filepos = Tell();
      pPage->width   = ph.linelen;
      pPage->height  = ph.pagelen;
      // Values 254/255 are known for RVS COM
      pPage->dpi = (ph.horiz_res == 0) ? 203 :
                   (ph.horiz_res == 255) ? 300 :
                   (ph.horiz_res == 254) ? 400 : 0;
      pPage->lpi = (ph.vert_res == 0) ?  98 :
                   (ph.vert_res == 1) ? 196 :
                   (ph.vert_res == 255) ? 300 :
                   (ph.vert_res == 254) ? 400 : 0;
      m_acPages.push_back(pPage);
      state = NEED_RECORD;
      nLineCount = 0;
      break;
    case NEED_RECORD :
      b1 = GetC(); // read record type
      if (Eof()) {
        m_acPages[GetPageCount()-1]->height = nLineCount;
        state = LAST_PAGE;
      } else if (b1 == 0) {
        // variable amount of bytes following
        b1 = GetC(); // LSB
        if (Eof())
          throw CSimpleException(CSimpleException::err_corruptfile);
        b2 = GetC(); // MSB
        if (Eof())
          throw CSimpleException(CSimpleException::err_corruptfile);
        w = ((b2 << 8) | b1);
        Seek(w, CFile::sk_current); // skip data
        if (Eof())
          throw CSimpleException(CSimpleException::err_corruptfile);
        ++nLineCount;
      } else if (b1 < 217) {
        // normal amount of bytes following
        Seek((long)b1, CFile::sk_current);  // skip data
        if (Eof()) {
          throw CSimpleException(CSimpleException::err_corruptfile);
        }
        ++nLineCount;
      } else if (b1 < 254) {
        // white skip
        nLineCount+=(b1 - 216);
      } else if (b1 < 255) {
        // 254 -> page header
        m_acPages[GetPageCount()-1]->height = nLineCount;
        nLineCount = 0;
        b1 = GetC(); // record length (usually 0x10)
        if (Eof())
          throw CSimpleException(CSimpleException::err_corruptfile);
        if (b1 == 0) {
          state = LAST_PAGE;
        } else {
          state = NEED_PAGEHEADER;
        }
      } else {
        // bad line or user info
        b1 = GetC(); // LSB
        if (Eof())
          throw CSimpleException(CSimpleException::err_corruptfile);
        if (b1 == 0) {
          ++nLineCount;
        } else {
          Seek(b1, CFile::sk_current);  // skip user info
          if (Eof())
            throw CSimpleException(CSimpleException::err_corruptfile);
        }
      }
      break;
    case LAST_PAGE :
      break;
    }
  } while(state != LAST_PAGE);
  return;
}

bool CSffFile::PageIsValid(int nPage)
{
	return ((nPage >= 0) && (nPage < GetPageCount()));
}

bool CSffFile::SeekPage(int nPage)
{
	if (!PageIsValid(nPage))
		return false;
  Seek(m_acPages[nPage]->filepos, CFile::sk_from_start);
	return true;
}

TSFFPage *CSffFile::GetPage(int nPage)
{
  return PageIsValid(nPage) ? m_acPages[nPage] : (TSFFPage *)NULL;
}

bool CSffFile::IsLowRes(int nPage)
{
  return PageIsValid(nPage) && (m_acPages[nPage]->lpi == 98);
}

sff_word CSffFile::GetHorizontalResolution(int nPage)
{
  return PageIsValid(nPage) ? m_acPages[nPage]->dpi :0;
}

sff_word CSffFile::GetVerticalResolution(int nPage)
{
  return PageIsValid(nPage) ? m_acPages[nPage]->lpi :0;
}

sff_dword CSffFile::GetPageWidth(int nPage)
{
  return PageIsValid(nPage) ? m_acPages[nPage]->width :0;
}

sff_dword CSffFile::GetPageHeight(int nPage)
{
  return PageIsValid(nPage) ? m_acPages[nPage]->height :0;
}

bool CSffFile::GetRecord(TSFFRecord& rec)
{
  sff_byte b1, b2;
  sff_word w;
  bool result;

  if (Eof()) {
    return false;
  }

  b1 = GetC();  // read record type
  if (Eof()) {
    result = FALSE;
  } else if (b1 == 0) {
    // variable amount of bytes following
    b1 = GetC(); // LSB
    b2 = GetC(); // MSB
    w = ((b2 << 8) | b1);
    rec.type  = NORMAL;
    rec.cb    = w;
    rec.pData = (sff_byte *)malloc(w);
    Read(rec.pData, w);
    result = TRUE;
  } else if (b1 < 217) {
    // normal amount of bytes following
    rec.type  = NORMAL;
    rec.cb    = b1;
    rec.pData = (sff_byte *)malloc(b1);
    Read(rec.pData, b1);
    result = TRUE;
  } else if (b1 < 254) {
    // white skip
    rec.type  = WHITESKIP;
    rec.cb    = (b1 - 216);
    rec.pData = 0;
    result = TRUE;
  } else if (b1 < 255) {
    // 254 -> page header
    result = FALSE;
  } else {
    // bad line or user info
    b1 = GetC(); // LSB
    if (b1 == 0) {
      rec.type  = BADLINE;
      rec.cb    = 1;
      rec.pData = 0;
    } else {
      rec.type  = USERINFO;
      rec.cb    = b1;
      rec.pData = (sff_byte *)malloc(b1);
      Read(rec.pData, b1);
    }
    result = TRUE;
  }
  return result;
}

bool CSffFile::DecodeRecord(TSFFRecord& rec, CBitSink& bitsink)
{
  bool  rc;

  if (rec.type != NORMAL)
    return FALSE;

  CHuffDecoder source(rec.pData, rec.cb);

  try {
    rec.runlength = source.DecodeLine(bitsink);
    rc = TRUE;
  }
  catch(CSimpleException e)
  {
    cerr << "ERROR: " << e.what() << endl;
    rc = FALSE;
  }
  return rc;
}
