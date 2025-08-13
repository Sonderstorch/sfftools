#ifndef __INPUT_H__
#define __INPUT_H__
//
// Headerfile for input classes
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

 $Id$

---RCS-Info--------------------------------------------------*/

typedef enum {
  NORMAL,
  WHITESKIP,
  BADLINE,
  USERINFO
} TSFFRecordType;

typedef struct{
  sff_dword sff_id;
  sff_byte  version;
  sff_byte  reserved;
  sff_word  user_info;
  sff_word  num_pages;
  sff_word  first_page;
  sff_dword last_page;
  sff_dword file_size;
} TSFFFileHeader;

typedef struct{
  sff_byte  vert_res;
  sff_byte  horiz_res;
  sff_byte  coding;
  sff_byte  specials;
  sff_word  linelen;
  sff_word  pagelen;
  sff_dword prev_page;
  sff_dword next_page;
} TSFFPageHeader;

typedef struct{
  TSFFRecordType type;
  sff_word       cb;
  sff_dword      runlength;
  sff_byte      *pData;
} TSFFRecord;

typedef struct{
  sff_dword width;
  sff_dword height;
  sff_word  dpi;
  sff_word  lpi;
  off_t     filepos;
} TSFFPage;

//-----------------------------------------------------------------

typedef std::vector<TSFFPage *> PAGEVECTOR;

//-----------------------------------------------------------------

class CSffFile : public CFile
{
protected:
  static sff_byte m_SFFID[4];
  PAGEVECTOR  m_acPages;

  void ScanFile();
    // throw CSimpleException

public:
  CSffFile(const std::string& strFileName);
  ~CSffFile();

  int       GetPageCount() { return m_acPages.size(); };

  bool      PageIsValid(int nPage);
  bool      SeekPage(int nPage);
  TSFFPage *GetPage(int nPage);

  bool      IsLowRes(int nPage);

  sff_word  GetHorizontalResolution(int nPage);
  sff_word  GetVerticalResolution(int nPage);

  sff_dword GetPageWidth(int nPage);
  sff_dword GetPageHeight(int nPage);

  bool      GetRecord(TSFFRecord& rec);
  bool      DecodeRecord(TSFFRecord& rec, CBitSink& bitsink);
};

#endif // __INPUT_H__
