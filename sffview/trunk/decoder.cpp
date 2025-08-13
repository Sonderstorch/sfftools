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

 $Id: decoder.cpp,v 1.2 2008/03/21 13:47:01 pschaefer Exp $

---RCS-Info--------------------------------------------------*/

#include <wx/wx.h>
#include <wx/version.h>

#include <iostream>

#include "sfftypes.h"
#include "common.h"
#include "codes.h"
#include "decoder.h"

using namespace std;

//-Types-----------------------------------------------------------

typedef enum {
  NEED_WHITE,
  NEED_BLACK,
  NEED_WHITETERM,
  NEED_BLACKTERM
} TDecoderState;

//-----------------------------------------------------------------

void CBitSource::NeedBits(int nCount)
{
  while ((m_dwByteCount > 0) && (m_wBitsAvail < nCount)) {
    m_dwAccu |= ((*m_pBuffer) << m_wBitsAvail);
    m_wBitsAvail += 8;
    --m_dwByteCount;
    ++m_pBuffer;
  }
}

void CBitSource::ClrBits(int nCount)
{
  m_wBitsAvail -= nCount;
  m_dwAccu = (m_dwAccu >> nCount);
}

sff_word CBitSource::GetBits(int nCount)
{
  return (sff_word)(m_dwAccu & ((1<<(nCount))-1));  // untere x Bits ausmaskieren
}

bool CBitSource::NoMoreBits()
{
  return ((m_dwByteCount <= 0) && (m_dwAccu == 0));
}

CBitSource::CBitSource(void *pBuffer, sff_dword nByteCount) :
	m_pBuffer((sff_byte *)pBuffer), 
	m_wBitsAvail(0),
	m_dwAccu(0),
	m_dwByteCount(nByteCount)
{ 
  /* sonst nix */ 
}

//-----------------------------------------------------------------

void CBitSink::SetBits(int nCount) 
{
  sff_byte *p;

  p = m_pBuffer + (m_dwBitPos >> 3);
#if defined(__WXMSW__) && (wxVERSION_NUMBER < 2600)
  sff_byte mask = 0x80 >> (m_dwBitPos % 8);
#else
  sff_byte mask = 0x01 << (m_dwBitPos % 8);
#endif
  m_dwBitPos += nCount;
  while (mask && nCount) {
    *p |= mask;
#if defined(__WXMSW__) && (wxVERSION_NUMBER < 2600)
    mask >>= 1;
#else
    mask <<= 1;
#endif
    nCount--;
  }
  p++;
  while (nCount >= 8) {
    *p++ = 0xFF;
    nCount -= 8;
  }
#if defined(__WXMSW__) && (wxVERSION_NUMBER < 2600)
  mask = 0x80;
#else
  mask = 0x01;
#endif
  while (nCount) {
    *p |= mask;
#if defined(__WXMSW__) && (wxVERSION_NUMBER < 2600)
    mask >>= 1;
#else
    mask <<= 1;
#endif
    nCount--;
  }

}

void CBitSink::ClearBits(int nCount) 
{
  m_dwBitPos += nCount;
}

CBitSink::CBitSink(void *pBuffer, sff_dword nByteCount) :
m_pBuffer((sff_byte *)pBuffer), 
m_dwBitPos(0),
m_dwByteCount(nByteCount),
m_dwBitCount(nByteCount * 8)
{ 
}

void CBitSink::Reset()
{
  m_dwBitPos = 0;
}

//-----------------------------------------------------------------

void CByteSink::SetBits(int nCount) 
{
  sff_byte *p;
  
  p = m_pBuffer;
  p += m_dwBitPos;
  for (int i=0; i<nCount; i++)
    *p++ = 0;
  m_dwBitPos += nCount;
};

//-----------------------------------------------------------------

int CHuffDecoder::FindToken(LPTABENTRY pTable)
{
  sff_word bits;
  while (pTable->code) {
    bits = GetBits(pTable->bits);
    if (bits == pTable->code) {
      ClrBits(pTable->bits);
      return pTable->run;
    }
    pTable++;
  }
  return -1;
}

int CHuffDecoder::DecodeLine(IBitSink& aBitSink)
{
  int iRunlength;
  m_dwRunlength = 0;
  TDecoderState state = NEED_WHITE;

  for (;;)
  {
    switch (state) {
    case NEED_WHITE :
      // we expect white_term or white_markup
      NeedBits(12);
      iRunlength = FindToken(aTermWhite);
      if ( iRunlength == RL_EOL) {
        goto exit; // EOL
      }
      if ( iRunlength >= 0 ) {
        if ( iRunlength > 0 ) {
          m_dwRunlength += iRunlength;
          aBitSink.ClearBits(iRunlength);
        }
        state = NEED_BLACK;
      } else {
        iRunlength = FindToken(aMarkUpWhite);
        if ( iRunlength == RL_EOL) {
          goto exit; // EOL
        }
        if (iRunlength >= 0) {
          if ( iRunlength > 0 ) {
            m_dwRunlength += iRunlength;
            aBitSink.ClearBits(iRunlength);
          }
          state = NEED_WHITETERM;
        } else {
          throw CSimpleException(CSimpleException::err_nowhitestart);
        }
      }
      break;
    case NEED_BLACK :
      // we expect black_term or black_markup
      NeedBits(13);
      iRunlength = FindToken(aTermBlack);
      if ( iRunlength == RL_EOL) {
        goto exit; // EOL
      }
      if (iRunlength >= 0) {
        if (iRunlength > 0 ) {
          m_dwRunlength += iRunlength;
          aBitSink.SetBits(iRunlength);
        }
        state = NEED_WHITE;
      } else {
        iRunlength = FindToken(aMarkUpBlack);
        if (iRunlength == RL_EOL) {
          goto exit; // EOL
        }
        if (iRunlength >= 0) {
          if (iRunlength > 0) {
            m_dwRunlength += iRunlength;
            aBitSink.SetBits(iRunlength);
          }
          state = NEED_BLACKTERM;
        } else {
          state = NEED_WHITE;
          // throw CSimpleException(CSimpleException::err_noblackcode);
          // [PS] changed to allow further decoding...
          //      currently no problems known, but we'll see...
        }
      }
      break;
    case NEED_WHITETERM :
      // expect White_Term only
      NeedBits(8);
      iRunlength = FindToken(aTermWhite);
      if (iRunlength == RL_EOL) {
        goto exit; // EOL
      }
      if (iRunlength >= 0) {
        if ( iRunlength > 0 ) {
          aBitSink.ClearBits(iRunlength);
          m_dwRunlength += iRunlength;
        }
        state = NEED_BLACK;
      } else {
        throw CSimpleException(CSimpleException::err_nowhiteterm);
      }
      break;
    case NEED_BLACKTERM :
      // expect Black_Term only
      NeedBits(12);
      iRunlength = FindToken(aTermBlack);
      if (iRunlength == RL_EOL) {
        goto exit; // EOL
      }
      if (iRunlength >= 0) {
        if ( iRunlength > 0 ) {
          m_dwRunlength += iRunlength;
          aBitSink.SetBits(iRunlength);
        }
        state = NEED_WHITE;
      } else {
        throw CSimpleException(CSimpleException::err_noblackterm);
      }
      break;
    }
    if (NoMoreBits())
      break;
  }
exit:
  return m_dwRunlength;
}
