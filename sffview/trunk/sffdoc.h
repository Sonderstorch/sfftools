#ifndef __SFFDOC_H__
#define __SFFDOC_H__
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

$Id: sffdoc.h,v 1.3 2008/03/21 13:47:01 pschaefer Exp $
 
---RCS-Info--------------------------------------------------*/

#include "wx/docview.h"

class CSffFile;

class CBitmapDecoder 
{
protected:
	CBitSink  m_sink;
  wxUint8   m_abBuffer[5120];
  wxUint8  *m_pBitmap;
  wxUint32  m_Width;
  wxUint32  m_WidthInBytes;
  wxUint32  m_Height;
  wxUint32  m_Scanline;
 
public:
	CBitmapDecoder(wxUint32 aWidth, wxUint32 aHeight);
	~CBitmapDecoder();
      
	void 				Reset();
	
	void 				WriteLine();
	void 				BlankLine();
	
	CBitSink&   GetBitSink();
	const wxUint8 *GetBitmap();
};

class SffDocument: public wxDocument
{
		DECLARE_DYNAMIC_CLASS(SffDocument)

protected:
		CSffFile       *m_pSffFile;
		CBitmapDecoder *m_pDecoder;
		
		wxBitmap       *m_pPageBmp;
		int             m_nPageIdx;
		
		void CreatePageBitmap();

public:
		SffDocument(void);
		~SffDocument(void);
    
		int GetPageCount();
		int GetCurrentPageIdx();

		void NextPage();
		void PrevPage();
		
		wxUint32 GetHeight();
		wxUint32 GetWidth();
		
		wxBitmap *GetPageBitmap() { return m_pPageBmp; };
    wxBitmap *GetPageBitmap(int nPage);

	virtual bool OnOpenDocument(const wxString& filename);
};

class SffView;

class SffPrintout : public wxPrintout
{
protected:
		SffDocument *m_Doc;
		int          m_nPageCount;

public:
		SffPrintout(SffDocument *doc = NULL);
    
		bool OnPrintPage(int page);
    bool HasPage(int page);
    bool OnBeginDocument(int startPage, int endPage);
    void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);

private:
    DECLARE_DYNAMIC_CLASS(SffPrintout)
    DECLARE_NO_COPY_CLASS(SffPrintout)
};

#endif // __SFFDOC_H__
