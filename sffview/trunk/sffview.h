#ifndef __SFFVIEW_H__
#define __SFFVIEW_H__
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

$Id: sffview.h,v 1.3 2011/10/18 19:39:38 pschaefer Exp $
 
---RCS-Info--------------------------------------------------*/

// ----------------------------------------------------------------------------
// Class declarations
// ----------------------------------------------------------------------------

class SffCanvas : public wxScrolledWindow
{
public:
	wxView *view;
    
	SffCanvas(wxView *v, wxFrame *frame, 
						const wxPoint& pos, 
						const wxSize& size);
    
	virtual void OnDraw(wxDC& dc);

	void OnMouseEvent(wxMouseEvent& event);
    
	DECLARE_EVENT_TABLE()
};

class SffView : public wxView
{
	DECLARE_DYNAMIC_CLASS(SffView)

public:
	enum TScale {
		FULLSCALE,
		FULLPAGE,
		FITWIDTH
	};

protected:
	wxMemoryDC *m_pMemDC;
	wxUint32    m_nBitmapWidth;
	wxUint32    m_nBitmapHeight;

	double	   m_nZoom;
	bool	     m_bXFlipped;
	bool	     m_bYFlipped;
	
	TScale      m_nScaleType;
	
	void SetBitmap(wxBitmap *pBmp);

public:
	wxFrame   *frame;
	SffCanvas *canvas;
    
	SffView() { canvas = (SffCanvas *)NULL; frame = (wxFrame *) NULL; };
	virtual ~SffView();
    
	void FlipX();
	void FlipY();
	
	void SetScale(TScale type);
	void CalcScale();
	
	bool OnCreate(wxDocument *doc, long flags);
	void OnDraw(wxDC *dc);
	void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);
	bool OnClose(bool deleteWindow = TRUE);
		
	virtual wxPrintout* OnCreatePrintout();

	DECLARE_EVENT_TABLE()
};

#endif // __SFFVIEW_H__
