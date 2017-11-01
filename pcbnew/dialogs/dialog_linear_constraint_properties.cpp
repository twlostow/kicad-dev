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

#include <dialogs/dialog_linear_constraint_properties.h>
#include <class_pcb_layer_box_selector.h>
#include <tools/selection_tool.h>
#include <class_constraint.h>
#include <wxPcbStruct.h>
#include <confirm.h>

#include <widgets/widget_net_selector.h>
#include <board_commit.h>

template<class Container>
static OPT<int> uniqueFieldValue( const Container& cont, std::function< int(const BOARD_ITEM *)> getField )
{
    int prev = 0;
    bool firstItem = true;
    for ( auto item : cont )
    {
        int val = getField( static_cast<BOARD_ITEM *> ( item ) );
        if( !firstItem && val != prev )
            return OPT<int>();
        prev = val;
        firstItem = false;
    }

    return prev;
}


DIALOG_LINEAR_CONSTRAINT_PROPERTIES::DIALOG_LINEAR_CONSTRAINT_PROPERTIES( PCB_BASE_FRAME* aParent, const SELECTION& aItems ) :
    DIALOG_LINEAR_CONSTRAINT_PROPERTIES_BASE( aParent ), m_items( aItems ),
    m_distance( aParent, m_distanceCtrl, m_distanceUnit )
{
    assert( !m_items.Empty() );

    setCommonVal( uniqueFieldValue( m_items, [] ( const BOARD_ITEM* item ) -> int {
        return static_cast<const CONSTRAINT_LINEAR*> (item)->GetDistance();
    } ), m_distanceCtrl, m_distance );

    auto layer = uniqueFieldValue( m_items, [] ( const BOARD_ITEM* item ) -> int {
        return static_cast<const CONSTRAINT_LINEAR*> (item)->GetLayer();
    } );

    m_layerCtrl->SetLayersHotkeys( false );
    m_layerCtrl->SetLayerSet( LSET::AllNonCuMask() );
    m_layerCtrl->SetBoardFrame( aParent );
    m_layerCtrl->Resync();

    if( layer )
        m_layerCtrl->SetLayerSelection( *layer );

    m_stdButtonsOK->SetDefault();

    // Pressing ENTER when any of the text input fields is active applies changes
    Connect( wxEVT_TEXT_ENTER, wxCommandEventHandler( DIALOG_LINEAR_CONSTRAINT_PROPERTIES::onOkClick ), NULL, this );
}


bool DIALOG_LINEAR_CONSTRAINT_PROPERTIES::Apply( COMMIT& aCommit )
{
    if( !check() )
        return false;

    for( auto item : m_items )
    {
        auto constraint = static_cast<CONSTRAINT_LINEAR*> ( item );
        aCommit.Modify( constraint );

        if( m_distance.Valid() )
            constraint->SetDistance ( m_distance.GetValue () );

        LAYER_NUM layer = m_layerCtrl->GetLayerSelection();

        if( layer != UNDEFINED_LAYER )
            constraint->SetLayer( (PCB_LAYER_ID) layer );

    }

    return true;
}


void DIALOG_LINEAR_CONSTRAINT_PROPERTIES::onClose( wxCloseEvent& aEvent )
{
    EndModal( 0 );
}


void DIALOG_LINEAR_CONSTRAINT_PROPERTIES::onCancelClick( wxCommandEvent& aEvent )
{
    EndModal( 0 );
}


void DIALOG_LINEAR_CONSTRAINT_PROPERTIES::onOkClick( wxCommandEvent& aEvent )
{
    if( check() )
        EndModal( 1 );
}


bool DIALOG_LINEAR_CONSTRAINT_PROPERTIES::check() const
{
    return true;
}
