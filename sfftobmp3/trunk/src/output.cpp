// Output classes
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

 $Id$

---RCS-Info--------------------------------------------------*/

#include <cstring>
#include <cassert>
#include <iostream>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

#ifdef WIN32
#include <io.h>
#endif

#include "sfftypes.h"
#include "common.h"

extern "C"
{
#define XMD_H
#include <jpeglib.h>
#include <tiffio.h>
}

#include "codes.h"
#include "decoder.h"
#include "output.h"

using namespace std;
namespace fs = boost::filesystem;

//-Constants-------------------------------------------------------

TBitMapFileHeader CBMPFilter::m_bmpHeader =
{
  0x4d42,                                 /* magic 'BM' */
    0,                                    /* filesize */
    0,                                    /* reserved */
    0,                                    /* reserved */
    sizeof(TBitMapFileHeader),            /* offset to bitmap data */
    0x28,                                 /* length of bitmap info header 0x28 -> Windows BMP */
    0,                                    /* width in pixels */
    0,                                    /* height in pixels */
    1,                                    /* no. of planes */
    1,                                    /* bpp -> monochrome */
    0,                                    /* compression -> none */
    0,                                    /* size of bitmap data in bytes rounded to 4 byte boundary */
    0,                                    /* horz. resolution in pixels/meter */
    0,                                    /* vert. resolution in pixels/meter */
    2,                                    /* no. of colours */
    2,                                    /* no. of important colours */
  { 255,255,255,  0 },                    /* palette */
  {   0,  0,  0,  0 }
};

//-----------------------------------------------------------------

CBitSink& COutputFilter::GetBitSink()
{
  m_sink.Reset();
  return m_sink;
}

void COutputFilter::BlankLine()
{
  ::memset(m_abBuffer, 0x00, sizeof(m_abBuffer));
}

void COutputFilter::Init(CFile *pFile)
{
  m_pFile = pFile;
}

void COutputFilter::EndPage()
{
}

void COutputFilter::Finalize()
{
  // base class does nothing
}

//-----------------------------------------------------------------

void CBMPFilter::BeginPage(sff_dword aPage, sff_dword aWidth,
                           sff_dword aHeight, sff_word aDpi,
                           sff_word aLpi)
{
  assert(m_pFile);

  m_bmpHeader.dwPixelWidth  = aWidth;
  m_bmpHeader.dwPixelHeight = aHeight;
  m_bmpHeader.dwPixelWidthPerMeter = (aDpi*10000+127)/254;
  m_bmpHeader.dwPixelHeightPerMeter = (aLpi*10000+127)/254;
  m_bmpHeader.dwSizeImage = (m_bmpHeader.dwPixelWidth >> 3) * m_bmpHeader.dwPixelHeight;
  m_nLineToWrite = aHeight;

  m_pFile->Seek(0, CFile::sk_from_start);
  m_pFile->Write(&m_bmpHeader, sizeof(m_bmpHeader));
}

void CBMPFilter::WriteLine()
{
  assert(m_pFile);

  sff_dword dwBytesPerLine = (m_bmpHeader.dwPixelWidth >> 3);
  /* we write lines from the end to the top of the file */
  if (m_nLineToWrite-- > 0) {
    m_pFile->Seek(dwBytesPerLine * m_nLineToWrite + sizeof(m_bmpHeader), CFile::sk_from_start);
    m_pFile->Write(m_abBuffer, dwBytesPerLine);
  }
}

//-----------------------------------------------------------------

void CPBMFilter::BeginPage(sff_dword aPage, sff_dword aWidth,
                           sff_dword aHeight, sff_word aDpi,
                           sff_word aLpi)
{
  char acBuf[50];

  assert(m_pFile);

  m_Width = aWidth;
  m_pFile->Seek(0, CFile::sk_from_start);
  sprintf(acBuf, "%s", "P4\n");
  m_pFile->Write(acBuf, strlen(acBuf));
  sprintf(acBuf, "%s", "# generated with SffToBmp\n");
  m_pFile->Write(acBuf, strlen(acBuf));
  sprintf(acBuf, "%ld %ld\n", (long int) aWidth, (long int) aHeight);
  m_pFile->Write(acBuf, strlen(acBuf));
}

void CPBMFilter::WriteLine()
{
  assert(m_pFile);
  m_pFile->Write(m_abBuffer, m_Width >> 3);
}

//-----------------------------------------------------------------

void CJPEGFilter::BeginPage(sff_dword aPage, sff_dword aWidth,
                            sff_dword aHeight, sff_word aDpi,
                            sff_word aLpi)
{
  assert(m_pFile);
  memset(&m_cinfo, 0, sizeof(m_cinfo));
  m_cinfo.err = jpeg_std_error(&m_jerr);
  jpeg_create_compress(&m_cinfo);
  m_cinfo.in_color_space = JCS_GRAYSCALE;
  jpeg_set_defaults(&m_cinfo);
  m_cinfo.image_width = aWidth;
  m_cinfo.image_height = aHeight;
  m_cinfo.input_components = 1;
  m_cinfo.dct_method = JDCT_FLOAT;
  m_cinfo.density_unit = 1;
  m_cinfo.X_density = aDpi;
  m_cinfo.Y_density = aLpi;
  jpeg_stdio_dest(&m_cinfo, m_pFile->GetFP());
  jpeg_set_quality(&m_cinfo, m_quality, FALSE);
  jpeg_set_colorspace(&m_cinfo, JCS_GRAYSCALE);
  jpeg_start_compress(&m_cinfo, TRUE);
}

void CJPEGFilter::WriteLine()
{
  m_row_pointer[0] = m_abBuffer;
  jpeg_write_scanlines(&m_cinfo, m_row_pointer, 1);
}

void CJPEGFilter::EndPage()
{
  assert(m_pFile);
  jpeg_finish_compress(&m_cinfo);
  jpeg_destroy_compress(&m_cinfo);
}

CBitSink& CJPEGFilter::GetBitSink()
{
  m_bytesink.Reset();
  return m_bytesink;
}

void CJPEGFilter::BlankLine()
{
  ::memset(m_abBuffer, 0xFF, sizeof(m_abBuffer));
}

//-----------------------------------------------------------------

void CTIFFFilter::Init(CFile *pFile)
{
  COutputFilter::Init(pFile);
}

void CTIFFFilter::BeginPage(sff_dword aPage, sff_dword aWidth,
                            sff_dword aHeight, sff_word aDpi,
                            sff_word aLpi)
{
  tm *pTime;
  char acTime[25];

  if (!m_tiffFile) {
#ifndef WIN32
    m_tiffFile = TIFFFdOpen(m_pFile->GetFN(), m_pFile->GetFileName(), "w");
#else
    m_tiffFile = TIFFFdOpen(_get_osfhandle(m_pFile->GetFN()),
      m_pFile->GetFileName(), "w");
#endif
  }
  m_nRow = 0;
  // --- TIFF required:
  TIFFSetField(m_tiffFile, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
  TIFFSetField(m_tiffFile, TIFFTAG_IMAGEWIDTH, aWidth);
  // TIFFTAG_IMAGELENGTH ...automaticaly set by TIFFWriteScanline()
  // TIFFTAG_STRIPOFFSETS ...automaticaly set by TIFFWriteScanline()
  TIFFSetField(m_tiffFile, TIFFTAG_ROWSPERSTRIP, aHeight);
  // TIFFTAG_STRIPBYTECOUNTS ...automaticaly set by TIFFWriteScanline()
  TIFFSetField(m_tiffFile, TIFFTAG_XRESOLUTION, (float)aDpi);
  TIFFSetField(m_tiffFile, TIFFTAG_YRESOLUTION, (float)aLpi);
  TIFFSetField(m_tiffFile, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
  // --- Class B required:
  TIFFSetField(m_tiffFile, TIFFTAG_BITSPERSAMPLE, 1);
  TIFFSetField(m_tiffFile, TIFFTAG_COMPRESSION, m_wCompression);
  TIFFSetField(m_tiffFile, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE);
  TIFFSetField(m_tiffFile, TIFFTAG_SAMPLESPERPIXEL, 1);
  // --- Class F required:
  TIFFSetField(m_tiffFile, TIFFTAG_FAXMODE, FAXMODE_CLASSF);
  TIFFSetField(m_tiffFile, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
  if (m_wCompression == COMPRESSION_CCITTFAX3) {
    TIFFSetField(m_tiffFile, TIFFTAG_GROUP3OPTIONS, GROUP3OPT_FILLBITS|GROUP3OPT_2DENCODING);
  }
  TIFFSetField(m_tiffFile, TIFFTAG_PAGENUMBER, aPage, m_nPageCount);
  // --- Class F recommended:
  TIFFSetField(m_tiffFile, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(m_tiffFile, TIFFTAG_SOFTWARE, "SffToBmp");
  // - Some viewers interpret this as fax sender
  TIFFSetField(m_tiffFile, TIFFTAG_IMAGEDESCRIPTION, "");
  // - Some viewers interpret this as receive time
  // - Has to be exactly 19 chars: 2007-01-01 24:00:00
  if ((pTime = ::gmtime(&m_nModTime))) {
    strftime(acTime, 20, "%Y-%m-%d %H:%M:%S", pTime);
  }
  acTime[19]='\0';
  TIFFSetField(m_tiffFile, TIFFTAG_DATETIME, acTime);
  // - not recommended, but required by TIFFWriteScanline(); why ?:
  TIFFSetField(m_tiffFile, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  //TIFFSetWarningHandler(NULL);
}

void CTIFFFilter::WriteLine()
{
  if (m_tiffFile) {
    TIFFWriteScanline(m_tiffFile, m_abBuffer, m_nRow++,0);
  }
}

void CTIFFFilter::EndPage()
{
  TIFFWriteDirectory(m_tiffFile);
}

void CTIFFFilter::Finalize()
{
  if (m_tiffFile) {
    TIFFClose(m_tiffFile);
  }
  m_tiffFile = NULL;
}

