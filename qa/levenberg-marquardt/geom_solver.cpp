#include <cmath>
#include <cstdio>
#include <memory>
#include <set>
#include <vector>

#include <wx/wx.h>

#include <math/vector2d.h>

#include <geometry/point_set.h>
#include <geometry/seg.h>
#include <geometry/geom_solver.h>

#include <levmar.h>

enum GS_CONSTRAINT_TYPE {
  GSC_FIXED_POS = (1<<0),
  GSC_SEGMENT_SLOPE = (1<<1),
  GSC_SEGMENT_LENGTH = (1<<2)
};


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

    for( auto constraint : m_constraints )
    {
        solver.AddSolvable( constraint );
    }

    for ( auto anchor : m_anchors )
    {
        if ( anchor->IsSolvable() )
            solver.AddParameter( anchor );
    }

    solver.Solve();

    return true;
}

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
