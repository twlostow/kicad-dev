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
#include <geometry/geom_solver.h>
#include <geometry/shape_arc.h>


#include <confirm.h>

#include "pcb_actions.h"
#include "selection_tool.h"
#include "outline_editor.h"
#include <board_commit.h>
#include <bitmaps.h>

#include <pcb_edit_frame.h>
#include <class_edge_mod.h>
#include <class_dimension.h>
#include <class_drawsegment.h>
//#include <class_constraint.h>

#include <class_zone.h>
#include <class_board.h>
#include <class_module.h>
#include <connectivity/connectivity_data.h>

#include <view/view.h>
#include <geometry/shape.h>
#include <geometry/shape_segment.h>
#include <geometry/geom_solver.h>

#include <pcb_painter.h>

static TOOL_ACTION filletCorner( "pcbnew.OutlineEditor.chamfer",
        AS_GLOBAL, 'X',
        _( "" ), _( "" ), NULL, AF_ACTIVATE );

static void commitToBoardItem( const GS_ITEM* aItem, BOARD_ITEM *aBoardItem )
{
    switch ( aItem->Type() )
    {
        /*case GST_LINEAR_CONSTRAINT:
        {
            auto c = static_cast<CONSTRAINT_LINEAR*> ( aBoardItem );
            auto gc = static_cast<const GS_LINEAR_CONSTRAINT*> ( aItem );

            c->SetP0( gc->GetP0() );
            c->SetP1( gc->GetP1() );
            c->SetMeasureLineOrigin( gc->GetOrigin() );

        break;
        }*/
        case GST_SEGMENT:
        {
            auto s = static_cast<DRAWSEGMENT*> ( aBoardItem );
            auto gs = static_cast<const GS_SEGMENT*> ( aItem );


            s->SetStart( (wxPoint) gs->CAnchor(0)->GetPos() );
            s->SetEnd( (wxPoint) gs->CAnchor(1)->GetPos() );
        break;
        }
        case GST_ARC:
        {
            auto s = static_cast<DRAWSEGMENT*> ( aBoardItem );
            auto gs = static_cast<const GS_ARC*> ( aItem );

            s->SetShape( S_ARC );
            s->SetArcStart( (wxPoint) gs->GetArc().GetP0() );
            s->SetCenter( (wxPoint) gs->GetArc().GetCenter() );
            s->SetAngle( gs->GetArc().GetCentralAngle() * 10.0);

        break;
        }
    }
}

class GEOM_PREVIEW : public KIGFX::VIEW_ITEM
{
public:
    GEOM_PREVIEW( std::shared_ptr<GS_SOLVER> aSolver ) :
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

        for( auto item : m_solver->Items() )
        {
            std::unique_ptr<BOARD_ITEM> copy(
                     static_cast<BOARD_ITEM*> ( item->GetParent()->Clone() ) );

            commitToBoardItem( item, copy.get() );

            gal->PushDepth();

            int layers[KIGFX::VIEW::VIEW_MAX_LAYERS], layers_count;
            copy->ViewGetLayers( layers, layers_count );
            aView->SortLayers( layers, layers_count );

            for( int i = 0; i < layers_count; i++ )
            {
                if( aView->IsLayerVisible( layers[i] ) )
                {
                    gal->AdvanceDepth();

                    if( !aView->GetPainter()->Draw( copy.get(), layers[i] ) )
                        copy->ViewDraw( layers[i], aView ); // Alternative drawing method
                }
            }

            gal->PopDepth();
        }


        gal->SetFillColor( KIGFX::COLOR4D( 1.0, 1.0, 1.0, 1.0 ) );
        gal->SetStrokeColor( KIGFX::COLOR4D( 1.0, 1.0, 1.0, 1.0 ) );
        gal->SetIsFill( true );
        gal->PushDepth();
        gal->SetLayerDepth( gal->GetMinDepth() );

        float size = aView->ToWorld( 10 );
        gal->SetLineWidth( 1.0 );
        int n = 0;

        gal->SetIsStroke( true );


        gal->SetIsStroke( false );
        gal->SetIsFill( true );

        for( auto item : m_solver->Items() )
        {
            if( !item->IsPrimary() )
                continue;

            for( auto anchor : item->GetAnchors() )
            {
                //if( anchor->IsSolvable() )
                  //  gal->DrawRectangle( anchor->GetPos() - size / 2, anchor->GetPos() + size / 2 );
                //else
                    gal->DrawCircle( anchor->GetPos(), size / 2 );

                n++;
            }
        }

        // printf("Drawn %d anchors\n", n);

        gal->PopDepth();
    }

    ///> @copydoc VIEW_ITEM::ViewGetLayers()
    virtual void ViewGetLayers( int aLayers[], int& aCount ) const override
    {
        aCount = 1;
        aLayers[0] = LAYER_GP_OVERLAY;
    }

private:
    std::shared_ptr<GS_SOLVER> m_solver;
};


OUTLINE_EDITOR::OUTLINE_EDITOR() :
    PCB_TOOL( "pcbnew.OutlineEditor" ), m_selectionTool( NULL )
{
    m_solver.reset( new GS_SOLVER );
    m_geomPreview.reset( new GEOM_PREVIEW( m_solver ) );

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
    m_selectionTool =
        static_cast<SELECTION_TOOL*>( m_toolMgr->FindTool( "pcbnew.InteractiveSelection" ) );

    if( !m_selectionTool )
    {
        DisplayError( NULL, wxT( "pcbnew.InteractiveSelection tool is not available" ) );
        return false;
    }


    return true;
}


void OUTLINE_EDITOR::addToSolver( BOARD_ITEM* aItem, bool aPrimary )
{
    switch( aItem->Type() )
    {
    case PCB_LINE_T:
    {
        auto ds = static_cast<DRAWSEGMENT*>(aItem);

        switch( ds->GetShape() )
        {
            case S_SEGMENT:
            {
            auto s = new GS_SEGMENT( ds->GetStart(), ds->GetEnd() );
            s->SetParent( aItem );
            //s->SetPrimary( aPrimary );

            /*if( ds->GetUserFlags() & DSF_CONSTRAIN_LENGTH )
                s->Constrain( CS_LENGTH, true );

            if( ds->GetUserFlags() & DSF_CONSTRAIN_DIRECTION )
                s->Constrain( CS_DIRECTION, true );*/

            m_solver->Add( s );
            break;
            }
            case S_ARC:
            {
                //auto s = new GS_ARC( ds->GetArcStart(), ds->GetArcEnd(), ds->GetCenter() );
                //s->SetParent( aItem );
                //s->SetPrimary( aPrimary );

                /*if( ds->GetUserFlags() & DSF_CONSTRAIN_START_ANGLE )
                    s->Constrain( CS_START_ANGLE, true );

                if( ds->GetUserFlags() & DSF_CONSTRAIN_CENTRAL_ANGLE )
                    s->Constrain( CS_CENTRAL_ANGLE, true );

                if( ds->GetUserFlags() & DSF_CONSTRAIN_RADIUS )
                    s->Constrain( CS_RADIUS, true );*/

                //printf("Add ARC!\n");

                //m_solver->Add( s );

                break;

            }
        }

        return;
    }

/*
    case PCB_CONSTRAINT_LINEAR_T:
    {
        auto    c   = static_cast<CONSTRAINT_LINEAR*>(aItem);
        auto    s   = new GS_LINEAR_CONSTRAINT ( c->GetP0(), c->GetP1(), c->GetDisplacementVector(), c->GetMeasureLineOrigin() );
        s->SetParent( aItem );
        s->SetPrimary( aPrimary );
        m_solver->Add( s );
        return;
    }
*/
    default:
        break;
    }
}


void OUTLINE_EDITOR::updateOutline()
{
    SELECTION& selection = m_selectionTool->GetSelection();

    if( !m_solver )
        return;

    printf("pre-clear\n");
    m_solver->Clear();

    int n = 0;
    for( auto item : board()->Drawings() )
    {
        if( !selection.Contains( item ) )
        {
            addToSolver( static_cast<BOARD_ITEM*> ( item ), false );
            n++;
        }
    }

    for( auto item : selection )
    {
        addToSolver( static_cast<BOARD_ITEM*> ( item ), true );
        n++;
    }

    printf("added %d items\n", n);
    m_solver->FindOutlines();
}

int OUTLINE_EDITOR::ChamferCorner( const TOOL_EVENT& aEvent )
{

}

int OUTLINE_EDITOR::FilletCorner( const TOOL_EVENT& aEvent )
{
    SELECTION& selection = m_selectionTool->GetSelection();
    if( !SELECTION_CONDITIONS::HasType( PCB_LINE_T ) ( selection ) )
        return 0;

    updateOutline();

    printf("FilletCorner!\n");


}

int OUTLINE_EDITOR::BreakOutline( const TOOL_EVENT& aEvent )
{

}

int OUTLINE_EDITOR::OnSelectionChange( const TOOL_EVENT& aEvent )
{
    SELECTION& selection = m_selectionTool->GetSelection();
    printf("selection: %p\n",&selection );

    auto cond = false; //SELECTION_CONDITIONS::HasType( PCB_CONSTRAINT_LINEAR_T ) ( selection );

    cond |= SELECTION_CONDITIONS::HasType( PCB_LINE_T ) ( selection );

    //m_solver->SetOverlay( view()->MakeOverlay() );

    if( !cond )
    {
        printf( "Not for OUTLINE_EDITOR!\n" );
        return 0;
    }

    Activate();

    view()->Add( m_geomPreview.get() );

    updateOutline();


    view()->Hide( &selection );

    for( auto item : m_solver->Items() )
        if( item->IsPrimary() )
           view()->Hide( item->GetParent() );



    bool modified = false;
    bool revert = false;

    // Main loop: keep receiving events
    while( OPT_TOOL_EVENT evt = Wait() )
    {
        if( !modified )
            updateEditedAnchor( *evt );

        if( evt->IsDrag( BUT_LEFT ) && m_editedAnchor )
        {
            if( !modified )
            {
                controls()->ForceCursorPosition( false );
// m_original = *m_editedAnchor;    // Save the original position
                controls()->SetAutoPan( true );
                //m_solver->StartMove();
                modified = true;
            }


            printf("MOVE!\n");
            m_solver->MoveAnchor( m_editedAnchor, controls()->GetCursorPosition() );
            m_solver->Run();

            view()->Update( m_geomPreview.get() );
        }
        else if( evt->IsMouseUp( BUT_LEFT ) )
        {
            controls()->SetAutoPan( false );

            if( modified )
            {
                if( m_solver->IsResultOK() )
                {
                    BOARD_COMMIT commit( this );

                    for( auto item : m_solver->Items() )
                        //if( item->IsPrimary() )
                        {
                            commit.Modify( item->GetParent() );
                            commitToBoardItem( item, item->GetParent() );
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
            // break;
        }
        else if(
            evt->Matches( m_selectionTool->ClearedEvent )
            || evt->Matches( m_selectionTool->UnselectedEvent )
            || evt->Matches( m_selectionTool->SelectedEvent ) )
        {
            break;
        }
        else
        {
            m_toolMgr->PassEvent();
        }
    }

    for( auto item : m_solver->Items() )
        //if( item->IsPrimary() )
            view()->Hide( item->GetParent(), false );

    view()->Remove( m_geomPreview.get() );
    view()->Hide( &selection, false );


    return 0;
}


void OUTLINE_EDITOR::setTransitions()
{
    Go( &OUTLINE_EDITOR::OnSelectionChange, SELECTION_TOOL::SelectedEvent );
    Go( &OUTLINE_EDITOR::OnSelectionChange, SELECTION_TOOL::UnselectedEvent );
    Go( &OUTLINE_EDITOR::OnSelectionChange, SELECTION_TOOL::ClearedEvent );
    Go( &OUTLINE_EDITOR::modifiedSelection, PCB_ACTIONS::selectionModified.MakeEvent() );
    Go( &OUTLINE_EDITOR::FilletCorner, filletCorner.MakeEvent() );
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
    double snapRadius = view()->ToWorld( 10.0 );

    if( aEvent.IsMotion() )
    {
        anchor = m_solver->FindAnchor( aEvent.Position(), snapRadius );
    }
    else if( aEvent.IsDrag( BUT_LEFT ) )
    {
        anchor = m_solver->FindAnchor( aEvent.DragOrigin(), snapRadius );
    }

    printf("upd %p\n", anchor);

    if( m_editedAnchor != anchor )
        setEditedAnchor( anchor );
}

int OUTLINE_EDITOR::modifiedSelection( const TOOL_EVENT& aEvent )
{
    printf("ModifiedSEL[outl]!\n");
    updateOutline();
    return 0;
}
