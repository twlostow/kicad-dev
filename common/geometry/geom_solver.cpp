#include <cmath>
#include <cstdio>
#include <memory>
#include <set>
#include <vector>
#include <deque>

#include <wx/wx.h>

#include <math/vector2d.h>

#include <geometry/point_set.h>
#include <geometry/seg.h>
#include <geometry/geom_solver.h>


void GS_SOLVER::Add( GS_ITEM* aItem )
{
    m_items.push_back( aItem );
    aItem->CreateAnchors( this );
}

static int mround( double x )
{
    return (int)floor(x+0.5);
}

bool GS_SOLVER::Run()
{
 //   printf("LM: %d equations, %d parameters\n", equationCount, pindex );

    LM_SOLVER solver( 1000 );

    
    printf("Solver: Anchors: %d constraints %d\n", m_anchors.size(), m_constraints.size() );

    m_constraints.clear();

   
    for( auto item : m_items )
    {
        for( auto c : item->Constraints() )
        {
            m_constraints.push_back(c);
        }
    }

    
   /*for( auto a : GetAnchors() )
    {
        if( a->IsSolvable() )
        {
            AddConstraint( new GS_NULL_CONSTRAINT( a ) );
            //printf( "solvable %d, pos: (%d, %d), disp: (%d, %d)\n", !!a->IsSolvable(),
             //       mround(a->GetPos().x), mround( a->GetPos().y ), (int) a->GetDisplacement().x,
               //     (int) a->GetDisplacement().y );
        }
    }*/

    for( auto constraint : m_constraints )
    {
        solver.AddSolvable( constraint );
    }

    for ( auto anchor : m_anchors )
    {
        printf("Proicess anchor %p\n", anchor );
        if ( anchor->IsSolvable() && anchor->GetParent()->IsPrimary() )
            solver.AddParameter( anchor );
    }

    for( auto a : m_anchors )
    {

        printf("****** %d %d \n", solver.GetSolvableCount(), solver.GetParameterCount() * 2 ); // fixme
        if( a->IsSolvable() && a->GetParent()->IsPrimary() )
        {
            printf("Create null constraint for %p\n", a );

            solver.AddSolvable( new GS_NULL_CONSTRAINT( a ) );

            if( solver.GetSolvableCount() >= solver.GetParameterCount() * 2 )
                break;
        }
    }


    m_resultOK = solver.Solve();

    for( auto item : m_items )
        item->SyncAnchors();

    printf("Solver: %d\n", !!m_resultOK );

    return m_resultOK;
}

#if 0

static void solve_parallel()
{
    GS_SEGMENT segA( VECTOR2I( 10, 10 ), VECTOR2I( 10, 20 ) );
    GS_SEGMENT segB( VECTOR2I( 10, 20 ), VECTOR2I( 100, 20 ) );
    GS_SEGMENT segC( VECTOR2I( 100, 20 ), VECTOR2I( 110, 10 ) );
    GS_SOLVER  solver;

    //solver.AddConstraint( new GS_NULL_CONSTRAINT( &segA ) );
    //solver.AddConstraint( new GS_NULL_CONSTRAINT( &segB ) );
  

    solver.Add( &segA );
    solver.Add( &segB );
    solver.Add( &segC );
    //solver.SetReferenceAnchor ( &segA, 0, VECTOR2D(100, 200));

    solver.AddConstraint( new GS_CONSTRAINT_SEGMENT_FIXED_DIRECTION( &segA ) );
    solver.AddConstraint( new GS_CONSTRAINT_SEGMENT_FIXED_DIRECTION( &segB ) );

    solver.MoveAnchor( &segA, 1, VECTOR2D( -5, -5 ) );

    for( auto a : solver.GetAnchors() )
    {
        if( a->IsSolvable() )
        {
            solver.AddConstraint( new GS_NULL_CONSTRAINT( a ) );
            printf( "solvable %d, pos: (%d, %d), disp: (%d, %d)\n", !!a->IsSolvable(),
                    mround(a->GetPos().x), mround( a->GetPos().y ), (int) a->GetDisplacement().x,
                    (int) a->GetDisplacement().y );
        }
    }

    solver.Run();

    for( auto a : solver.GetAnchors() )
    {
        if( a->IsSolvable() )
        {
            printf( "solvable %d, pos: (%d, %d), disp: (%d, %d)\n", !!a->IsSolvable(),
                mround ( a->GetPos().x ), mround ( a->GetPos().y ), mround( - a->GetOriginPos().x + a->GetPos().x ),
                mround( - a->GetOriginPos().y + a->GetPos().y  ) );
        }
    }



}



#endif

void GS_SOLVER::FindOutlines( GS_ITEM *rootItem )
{
    std::deque<GS_ANCHOR*>  Q;
    std::set<GS_ANCHOR*>    processed;
    std::set<GS_ITEM*> outlineItems;

    outlineItems.insert(rootItem);

    for( auto item : m_items )
    {
        item->SetPrimary ( false );
    }

    for( auto a : rootItem->GetAnchors() )
    {
        Q.push_back( a );
     //   processed.insert( a );
    }

    printf("FindOutlines: init Q %d\n", Q.size() );

    while( !Q.empty() )
    {
        auto current = Q.front();
        Q.pop_front();

        auto scanLinks = [&] ( GS_ANCHOR* anchor )
        {
            processed.insert( anchor );

            printf("scan anchor %p\n", anchor);
            for( const auto litem : anchor->CLinkedItems() )
            {
                printf("scan link %p\n", litem);
                for( auto ll : litem->GetAnchors() )
                {
                    printf("scan la %p\n", ll);
                    if( processed.find( ll ) != processed.end() )
                    {
                                     printf("Scan %p\n", ll);
                                     processed.insert( ll );
                                     Q.push_back( ll );
                                     outlineItems.insert( litem );
                                 }
                             }
                         };
        };

        for(auto anchor : m_anchors )
            if( anchor->GetPos() ==  current->GetPos() && (processed.find(anchor) == processed.end() ) )
                scanLinks (anchor);
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

        printf( "d %d %d\n", d.x, d.y );

        if( abs( d.x ) <= snapRadius && abs( d.y ) <= snapRadius )
            return anchor;
    }

    return nullptr;
}
