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

#include <memory>
#include <view/view.h>

#include <router/pns_item.h>

using std::unique_ptr;

class PCB_DRAW_PANEL_GAL;
class BOARD;
class PNS_KICAD_IFACE;

namespace PNS {
class ROUTER;
};

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

    PNS::ITEM *findItemByPosition( VECTOR2I pos, PNS::ITEM::PnsKind kind);
    void OnExit(wxCommandEvent& event);
    void OnMotion( wxMouseEvent& aEvent );
    void OnMenuFileOpen( wxCommandEvent& WXUNUSED(event) );

    void buildView();

    unique_ptr < PCB_DRAW_PANEL_GAL > m_galPanel;
    unique_ptr < PNS::ROUTER > m_router;
    unique_ptr < PNS_KICAD_IFACE > m_iface;
    unique_ptr < BOARD > m_board;
};


#endif // _WX_ISOSURF_H_
