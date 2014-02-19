/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 1992-2013 Kicad Developers, see AUTHORS.txt for contributors.
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

/**
 * @file dialog_page_settings.cpp
 */


#include <class_sch_screen.h>
#include <general.h>
#include <worksheet.h>
#include <dialog_sch_page_settings.h>


void DIALOG_SCH_PAGES_SETTINGS::initDialog()
{
    wxString    msg;
    wxString format = m_TextSheetCount->GetLabel();
    msg.Printf( format, m_screen->m_NumberOfScreens );
    m_TextSheetCount->SetLabel( msg );

    format = m_TextSheetNumber->GetLabel();
    msg.Printf( format, m_screen->m_ScreenNumber );
    m_TextSheetNumber->SetLabel( msg );

    DIALOG_PAGES_SETTINGS::initDialog();
		
    m_TextSheetCount->Show( true );
    m_TextSheetNumber->Show( true );
    m_RevisionExport->Show( true );
    m_DateExport->Show( true );
    m_TitleExport->Show( true );
    m_CompanyExport->Show( true );
    m_Comment1Export->Show( true );
    m_Comment2Export->Show( true );
    m_Comment3Export->Show( true );
    m_Comment4Export->Show( true );

    GetPageLayoutInfoFromDialog();
    UpdatePageLayoutExample();

    // Make the OK button the default.
    m_sdbSizer1OK->SetDefault();
    m_initialized = true;
}

bool DIALOG_SCH_PAGES_SETTINGS::SavePageSettings()
{

    bool retSuccess = DIALOG_PAGES_SETTINGS::SavePageSettings();
    
    if(!retSuccess)
    	return false;
    	
    // Exports settings to other sheets if requested:
    SCH_SCREEN* screen;

    // Build the screen list
    SCH_SCREENS ScreenList;

    // Update title blocks for all screens
    for( screen = ScreenList.GetFirst(); screen != NULL; screen = ScreenList.GetNext() )
    {
        if( screen == m_screen )
            continue;

        TITLE_BLOCK tb2 = screen->GetTitleBlock();

        if( m_RevisionExport->IsChecked() )
            tb2.SetRevision( m_tb.GetRevision() );

        if( m_DateExport->IsChecked() )
            tb2.SetDate( m_tb.GetDate() );

        if( m_TitleExport->IsChecked() )
            tb2.SetTitle( m_tb.GetTitle() );

        if( m_CompanyExport->IsChecked() )
            tb2.SetCompany( m_tb.GetCompany() );

        if( m_Comment1Export->IsChecked() )
            tb2.SetComment1( m_tb.GetComment1() );

        if( m_Comment2Export->IsChecked() )
            tb2.SetComment2( m_tb.GetComment2() );

        if( m_Comment3Export->IsChecked() )
            tb2.SetComment3( m_tb.GetComment3() );

        if( m_Comment4Export->IsChecked() )
            tb2.SetComment4( m_tb.GetComment4() );

        screen->SetTitleBlock( tb2 );
		}
		
    return true;
}
