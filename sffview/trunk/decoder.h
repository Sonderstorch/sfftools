#ifndef __DECODER_H__
#define __DECODER_H__
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

 $Id: decoder.h,v 1.2 2008/03/21 13:47:01 pschaefer Exp $

---RCS-Info--------------------------------------------------*/

class CBitSource
{
private:
  sff_byte *m_pBuffer;
  sff_word  m_wBitsAvail;
  sff_dword m_dwAccu;
  sff_dword m_dwByteCount;

public:
  void NeedBits(int nCount);
  void ClrBits(int nCount);
  sff_word GetBits(int nCount);
  bool NoMoreBits();

  CBitSource(void *pBuffer, sff_dword nByteCount);
};

//-----------------------------------------------------------------

class IBitSink
{
public:
  virtual void SetBits(int nCount) = 0;  
  virtual void ClearBits(int nCount) = 0;
};

//-----------------------------------------------------------------

class CBitSink : public IBitSink
{
protected:
  sff_byte *m_pBuffer;
  
  sff_dword m_dwBitPos;
  sff_dword m_dwByteCount;
  sff_dword m_dwBitCount;

public:
  void Reset();

  virtual void SetBits(int nCount); 
  virtual void ClearBits(int nCount);
  
  CBitSink(void *pBuffer, sff_dword nByteCount);
};

//-----------------------------------------------------------------

// writes ones
class CByteSink : public CBitSink
{
public:
  CByteSink(void *pBuffer, sff_dword nByteCount) :
    CBitSink(pBuffer, nByteCount) { /* sonst nix */ };

  void SetBits(int nCount);
};

//-----------------------------------------------------------------

class CHuffDecoder : public CBitSource 
{
protected:
  sff_dword m_dwRunlength;

public:
  CHuffDecoder(sff_byte *pBuffer, sff_dword nByteCount) : 
    CBitSource(pBuffer, nByteCount) { /* sonst nix */ };

  int FindToken(LPTABENTRY pTable);
  int DecodeLine(IBitSink& aBitSink);
};

#endif // __DECODER_H__
