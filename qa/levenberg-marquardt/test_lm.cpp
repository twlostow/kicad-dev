#include <cmath>
#include <cstdio>
#include <levmar.h>
#include <memory>
#include <set>
#include <vector>

#include <wx/wx.h>

#include <geometry/point_set.h>
#include <geometry/seg.h>
#include <math/vector2d.h>

#include "levenberg_marquardt.h"

enum GS_CONSTRAINT_TYPE {
  GSC_FIXED_POS = (1<<0),
  GSC_SEGMENT_SLOPE = (1<<1),
  GSC_SEGMENT_LENGTH = (1<<2)
};


/* F1 x^2 + y^2 - 1 = 0 */
/* F2: 2*x - y = 0 */
double my_dfunc( double* p, int index, int dindex, void* udata )
{
    auto x = p[0], y = p[1];
    switch( index )
    {
    case 0:
        if( dindex == 0 ) /* df1/dx */
            return 2 * x;
        else /* df1/dy */
            return 2 * y;

    case 1:
        if( dindex == 0 ) /* df1/dx */
            return 2;
        else /* df1/dy */
            return -1;
    }
}

double my_func( double* p, int index, void* udata )
{
    auto x = p[0], y = p[1];
    //printf("Evalf %.10f %.10f\n", x, y);
    switch( index )
    {
    case 0: return x * x + y * y - 1;
    case 1: return 2 * x - y;
    }
}





class GS_ITEM;
class GS_SOLVER;



class GS_ANCHOR : public LM_PARAMETER
{
public:
    friend class GS_SOLVER;

    virtual int LmGetDimension() override
    {
        return 2;
    };

    virtual void LmSetValue( int index, double value ) override
    {
        if( index == 0 )
            m_pos.x = value;
        else
            m_pos.y = value;
    }

    virtual double LmGetValue( int index ) override
    {
        return (index == 0 ? m_pos.x : m_pos.y );
    }

    GS_ANCHOR( VECTOR2D aPos, bool aIsSolvable )
            : m_pos( aPos ),
              m_originPos( aPos ),
              m_solvable( aIsSolvable ), 
              m_index( getNextIndex() ),
              m_fixed( false )
    {
    }

    const VECTOR2D& GetPos() const
    {
        return m_pos;
    }

    const VECTOR2D& GetOriginPos() const
    {
        return m_originPos;
    }

    void SetPos( const VECTOR2D& aPos )
    {
        m_pos = aPos;
        m_originPos = aPos;
    }

    bool IsSolvable() const
    {
        return m_solvable;
    }

    bool IsMoved() const
    {
        return m_displacement != VECTOR2D( 0, 0 );
    }

    const VECTOR2D& GetDisplacement() const
    {
        return m_displacement;
    }

    void SetDisplacement( const VECTOR2D& aV )
    {
        m_displacement = aV;
    }
    
    void Link( GS_ITEM *aItem )
    {
        m_linkedItems.push_back(aItem);
    }

    void SetFixed( bool aFixed )
    {
        m_fixed = aFixed;
    }

    bool IsFixed() const 
    {
        return m_fixed;
    }

private:
    static int getNextIndex()
    {
        static int currentIndex = 0;
        return ( currentIndex++ );
    }

    int m_parameterIndex;
    bool m_fixed;
    bool                  m_solvable;
    VECTOR2D              m_originPos;
    VECTOR2D              m_pos;
    VECTOR2D              m_displacement;
    std::vector<GS_ITEM*> m_linkedItems;
    int                   m_index;
};

class GS_CONSTRAINT;

class GS_ITEM 
{
public:
    GS_ITEM( int aAnchorCount )
    {
        printf("Create item [%p, %d anchors]\n", this, aAnchorCount );
        m_anchors.resize( aAnchorCount );
    }

    virtual void AddConstraint( GS_CONSTRAINT *aConstraint )
    {
      m_constraints.push_back( aConstraint );
    }

    int GetConstraintCount() const { return m_constraints.size(); }

    virtual void CreateAnchors( GS_SOLVER* aSolver ) = 0;
    virtual void UpdateAnchors(){};
    GS_ANCHOR*   Anchor( int index )
    {
        printf("GetAnchor [%p] %d cnt %d\n", this, index, m_anchors.size() );
        return m_anchors[index];
    }

protected:
    uint32_t m_constrain;
    std::vector<GS_ANCHOR*> m_anchors;
    std::vector<GS_CONSTRAINT*> m_constraints;
};


class GS_CONSTRAINT : public LM_SOLVABLE {

    public:
        GS_CONSTRAINT( GS_ITEM* aParent ) : m_parent ( aParent ) {};
        virtual ~GS_CONSTRAINT() {};

    protected:

        GS_ITEM *m_parent;
};


class GS_REFERENCE_ANCHOR : public GS_ITEM
{
public:
    GS_REFERENCE_ANCHOR( GS_ANCHOR* aParent ) : GS_ITEM( 1 ), m_anchor( aParent )
    {
    }

    virtual void CreateAnchors( GS_SOLVER* aSolver ) override
    {
        m_anchors[0] = m_anchor;
        m_originPos = m_anchor->GetPos();
    }

private:
    GS_ANCHOR* m_anchor;
    VECTOR2D m_originPos;

};

#if 0

class GS_CONSTRAINT_FIXED_POS : public GS_CONSTRAINT {


    public:
        GS_CONSTRAINT_FIXED_POS( GS_REFERENCE_ANCHOR* aParent, VECTOR2D fixedPos ) : GS_CONSTRAINT( aParent ), m_fixedPos (fixedPos) {};


    virtual void LmFunc( LM_PARAMETER *params, double *x )
    {
        auto a0 = parent()->Anchor(0);
        auto p = a0->GetPos();
        auto orig = a0->GetOriginPos();

        x[0] = p.x - m_fixedPos.x;
        x[1] = p.y - m_fixedPos.y;

        printf("Ref (%.10f %.10f) err (%.10f %.10f)\n", orig.x, orig.y, x[0], x[1] );
    }

    virtual void LmDFunc( LM_PARAMETER *params, double *dx, int equationIndex ) override
    {
        dx[ 0 ] = 1.0;
        dx[ 1 ] = 1.0;
    }

    virtual int LmGetEquationCount()
    {
        return 2;
    }


    protected:

        GS_REFERENCE_ANCHOR* parent() const { return static_cast<GS_REFERENCE_ANCHOR*> (m_parent ); }
        VECTOR2D m_fixedPos;

};
#endif


class GS_SOLVER
{
    struct COMPARE_ANCHORS;

public:
    typedef std::set<GS_ANCHOR*, COMPARE_ANCHORS> ANCHOR_SET;

    GS_SOLVER(){};
    ~GS_SOLVER(){};


    void Add( GS_ITEM* aItem );
    bool Run();

    void SetReferenceAnchor( GS_ITEM *aItem, int aAnchor, VECTOR2D aPos )
    {
        //auto ref = new GS_REFERENCE_ANCHOR( aItem->Anchor( aAnchor ) );
        //AddConstraint( new GS_CONSTRAINT_FIXED_POS ( ref, aPos ) );
        //Add( ref );
    }

    GS_ANCHOR* CreateAnchor( const VECTOR2D& aPos, bool aSolvable )
    {
        auto a = new GS_ANCHOR( aPos, aSolvable );


        auto it = m_anchors.insert( a );
        //printf( "createAnchor %d %d %d %p %d\n", (int) aPos.x, (int) aPos.y, !!aSolvable,
        //            &( *it.first ), !!it.second );

        if( *it.first != a )
            delete a;

        return ( *it.first );
    }

    int GetAnchorCount() const
    {
        return m_anchors.size();
    }

    const ANCHOR_SET& GetAnchors() const
    {
        return m_anchors;
    }

    void MoveAnchor( GS_ITEM* parent, int index, const VECTOR2D& aDisp )
    {
        parent->Anchor( index )->SetDisplacement( aDisp );
    }

    void AddConstraint( GS_CONSTRAINT* constraint )
    {
        m_constraints.push_back( constraint );
    }

private:
    struct COMPARE_ANCHORS
    {
        bool operator()( const GS_ANCHOR* a, const GS_ANCHOR* b ) const
        {
            auto c = LexicographicalCompare( a->m_pos, b->m_pos );
            //printf("comp %p %p %d %d %d %d %d %d -> %d\n", a, b, (int)a->m_pos.x, (int)a->m_pos.y, (int)b->m_pos.x, (int)b->m_pos.y, !!a->m_solvable, !!b->m_solvable, c  );
            if( a->m_solvable && b->m_solvable )
            {
                if( !c )
                    return false;

                return c < 0;
            }
            else
            {
                if( c != 0 )
                    return c < 0;
                return a->m_index < b->m_index;
            }
        }
    };

    GS_ANCHOR **m_anchorMap;
    std::unique_ptr<LM_SOLVER> m_solver;
    std::vector<GS_ITEM*>      m_items;
    std::vector<GS_CONSTRAINT*> m_constraints;
    ANCHOR_SET                 m_anchors;
};

#if 0
class GS_CONSTRAINT_SEGMENT_FIXED_LENGTH : public GS_ITEM
{

    virtual int LmGetEquationCount() 
    {
            return 2;
    }


    virtual void LmFunc( LM_PARAMETER *params, double *x )  {};
    virtual void LmDFunc( LM_PARAMETER *params, double *dx )  {};

};
#endif


class GS_NULL_CONSTRAINT : public GS_CONSTRAINT
{
    public:
           GS_NULL_CONSTRAINT( GS_ANCHOR* aAnchor ) : GS_CONSTRAINT( nullptr ) { m_anchor = aAnchor; };


    virtual int LmGetEquationCount()
    {
        return 2;
    }

    virtual void LmFunc(double *x )  {
        //printf("anchor %p p0 %.1f p1 %.1f\n", m_anchor, params[0], params[1]);
        auto newpos = m_anchor->GetOriginPos() + m_anchor->GetDisplacement();
        x[0] = newpos.x - m_anchor->GetPos().x;
        x[1] = newpos.y - m_anchor->GetPos().y;
        printf("error %.1f %.1f\n", x[0], x[1]);
    };

    virtual void LmDFunc( double *dx, int equationIndex )  
    {
        printf("LMDFunc [eqn %d]\n", equationIndex );
        // x[0] = x coordinate, y[0] = y coordinate
        if( equationIndex == 0)
        {
            dx[0] = -1.0; // d (x coordinate) over dx
            dx[1] = 0.0; // d (y coordinate) over dy
        } else {
            dx[0] = 0.0; // d (x coordinate) over dx
            dx[1] = -1.0; // d (y coordinate) over dy
        }
    };

    GS_ANCHOR* m_anchor;
};

class GS_SEGMENT : public GS_ITEM
{
public:
    GS_SEGMENT( SEG aSeg ) : GS_ITEM( 3 ), m_seg( aSeg )
    {
    }

    GS_SEGMENT( const VECTOR2I& aA, const VECTOR2I& aB ) : GS_ITEM( 3 ), m_seg( SEG( aA, aB ) )
    {
    }

    virtual void CreateAnchors( GS_SOLVER* aSolver ) override
    {
        m_anchors[0] = aSolver->CreateAnchor( m_seg.A, true );
        m_anchors[1] = aSolver->CreateAnchor( m_seg.B, true );
        m_anchors[2] = aSolver->CreateAnchor( ( m_seg.B + m_seg.A ) / 2, false );
    }

private:
    SEG m_seg;
};


void GS_SOLVER::Add( GS_ITEM* aItem )
{
    m_items.push_back( aItem );
    aItem->CreateAnchors( this );
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

void solve_parallel()
{
    GS_SEGMENT segA( VECTOR2I( 10, 20 ), VECTOR2I( 10, 10 ) );
    GS_SEGMENT segB( VECTOR2I( 10, 10 ), VECTOR2I( 100, 10 ) );
    GS_SEGMENT segC( VECTOR2I( 100, 10 ), VECTOR2I( 110, 20 ) );
    GS_SOLVER  solver;

    //solver.AddConstraint( new GS_NULL_CONSTRAINT( &segA ) );
    //solver.AddConstraint( new GS_NULL_CONSTRAINT( &segB ) );
    //solver.AddConstraint( new GS_NULL_CONSTRAINT( &segC ) );

    solver.Add( &segA );
    solver.Add( &segB );
    solver.Add( &segC );
    //solver.SetReferenceAnchor ( &segA, 0, VECTOR2D(100, 200));

    solver.MoveAnchor( &segA, 0, VECTOR2D( 0, 10 ) );

    for( auto a : solver.GetAnchors() )
    {
        if( a->IsSolvable() )
        {
            solver.AddConstraint( new GS_NULL_CONSTRAINT( a ) );
            printf( "solvable %d, pos: (%d, %d), disp: (%d, %d)\n", !!a->IsSolvable(),
                    (int) a->GetPos().x, (int) a->GetPos().y, (int) a->GetDisplacement().x,
                    (int) a->GetDisplacement().y );
        }
    }

    solver.Run();

    for( auto a : solver.GetAnchors() )
    {
        if( a->IsSolvable() )
        {
            printf( "solvable %d, pos: (%d, %d), disp: (%d, %d)\n", !!a->IsSolvable(),
                (int) a->GetPos().x, (int) a->GetPos().y, (int) a->GetDisplacement().x,
                (int) a->GetDisplacement().y );
        }
    }



}

#if 1
int main()
{
    solve_parallel();
#if 0
  LM_SOLVER lm(2, 2);
  
  lm.AddEquation( 0, my_func, my_dfunc );
  lm.AddEquation( 1, my_func, my_dfunc );
  lm.Solve();
#endif


    return 0;
}
#endif

#if 0
#define ROSD 105.0


/* Rosenbrock function, global minimum at (1, 1) */
void ros(double *p, double *x, int m, int n, void *data)
{
   int i;

  printf("p0 %.10f p0 %.10f\n", p[0], p[1]);

  for(i=0; i<n; ++i)
    x[i]=((1.0-p[0])*(1.0-p[0]) + ROSD*(p[1]-p[0]*p[0])*(p[1]-p[0]*p[0]));
}

void jacros(double *p, double *jac, int m, int n, void *data)
{
 int i, j;

  for(i=j=0; i<n; ++i){
    jac[j++]=(-2 + 2*p[0]-4*ROSD*(p[1]-p[0]*p[0])*p[0]);
    jac[j++]=(2*ROSD*(p[1]-p[0]*p[0]));
  }
}

int main()
{
    int m, n, i;
    double p[2];
    double x[2];

    m=2; n=2;
    p[0]=-1.2; p[1]=1.0;
    for(i=0; i<n; i++) x[i]=0.0;
    int ret=dlevmar_der(ros, jacros, p, x, m, n, 10000, NULL, NULL, NULL, NULL, NULL); // with analytic Jacobian
    printf("ret %d\n", ret);

    printf("x %.10f y %.10f p0 %.10f p1 %.10f\n", x[0], x[1], p[0], p[1]);
  return 0;
}

#endif