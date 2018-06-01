

/**
 * @file move-drag_pads.cpp
 * @brief Edit footprint pads.
 */

#include <fctsys.h>
#include <gr_basic.h>
#include <common.h>
#include <class_drawpanel.h>
#include <trigo.h>
#include <block_commande.h>
#include <pcb_base_frame.h>

#include <class_board.h>
#include <class_module.h>

#include <pcbnew.h>
#include <drag.h>


static D_PAD*  s_CurrentSelectedPad;
static wxPoint Pad_OldPos;


/* Cancel move pad command.
 */
static void Abort_Move_Pad( DRAW_PANEL_BASE* aPanel, wxDC* DC )
{
    D_PAD* pad = s_CurrentSelectedPad;
    auto panel = static_cast<EDA_DRAW_PANEL*>( aPanel );
    
    panel->SetMouseCapture( NULL, NULL );

    if( pad == NULL )
        return;

    pad->Draw( panel, DC, GR_XOR );
    pad->ClearFlags();
    pad->SetPosition( Pad_OldPos );
    pad->Draw( panel, DC, GR_XOR );

    // Pad move in progress: restore origin of dragged tracks, if any.
    for( unsigned ii = 0; ii < g_DragSegmentList.size(); ii++ )
    {
        TRACK* track = g_DragSegmentList[ii].m_Track;
        track->Draw( panel, DC, GR_XOR );
        track->SetState( IN_EDIT, false );
        track->ClearFlags();
        g_DragSegmentList[ii].RestoreInitialValues();
        track->Draw( panel, DC, GR_OR );
    }

    EraseDragList();
    s_CurrentSelectedPad = NULL;
}


/* Draw in drag mode when moving a pad.
 */
static void Show_Pad_Move( DRAW_PANEL_BASE* aPanel, wxDC* aDC, const wxPoint& aPosition,
                           bool aErase )
{
    TRACK*       Track;
    D_PAD*       pad    = s_CurrentSelectedPad;
    auto panel = static_cast<EDA_DRAW_PANEL*>( aPanel );

    if( pad == NULL )       // Should not occur
        return;

    if( aErase )
        pad->Draw( panel, aDC, GR_XOR );

    pad->SetPosition( panel->GetParent()->GetCrossHairPosition() );
    pad->Draw( panel, aDC, GR_XOR );

    for( unsigned ii = 0; ii < g_DragSegmentList.size(); ii++ )
    {
        Track = g_DragSegmentList[ii].m_Track;

        if( aErase )
            Track->Draw( panel, aDC, GR_XOR );

        g_DragSegmentList[ii].SetTrackEndsCoordinates( wxPoint(0, 0) );

        Track->Draw( panel, aDC, GR_XOR );
    }
}


// Function to initialize the "move pad" command
void PCB_BASE_FRAME::StartMovePad( D_PAD* aPad, wxDC* aDC, bool aDragConnectedTracks )
{
    if( aPad == NULL )
        return;

    s_CurrentSelectedPad = aPad;

    Pad_OldPos = aPad->GetPosition();

    SetMsgPanel( aPad );
    m_canvas->SetMouseCapture( Show_Pad_Move, Abort_Move_Pad );

    // Draw the pad, in SKETCH mode
    aPad->Draw( GetLegacyCanvas(), aDC, GR_XOR );
    aPad->SetFlags( IS_MOVED );
    aPad->Draw( GetLegacyCanvas(), aDC, GR_XOR );

    EraseDragList();

    // Build the list of track segments to drag if the command is a drag pad
    if( aDragConnectedTracks )
    {
        DRAG_LIST drglist( GetBoard() );
        drglist.BuildDragListe( aPad );
        UndrawAndMarkSegmentsToDrag( GetLegacyCanvas(), aDC );
    }
}


// Routine to place a moved pad.
void PCB_BASE_FRAME::PlacePad( D_PAD* aPad, wxDC* DC )
{
    int     dX, dY;
    TRACK*  track;

    if( aPad == NULL )
        return;

    MODULE* module = aPad->GetParent();

    ITEM_PICKER       picker( NULL, UR_CHANGED );
    PICKED_ITEMS_LIST pickList;

    // Save dragged track segments in undo list
    for( unsigned ii = 0; ii < g_DragSegmentList.size(); ii++ )
    {
        track = g_DragSegmentList[ii].m_Track;

        // Set the old state
        if( g_DragSegmentList[ii].m_Pad_Start )
            track->SetStart( Pad_OldPos );

        if( g_DragSegmentList[ii].m_Pad_End )
            track->SetEnd( Pad_OldPos );

        picker.SetItem( track );
        pickList.PushItem( picker );
    }

    // Save old module and old items values
    aPad->ClearFlags();
    wxPoint pad_curr_position = aPad->GetPosition();

    aPad->SetPosition( Pad_OldPos );

    if( g_DragSegmentList.size() == 0 )
        SaveCopyInUndoList( module, UR_CHANGED );
    else
    {
        picker.SetItem( module );
        pickList.PushItem( picker );
        SaveCopyInUndoList( pickList, UR_CHANGED );
    }

    aPad->SetPosition( pad_curr_position );
    aPad->Draw( GetLegacyCanvas(), DC, GR_XOR );

    // Redraw dragged track segments
    for( unsigned ii = 0; ii < g_DragSegmentList.size(); ii++ )
    {
        track = g_DragSegmentList[ii].m_Track;

        // Set the new state
        if( g_DragSegmentList[ii].m_Pad_Start )
            track->SetStart( aPad->GetPosition() );

        if( g_DragSegmentList[ii].m_Pad_End )
            track->SetEnd( aPad->GetPosition() );

        if( DC )
            track->Draw( GetLegacyCanvas(), DC, GR_XOR );

        track->SetState( IN_EDIT, false );
        track->ClearFlags();

        if( DC )
            track->Draw( GetLegacyCanvas(), DC, GR_OR );
    }

    // Compute local coordinates (i.e refer to module position and for module orient = 0)
    dX = aPad->GetPosition().x - Pad_OldPos.x;
    dY = aPad->GetPosition().y - Pad_OldPos.y;

    RotatePoint( &dX, &dY, -module->GetOrientation() );

    aPad->SetX0( dX + aPad->GetPos0().x );
    aPad->SetY0( dY + aPad->GetPos0().y );

    if( DC )
        aPad->Draw( GetLegacyCanvas(), DC, GR_OR );

    module->CalculateBoundingBox();
    module->SetLastEditTime();

    EraseDragList();

    OnModify();
    m_canvas->SetMouseCapture( NULL, NULL );
}
