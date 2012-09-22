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
#include <wx/wx.h>
#include <wx/wfstream.h>
#include <wx/toolbar.h>
#include <wx/docview.h>
#include <wx/cmdproc.h>
#include <wx/dcprint.h>

#include "sfftypes.h"
#include "common.h"
#include "codes.h"
#include "decoder.h"
#include "sfffile.h"
#include "sffview.h"
#include "sffapp.h"
#include "sffdoc.h"

//-----------------------------------------------------------------------------
// SffView
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(SffView, wxView)

BEGIN_EVENT_TABLE(SffView, wxView)
END_EVENT_TABLE()

SffView::~SffView()
{
	if (m_pMemDC) 
		delete m_pMemDC;
}

// What to do when a view is created. Creates actual
// windows for displaying the view.
bool SffView::OnCreate(wxDocument *doc, long WXUNUSED(flags) )
{
	m_pMemDC = new wxMemoryDC();
	
	m_nZoom = 1.0;
	m_bXFlipped		= false;
	m_bYFlipped		= false;

	m_nScaleType = FITWIDTH;
		
	frame = GetMainFrame();
	canvas = GetMainFrame()->canvas;
	canvas->view = this;

	// Associate the appropriate frame with this view.
	SetFrame(frame);

	// Make sure the document manager knows that this is the
	// current view.
	Activate(TRUE);

	// Initialize the edit menu Undo and Redo items
	doc->GetCommandProcessor()->Initialize();

	return TRUE;
}

void SffView::SetBitmap(wxBitmap *pBmp)
{
	if (pBmp) {
		m_pMemDC->SelectObject(*pBmp);
		m_nBitmapHeight = pBmp->GetHeight();
		m_nBitmapWidth  = pBmp->GetWidth();
		CalcScale();
	} else {
		m_pMemDC->SelectObject(wxNullBitmap);
		m_nBitmapHeight = m_nBitmapWidth = 0;
	}
}

void SffView::SetScale(TScale type)
{
	TScale old = m_nScaleType;
	m_nScaleType = type;
	if (m_nScaleType != old) {
		CalcScale();
		canvas->Refresh();
	}
}

void SffView::CalcScale()
{
	int w, h;
	switch (m_nScaleType) 
	{
	case FULLSCALE:
		m_nZoom=1;
		canvas->SetScrollbars( 1, 1, m_nBitmapWidth, m_nBitmapHeight);
		canvas->Refresh();
		break;
	case FITWIDTH:
		frame->GetClientSize(&w, &h);
		m_nZoom=w; m_nZoom = m_nZoom/m_nBitmapWidth;
		canvas->SetScrollbars( 1, 1, 1, (int)(m_nBitmapHeight*m_nZoom));
		canvas->Refresh();
		break;
	case FULLPAGE:
		frame->GetClientSize(&w, &h);
		m_nZoom=h; m_nZoom /= m_nBitmapHeight;
		canvas->SetScrollbars( 1, 1, (int)(m_nBitmapWidth*m_nZoom), 1);
		canvas->Refresh();
		break;
	}
}

void SffView::OnDraw(wxDC *dc)
{
	// Flipping is currently not supported due to wxGTK not honouring
	// the mapping mode while blitting. This seems to be badly horked
	// currently (wxGTK 2.5.2)
	dc->SetUserScale(m_nZoom, m_nZoom);
#if defined(__WXGTK__) && !wxCHECK_VERSION(2,8,0)
	if (m_nZoom != 1) {
		// Strange misbehaviour of wxGTK -> if we blit zoomed, the colours
		// get inversed ??!? Workaround: wxSRC_INVERT
		dc->Blit(0,0,m_nBitmapWidth,m_nBitmapHeight,m_pMemDC,0,0,wxSRC_INVERT);
	} else {
		dc->Blit(0,0,m_nBitmapWidth,m_nBitmapHeight,m_pMemDC,0,0,wxCOPY);
	}
#else
	dc->Blit(0,0,m_nBitmapWidth,m_nBitmapHeight,m_pMemDC,0,0,wxCOPY);
#endif
}

void SffView::OnUpdate(wxView *WXUNUSED(sender), wxObject *WXUNUSED(hint))
{
	if (canvas) {
		SetBitmap(((SffDocument *)GetDocument())->GetPageBitmap());
		canvas->Refresh();
	}
}

void SffView::FlipX()
{
	m_bXFlipped = !m_bXFlipped;
	canvas->Refresh();
}

void SffView::FlipY()
{
	m_bYFlipped = !m_bYFlipped;
	canvas->Refresh();
}

// Clean up windows used for displaying the view.
bool SffView::OnClose(bool deleteWindow)
{
  if (!GetDocument()->Close())
		return FALSE;
    
  canvas->view = (wxView *) NULL;
  canvas = (SffCanvas *) NULL;
  
  wxString s(wxTheApp->GetAppName());
  if (frame) frame->SetTitle(s);
  
  SetFrame((wxFrame *) NULL);
  
  Activate(FALSE);
  
  return TRUE;
}

wxPrintout* SffView::OnCreatePrintout()
{
	return new SffPrintout((SffDocument *)GetDocument());
}

//-----------------------------------------------------------------------------
// SffCanvas
//-----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(SffCanvas, wxScrolledWindow)
END_EVENT_TABLE()

// Define a constructor for my canvas
SffCanvas::SffCanvas(wxView *v, wxFrame *frame, 
	const wxPoint& pos, const wxSize& size) :
    wxScrolledWindow(frame, -1, pos, size, wxSUNKEN_BORDER)
{
	view = v;
	SetBackgroundColour(_T("WHITE"));
}

// Define the repainting behaviour
void SffCanvas::OnDraw(wxDC& dc)
{
	if (view) {
	  view->OnDraw(&dc);
	}
}
