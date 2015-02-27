/////////////////////////////////////////////////////////////////////////////
// Name:        isosurf.h
// Purpose:     wxGLCanvas demo program
// Author:      Brian Paul (original gltk version), Wolfram Gloger
// Modified by: Julian Smart
// Created:     04/01/98
// RCS-ID:      $Id$
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////


#ifndef __TEST_WINDOW_H
#define __TEST_WINDOW_H

#include <view/view.h>

class EDA_DRAW_PANEL_GAL;
class BOARD;

namespace KIGFX {
    class VIEW;
};

// Define a new application type
class MyApp : public wxApp
{
public:
    virtual bool OnInit();

    virtual void OnInitCmdLine(wxCmdLineParser& parser);
    virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
};

class GAL_TEST_FRAME : public wxFrame
{
public:
    GAL_TEST_FRAME(wxFrame *frame,
            const wxString& title,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxDEFAULT_FRAME_STYLE);

    virtual ~GAL_TEST_FRAME();

    void SetBoard( BOARD * b);

private :
    void OnExit(wxCommandEvent& event);
    void OnMotion(wxMouseEvent& event);
    void OnMenuFileOpen( wxCommandEvent& WXUNUSED(event) );

    void buildView();

    EDA_DRAW_PANEL_GAL *m_galPanel;
    boost::shared_ptr<KIGFX::VIEW_OVERLAY> m_ovl;
};


#endif // _WX_ISOSURF_H_

