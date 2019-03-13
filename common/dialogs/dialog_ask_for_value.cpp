/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015-2017 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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

#include <confirm.h>
#include <dialogs/dialog_ask_for_value.h>
#include <draw_frame.h>

DIALOG_ASK_FOR_VALUE::DIALOG_ASK_FOR_VALUE(  wxWindow* aParent,
                const wxString& aTitle,
                const wxString& aMessage,
                int minValue,
                int maxValue,
                int &rval ) :
    DIALOG_ASK_FOR_VALUE_BASE( aParent ),
    m_value( static_cast<EDA_DRAW_FRAME*>(aParent), m_valueUnit, m_valueCtrl, m_valueUnit ), // fixme: label
    m_minValue ( minValue ),
    m_maxValue ( maxValue ),
    m_rValue ( rval )
{

    SetTitle ( aTitle );
    m_valueLabel->SetLabel ( aMessage );
    m_value.SetValue( m_rValue );
    m_stdButtonsOK->SetDefault();

    // Pressing ENTER when any of the text input fields is active applies changes
    Connect( wxEVT_TEXT_ENTER, wxCommandEventHandler(
                    DIALOG_ASK_FOR_VALUE::onOkClick ), NULL, this );
}

void DIALOG_ASK_FOR_VALUE::onClose( wxCloseEvent& aEvent )
{
    EndModal( 0 );
}


void DIALOG_ASK_FOR_VALUE::onCancelClick( wxCommandEvent& aEvent )
{
    EndModal( 0 );
}


void DIALOG_ASK_FOR_VALUE::onOkClick( wxCommandEvent& aEvent )
{
    if( check() )
    {
        int val = m_value.GetValue();

        m_rValue = std::max( m_minValue, std::min( m_maxValue, val ) );
        EndModal( 1 );
    }
}


bool DIALOG_ASK_FOR_VALUE::check() const
{
    // FIXME: implement
    return true;
}
