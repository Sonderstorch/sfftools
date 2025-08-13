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

  $Id: sffapp.cpp,v 1.4 2008/03/21 13:47:02 pschaefer Exp $
  
---RCS-Info--------------------------------------------------*/

#include <wx/wx.h>

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You must set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in setup.h!
#endif

#include <wx/docview.h>
#include <wx/config.h>
#include <wx/printdlg.h>

#include "sfftypes.h"
#include "common.h"
#include "codes.h"
#include "decoder.h"
#include "sffdoc.h"
#include "sffview.h"
#include "sffapp.h"

#ifndef __WXMSW__
#include "bitmaps/open.xpm"
#include "bitmaps/help.xpm"
#include "bitmaps/prev.xpm"
#include "bitmaps/next.xpm"
#include "bitmaps/zoomin.xpm"
#include "bitmaps/zoomout.xpm"
#include "bitmaps/flipx.xpm"
#include "bitmaps/flipy.xpm"
#include "bitmaps/fit_window.xpm"
#include "bitmaps/fit_width.xpm"
#include "bitmaps/actual_size.xpm"
#endif

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// Commands that are always available

#define ID_ABOUT      1

// Commands that are available only if file loaded

#define ID_ZOOMIN     10
#define ID_ZOOMOUT    11
#define ID_ZOOMNORMAL 12
#define ID_ZOOMWIDTH  13
#define ID_ZOOMHEIGHT 14
#define ID_FLIPX      15
#define ID_FLIPY      16

#define ID_FILELOADED1 ID_ZOOMIN
#define ID_FILELOADED2 ID_FLIPY

// Commands that are available only if multipaged file loaded

#define ID_PREVPAGE   20
#define ID_NEXTPAGE   21

#define ID_MULTIPAGE1 ID_PREVPAGE
#define ID_MULTIPAGE2 ID_NEXTPAGE

const int ID_TOOLBAR = 500;

SffFrame *frame = (SffFrame *) NULL;

IMPLEMENT_APP(SffApp)

// ----------------------------------------------------------------------------
// SffApp
// ----------------------------------------------------------------------------

SffApp::SffApp(void)
{
  m_docManager = (wxDocManager *) NULL;
}

bool SffApp::OnInit(void)
{
  SetAppName(wxT("SFF Viewer"));
    
  //// Create a document manager
  m_docManager = new wxDocManager();
  m_config = new wxConfig(wxT("sffview"));
  
  //// Create a template relating drawing documents to their views
  (void) new wxDocTemplate(m_docManager, wxT("SFF File"), wxT("*.sff;*.SFF"),
    wxT(""), wxT("sff"), wxT("SFF Doc"), wxT("SFF View"),
    CLASSINFO(SffDocument), CLASSINFO(SffView));
  
  m_docManager->SetMaxDocsOpen(2);
  
  //// Create the main frame window
  int x = m_config->Read(_T("/Window/x"), 1);
  int y = m_config->Read(_T("/Window/y"), 1);
  int w = m_config->Read(_T("/Window/w"), 750);
  int h = m_config->Read(_T("/Window/h"), 600);

  frame = new SffFrame(m_docManager, (wxFrame *) NULL, m_config, -1, 
    GetAppName(), wxPoint(x,y), wxSize(w,h),
    wxDEFAULT_FRAME_STYLE);
  
  //// Give it an icon (this is ignored in MDI mode: uses resources)
#ifdef __WXMSW__
  frame->SetIcon(wxIcon("sffview"));
#endif
    
  //// Make a menubar
  wxMenu *file_menu = new wxMenu;
  
  file_menu->Append(new wxMenuItem(file_menu, wxID_OPEN, 
    wxT("&Open..."), wxT("Opens a SFF fax file")));
  file_menu->Append(new wxMenuItem(file_menu, wxID_CLOSE, 
    wxT("&Close"), wxT("Closes currently loaded file")));
  file_menu->AppendSeparator();
  file_menu->Append(new wxMenuItem(file_menu, wxID_PRINT, 
    wxT("&Print..."), wxT("Prints the currently loaded file")));
  file_menu->Append(new wxMenuItem(file_menu, wxID_PRINT_SETUP, 
    wxT("Print &Setup..."), wxT("Let you choose a printer to print to")));
  file_menu->Append(new wxMenuItem(file_menu, wxID_PREVIEW, 
    wxT("&Print Pre&view..."), wxT("Displays the file as it would be printed")));
  file_menu->AppendSeparator();
  file_menu->Append(new wxMenuItem(file_menu, wxID_EXIT, 
    wxT("E&xit"), wxT("Quits the application")));
  
  // A nice touch: a history of files visited. Use this menu.
  m_docManager->FileHistoryLoad(*m_config);
  m_docManager->FileHistoryUseMenu(file_menu);
  m_docManager->FileHistoryAddFilesToMenu();
  
  wxMenu *view_menu = new wxMenu;
  view_menu->Append(new wxMenuItem(view_menu, ID_PREVPAGE, 
    wxT("&Previous Page"), wxT("Displays the previous page of a multipage document")));
  view_menu->Append(new wxMenuItem(view_menu, ID_NEXTPAGE, 
    wxT("&Next Page"), wxT("Displays the next page of a multipage document")));
  view_menu->AppendSeparator();
  view_menu->Append(new wxMenuItem(view_menu, ID_ZOOMNORMAL, 
    wxT("&Actual size"), wxT("Displays the page at normal scale")));
  view_menu->Append(new wxMenuItem(view_menu, ID_ZOOMWIDTH, 
    wxT("Fit &width"), wxT("Fit page width in window")));
  view_menu->Append(new wxMenuItem(view_menu, ID_ZOOMHEIGHT, 
    wxT("&Fit in window"), wxT("Fit whole page in window")));
/*
	view_menu->AppendSeparator();
  view_menu->Append(new wxMenuItem(view_menu, ID_FLIPX, 
    "Flip &horizontal", "Mirrors the page horizontal"));
  view_menu->Append(new wxMenuItem(view_menu, ID_FLIPY, 
    "Flip &vertical", "Mirrors the page vertical"));
*/
  wxMenu *help_menu = new wxMenu;
  help_menu->Append(new wxMenuItem(view_menu, ID_ABOUT, 
    wxT("&About"), wxT("Shows information about the application")));
  
  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(file_menu, wxT("&File"));
  menu_bar->Append(view_menu, wxT("&View"));
  menu_bar->Append(help_menu, wxT("&Help"));
  
  frame->canvas = frame->CreateCanvas((wxView *) NULL, frame);
  
  //// Associate the menu bar with the frame
  frame->SetMenuBar(menu_bar);
  frame->RecreateToolbar();
  
  frame->CreateStatusBar(2);
  int widths[] = { -1, 100 };
  frame->SetStatusWidths( 2, widths );
  frame->Show(TRUE);
  
  SetTopWindow(frame);
  
  if (argc > 1) {
    m_docManager->CreateDocument(argv[1], wxDOC_SILENT);
  }
  
  return TRUE;
}

int SffApp::OnExit(void)
{
  m_docManager->FileHistorySave(*m_config);
  delete m_docManager;
  delete m_config;
  return 0;
}

// ----------------------------------------------------------------------------
// SffFrame
// ----------------------------------------------------------------------------

IMPLEMENT_CLASS(SffFrame, wxDocParentFrame)
BEGIN_EVENT_TABLE(SffFrame, wxDocParentFrame)
  EVT_MENU(ID_ABOUT, SffFrame::OnAbout)
  EVT_MENU(ID_PREVPAGE, SffFrame::OnPrevPage)
  EVT_MENU(ID_NEXTPAGE, SffFrame::OnNextPage)
//  EVT_MENU(ID_ZOOMIN, SffFrame::OnZoomIn)
//  EVT_MENU(ID_ZOOMOUT, SffFrame::OnZoomOut)
  EVT_MENU(ID_FLIPX, SffFrame::OnFlipX)
  EVT_MENU(ID_FLIPY, SffFrame::OnFlipY)
	EVT_MENU(ID_ZOOMNORMAL, SffFrame::OnZoomNormal)
	EVT_MENU(ID_ZOOMWIDTH, SffFrame::OnFitWidth)
	EVT_MENU(ID_ZOOMHEIGHT, SffFrame::OnFitHeight)
  EVT_UPDATE_UI_RANGE(ID_FILELOADED1, ID_FILELOADED2, SffFrame::OnUpdateFileOps) 
  EVT_UPDATE_UI_RANGE(ID_MULTIPAGE1, ID_MULTIPAGE2, SffFrame::OnUpdateMultipage) 
  EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9, SffFrame::OnMRUFile)
  EVT_SIZE(SffFrame::OnSize)
END_EVENT_TABLE()

SffFrame::SffFrame(wxDocManager *manager, wxFrame *frame, wxConfig *config,
                   wxWindowID id, const wxString& title,
                   const wxPoint& pos, const wxSize& size, const long type) :
                      wxDocParentFrame(manager, frame, id, title, pos, size, type)
{
  canvas = (SffCanvas *) NULL;
  m_config = config;
}

void SffFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
  (void)wxMessageBox(
    wxT("This is SffView 0.5, a program to view structured fax files (sff)\n\n")
    wxT("This software and its documentation is\n")
    wxT("Copyright (C) 2000-2012 Peter Schaefer-Hutter\n\n")
    wxT("Permission to use, copy, modify, and distribute this software and its ")
    wxT("documentation for any purpose and without fee is hereby granted, provided ")
    wxT("that the above copyright notice appear in all copies. This software ")
    wxT("is provided 'as is' without expressed or implied warranty.\n\n")
    wxT("You can contact the author by email at peter.schaefer@gmx.de"),
    wxT("About..."), wxICON_INFORMATION | wxOK );
}

void SffFrame::RecreateToolbar()
{
  // delete and recreate the toolbar
  wxToolBarBase *toolBar = GetToolBar();
  delete toolBar;
  
  SetToolBar(NULL);
  
  long style = wxNO_BORDER | wxTB_FLAT | wxTB_DOCKABLE | wxTB_HORIZONTAL;
  
  toolBar = CreateToolBar(style, ID_TOOLBAR);
  toolBar->SetMargins( 4, 4 );
  
  // Set up toolbar
  wxBitmap toolBarBitmaps[11];
  
  toolBarBitmaps[0] = wxBITMAP(open);
  toolBarBitmaps[1] = wxBITMAP(prev);
  toolBarBitmaps[2] = wxBITMAP(next);
  toolBarBitmaps[3] = wxBITMAP(help);
  toolBarBitmaps[4] = wxBITMAP(zoomin);
  toolBarBitmaps[5] = wxBITMAP(zoomout);
  toolBarBitmaps[6] = wxBITMAP(flipx);
  toolBarBitmaps[7] = wxBITMAP(flipy);
  toolBarBitmaps[8] = wxBITMAP(fit_window);
  toolBarBitmaps[9] = wxBITMAP(fit_width);
  toolBarBitmaps[10] = wxBITMAP(actual_size);
  
#ifdef __WXMSW__
  int width = 24;
#else
  int width = 16;
#endif
  
  int currentX = 5;
  
  toolBar->AddTool(wxID_OPEN, wxEmptyString, toolBarBitmaps[0], wxT("Open File"));
  currentX += width + 5;
  toolBar->AddSeparator();
  toolBar->AddTool(ID_PREVPAGE, wxEmptyString, toolBarBitmaps[1], wxT("Previous Page"));
  currentX += width + 5;
  toolBar->AddTool(ID_NEXTPAGE, wxEmptyString, toolBarBitmaps[2], wxT("Next Page"));
  currentX += width + 5;
  toolBar->AddSeparator();
	toolBar->AddTool(ID_ZOOMNORMAL, wxEmptyString, toolBarBitmaps[10], wxT("Actual Size"));
  currentX += width + 5;
	toolBar->AddTool(ID_ZOOMWIDTH, wxEmptyString, toolBarBitmaps[9], wxT("Fit Width"));
  currentX += width + 5;
	toolBar->AddTool(ID_ZOOMHEIGHT, wxEmptyString, toolBarBitmaps[8], wxT("Fit In Window"));
  currentX += width + 5;
/*
	toolBar->AddTool(ID_ZOOMIN, toolBarBitmaps[4], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, "Zoom +");
  currentX += width + 5;
  toolBar->AddTool(ID_ZOOMOUT, toolBarBitmaps[5], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, "Zoom -");
  currentX += width + 5;
  toolBar->AddTool(ID_FLIPX, toolBarBitmaps[6], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, "Horiz. spiegeln");
  currentX += width + 5;
  toolBar->AddTool(ID_FLIPY, toolBarBitmaps[7], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, "Vert. spiegeln");
  currentX += width + 5;
*/
  toolBar->AddSeparator();
  toolBar->AddTool(ID_ABOUT, wxEmptyString, toolBarBitmaps[3], wxT("Open About Dialog"));
  
  toolBar->Realize();
  
  toolBar->SetRows(1);
}

// Updates UI in respect to functions that are 
// only available if a file is loaded

void SffFrame::OnUpdateFileOps(wxUpdateUIEvent& event)
{
  event.Enable(m_docManager->GetCurrentDocument() != NULL);
}

// Updates UI in respect to functions that are 
// only available if a multipaged file is loaded

void SffFrame::OnUpdateMultipage(wxUpdateUIEvent& event)
{
	event.Enable(false);
	SffDocument *pDoc = (SffDocument *) 
	m_docManager->GetCurrentDocument();
	if ((pDoc != NULL) && (pDoc->GetPageCount() > 1)) {
		if (event.GetId() == ID_PREVPAGE) {
			event.Enable(pDoc->GetCurrentPageIdx() > 0);
		} else if (event.GetId() == ID_NEXTPAGE) {
			event.Enable(pDoc->GetCurrentPageIdx() < pDoc->GetPageCount()-1);
		} else {
			event.Enable(false);
		}
		return;
	}
	event.Enable(false);
}

// Creates a canvas. Called in OnInit as a child of the main window
SffCanvas *SffFrame::CreateCanvas(wxView *view, wxFrame *parent)
{
  int width, height;
  parent->GetClientSize(&width, &height);
  
  // Non-retained canvas
  SffCanvas *canvas = new SffCanvas(view, parent, 
    wxPoint(0, 0), wxSize(width, height));
  //canvas->SetCursor(wxCursor(wxCURSOR_HAND));
  
  // Give it scrollbars
  //    canvas->SetScrollbars(10, 10, 50, 50);
//  canvas->Clear();
  
  return canvas;
}

void SffFrame::OnNextPage(wxCommandEvent& WXUNUSED(event))
{
  SffDocument *pDoc = (SffDocument *)
    m_docManager->GetCurrentDocument();
  if ((pDoc != NULL) && (pDoc->GetPageCount() > 1)) {
    pDoc->NextPage();
  }
}

void SffFrame::OnPrevPage(wxCommandEvent& WXUNUSED(event))
{
  SffDocument *pDoc = (SffDocument *)
    m_docManager->GetCurrentDocument();
  if ((pDoc != NULL) && (pDoc->GetPageCount() > 1)) {
    pDoc->PrevPage();
  }
}

void SffFrame::OnFlipX(wxCommandEvent& WXUNUSED(event))
{
	SffView *pView = (SffView *)
		m_docManager->GetCurrentView();
	if (pView != NULL) {
		pView->FlipX();
	}
}

void SffFrame::OnFlipY(wxCommandEvent& WXUNUSED(event))
{
	SffView *pView = (SffView *)
		m_docManager->GetCurrentView();
	if (pView != NULL) {
		pView->FlipY();
	}
}

void SffFrame::OnFitWidth(wxCommandEvent& WXUNUSED(event))
{
	SffView *pView = (SffView *)
		m_docManager->GetCurrentView();
	if (pView != NULL) {
		pView->SetScale(SffView::FITWIDTH);
	}
}

void SffFrame::OnFitHeight(wxCommandEvent& WXUNUSED(event))
{
	SffView *pView = (SffView *)
		m_docManager->GetCurrentView();
	if (pView != NULL) {
		pView->SetScale(SffView::FULLPAGE);
	}
}

void SffFrame::OnZoomNormal(wxCommandEvent& WXUNUSED(event))
{
	SffView *pView = (SffView *)
		m_docManager->GetCurrentView();
	if (pView != NULL) {
		pView->SetScale(SffView::FULLSCALE);
	}
}

void SffFrame::OnSize(wxSizeEvent& event)
{
	wxFrame::OnSize(event);
	
	SffView *pView = (SffView *)
		m_docManager->GetCurrentView();
	if (pView != NULL) {
		pView->CalcScale();
	}
}

void SffFrame::OnMRUFile(wxCommandEvent& event)
{
  wxString f(m_docManager->GetHistoryFile(event.GetId() - wxID_FILE1));
  if (!f.IsEmpty())
    (void)m_docManager->CreateDocument(f, wxDOC_SILENT);
}

bool SffFrame::Destroy()
{
  int x, y, w, h;
  GetPosition(&x, &y); GetSize(&w, &h);
  m_config->Write(_T("/Window/x"), (long)x);
  m_config->Write(_T("/Window/y"), (long)y);
  m_config->Write(_T("/Window/w"), (long)w);
  m_config->Write(_T("/Window/h"), (long)h);
  return wxDocParentFrame::Destroy();
}

// ---------------------------------------------------

SffFrame *GetMainFrame(void)
{
  return frame;
}
