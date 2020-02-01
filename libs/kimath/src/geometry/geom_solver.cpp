#include <cmath>
#include <cstdio>
#include <deque>
#include <memory>
#include <set>
#include <vector>

#include <wx/wx.h>

#include <math/vector2d.h>

#include <geometry/geom_solver.h>
#include <geometry/point_set.h>
#include <geometry/seg.h>

void GS_SEGMENT::CreateAnchors( GS_SOLVER* aSolver )
{
    m_anchors[0] = aSolver->CreateAnchor( m_seg.A, true );
    m_anchors[1] = aSolver->CreateAnchor( m_seg.B, true );
    m_anchors[2] = aSolver->CreateAnchor( ( m_seg.B + m_seg.A ) / 2, false );
    m_anchors[0]->SetParent( this );
    m_anchors[1]->SetParent( this );
    m_anchors[2]->SetParent( this );
    m_anchors[0]->Link( this );
    m_anchors[1]->Link( this );
    m_anchors[2]->Link( this );
}

void GS_SEGMENT::SyncAnchors()
{
    auto a = m_anchors[0]->GetPos();
    auto b = m_anchors[1]->GetPos();
    m_anchors[2]->SetPos( ( a + b ) / 2.0 );
}


void GS_ARC::Update( const VECTOR2I& start, const VECTOR2I& end, const VECTOR2I& center )
{
    m_arc.ConstructFromCenterAndCorners( start, end, center );
}

void GS_ARC::CreateAnchors( GS_SOLVER* aSolver )
{
    m_anchors[0] = aSolver->CreateAnchor( m_arc.GetP0(), true );
    m_anchors[1] = aSolver->CreateAnchor( m_arc.GetP1(), true );
    m_anchors[2] = aSolver->CreateAnchor( m_arc.GetCenter(), true );
    m_anchors[0]->SetParent( this );
    m_anchors[1]->SetParent( this );
    m_anchors[2]->SetParent( this );
    m_anchors[0]->Link( this );
    m_anchors[1]->Link( this );
    m_anchors[2]->Link( this );
}

static inline bool anchorsEqual( const GS_ANCHOR* a, const GS_ANCHOR* b )
{
    const double epsilon = 10.0;

    if( a->IsSolvable() != b->IsSolvable() )
        return false;

    if( fabs( a->GetPos().x - b->GetPos().x ) > epsilon )
        return false;
    if( fabs( a->GetPos().y - b->GetPos().y ) > epsilon )
        return false;

    return true;
}


GS_ANCHOR* GS_SOLVER::findFixedAnchor( GS_ANCHOR *aMovedAnchor )
{
    
}

GS_ANCHOR* GS_SOLVER::CreateAnchor( const VECTOR2D& aPos, bool aSolvable )
{
    auto a = new GS_ANCHOR( aPos, aSolvable );


    for( auto a2 : m_anchors )
    {
        if( anchorsEqual( a, a2 ) )
        {
            delete a;
            return a2;
        }
    }

    m_anchors.push_back( a );
    return a;
}


void GS_SOLVER::Add( GS_ITEM* aItem )
{
    m_items.push_back( aItem );
    aItem->CreateAnchors( this );
}

static int mround( double x )
{
    return (int) floor( x + 0.5 );
}

bool GS_SOLVER::Run()
{
    LM_SOLVER solver;

    m_constraints.clear();

    for( auto item : m_items )
    {
        for( auto c : item->Constraints() )
        {
            m_constraints.push_back( c );
        }
    }


    for( auto constraint : m_constraints )
    {
        if( constraint->GetParent()->IsPrimary() )
        {
            solver.AddEquation( constraint );
        }
    }

    for( auto anchor : m_anchors )
    {
        if( anchor->IsSolvable() && anchor->GetParent()->IsPrimary() )
        {
            printf( "** add anchor %p %.3f %.3f, parent %p type %d\n", anchor, anchor->GetPos().x,
                    anchor->GetPos().y, anchor->GetParent(), anchor->GetParent()->Type() );

            solver.AddParameter( anchor );
        }
    }

    for( auto a : m_anchors )
    {
        if( a->IsSolvable() && a->GetParent()->IsPrimary() && a->IsMoved() )
        {
            solver.AddEquation( new GS_POSITION_CONSTRAINT( a ) );
        }
    }


    m_resultOK = solver.Solve();

    for( auto item : m_items )
        item->SyncAnchors();

    return m_resultOK;
}

void GS_SOLVER::FindOutlines( GS_ITEM* rootItem )
{
    std::deque<GS_ANCHOR*> Q;
    std::set<GS_ANCHOR*>   processed;
    std::set<GS_ITEM*>     outlineItems;

    outlineItems.insert( rootItem );

    printf( "FindOutlines: %d anchors\n", m_anchors.size() );

    for( auto a : m_anchors )
    {
        if( a->IsSolvable() )
            printf( "- a %.3f %.3f owner %p\n", a->GetPos().x, a->GetPos().y, a->GetParent() );
    }

    for( auto item : m_items )
    {
        item->SetPrimary( false );
    }

    for( auto a : rootItem->GetAnchors() )
    {
        Q.push_back( a );
    }

    printf( "FindOutlines: init Q %d\n", Q.size() );

    while( !Q.empty() )
    {
        auto current = Q.front();
        Q.pop_front();
        processed.insert( current );

        for( const auto litem : current->CLinkedItems() )
        {
            printf( "scan link %p\n", litem );
            bool match = false;
            outlineItems.insert( litem );

            for( auto ll : litem->GetAnchors() )
            {
                if( anchorsEqual( ll, current ) )
                {
                    match = true;
                    break;
                }
            }

            if( match )
            {
                for( auto ll : litem->GetAnchors() )
                {
                    if( processed.find( ll ) == processed.end() )
                    {
                        Q.push_back( ll );
                    }
                }
            }
        }
    }

    for( auto item : outlineItems )
    {
        item->SetPrimary( true );
    }

    //return true;
}

void GS_SOLVER::Clear()
{
    m_anchors.clear();
    m_items.clear();
    m_constraints.clear();
}

GS_ANCHOR* GS_SOLVER::FindAnchor( VECTOR2D pos, double snapRadius )
{
    for( auto anchor : m_anchors )
    {
        auto d = anchor->GetPos() - pos;

        //printf( "d %d %d\n", d.x, d.y );

        if( abs( d.x ) <= snapRadius && abs( d.y ) <= snapRadius )
            return anchor;
    }

    return nullptr;
}

int GS_POSITION_CONSTRAINT::LmGetEquationCount()
{
    return 2;
}

void GS_POSITION_CONSTRAINT::LmFunc( double* x )
{
    auto newpos = m_anchor->GetOriginPos() + m_anchor->GetDisplacement();

    x[0] = newpos.x - m_anchor->GetPos().x;
    x[1] = newpos.y - m_anchor->GetPos().y;
};

void GS_POSITION_CONSTRAINT::LmDFunc( double* dx, int equationIndex )
{
    int idx = m_anchor->LmGetIndex();

    if( equationIndex == 0 )
    {
        dx[idx] = -1.0;    // d (x coordinate) over dx
        dx[idx + 1] = 0.0; // d (x coordinate) over dy
    }
    else
    {
        dx[idx] = 0.0;      // d (y coordinate) over dx
        dx[idx + 1] = -1.0; // d (y coordinate) over dy
    }
};

int GS_CONSTRAINT_SEGMENT_FIXED_DIRECTION::LmGetEquationCount()
{
    return 1;
}

void GS_CONSTRAINT_SEGMENT_FIXED_DIRECTION::LmFunc( double* x )
{
    auto a0 = m_parent->Anchor( 0 );
    auto a1 = m_parent->Anchor( 1 );

    auto d_new = ( a1->GetPos() - a0->GetPos() ).Resize( 1 );
    auto d_old = ( a1->GetOriginPos() - a0->GetOriginPos() ).Resize( 1 );

    double error = d_new.Cross( d_old );
    x[0] = error;
};

void GS_CONSTRAINT_SEGMENT_FIXED_DIRECTION::LmDFunc( double* dx, int equationIndex )
{
    auto a0 = m_parent->Anchor( 0 );
    auto a1 = m_parent->Anchor( 1 );
    auto d_new = ( a1->GetPos() - a0->GetPos() ).Resize( 1 );
    auto d_old = ( a1->GetOriginPos() - a0->GetOriginPos() ).Resize( 1 );

    // 4 parameters involved
    // d_new.x = a1.x - a0.x
    // d_new.y = a1.y - a0.y
    // cross : x0 * y1 - y0 * x1
    // error = d_new.x * d_old.y - d_new.y * d_old.x
    // error = (a1.x - a0.x) * (d_old.y) - (a1.y - a0.y) * d_old.x

    dx[a0->LmGetIndex()] = -d_old.y;     // derr / d(a0.x)
    dx[a0->LmGetIndex() + 1] = d_old.x;  // derr / d(a0.y)
    dx[a1->LmGetIndex()] = d_old.y;      // derr / d(a1.x)
    dx[a1->LmGetIndex() + 1] = -d_old.x; // derr / d(a1.y)
};


int GS_CONSTRAINT_ARC_FIXED_ANGLES::LmGetEquationCount()
{
    return 4;
}

void GS_CONSTRAINT_ARC_FIXED_ANGLES::LmFunc( double* x )
{
    auto a0 = m_parent->Anchor( 0 );
    auto a1 = m_parent->Anchor( 1 );
    auto a2 = m_parent->Anchor( 2 );

    auto p0_orig = a0->GetOriginPos();
    auto p1_orig = a1->GetOriginPos();
    auto pc_orig = a2->GetOriginPos();

    VECTOR2D d0_orig = p0_orig - pc_orig;
    VECTOR2D d1_orig = p1_orig - pc_orig;

    auto a0_orig = atan2( d0_orig.y, d0_orig.x ) * 180.0 / M_PI;
    auto a1_orig = atan2( d1_orig.y, d1_orig.x ) * 180.0 / M_PI;

    auto p0_cur = a0->GetPos();
    auto p1_cur = a1->GetPos();
    auto pc_cur = a2->GetPos();

    VECTOR2D d0_cur = p0_cur - pc_cur;
    VECTOR2D d1_cur = p1_cur - pc_cur;

    auto a0_cur = atan2( d0_cur.y, d0_cur.x ) * 180.0 / M_PI;
    auto a1_cur = atan2( d1_cur.y, d1_cur.x ) * 180.0 / M_PI;

    //      printf("d0_cur %.1f %.1f d1_cur %.1f %.1f\n", d0_cur.x, d0_cur.y, d1_cur.x, d1_cur.y );
    //        printf("a0_orig %.1f a1 %.1f cur %.1f %.1f\n", a0_orig, a1_orig, a0_cur, a1_cur );

    x[0] = a0_orig - a0_cur;
    x[1] = a1_orig - a1_cur;
    x[2] = d0_orig.x - d0_cur.x - d1_orig.x + d1_cur.x;
    x[3] = d0_orig.y - d0_cur.y - d1_orig.y + d1_cur.y;
}

static double pow2( double x )
{
    return x * x;
}

void GS_CONSTRAINT_ARC_FIXED_ANGLES::LmDFunc( double* dx, int equationIndex )
{
    auto a0 = m_parent->Anchor( 0 );
    auto a1 = m_parent->Anchor( 1 );
    auto a2 = m_parent->Anchor( 2 );

    auto p0_cur = a0->GetPos();
    auto p1_cur = a1->GetPos();
    auto pc_cur = a2->GetPos();

    if( equationIndex == 0 )
    {
        // d(a0) / d(coordinate)
        dx[a0->LmGetIndex()] =
                -180.0 * ( -p0_cur.y + pc_cur.y )
                / ( M_PI * ( pow2( p0_cur.x - pc_cur.x ) + pow2( p0_cur.y - pc_cur.y ) ) );
        dx[a0->LmGetIndex() + 1] =
                -180.0 * ( p0_cur.x - pc_cur.x )
                / ( M_PI * ( pow2( p0_cur.x - pc_cur.x ) + pow2( p0_cur.y - pc_cur.y ) ) );
        dx[a1->LmGetIndex()] = 0.0;
        dx[a1->LmGetIndex() + 1] = 0.0;
        dx[a2->LmGetIndex()] =
                180.0 * ( -p0_cur.y + pc_cur.y )
                / ( M_PI * ( pow2( p0_cur.x - pc_cur.x ) + pow2( p0_cur.y - pc_cur.y ) ) );
        dx[a2->LmGetIndex() + 1] =
                180.0 * ( p0_cur.x - pc_cur.x )
                / ( M_PI * ( pow2( p0_cur.x - pc_cur.x ) + pow2( p0_cur.y - pc_cur.y ) ) );
    }
    else if( equationIndex == 1 )
    {
        // d(a1) / d(coordinate)
        dx[a0->LmGetIndex()] = 0.0;
        dx[a0->LmGetIndex() + 1] = 0.0;
        dx[a1->LmGetIndex()] =
                -180.0 * ( -p1_cur.y + pc_cur.y )
                / ( M_PI * ( pow2( p1_cur.x - pc_cur.x ) + pow2( p1_cur.y - pc_cur.y ) ) );
        dx[a1->LmGetIndex() + 1] =
                -180.0 * ( p1_cur.x - pc_cur.x )
                / ( M_PI * ( pow2( p1_cur.x - pc_cur.x ) + pow2( p1_cur.y - pc_cur.y ) ) );
        dx[a2->LmGetIndex()] =
                180.0 * ( -p1_cur.y + pc_cur.y )
                / ( M_PI * ( pow2( p1_cur.x - pc_cur.x ) + pow2( p1_cur.y - pc_cur.y ) ) );
        dx[a2->LmGetIndex() + 1] =
                180.0 * ( p1_cur.x - pc_cur.x )
                / ( M_PI * ( pow2( p1_cur.x - pc_cur.x ) + pow2( p1_cur.y - pc_cur.y ) ) );
    }
    else if( equationIndex == 2 )
    {
        dx[a0->LmGetIndex()] = -1.0;
        dx[a0->LmGetIndex() + 1] = 0.0;
        dx[a1->LmGetIndex()] = 1.0;
        dx[a1->LmGetIndex() + 1] = 0.0;
        dx[a2->LmGetIndex()] = 0.0;
        dx[a2->LmGetIndex() + 1] = 0.0;
    }
    else
    {
        dx[a0->LmGetIndex()] = 0.0;
        dx[a0->LmGetIndex() + 1] = -1.0;
        dx[a1->LmGetIndex()] = 0.0;
        dx[a1->LmGetIndex() + 1] = 1.0;
        dx[a2->LmGetIndex()] = 0.0;
        dx[a2->LmGetIndex() + 1] = 0.0;
    }
}