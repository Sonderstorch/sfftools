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

  $Id: sffdoc.cpp,v 1.4 2011/10/18 19:39:38 pschaefer Exp $
  
---RCS-Info--------------------------------------------------*/

#include <wx/wx.h>

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You must set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in setup.h!
#endif

#include "sfftypes.h"
#include "common.h"
#include "codes.h"
#include "decoder.h"
#include "sfffile.h"
#include "sffdoc.h"
#include "sffview.h"

#include <iostream>
using namespace std;

// ---------------------------------------------------------------------

CBitmapDecoder::CBitmapDecoder(wxUint32 aWidth, wxUint32 aHeight) :
		m_pBitmap(0),
		m_Width(aWidth),
		m_Height(aHeight),
		m_Scanline(0),
		m_sink(m_abBuffer, sizeof(m_abBuffer))
{  
	m_WidthInBytes = aWidth>>3;
	m_Width = aWidth;
	m_Height = aHeight;
	m_pBitmap = new wxUint8[m_WidthInBytes*aHeight];
	memset(m_pBitmap, 0x00, m_WidthInBytes*aHeight);
	Reset();
}

CBitmapDecoder::~CBitmapDecoder()
{
  if (m_pBitmap) delete m_pBitmap;
}
	
void CBitmapDecoder::Reset()
{
	m_Scanline = 0;
	m_sink.Reset();
}

void CBitmapDecoder::BlankLine()
{
	::memset(m_abBuffer, 0x00, sizeof(m_abBuffer));
}

CBitSink& CBitmapDecoder::GetBitSink()
{
	m_sink.Reset();
	return m_sink;
}

void CBitmapDecoder::WriteLine()
{
	if (m_Scanline < m_Height) {
		memcpy(m_pBitmap+(m_WidthInBytes*m_Scanline), m_abBuffer, m_WidthInBytes);
		++m_Scanline;
	}
}

const wxUint8 *CBitmapDecoder::GetBitmap()
{
	return m_pBitmap;
}

// ---------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(SffDocument, wxDocument)

SffDocument::SffDocument(void) :
	m_pSffFile(NULL),
	m_pDecoder(NULL),
	m_pPageBmp(NULL),
	m_nPageIdx(0)
{
}

SffDocument::~SffDocument(void)
{
  if (m_pSffFile) {
    delete m_pSffFile;
	}
	if (m_pPageBmp) {
		delete m_pPageBmp;
	}
	if (m_pDecoder) {
	  delete m_pDecoder;
	}
}

bool SffDocument::OnOpenDocument(const wxString& filename)
{
  bool rc =  false;
  
  try {
    wxBusyCursor wait;
    m_nPageIdx = 0;
    std::string strFN(filename.ToStdString());
    m_pSffFile = new CSffFile(strFN);
		m_pDecoder = new CBitmapDecoder(GetWidth(), GetHeight());
    SetFilename(filename, TRUE);
		Modify(FALSE);
    CreatePageBitmap();
    rc = true;
  } 
  catch (const CSimpleException& e) {
    wxLogError( (wxChar *) e.what().c_str() );
  }
  return rc;
}

int SffDocument::GetPageCount()
{
  wxASSERT(m_pSffFile != NULL);
  return (m_pSffFile != NULL) ? m_pSffFile->GetPageCount() : 0;
}

int SffDocument::GetCurrentPageIdx()
{
  wxASSERT(m_pSffFile != NULL);
  return m_nPageIdx;
}

void SffDocument::CreatePageBitmap()
{
	int i;
	int lines, cols;
	bool bLowRes;
	
	if (m_pSffFile == NULL)
		return;
	
  try
  {
		TSFFRecord rec;
		// CDCFile outfile;
		m_pSffFile->SeekPage(m_nPageIdx);
		bLowRes = m_pSffFile->IsLowRes(m_nPageIdx);
		lines = GetHeight();
		cols  = GetWidth();
		// Decode SFF Records ...
		m_pDecoder->Reset();
    while ((lines > 0) && m_pSffFile->GetRecord(rec)) {
      switch(rec.type) {
			case NORMAL :
				m_pDecoder->BlankLine();
				m_pSffFile->DecodeRecord(rec, m_pDecoder->GetBitSink());
				m_pDecoder->WriteLine(); --lines;
				if (bLowRes) { // double line if low-res
					m_pDecoder->WriteLine(); --lines;
				}
				if (rec.pData != 0) free(rec.pData);
				break;
			case USERINFO :
				// not supported
				if (rec.pData != 0) free(rec.pData);
				break;
			case BADLINE :
			case WHITESKIP :
				// a white skip is, ah, skipped ;)
				m_pDecoder->BlankLine();
				for (i=0; i < rec.cb; ++i) {
					m_pDecoder->WriteLine(); --lines;
					if (bLowRes) { // double line if low-res
						m_pDecoder->WriteLine(); --lines;
					}
				}
				break;
			default :
				break;
      }
    }
		wxBitmap *pOld = m_pPageBmp;
		m_pPageBmp = new wxBitmap((const char*)m_pDecoder->GetBitmap(), 
			GetWidth(), GetHeight());
		UpdateAllViews();
		if (pOld) {
			delete pOld;
		}
  }
  catch (CSimpleException e) {
    wxLogError( (wxChar *)e.what().c_str() );
  }
}

wxBitmap *SffDocument::GetPageBitmap(int nPage)
{
	int i;
	int lines, cols;
	bool bLowRes;

	
	if (m_pSffFile == NULL)
		return NULL;

  wxBitmap *pBmp = NULL;
	
  try
  {
		TSFFRecord rec;
		// CDCFile outfile;
		m_pSffFile->SeekPage(nPage-1);
		bLowRes = m_pSffFile->IsLowRes(nPage-1);
		lines = GetHeight();
		cols  = GetWidth();
		// Decode SFF Records ...
		m_pDecoder->Reset();
    while ((lines > 0) && m_pSffFile->GetRecord(rec)) {
      switch(rec.type) {
			case NORMAL :
				m_pDecoder->BlankLine();
				m_pSffFile->DecodeRecord(rec, m_pDecoder->GetBitSink());
				m_pDecoder->WriteLine(); --lines;
				if (bLowRes) { // double line if low-res
					m_pDecoder->WriteLine(); --lines;
				}
				if (rec.pData != 0) free(rec.pData);
				break;
			case USERINFO :
				// not supported
				if (rec.pData != 0) free(rec.pData);
				break;
			case BADLINE :
			case WHITESKIP :
				// a white skip is, ah, skipped ;)
				m_pDecoder->BlankLine();
				for (i=0; i < rec.cb; ++i) {
					m_pDecoder->WriteLine(); --lines;
					if (bLowRes) { // double line if low-res
						m_pDecoder->WriteLine(); --lines;
					}
				}
				break;
			default :
				break;
      }
    }
		pBmp = new wxBitmap((const char*)m_pDecoder->GetBitmap(), 
			GetWidth(), GetHeight());
  }
  catch (CSimpleException e) {
    wxLogError( (wxChar *)e.what().c_str() );
  }

  return pBmp;
}

wxUint32 SffDocument::GetHeight()
{
  wxASSERT(m_pSffFile != NULL);
  if (m_pSffFile != NULL) {
		wxUint32 h = m_pSffFile->GetPageHeight(m_nPageIdx);
    return m_pSffFile->IsLowRes(m_nPageIdx) ? h * 2 : h;
  }
	return 0;
}

wxUint32 SffDocument::GetWidth()
{
  wxASSERT(m_pSffFile != NULL);
  if (m_pSffFile != NULL) {
    return m_pSffFile->GetPageWidth(m_nPageIdx);
  }
	return 0;
}

void SffDocument::NextPage()
{
  wxASSERT(m_pSffFile != NULL);
  if (m_pSffFile != NULL) {
    if (m_nPageIdx < m_pSffFile->GetPageCount()) {
      ++m_nPageIdx;
	    CreatePageBitmap();
    }
  }
}

void SffDocument::PrevPage()
{
  wxASSERT(m_pSffFile != NULL);
  if (m_nPageIdx >= 1) {
    --m_nPageIdx;
    CreatePageBitmap();
  }
}
/*
void SffDocument::PreparePage(int nPage)
{
  wxASSERT(m_pSffFile != NULL);
  if (m_nPageIdx >= 1) {
    --m_nPageIdx;
    CreatePageBitmap();
  }
}
*/
// ---------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(SffPrintout, wxPrintout)

SffPrintout::SffPrintout(SffDocument *doc) : 
	wxPrintout() 
{ 
	m_Doc = doc;
	m_nPageCount = doc->GetPageCount();
};

bool SffPrintout::OnPrintPage(int page)
{
    wxBitmap *pBmp = m_Doc->GetPageBitmap(page);
		int nBitmapHeight = pBmp->GetHeight();
		int nBitmapWidth  = pBmp->GetWidth();

    wxDC *dc = GetDC();

    wxMemoryDC mem_dc;
    mem_dc.SelectObject(*pBmp);

    // Get the logical pixels per inch of screen and printer
    int ppiScreenX, ppiScreenY;
    GetPPIScreen(&ppiScreenX, &ppiScreenY);
    wxUnusedVar(ppiScreenY);
    int ppiPrinterX, ppiPrinterY;
    GetPPIPrinter(&ppiPrinterX, &ppiPrinterY);
    wxUnusedVar(ppiPrinterY);

    // This scales the DC so that the printout roughly represents the
    // the screen scaling. The text point size _should_ be the right size
    // but in fact is too small for some reason. This is a detail that will
    // need to be addressed at some point but can be fudged for the
    // moment.
    float scale = (float)((float)ppiPrinterX/(float)ppiScreenX);

    // Now we have to check in case our real page size is reduced
    // (e.g. because we're drawing to a print preview memory DC)
    int pageWidth, pageHeight;
    int w, h;
    dc->GetSize(&w, &h);
    GetPageSizePixels(&pageWidth, &pageHeight);
    wxUnusedVar(pageHeight);

    // If printer pageWidth == current DC width, then this doesn't
    // change. But we might be the preview bitmap width, so scale down.
    float overallScale = scale * (float)(w/(float)pageWidth);
    float zoomW = w; zoomW = zoomW/(nBitmapWidth*overallScale);
    float zoomH = h; zoomH = zoomH/(nBitmapHeight*overallScale);
    dc->SetUserScale(overallScale * zoomW, overallScale * zoomH);
    dc->SetBackground(*wxWHITE_BRUSH);
#if defined(__WXGTK__) && !wxCHECK_VERSION(2,8,0)
    if (pageWidth != w) {
    	// Strange misbehaviour of older wxGTK -> if we blit zoomed, the colours
  		// get inversed ??!? Workaround: wxSRC_INVERT
      dc->Blit(0,0,nBitmapWidth,nBitmapHeight,&mem_dc,0,0,wxSRC_INVERT);
  	} else {
      dc->Blit(0,0,nBitmapWidth,nBitmapHeight,&mem_dc,0,0,wxCOPY);
  	}
#else
    dc->Blit(0,0,nBitmapWidth,nBitmapHeight,&mem_dc,0,0,wxCOPY);
#endif
    return TRUE;
}

bool SffPrintout::HasPage(int pageNum)
{
	return (pageNum <= m_nPageCount);
}

bool SffPrintout::OnBeginDocument(int startPage, int endPage)
{
	if (!wxPrintout::OnBeginDocument(startPage, endPage))
			return FALSE;

	return TRUE;
}

void SffPrintout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
	*minPage = 1;
	*maxPage = m_nPageCount;
	*selPageFrom = 1;
	*selPageTo = 1;
}
