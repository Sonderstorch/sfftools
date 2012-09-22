#ifndef __SFFAPP_H__
#define __SFFAPP_H__
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

$Id: sffapp.h,v 1.4 2008/03/21 13:47:01 pschaefer Exp $
 
---RCS-Info--------------------------------------------------*/

// ----------------------------------------------------------------------------
// Class declarations
// ----------------------------------------------------------------------------

// forwards

class wxDocManager;
class wxConfig;
class SffCanvas;

// Main application class

class SffApp : public wxApp
{
public:
    SffApp(void);

    bool OnInit(void);
    int  OnExit(void);

protected:
    wxDocManager* m_docManager;
    wxConfig    * m_config;
};

DECLARE_APP(SffApp)

// Main frame class

class SffFrame : public wxDocParentFrame
{
    DECLARE_CLASS(SffFrame)

public:
    SffCanvas *canvas;
    
    SffFrame(wxDocManager *manager, wxFrame *frame,
            wxConfig *config,
						wxWindowID id, const wxString& title, 
						const wxPoint& pos, const wxSize& size,
						const long type);
    
    SffCanvas *CreateCanvas(wxView *view, wxFrame *parent);

		void RecreateToolbar();
		void OnUpdateFileOps(wxUpdateUIEvent& event);
		void OnUpdateMultipage(wxUpdateUIEvent& event);

		void OnAbout(wxCommandEvent& event);
		void OnNextPage(wxCommandEvent& event);
		void OnPrevPage(wxCommandEvent& event);

		void OnFlipX(wxCommandEvent& event);
		void OnFlipY(wxCommandEvent& event);
		
		void OnZoomNormal(wxCommandEvent& event);
		void OnFitWidth(wxCommandEvent& event);
		void OnFitHeight(wxCommandEvent& event);

		void OnMRUFile(wxCommandEvent& event);
		void OnSize(wxSizeEvent& event);
		bool Destroy();
		
    DECLARE_EVENT_TABLE()

protected:
    wxConfig *m_config;
};

extern SffFrame *GetMainFrame(void);

#endif // __SFFAPP_H__
