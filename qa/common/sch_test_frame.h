/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013-2017 CERN
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef __SCH_TEST_FRAME_H
#define __SCH_TEST_FRAME_H

#include <wx/wx.h>
#include <wx/app.h>

#include <kiway_player.h>
#include <pgm_base.h>

#include <memory>

using std::unique_ptr;

class SCH_DRAW_PANEL_GAL;
class SCH_SHEET;
class LIB_PART;

class TOOL_MANAGER;
class TOOL_DISPATCHER;
class ACTIONS;


namespace KIGFX {
    class VIEW;
};

// Define a new application type
class GAL_TEST_APP : public wxApp
{
public:
    virtual bool OnInit();

    virtual void OnInitCmdLine(wxCmdLineParser& parser);
    virtual bool OnCmdLineParsed(wxCmdLineParser& parser);

private:
    wxString m_filename;
};

class SCH_TEST_FRAME : public KIWAY_PLAYER
{
public:
    SCH_TEST_FRAME( KIWAY* aKiway, wxWindow* aParent);
    //, FRAME_T aFrameType,
    //        const wxString& aTitle, const wxPoint& aPos, const wxSize& aSize,
    //        long aStyle, const wxString& aWdoName = wxFrameNameStr );

    virtual ~SCH_TEST_FRAME();

    void SetLibraryComponent( LIB_PART *part );
    void SetSchematic( SCH_SHEET* sheet );

    LIB_PART * LoadAndDisplayPart ( const std::string& filename );
    SCH_SHEET* LoadAndDisplaySchematic ( const std::string& filename );

protected:

    virtual void OnExit(wxCommandEvent& event);
    virtual void OnMotion( wxMouseEvent& aEvent );
    virtual void OnMenuFileOpen( wxCommandEvent& WXUNUSED(event) );

    void buildView();

    unique_ptr < SCH_DRAW_PANEL_GAL > m_galPanel;
    LIB_PART* m_part;
    SCH_SHEET* m_sheet;
    unique_ptr < TOOL_MANAGER > m_toolManager;
    unique_ptr < TOOL_DISPATCHER > m_toolDispatcher;
    unique_ptr < ACTIONS > m_pcbActions;
};

wxFrame* CreateMainFrame( const std::string& aFileName );

#endif
