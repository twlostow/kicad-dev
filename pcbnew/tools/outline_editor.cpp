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

#include <class_zone.h>
#include <class_board.h>
#include <class_module.h>
#include <connectivity.h>

#include <view/view.h>
#include <geometry/shape.h>
#include <geometry/shape_segment.h>
#include <pcb_painter.h>

class GS_ITEM;

enum GS_CONSTRAINT_TYPE
{
    CS_START_ANGLE = 1,
    CS_RADIUS = 2,
    CS_CENTRAL_ANGLE= 4,
    CS_LENGTH = 8,
    CS_DIRECTION = 16
};

enum GS_ITEM_TYPE
{
    GST_SEGMENT = 0
};

class GS_ANCHOR
{
public:
    GS_ANCHOR() {};

    GS_ANCHOR( GS_ITEM*parent, int id, VECTOR2I pos)
     : m_parent(parent),
     m_pos(pos),
     m_id(id),
     m_changedPos(false)
     {

    }

    void SetPos( const VECTOR2I& aPos )
    {
        m_pos = aPos;
    }

    const VECTOR2I& GetPos() const { return m_pos; }

    void SetNextPos( const VECTOR2I& aPos )
    {
        m_nextPos = aPos;
        if( aPos != m_pos )
            m_changedPos = true;
    }

    const VECTOR2I GetNextPos() const { return m_nextPos; }

    void UpdatePos()
    {
        m_pos = m_nextPos;
        m_changedPos = false;
    }

    bool PositionChanged() const
    {
        return m_changedPos;
    }

    GS_ITEM *GetParent() const { return m_parent; }

    void ClearLinks() { m_linkedAnchors.clear(); }
    void Link( GS_ANCHOR* anchor ) { m_linkedAnchors.insert(anchor); }
    std::set<GS_ANCHOR*> GetLinks() const { return m_linkedAnchors; }


    int GetId() const { return m_id; }

    int m_flags;
    GS_ITEM* m_parent;
    VECTOR2I m_pos;
    VECTOR2I m_nextPos;
    VECTOR2I m_offset;
    std::set<GS_ANCHOR*> m_linkedAnchors;
    int m_id;
    bool m_changedPos;

};

class GS_ITEM
{
public:
    GS_ITEM( GS_ITEM_TYPE type, BOARD_ITEM *parent = nullptr ) : m_type(type), m_parent(parent) {};
    virtual ~GS_ITEM() {};

    virtual void MoveAnchor(int id, VECTOR2I p, std::vector<GS_ANCHOR*>& changedAnchors ) = 0;
    virtual void SaveState() = 0;
    virtual void RestoreState() = 0;
    virtual void UpdateAnchors() = 0;

    const std::vector<GS_ANCHOR*>& GetAnchors() const{ return  m_anchors; }

    void SetPrimary ( bool primary ) { m_primary = primary; }
    bool IsPrimary() const { return m_primary; }

    GS_ITEM_TYPE Type() const { return m_type; }

    BOARD_ITEM* GetParent() const { return m_parent; }

    void Constrain ( int aWhat, bool aEnabled )
    {
        m_constrain &= ~aWhat;
        if (aEnabled)
            m_constrain |= aWhat;
        printf("constrain %x %d\n", aWhat, !!aEnabled );

    }

    bool IsConstrained( int aWhat )
    {
        return (m_constrain & aWhat) == aWhat;
    }

protected:
    GS_ITEM_TYPE m_type;
    int m_constrain = 0;
    std::vector<GS_ANCHOR*> m_anchors;
    BOARD_ITEM *m_parent = nullptr;
    bool m_primary = false;
};

class GS_SEGMENT : public GS_ITEM
{
public:
    GS_SEGMENT( DRAWSEGMENT* aSeg ) : GS_ITEM( GST_SEGMENT, aSeg )
    {
        m_p0 = aSeg->GetStart();
        m_p1 = aSeg->GetEnd();
        m_dir = m_p1 - m_p0;
        m_anchors.push_back( new GS_ANCHOR( this, 0, m_p0 ) );
        m_anchors.push_back( new GS_ANCHOR( this, 1, m_p1 ) );
        m_parent = aSeg;
    }

    virtual void MoveAnchor(int id, VECTOR2I p, std::vector<GS_ANCHOR*>& changedAnchors )
    {
        printf("MoveAnchor id %d constraint %x\n", id, m_constrain);

        switch(id)
        {
            case 0:
                if( m_constrain == ( CS_DIRECTION | CS_LENGTH ) )
                {
                    auto d = m_anchors[0]->GetPos() - p;
                    m_anchors[0]->SetNextPos( m_anchors[0]->GetPos() - d );
                    m_anchors[1]->SetNextPos( m_anchors[1]->GetPos() - d );
                }
                else if ( m_constrain == CS_DIRECTION )
                {
                    printf("ConstrainDir!\n");
                    SEG s( p, p + m_dir );
                    m_anchors[0]->SetNextPos( p );
                    m_anchors[1]->SetNextPos( s.LineProject( m_anchors[1]->GetPos() ) );
                }
                else
                {
                    m_anchors[0]->SetNextPos(p);
                    m_anchors[1]->SetNextPos( m_anchors[1]->GetPos() );
                }
                break;
            case 1:
                if( m_constrain == ( CS_DIRECTION | CS_LENGTH ) )
                {
                    auto d = m_anchors[1]->GetPos() - p;
                    m_anchors[0]->SetNextPos( m_anchors[0]->GetPos() - d );
                    m_anchors[1]->SetNextPos( m_anchors[1]->GetPos() - d );
                }
                else if ( m_constrain == CS_DIRECTION )
                {
                    printf("ConstrainDir2!\n");
                    SEG s( p, p + m_dir );
                    m_anchors[1]->SetNextPos( p );
                    m_anchors[0]->SetNextPos( s.LineProject( m_anchors[0]->GetPos() ) );
                }
                else
                {
                    m_anchors[0]->SetNextPos( m_anchors[0]->GetPos() );
                    m_anchors[1]->SetNextPos(p);
                }
                break;
            default:
                assert(false);
        }

        if( m_anchors[0]->PositionChanged() )
            changedAnchors.push_back(m_anchors[0]);
        if( m_anchors[1]->PositionChanged() )
            changedAnchors.push_back(m_anchors[1]);
    }

    virtual void SaveState()
    {
        m_p0_saved = m_p0;
        m_p1_saved = m_p1;
    }

    virtual void RestoreState()
    {
        m_p0 = m_p0_saved;
        m_p1 = m_p1_saved;
        m_anchors[0]->SetPos(m_p0);
        m_anchors[1]->SetPos(m_p1);
    }

    virtual void UpdateAnchors()
    {
       m_anchors[0]->UpdatePos();
       m_anchors[1]->UpdatePos();
       m_p0 = m_anchors[0]->GetPos();
       m_p1 = m_anchors[1]->GetPos();
    }

    VECTOR2I GetStart() const { return m_p0; }
    VECTOR2I GetEnd() const { return m_p1; }


private:
    VECTOR2I m_p0, m_p1, m_dir;
    VECTOR2I m_p0_saved, m_p1_saved;
};

class GEOM_SOLVER
{
    friend class GEOM_PREVIEW;
    struct DISPLACEMENT
    {
        DISPLACEMENT( GS_ANCHOR *aAnchor, VECTOR2I aP ) :
            anchor(aAnchor),
            p(aP)
            {};

        GS_ANCHOR *anchor;
        VECTOR2I p;
    };

public:
    GEOM_SOLVER()
    {

    }

    ~GEOM_SOLVER()
    {

    }



    GS_ANCHOR *FindAnchor ( VECTOR2I pos, double snapRadius )
    {
        for ( auto anchor : AllAnchors() )
        {
            auto d = anchor->GetPos() - pos;

            if ( abs(d.x) <= snapRadius && abs(d.y) <= snapRadius )
                return anchor;

        }

        return nullptr;
    }

    void StartMove()
    {
        for ( auto item : m_items )
            item->SaveState();
    }

    bool ValidateContinuity()
    {
        for( auto anchor : AllAnchors() )
        {
            int d_max = 0;

            for( auto link : anchor->GetLinks() )
            {
                d_max = std::max( d_max, ( link->GetPos() - anchor->GetPos() ).EuclideanNorm() );

                        if( d_max > c_epsilon )
                    return false;
            }
        }

        return true;
    }

    bool m_lastResultOk = false;

    bool IsResultOK() const
    {
        return m_lastResultOk;
    }

    bool MoveAnchor( GS_ANCHOR *refAnchor, VECTOR2I pos )
    {
        std::deque<DISPLACEMENT> Q;

        m_lastResultOk = false;

        printf("move-a %p (%d %d)\n", refAnchor, pos.x, pos.y);

        for ( auto item : m_items )
            item->RestoreState();

            for ( auto anchor : AllAnchors() )
                printf(" a %p parent %p\n", anchor, anchor->GetParent() );
        const int iterLimit = 100;
        int iter = 0;

        Q.push_back( DISPLACEMENT( refAnchor, pos ) );

        while ( !Q.empty() && iter < iterLimit )
        {
            printf("iter %d disps %d\n", iter, Q.size() );
            iter ++;

            std::set<GS_ANCHOR*> moved;
            for ( auto& displacement : Q )
            {
                if ( moved.find( displacement.anchor ) == moved.end() )
                {
                    printf("do %p\n", displacement.anchor);
                    std::vector<GS_ANCHOR*> changedAnchors;
                    displacement.anchor->GetParent()->MoveAnchor( displacement.anchor->GetId(), displacement.p, changedAnchors );
                    printf("changed links : %d\n", changedAnchors.size() );

                    for ( auto am : changedAnchors )
                    {
                        printf("  change %p\n", am);
                        moved.insert(am);
                    }
                }
            }

            Q.clear();

            printf("Moved links : %d\n", moved.size() );

            for ( auto anchor : moved )
            {
                printf("anchr %p\N", anchor );
                for ( auto link : anchor->GetLinks() )
                {
                    printf("- provess link: %p\n", link);
                    Q.push_back ( DISPLACEMENT( link, anchor->GetNextPos() ) );
                }
            }

            std::random_shuffle ( Q.begin(), Q.end() );

            for ( auto anchor : moved )
            {
                printf("update %p %p\n", anchor, anchor->GetParent());
                anchor->GetParent()->UpdateAnchors();
            }

            if ( ValidateContinuity() )
                break;
        }

        if ( iter == iterLimit )
            m_lastResultOk = false;

        m_lastResultOk = true;

        return m_lastResultOk;
    }

    void Add ( BOARD_ITEM *aItem, bool aPrimary = false )
    {
        if (aItem->Type() != PCB_LINE_T )
            return;

        auto ds = static_cast<DRAWSEGMENT*>(aItem);

        if(ds->GetShape() == S_SEGMENT)
        {
            auto s = new GS_SEGMENT( ds ) ;
            s->SetPrimary(aPrimary);

            printf("UFLags %x\n", ds->GetUserFlags() );
            if ( ds->GetUserFlags() & DSF_CONSTRAIN_LENGTH )
                s->Constrain (CS_LENGTH, true);
            if ( ds->GetUserFlags() & DSF_CONSTRAIN_DIRECTION )
                s->Constrain (CS_DIRECTION, true);

            m_items.push_back( s );
        }
    }

    void CommitChanges()
    {

    }

    const std::vector<GS_ANCHOR*> AllAnchors()
    {
        std::vector<GS_ANCHOR*> allAnchors;

        for( auto item:  m_items )
        {
            const auto& ia = item->GetAnchors();
            std::move( ia.begin(), ia.end(), std::back_inserter(allAnchors) );
        }
        printf("anchors: %d\n", allAnchors.size() );
        return allAnchors;
    }

    const std::vector<GS_ITEM*>& AllItems()
    {
        return m_items;
    }

    bool findOutlineElements( GS_ITEM* aItem )
    {
        std::deque<GS_ANCHOR*> Q;
        std::set<GS_ANCHOR*> processed;

        for( auto a : aItem->GetAnchors() )
        {
            Q.push_back( a );
            processed.insert( a );
        }

    //    printf("Init: %d a\n", Q.size() );
        while( !Q.empty() )
        {
            auto current = Q.front();
            Q.pop_front();

            for ( auto link : current->GetLinks() )
            {
                for ( auto ll : link->GetParent()->GetAnchors() )
                {
                //    printf("Do link %p\n", ll );
                    if( processed.find( ll ) == processed.end() )
                    {
                    //    printf("Scan %p\n", ll);
                        processed.insert( ll );
                        Q.push_back( ll );
                    }
                }
            }
        }

        for ( auto anchor : processed )
        {
            anchor->GetParent()->SetPrimary( true );
        }

        return true;
    //    printf("Linked anchors : %d\n", processed.size() );
    }

    const int c_epsilon = 1;

    void findAnchorLinks()
    {
        auto anchors = AllAnchors();

        for (auto a1 : anchors)
        {
            a1->ClearLinks();

            for( auto a2 : anchors )
            {
                int dist = (a1->GetPos() - a2->GetPos() ).EuclideanNorm();

                if(dist <= c_epsilon && a1 != a2 )
                {
                    a1->Link(a2);
                }
            }

        //    printf("anchor (%d %d): %d links\n", a1->GetPos().x, a1->GetPos().y, a1->GetLinks().size() );
        }
    }

    void FindOutlines()
    {
        findAnchorLinks();

        for ( auto item : m_items )
            if ( item->IsPrimary() )
                findOutlineElements( item );
    }


    std::vector<GS_ITEM*> m_items;
};

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

        float size1 = aView->ToWorld( 10 );
        float size2 = aView->ToWorld( 4 );
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
            for ( auto anchor : item->GetAnchors() )
            {
                float size = anchor->GetParent()->IsPrimary() ? size1 : size2;

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

int OUTLINE_EDITOR::OnSelectionChange( const TOOL_EVENT& aEvent )
{
    SELECTION& selection = m_selectionTool->GetSelection();


    Activate();

    m_solver.reset ( new GEOM_SOLVER );
    m_preview.reset ( new GEOM_PREVIEW ( m_solver ) );

    for ( auto item : board()->Drawings() )
    {
//        if( !selection.Contains(item) )
    //        m_solver->Add( static_cast<BOARD_ITEM*> ( item ), false );
    }

    for ( auto item : selection )
    {
//        m_solver->Add( static_cast<BOARD_ITEM*> ( item ), true );
    }

//    m_solver->FindOutlines();

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
                        printf("Commit %p\n", item);
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
