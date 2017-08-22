/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013-2017 CERN
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

#include <functional>
using namespace std::placeholders;

#include <tool/tool_manager.h>
#include <view/view_controls.h>
#include <gal/graphics_abstraction_layer.h>
#include <geometry/seg.h>
#include <geometry/constraint_solver.h>

#include <confirm.h>

#include "pcb_actions.h"
#include "selection_tool.h"
#include "outline_editor.h"
#include <board_commit.h>
#include <bitmaps.h>

#include <wxPcbStruct.h>
#include <class_edge_mod.h>
#include <class_dimension.h>
#include <class_drawsegment.h>
#include <class_constraint.h>

#include <class_zone.h>
#include <class_board.h>
#include <class_module.h>
#include <connectivity.h>

#include <view/view.h>
#include <geometry/shape.h>
#include <geometry/shape_segment.h>
#include <geometry/constraint_solver.h>

#include <pcb_painter.h>

OUTLINE_EDITOR::OUTLINE_EDITOR() :
    PCB_TOOL( "pcbnew.OutlineEditor" ), m_selectionTool( NULL )
{
}


OUTLINE_EDITOR::~OUTLINE_EDITOR()
{
}

void OUTLINE_EDITOR::Reset( RESET_REASON aReason )
{

}


bool OUTLINE_EDITOR::Init()
{
    // Find the selection tool, so they can cooperate
    m_selectionTool = static_cast<SELECTION_TOOL*>( m_toolMgr->FindTool( "pcbnew.InteractiveSelection" ) );

    if( !m_selectionTool )
    {
        DisplayError( NULL, wxT( "pcbnew.InteractiveSelection tool is not available" ) );
        return false;
    }


    return true;
}

class GEOM_PREVIEW : public KIGFX::VIEW_ITEM
{
public:
    GEOM_PREVIEW( std::shared_ptr<GEOM_SOLVER> aSolver ) :
        m_solver( aSolver )
    {

    }

    ~GEOM_PREVIEW()
    {

    }


    ///> @copydoc VIEW_ITEM::ViewBBox()
    virtual const BOX2I ViewBBox() const override
    {
        BOX2I b;
        b.SetMaximum();
        return b;
    }

    ///> @copydoc VIEW_ITEM::ViewDraw()
    virtual void ViewDraw( int aLayer, KIGFX::VIEW* aView ) const override
    {
        auto gal = aView->GetGAL();

        gal->SetFillColor( KIGFX::COLOR4D( 1.0, 1.0, 1.0, 1.0 ) );
        gal->SetStrokeColor( KIGFX::COLOR4D( 1.0, 1.0, 1.0, 1.0 ) );
        gal->SetIsFill( true );
        gal->PushDepth();
        gal->SetLayerDepth( gal->GetMinDepth() );

        float size = aView->ToWorld( 10 );
        gal->SetLineWidth( 1.0 );
        int n= 0;

        gal->SetIsStroke( true );


        for( auto item : m_solver->m_items )
        {
            if ( !item->IsPrimary() )
                continue;

            switch(item->Type() )
            {
                case GST_SEGMENT:
                {
                    auto s = static_cast<GS_SEGMENT*>(item);
                    gal->DrawLine( s->GetStart(), s->GetEnd() );
                }
            }
        }
        gal->SetIsStroke( false );
        gal->SetIsFill( true );



        for( auto item : m_solver->m_items )
        {
            if ( !item->IsPrimary() )
                continue;

            for ( auto anchor : item->GetAnchors() )
            {
                gal->DrawRectangle( anchor->GetPos() - size / 2, anchor->GetPos() + size / 2 );
                n++;
            }
        }

    //    printf("Drawn %d anchors\n", n);

        gal->PopDepth();

    }

    ///> @copydoc VIEW_ITEM::ViewGetLayers()
    virtual void ViewGetLayers( int aLayers[], int& aCount ) const override
    {
        aCount = 1;
        aLayers[0] = LAYER_GP_OVERLAY ;
    }

private:
    std::shared_ptr<GEOM_SOLVER> m_solver;
};



int OUTLINE_EDITOR::OnSelectionChange( const TOOL_EVENT& aEvent )
{
    SELECTION& selection = m_selectionTool->GetSelection();


    auto cond = SELECTION_CONDITIONS::HasType( PCB_CONSTRAINT_LINEAR_T ) ( selection );
    cond |= SELECTION_CONDITIONS::HasType( PCB_LINE_T ) ( selection );

    if(!cond)
    {
        printf("Not for OUTLINE_EDITOR!\n");
        return 0;
    }

    Activate();

    m_solver.reset ( new GEOM_SOLVER );
    m_preview.reset ( new GEOM_PREVIEW ( m_solver ) );

    for ( auto item : board()->Drawings() )
    {
        if( !selection.Contains(item) )
            m_solver->Add( static_cast<BOARD_ITEM*> ( item ), false );
    }

    for ( auto item : selection )
    {
        m_solver->Add( static_cast<BOARD_ITEM*> ( item ), true );
    }

    m_solver->FindOutlines();

    view()->Update( m_preview.get() );
    view()->Add( m_preview.get() );

    bool modified = false;
    bool revert = false;


    // Main loop: keep receiving events
    while( OPT_TOOL_EVENT evt = Wait() )
    {

        if ( !modified )
            updateEditedAnchor( *evt );

        if( evt->IsDrag( BUT_LEFT ) && m_editedAnchor )
        {
            if( !modified )
            {

                controls()->ForceCursorPosition( false );
//                m_original = *m_editedAnchor;    // Save the original position
                controls()->SetAutoPan( true );
                m_solver->StartMove();
                modified = true;
            }

            m_solver->MoveAnchor( m_editedAnchor, controls()->GetCursorPosition() );
            view()->Update( m_preview.get() );
        }
        else if( evt->IsMouseUp( BUT_LEFT ) )
        {
            controls()->SetAutoPan( false );

            if( modified )
            {
                if ( m_solver->IsResultOK() )
                {
                    BOARD_COMMIT commit( frame() );

                    for ( auto item : m_solver->AllItems() )
                    {
                        if ( !item->IsPrimary() )
                            continue;

                        switch( item->Type() )
                        {
                            case GST_SEGMENT:
                            {
                                auto gs = static_cast<GS_SEGMENT*>( item );
                                auto ds = static_cast<DRAWSEGMENT*> ( item->GetParent() );

                                commit.Modify(ds);

                                ds->SetStart( (wxPoint) gs->GetStart() );
                                ds->SetEnd( (wxPoint) gs->GetEnd() );
                                break;
                            }


                            default:
                                break;
                        }
                    }

                    commit.Push( _( "Drag node" ) );
                    modified = false;

                }

            }

            m_toolMgr->PassEvent();
        }
        else if( evt->IsCancel() )
        {

                // Let the selection tool receive the event too
                m_toolMgr->PassEvent();

                // Do not exit right now, let the selection clear the selection
                //break;
        }
        else if (
            evt->Matches( m_selectionTool->ClearedEvent ) ||
            evt->Matches( m_selectionTool->UnselectedEvent ) ||
            evt->Matches( m_selectionTool->SelectedEvent ) )
        {
            break;
        }
        else
        {
            m_toolMgr->PassEvent();
        }
    }


    view()->Remove( m_preview.get() );

    return 0;
}


void OUTLINE_EDITOR::setTransitions()
{
    Go( &OUTLINE_EDITOR::OnSelectionChange, SELECTION_TOOL::SelectedEvent );
    Go( &OUTLINE_EDITOR::OnSelectionChange, SELECTION_TOOL::UnselectedEvent );
}

void OUTLINE_EDITOR::setEditedAnchor( GS_ANCHOR* aAnchor )
{

    if( aAnchor )
    {
        controls()->ForceCursorPosition( true, aAnchor->GetPos() );
        controls()->ShowCursor( true );
        controls()->SetSnapping( true );
    }
    else
    {
        controls()->ShowCursor( false );
        controls()->SetSnapping( false );
        controls()->ForceCursorPosition( false );
    }

    m_editedAnchor = aAnchor;
}

void OUTLINE_EDITOR::updateEditedAnchor( const TOOL_EVENT& aEvent )
{
    GS_ANCHOR* anchor = m_editedAnchor;
    double snapRadius = view()->ToWorld(10.0);

    if( aEvent.IsMotion() )
    {
        anchor = m_solver->FindAnchor( aEvent.Position(), snapRadius );
    }
    else if( aEvent.IsDrag( BUT_LEFT ) )
    {
        anchor = m_solver->FindAnchor( aEvent.DragOrigin(), snapRadius );
    }

    if( m_editedAnchor != anchor )
        setEditedAnchor( anchor );
}
