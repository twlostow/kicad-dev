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

#include <dialogs/dialog_graphic_arc_properties.h>
#include <pcb_layer_box_selector.h>
#include <tools/selection_tool.h>
#include <class_drawsegment.h>
#include <confirm.h>

#include <board_commit.h>


template <class Container>
static OPT<int> uniqueFieldValue( const Container& cont,
        std::function<int (const BOARD_ITEM*)> getField )
{
    int prev = 0;
    bool firstItem = true;

    for( auto item : cont )
    {
        int val = getField( static_cast<BOARD_ITEM*> ( item ) );

        if( !firstItem && val != prev )
            return OPT<int>();

        prev = val;
        firstItem = false;
    }

    return prev;
}


template <class Container>
static OPT<double> uniqueFieldValueDbl( const Container& cont,
        std::function<double (const BOARD_ITEM*)> getField )
{
    double  prev = 0;
    bool    firstItem = true;

    for( auto item : cont )
    {
        double val = getField( static_cast<BOARD_ITEM*> ( item ) );

        if( !firstItem && val != prev )
            return OPT<double>();

        prev = val;
        firstItem = false;
    }

    return prev;
}


DIALOG_GRAPHIC_ARC_PROPERTIES::DIALOG_GRAPHIC_ARC_PROPERTIES( PCB_BASE_FRAME* aParent,
        const SELECTION& aItems ) :
    DIALOG_GRAPHIC_ARC_PROPERTIES_BASE( aParent ), m_items( aItems ),
    m_startX( aParent, nullptr, m_startXCtrl, m_startXUnit ),
    m_startY( aParent, nullptr, m_startYCtrl, m_startYUnit ),
    m_endX( aParent, nullptr, m_endXCtrl, m_endXUnit ),
    m_endY( aParent, nullptr, m_endYCtrl, m_endYUnit ),
    m_centerX( aParent, nullptr, m_centerXCtrl, m_centerXUnit ),
    m_centerY( aParent, nullptr, m_centerYCtrl, m_centerYUnit ),
    m_radius( aParent, nullptr, m_radiusCtrl, m_radiusUnit ),
    m_width( aParent, nullptr, m_widthCtrl, m_widthUnit ),
    m_startAngle( aParent, nullptr, m_startAngleCtrl, m_startAngleUnit ),
    m_centralAngle( aParent, nullptr, m_centralAngleCtrl, m_centralAngleUnit )
{
    assert( !m_items.Empty() );

    m_startAngle.SetUnits( DEGREES );
    m_centralAngle.SetUnits( DEGREES );

    setDefinitionMode( ADM_BY_ENDPOINTS );

    // auto v = getUniquePropertyValue ( m_items, &DRAWSEGMENT::GetStart );

    m_startX.SetValue( uniqueFieldValue( m_items, [] ( const BOARD_ITEM* item ) -> int {
        return static_cast<const DRAWSEGMENT*> (item)->GetStart().x;
    } ) );

    m_startY.SetValue( uniqueFieldValue( m_items, [] ( const BOARD_ITEM* item ) -> int {
        return static_cast<const DRAWSEGMENT*> (item)->GetStart().y;
    } ) );

    m_endX.SetValue( uniqueFieldValue( m_items, [] ( const BOARD_ITEM* item ) -> int {
        return static_cast<const DRAWSEGMENT*> (item)->GetEnd().x;
    } ) );

    m_endY.SetValue( uniqueFieldValue( m_items, [] ( const BOARD_ITEM* item ) -> int {
        return static_cast<const DRAWSEGMENT*> (item)->GetEnd().y;
    } ) );

    m_centerX.SetValue( uniqueFieldValue( m_items, [] ( const BOARD_ITEM* item ) -> int {
        return static_cast<const DRAWSEGMENT*> (item)->GetCenter().x;
    } ) );

    m_centerY.SetValue( uniqueFieldValue( m_items, [] ( const BOARD_ITEM* item ) -> int {
        return static_cast<const DRAWSEGMENT*> (item)->GetCenter().y;
    } ) );

    m_width.SetValue( uniqueFieldValue( m_items, [] ( const BOARD_ITEM* item ) -> int {
        return static_cast<const DRAWSEGMENT*> (item)->GetWidth();
    } ) );

    m_radius.SetValue( uniqueFieldValue( m_items, [] ( const BOARD_ITEM* item ) -> int {
        return static_cast<const DRAWSEGMENT*> (item)->GetRadius();
    } ) );

    m_startAngle.SetValue( uniqueFieldValueDbl( m_items, [] ( const BOARD_ITEM* item ) -> double {
        return static_cast<const DRAWSEGMENT*> (item)->GetArcAngleStart();
    } ) );

    m_centralAngle.SetValue( uniqueFieldValueDbl( m_items, [] ( const BOARD_ITEM* item ) -> double {
        return static_cast<const DRAWSEGMENT*> (item)->GetAngle();
    } ) );

    auto layer = uniqueFieldValue( m_items, [] ( const BOARD_ITEM* item ) -> int {
        return static_cast<const DRAWSEGMENT*> (item)->GetLayer();
    } );

    m_layerCtrl->SetLayersHotkeys( false );
    //m_layerCtrl->SetLayerSet( LSET::AllCuMask() );
    m_layerCtrl->SetBoardFrame( aParent );
    m_layerCtrl->Resync();

    if( layer )
        m_layerCtrl->SetLayerSelection( *layer );

    auto fixPosition = uniqueFieldValue( m_items, [] ( const BOARD_ITEM* item ) -> int {
        return static_cast<const DRAWSEGMENT*> (item)->GetUserFlags() & DSF_CONSTRAIN_POSITION;
    } );

    auto fixStartAngle = uniqueFieldValue( m_items, [] ( const BOARD_ITEM* item ) -> int {
        return static_cast<const DRAWSEGMENT*> (item)->GetUserFlags() & DSF_CONSTRAIN_START_ANGLE;
    } );

    auto fixCentralAngle = uniqueFieldValue( m_items, [] ( const BOARD_ITEM* item ) -> int {
        return static_cast<const DRAWSEGMENT*> (item)->GetUserFlags() & DSF_CONSTRAIN_CENTRAL_ANGLE;
    } );

    auto fixRadius = uniqueFieldValue( m_items, [] ( const BOARD_ITEM* item ) -> int {
        return static_cast<const DRAWSEGMENT*> (item)->GetUserFlags() & DSF_CONSTRAIN_RADIUS;
    } );

    if( fixPosition )
        m_fixPosition->Set3StateValue( *fixPosition ? wxCHK_CHECKED : wxCHK_UNCHECKED );
    else
        m_fixPosition->Set3StateValue( wxCHK_UNDETERMINED );

    if( fixStartAngle )
        m_fixStartAngle->Set3StateValue( *fixStartAngle ? wxCHK_CHECKED : wxCHK_UNCHECKED );
    else
        m_fixStartAngle->Set3StateValue( wxCHK_UNDETERMINED );

    if( fixCentralAngle )
        m_fixCentralAngle->Set3StateValue( *fixCentralAngle ? wxCHK_CHECKED : wxCHK_UNCHECKED );
    else
        m_fixCentralAngle->Set3StateValue( wxCHK_UNDETERMINED );

    if( fixRadius )
        m_fixRadius->Set3StateValue( *fixRadius ? wxCHK_CHECKED : wxCHK_UNCHECKED );
    else
        m_fixRadius->Set3StateValue( wxCHK_UNDETERMINED );


    m_stdButtonsOK->SetDefault();

    // Pressing ENTER when any of the text input fields is active applies changes
    Connect( wxEVT_TEXT_ENTER, wxCommandEventHandler(
                    DIALOG_GRAPHIC_ARC_PROPERTIES::onOkClick ), NULL, this );
// #endif
}


bool DIALOG_GRAPHIC_ARC_PROPERTIES::Apply( COMMIT& aCommit )
{
    if( !check() )
        return false;

    for( auto item : m_items )
    {
        auto ds = static_cast<DRAWSEGMENT*> ( item );
        aCommit.Modify( ds );

        switch( m_definitionMode )
        {
        case ADM_BY_ENDPOINTS:
        {
            wxPoint start = ds->GetStart();

            if( !m_startX.IsIndeterminate() )
                start.x = m_startX.GetValue();

            if( !m_startY.IsIndeterminate() )
                start.y = m_startY.GetValue();

            ds->SetStart( start );

            wxPoint end = ds->GetEnd();

            if( !m_endX.IsIndeterminate() )
                end.x = m_endX.GetValue();

            if( !m_endY.IsIndeterminate() )
                end.y = m_endY.GetValue();

            ds->SetEnd( end );

            wxPoint center = ds->GetCenter();

            if( !m_centerX.IsIndeterminate() )
                center.x = m_centerX.GetValue();

            if( !m_centerY.IsIndeterminate() )
                center.y = m_centerY.GetValue();

            ds->SetCenter( center );
            break;
        }

        case ADM_BY_ANGLES:
        {
            auto center = ds->GetCenter();

            if( !m_centerX.IsIndeterminate() )
                center.x = m_centerX.GetValue();

            if( !m_centerY.IsIndeterminate() )
                center.y = m_centerY.GetValue();

            int radius = !m_radius.IsIndeterminate() ? m_radius.GetValue() : ds->GetRadius();
            double startAngle =
                !m_startAngle.IsIndeterminate() ? m_startAngle.GetValueDbl() : ds->GetArcAngleStart();
            double centralAngle =
                !m_centralAngle.IsIndeterminate() ? m_centralAngle.GetValueDbl() : ds->GetAngle();

            VECTOR2D start = VECTOR2D( radius, 0 ).Rotate( startAngle / 10.0 * M_PI / 180.0 );

            ds->SetArcStart(
                    wxPoint(
                            KiROUND( start.x + center.x ),
                            KiROUND( start.y + center.y )
                            )
                    );

            ds->SetAngle( centralAngle );

            break;
        }

        default:
            assert( false );
            break;
        }


        if( !m_width.IsIndeterminate() )
            ds->SetWidth( m_width.GetValue() );

        LAYER_NUM layer = m_layerCtrl->GetLayerSelection();

        if( layer != UNDEFINED_LAYER )
            ds->SetLayer( (PCB_LAYER_ID) layer );

        if( m_fixPosition->Get3StateValue() != wxCHK_UNDETERMINED )
            ds->SetUserFlags( DSF_CONSTRAIN_POSITION,
                    m_fixPosition->Get3StateValue() == wxCHK_CHECKED );


        if( m_fixRadius->Get3StateValue() != wxCHK_UNDETERMINED )
            ds->SetUserFlags( DSF_CONSTRAIN_RADIUS,
                    m_fixRadius->Get3StateValue() == wxCHK_CHECKED );

        if( m_fixStartAngle->Get3StateValue() != wxCHK_UNDETERMINED )
            ds->SetUserFlags( DSF_CONSTRAIN_START_ANGLE,
                    m_fixStartAngle->Get3StateValue() == wxCHK_CHECKED );

        if( m_fixCentralAngle->Get3StateValue() != wxCHK_UNDETERMINED )
            ds->SetUserFlags( DSF_CONSTRAIN_CENTRAL_ANGLE,
                    m_fixCentralAngle->Get3StateValue() == wxCHK_CHECKED );
    }

    return true;
}


void DIALOG_GRAPHIC_ARC_PROPERTIES::onClose( wxCloseEvent& aEvent )
{
    EndModal( 0 );
}


void DIALOG_GRAPHIC_ARC_PROPERTIES::onCancelClick( wxCommandEvent& aEvent )
{
    EndModal( 0 );
}


void DIALOG_GRAPHIC_ARC_PROPERTIES::onOkClick( wxCommandEvent& aEvent )
{
    if( check() )
        EndModal( 1 );
}


bool DIALOG_GRAPHIC_ARC_PROPERTIES::check() const
{
    // FIXME: implement
    return true;
}


void DIALOG_GRAPHIC_ARC_PROPERTIES::setDefinitionMode( ARC_DEFINITION_MODE aMode )
{
    m_definitionMode = aMode;
    m_startAngleCtrl->Enable( aMode == ADM_BY_ANGLES );
    m_centralAngleCtrl->Enable( aMode == ADM_BY_ANGLES );
    m_radiusCtrl->Enable( aMode == ADM_BY_ANGLES );
    m_startAngleUnit->Enable( aMode == ADM_BY_ANGLES );
    m_centralAngleUnit->Enable( aMode == ADM_BY_ANGLES );
    m_radiusUnit->Enable( aMode == ADM_BY_ANGLES );

    m_startXCtrl->Enable( aMode == ADM_BY_ENDPOINTS );
    m_startYCtrl->Enable( aMode == ADM_BY_ENDPOINTS );
    m_startXUnit->Enable( aMode == ADM_BY_ENDPOINTS );
    m_startYUnit->Enable( aMode == ADM_BY_ENDPOINTS );
    m_endXCtrl->Enable( aMode == ADM_BY_ENDPOINTS );
    m_endYCtrl->Enable( aMode == ADM_BY_ENDPOINTS );
    m_endXUnit->Enable( aMode == ADM_BY_ENDPOINTS );
    m_endYUnit->Enable( aMode == ADM_BY_ENDPOINTS );
}


void DIALOG_GRAPHIC_ARC_PROPERTIES::onDefineAsCoords( wxCommandEvent& event )
{
    setDefinitionMode( ADM_BY_ENDPOINTS );
}


void DIALOG_GRAPHIC_ARC_PROPERTIES::onDefineAsAngleRadius( wxCommandEvent& event )
{
    setDefinitionMode( ADM_BY_ANGLES );
}
