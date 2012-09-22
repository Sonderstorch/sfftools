#ifndef __OUTPUT_H__
#define __OUTPUT_H__
//
// Headerfile for output classes
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
//    Gernot Hillier <ghillie@suse.de> (StdOut output support)
//    Ulf Zibis <ulf.zibis@gmx.de> (Resolution preserving output, Cleanups,
//       inheritance from CFile, destructor handling, preserving filetime)
//
// You can contact the original author by email at peter.schaefer@gmx.de.
//
// I'm always pleased to hear that somebody is actually using my software.
// If you can manage it, e-mail me a quick notice. Thanks!
//
/*-RCS-Info----------------------------------------------------

 $Id: output.h,v 1.4 2009/08/23 12:58:37 pschaefer Exp $

---RCS-Info--------------------------------------------------*/

#include "decoder.h"

#pragma pack(1)

typedef struct {
    sff_byte    bBlue;
    sff_byte    bGreen;
    sff_byte    bRed;
    sff_byte    bNOP;
} TBmpRGB;

typedef struct {
    sff_word    wType;
    sff_dword   dwFileSize;
    sff_word    wNOP1;
    sff_word    wNOP2;
    sff_dword   dwOffset;
    sff_dword   dwBitmapSize;
    sff_dword   dwPixelWidth;
    sff_dword   dwPixelHeight;
    sff_word    wPlanes;
    sff_word    wCountBits;
    sff_dword   dwCompression;
    sff_dword   dwSizeImage;
    sff_dword   dwPixelWidthPerMeter;
    sff_dword   dwPixelHeightPerMeter;
    sff_dword   dwUsedColors;
    sff_dword   dwImportantColors;
    TBmpRGB     sCol1;
    TBmpRGB     sCol2;
} TBitMapFileHeader;

#pragma pack()

//-----------------------------------------------------------------

class COutputFilter
{
protected:
  CBitSink    m_sink;
  sff_byte    m_abBuffer[5120];
  sff_word    m_nPageCount;
  std::string m_strExtension;
  CFile      *m_pFile;

public:
  COutputFilter(const std::string& strExt, sff_word nPagecount) :
      m_strExtension(strExt),
      m_nPageCount(nPagecount),
      m_sink(m_abBuffer, sizeof(m_abBuffer)),
      m_pFile(0)
       { /* nth. else */ }

  virtual void Init(CFile *pFile);
  virtual void BeginPage(sff_dword aPage, sff_dword aWidth,
                         sff_dword aHeight, sff_word aDpi,
                         sff_word aLpi) = 0;
  virtual void EndPage();
  virtual void Finalize();

  virtual CBitSink& GetBitSink();
  virtual void BlankLine();
  virtual void WriteLine() = 0;

  const std::string& GetExtension() { return m_strExtension; };
};

//-----------------------------------------------------------------

class CBMPFilter : public COutputFilter
{
protected:
  static TBitMapFileHeader m_bmpHeader;
  int m_nLineToWrite;

public:
  CBMPFilter(sff_word nPagecount) :
    COutputFilter(".bmp", nPagecount),
    m_nLineToWrite(0) { /* nth. else */ };

  void BeginPage(sff_dword aPage, sff_dword aWidth,
                 sff_dword aHeight, sff_word aDpi,
                 sff_word aLpi);
  void WriteLine();
};

//-----------------------------------------------------------------

class CPBMFilter : public COutputFilter
{
protected:
  sff_dword m_Width;

public:
  CPBMFilter(sff_word nPagecount) :
    COutputFilter(".pbm", nPagecount),
    m_Width(0)
    { /* nth. else */ };

  void BeginPage(sff_dword aPage, sff_dword aWidth,
                 sff_dword aHeight, sff_word aDpi,
                 sff_word aLpi);
  void WriteLine();
};

//-----------------------------------------------------------------

class CJPEGFilter : public COutputFilter
{
protected:
  CByteSink             m_bytesink;
  jpeg_compress_struct  m_cinfo;
  jpeg_error_mgr        m_jerr;
  JSAMPROW              m_row_pointer[1];
  int                   m_quality;

public:
  CJPEGFilter(sff_word nPagecount, int nQuality = 40) :
    COutputFilter(".jpg", nPagecount),
    m_quality(nQuality),
    m_bytesink(m_abBuffer, sizeof(m_abBuffer))
    { /* nth. else */ };

  CBitSink& GetBitSink();
  void BlankLine();

  void BeginPage(sff_dword aPage, sff_dword aWidth,
                 sff_dword aHeight, sff_word aDpi,
                 sff_word aLpi);
  void WriteLine();
  void EndPage();
};

//-----------------------------------------------------------------

class CTIFFFilter : public COutputFilter
{
protected:
  TIFF *             m_tiffFile;
  unsigned           m_nRow;
  sff_dword          m_wCompression;
  time_t             m_nModTime;

public:
  CTIFFFilter(sff_word nPagecount, sff_word nCompression, time_t nModTime) :
    COutputFilter(".tif", nPagecount),
    m_wCompression(nCompression),
    m_tiffFile(NULL),
    m_nModTime(nModTime),
    m_nRow(0)
    { /* nth. else */ };

  void Init(CFile *pFile);

  void BeginPage(sff_dword aPage, sff_dword aWidth,
                 sff_dword aHeight, sff_word aDpi,
                 sff_word aLpi);
  void WriteLine();
  void EndPage();
  void Finalize();
};

#endif // __OUTPUT_H__
