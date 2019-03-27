#ifndef __GEOM_SOLVER_H
#define __GEOM_SOLVER_H

#include <cmath>
#include <cstdio>
#include <memory>
#include <set>
#include <vector>

#include <wx/wx.h>

#include <geometry/point_set.h>
#include <geometry/seg.h>
#include <geometry/shape_arc.h>
#include <math/vector2d.h>

#include <math/levenberg_marquardt.h>

enum GS_CONSTRAINT_TYPE {
  GSC_FIXED_POS = (1<<0),
  GSC_SEGMENT_SLOPE = (1<<1),
  GSC_SEGMENT_LENGTH = (1<<2)
};

enum GS_ITEM_TYPE {
    GST_SEGMENT = 0,
    GST_ARC,
    GST_REFERENCE_ANCHOR
};

class GS_ITEM;
class GS_SOLVER;
class BOARD_ITEM;

class GS_ANCHOR : public LM_PARAMETER
{
public:

  GS_ANCHOR( VECTOR2D aPos, bool aIsSolvable, GS_ITEM* aParent = nullptr )
            : m_pos( aPos ),
              m_originPos( aPos ),
              m_solvable( aIsSolvable ), 
              m_index( getNextIndex() ),
              m_fixed( false ),
              m_parent( aParent )
    {
    }

    virtual ~GS_ANCHOR() {}

    friend class GS_SOLVER;

    GS_ITEM* GetParent() const { return m_parent; }
    void SetParent( GS_ITEM *aParent ) { m_parent = aParent; }

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

    virtual double LmGetInitialValue( int index ) override
    {
        return (index == 0 ? m_originPos.x : m_originPos.y );
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
        //printf("SetDisplace %p %.1f %.1f\n", this, aV.x, aV.y );
        m_displacement = aV;
        m_hasDisplacement = true;
    }
    
    bool HasDisplacement() const { return m_hasDisplacement; }

    std::vector<GS_ITEM*>& LinkedItems() { return m_linkedItems; }
    const std::vector<GS_ITEM*>& CLinkedItems() const { return m_linkedItems; }

    void Link( GS_ITEM *aItem )
    {
        //printf("anchor %p link %p\n", this, aItem );
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

    bool m_hasDisplacement = false;
    int m_parameterIndex;
    bool m_fixed;
    bool                  m_solvable;
    VECTOR2D              m_originPos;
    VECTOR2D              m_pos;
    VECTOR2D              m_displacement;
    std::vector<GS_ITEM*> m_linkedItems;
    GS_ITEM*    m_parent;
    int                   m_index;
};

class GS_CONSTRAINT;

class GS_ITEM
{
public:
    GS_ITEM( int aAnchorCount, GS_ITEM_TYPE aType )
    {
        //printf("Create item [%p, %d anchors]\n", this, aAnchorCount );
        m_anchors.resize( aAnchorCount );
        m_type = aType;
        m_parent = nullptr;
        m_isPrimary = false;
    }

    virtual void AddConstraint( GS_CONSTRAINT *aConstraint )
    {
      m_constraints.push_back( aConstraint );
    }

    int GetConstraintCount() const { return m_constraints.size(); }

    std::vector<GS_CONSTRAINT*> Constraints() { return m_constraints; }

    virtual void CreateAnchors( GS_SOLVER* aSolver ) = 0;
    virtual void UpdateAnchors(){};
    GS_ANCHOR*   Anchor( int index )
    {
        //printf("GetAnchor [%p] %d cnt %d\n", this, index, m_anchors.size() );
        return m_anchors[index];
    }

    const GS_ANCHOR*   CAnchor( int index ) const
    {
        //printf("GetAnchor [%p] %d cnt %d\n", this, index, m_anchors.size() );
        return m_anchors[index];
    }


    GS_ITEM_TYPE Type() const { return m_type; }

    void SetParent( BOARD_ITEM* aParent ) { m_parent = aParent; }
    BOARD_ITEM* GetParent() const { return m_parent; }

    const std::vector<GS_ANCHOR*>& GetAnchors() const { return m_anchors; }

    bool IsPrimary() const { return m_isPrimary; }
    void SetPrimary( bool aPrim ) { m_isPrimary = aPrim; }

    virtual void SyncAnchors(){};

protected:
    BOARD_ITEM* m_parent;
    GS_ITEM_TYPE m_type;
    bool m_isPrimary;
    uint32_t m_constrain;
    std::vector<GS_ANCHOR*> m_anchors;
    std::vector<GS_CONSTRAINT*> m_constraints;
};


class GS_CONSTRAINT : public LM_EQUATION {

    public:
        GS_CONSTRAINT( GS_ITEM* aParent ) : m_parent ( aParent ) {};
        virtual ~GS_CONSTRAINT() {};

        GS_ITEM *GetParent() const { return m_parent; }

    protected:

        GS_ITEM *m_parent;
};


class GS_REFERENCE_ANCHOR : public GS_ITEM
{
public:
    GS_REFERENCE_ANCHOR( GS_ANCHOR* aParent ) : GS_ITEM( 1, GST_REFERENCE_ANCHOR ), m_anchor( aParent )
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

static inline bool anchorsEqual( const GS_ANCHOR *a, const GS_ANCHOR *b )
{
    const double epsilon = 10.0;

    if( a->IsSolvable() != b->IsSolvable() )
        return false;

    if (fabs( a->GetPos().x - b->GetPos().x ) > epsilon)
        return false;
    if (fabs( a->GetPos().y - b->GetPos().y ) > epsilon)
        return false;

    return true;
}


class GS_SOLVER
{
    struct COMPARE_ANCHORS;

public:
    typedef std::vector<GS_ANCHOR*> ANCHOR_SET;

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


        for (auto a2 : m_anchors)
        {
            if( anchorsEqual( a, a2 ) )
            {
                delete a;
                return a2;
            }
        }
        
        m_anchors.push_back(a);
        return a;
    }

    int GetAnchorCount() const
    {
        return m_anchors.size();
    }

    const ANCHOR_SET& GetAnchors() const
    {
        return m_anchors;
    }

    /*void MoveAnchor( GS_ITEM* parent, int index, const VECTOR2D& aDisp )
    {
        parent->Anchor( index )->SetDisplacement( aDisp );
    }*/

    void MoveAnchor( GS_ANCHOR* aAnchor, const VECTOR2D& aNewPos )
    {
        //printf("movea %p %.0f %.0f\n", aAnchor, aNewPos.x, aNewPos.y );
        aAnchor->SetDisplacement( aNewPos - aAnchor->GetOriginPos() );
    }
    
    void AddConstraint( GS_CONSTRAINT* constraint )
    {
        m_constraints.push_back( constraint );
    }

    void Clear();
    void FindOutlines( GS_ITEM *rootItem );
    bool IsResultOK()const { return m_resultOK; };

    GS_ANCHOR* FindAnchor( VECTOR2D pos, double snapRadius );


    std::vector<GS_ITEM*>& Items() { return m_items; }
private:


    //GS_ANCHOR **m_anchorMap;
    std::unique_ptr<LM_SOLVER> m_solver;
    std::vector<GS_ITEM*>      m_items;
    std::vector<GS_CONSTRAINT*> m_constraints;
    ANCHOR_SET                 m_anchors;
    bool m_resultOK;
};



class GS_NULL_CONSTRAINT : public GS_CONSTRAINT
{
    public:
           GS_NULL_CONSTRAINT( GS_ANCHOR* aAnchor ) : GS_CONSTRAINT( nullptr ) { m_anchor = aAnchor; };


    virtual int LmGetEquationCount()
    {
        return 2;
    }

    virtual void LmFunc(double *x )  {
        
        
        if ( !m_anchor->HasDisplacement() )
        {
            //printf("NullC [%p] no Disp\n", this );
            x[0] = x[1] = 0.0;
            return;
        }
        
        auto newpos = m_anchor->GetOriginPos() + m_anchor->GetDisplacement();
        //printf("****** anchor %p \n", m_anchor );

        //printf("NullC [%p] func %.0f %.0f curpos %.0f %.0f\n", this, newpos.x, newpos.y, m_anchor->GetPos().x, m_anchor->GetPos().y);
        

        x[0] = newpos.x - m_anchor->GetPos().x;
        x[1] = newpos.y - m_anchor->GetPos().y;
        //printf("error %.1f %.1f\n", x[0], x[1]);
    };

    virtual void LmDFunc( double *dx, int equationIndex )  
    {
        int idx = m_anchor->LmGetIndex();

        //printf("%p LMDFunc [eqn %d, idx %d]\n", this, equationIndex, idx );

        if ( !m_anchor->HasDisplacement() )
        {
            dx[idx] = 0.0;
            dx[idx + 1] = 0.0;
            return;
        }
        // x[0] = x coordinate, y[0] = y coordinate
        if( equationIndex == 0)
        {
            dx[idx] = -1.0; // d (x coordinate) over dx
            dx[idx + 1] = 0.0; // d (x coordinate) over dy
        } else {
            dx[idx] = 0.0; // d (y coordinate) over dx
            dx[idx + 1] = -1.0; // d (y coordinate) over dy
        }
    };

    GS_ANCHOR* m_anchor;
};

class GS_SEGMENT : public GS_ITEM
{
public:
    GS_SEGMENT( SEG aSeg ) : GS_ITEM( 3, GST_SEGMENT ), m_seg( aSeg )
    {
    }

    GS_SEGMENT( const VECTOR2I& aA, const VECTOR2I& aB ) : GS_ITEM( 3, GST_SEGMENT ), m_seg( SEG( aA, aB ) )
    {
    }

    virtual void CreateAnchors( GS_SOLVER* aSolver ) override
    {
        m_anchors[0] = aSolver->CreateAnchor( m_seg.A, true );
        m_anchors[1] = aSolver->CreateAnchor( m_seg.B, true );
        m_anchors[2] = aSolver->CreateAnchor( ( m_seg.B + m_seg.A ) / 2, false );
        m_anchors[0]->SetParent(this);
        m_anchors[1]->SetParent(this);
        m_anchors[2]->SetParent(this);
        m_anchors[0]->Link(this);
        m_anchors[1]->Link(this);
        m_anchors[2]->Link(this);
    }

    virtual void SyncAnchors() override
    {
        auto a = m_anchors[0]->GetPos();
        auto b = m_anchors[1]->GetPos();
        m_anchors[2]->SetPos( (a+b) / 2.0 );
    }

private:
    SEG m_seg;
};

class GS_ARC : public GS_ITEM
{
public:
    GS_ARC( SHAPE_ARC aArc ) : GS_ITEM( 3, GST_ARC ), m_arc( aArc )
    {
    }

    GS_ARC( const VECTOR2I& start, const VECTOR2I& end, const VECTOR2I& center ) : GS_ITEM( 3, GST_ARC )
    {
        Update( start, end, center );
    }

    void Update( const VECTOR2I& start, const VECTOR2I& end, const VECTOR2I& center )
    {
        m_arc.ConstructFromCenterAndCorners( start, end, center );
    }

    virtual void CreateAnchors( GS_SOLVER* aSolver ) override
    {
        m_anchors[0] = aSolver->CreateAnchor( m_arc.GetP0(), true );
        m_anchors[1] = aSolver->CreateAnchor( m_arc.GetP1(), true );
        m_anchors[2] = aSolver->CreateAnchor( m_arc.GetCenter(), true );
        m_anchors[0]->SetParent(this);
        m_anchors[1]->SetParent(this);
        m_anchors[2]->SetParent(this);
        m_anchors[0]->Link(this);
        m_anchors[1]->Link(this);
        m_anchors[2]->Link(this);
    }

    const SHAPE_ARC& GetArc() const { return m_arc; }

private:
    SHAPE_ARC m_arc;
};

class GS_CONSTRAINT_SEGMENT_FIXED_DIRECTION : public GS_CONSTRAINT
{
public:
    GS_CONSTRAINT_SEGMENT_FIXED_DIRECTION( GS_SEGMENT* aSeg ) : GS_CONSTRAINT( aSeg ) { };

    virtual int LmGetEquationCount()
    {
        return 1;
    }

    virtual void LmFunc( double *x )  
    {
        auto a0 = m_parent->Anchor(0);
        auto a1 = m_parent->Anchor(1);

        auto d_new = (a1->GetPos() - a0->GetPos()).Resize( 1 );
        auto d_old = (a1->GetOriginPos() - a0->GetOriginPos()).Resize( 1 );

        //printf("a0 : %.2f %.2f a1 %.2f %.2f\n", a0->GetPos().x, a0->GetPos().y, a1->GetPos().x, a1->GetPos().y );

        double error = d_new.Cross(d_old);
        //printf("----------- d_new %.10f %.10f d_old %.10f %.10f err %.10f\n", d_new.x, d_new.y, d_old.x, d_old.y, error);
        x[0] = error;
    };

    virtual void LmDFunc( double *dx, int equationIndex )
    {
        auto a0 = m_parent->Anchor(0);
        auto a1 = m_parent->Anchor(1);
        auto d_new = (a1->GetPos() - a0->GetPos() ).Resize( 1 );
        auto d_old = (a1->GetOriginPos() - a0->GetOriginPos()).Resize( 1 );

        // 4 parameters involved
        // d_new.x = a1.x - a0.x
        // d_new.y = a1.y - a0.y
        // cross : x0 * y1 - y0 * x1
        // error = d_new.x * d_old.y - d_new.y * d_old.x
        // error = (a1.x - a0.x) * (d_old.y) - (a1.y - a0.y) * d_old.x
        
        dx [ a0->LmGetIndex() ] =  -d_old.y; // derr / d(a0.x)
        dx [ a0->LmGetIndex() + 1 ] =  d_old.x; // derr / d(a0.y)
        dx [ a1->LmGetIndex() ] =  d_old.y; // derr / d(a1.x)
        dx [ a1->LmGetIndex() + 1 ] =  -d_old.x; // derr / d(a1.y)
    };

};

class GS_CONSTRAINT_SEGMENT_FIXED_LENGTH : public GS_CONSTRAINT
{
public:
    GS_CONSTRAINT_SEGMENT_FIXED_LENGTH( GS_SEGMENT* aSeg ) : GS_CONSTRAINT( aSeg ) { };

    virtual int LmGetEquationCount()
    {
        return 1;
    }

    virtual void LmFunc( double *x )  
    {
        auto a0 = m_parent->Anchor(0);
        auto a1 = m_parent->Anchor(1);

        auto d_new = (a1->GetPos() - a0->GetPos()).EuclideanNorm();
        auto d_old = (a1->GetOriginPos() - a0->GetOriginPos()).EuclideanNorm();

        //printf("a0 : %.2f %.2f a1 %.2f %.2f\n", a0->GetPos().x, a0->GetPos().y, a1->GetPos().x, a1->GetPos().y );

        double error = d_old - d_new;
        //printf("----------- d_new %.10f %.10f d_old %.10f %.10f err %.10f\n", d_new.x, d_new.y, d_old.x, d_old.y, error);
        x[0] = error;
    };

    virtual void LmDFunc( double *dx, int equationIndex )
    {
        auto a0 = m_parent->Anchor(0);
        auto a1 = m_parent->Anchor(1);
        auto d_new = (a1->GetPos() - a0->GetPos());
        auto d_old = (a1->GetOriginPos() - a0->GetOriginPos());

        // 4 parameters involved
        // d_new.x = a1.x - a0.x
        // d_new.y = a1.y - a0.y
        // l_old = const
        // l_new = sqrt( d_new.x^2 + d_new.y ^ 2 )
        // err = l_old - l_new
        // derr/d(a0.x) = -sqrt( x*x + y*y ) = 1/(2*sqrt()) * (-2*a0.x)
        // derr/d(a0.y) = -sqrt( x*x + y*y ) = 1/(2*sqrt()) * (-2*a0.y)
        // derr/d(a1.x) = -sqrt( x*x + y*y ) = 1/(2*sqrt()) * (2*a1.x)
        // derr/d(a1.y) = -sqrt( x*x + y*y ) = 1/(2*sqrt()) * (2*a1.y)

        //d(b - a)^2 / db = 2(b-a)

        double s = sqrt( d_new.x * d_new.x + d_new.y * d_new.y );

        dx [ a0->LmGetIndex() ] =  1.0 * s * a0->GetPos().x; // derr / d(a0.x)
        dx [ a0->LmGetIndex() + 1 ] =  1.0 * s * a0->GetPos().y; // derr / d(a0.y)
        dx [ a1->LmGetIndex() ] =  -1.0 * s * a1->GetPos().y; // derr / d(a1.x)
        dx [ a1->LmGetIndex() + 1 ] =  -1.0 * s * a1->GetPos().y; // derr / d(a1.y)
    };

};

class GS_CONSTRAINT_ARC_FIXED_ANGLES : public GS_CONSTRAINT
{
public:
    GS_CONSTRAINT_ARC_FIXED_ANGLES( GS_ARC* aArc ) : GS_CONSTRAINT( aArc ) {
    };

    virtual int LmGetEquationCount()
    {
        return 4;
    }

    virtual void LmFunc( double *x )
    {
        auto a0 = m_parent->Anchor(0);
        auto a1 = m_parent->Anchor(1);
        auto a2 = m_parent->Anchor(2);
        
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

        printf("d0_cur %.1f %.1f d1_cur %.1f %.1f\n", d0_cur.x, d0_cur.y, d1_cur.x, d1_cur.y );
        printf("a0_orig %.1f a1 %.1f cur %.1f %.1f\n", a0_orig, a1_orig, a0_cur, a1_cur );

        //double l0x_cur = d0_cur.x * d0_cur.x + d0_cur.y * d0_cur.y;
        //double l1y_cur = d1_cur.x * d1_cur.x + d1_cur.y * d1_cur.y;

        x[0] = a0_orig - a0_cur;
        x[1] = a1_orig - a1_cur;
        x[2] = d0_orig.x - d0_cur.x - d1_orig.x + d1_cur.x;
        x[3] = d0_orig.y - d0_cur.y - d1_orig.y + d1_cur.y;

    }

    static double pow2( double x )
    {
        return x*x;
    }

  virtual void LmDFunc( double *dx, int equationIndex )
  {
        auto a0 = m_parent->Anchor(0);
        auto a1 = m_parent->Anchor(1);
        auto a2 = m_parent->Anchor(2);
        
        auto p0_cur = a0->GetPos();
        auto p1_cur = a1->GetPos();
        auto pc_cur = a2->GetPos();

      if( equationIndex == 0 )
      {
        // d(a0) / d(coordinate)
            dx[ a0->LmGetIndex() ] = -180.0*(-p0_cur.y + pc_cur.y)/(M_PI*(pow2(p0_cur.x - pc_cur.x) + pow2(p0_cur.y - pc_cur.y)));
            dx[ a0->LmGetIndex() + 1 ] = -180.0*(p0_cur.x - pc_cur.x)/(M_PI*(pow2(p0_cur.x - pc_cur.x) + pow2(p0_cur.y - pc_cur.y)));
            dx[ a1->LmGetIndex() ] = 0.0;
            dx[ a1->LmGetIndex() + 1 ] = 0.0;
            dx[ a2->LmGetIndex() ] = 180.0*(-p0_cur.y + pc_cur.y)/(M_PI*(pow2(p0_cur.x - pc_cur.x) + pow2(p0_cur.y - pc_cur.y)));
            dx[ a2->LmGetIndex() + 1] = 180.0*(p0_cur.x - pc_cur.x)/(M_PI*(pow2(p0_cur.x - pc_cur.x) + pow2(p0_cur.y - pc_cur.y)));
      } else if (equationIndex == 1) {
          // d(a1) / d(coordinate)
            dx [ a0->LmGetIndex() ] = 0.0;
            dx [ a0->LmGetIndex() + 1 ] = 0.0;
            dx [ a1->LmGetIndex() ] =-180.0*(-p1_cur.y + pc_cur.y)/(M_PI*(pow2(p1_cur.x - pc_cur.x) + pow2(p1_cur.y - pc_cur.y)));
            dx [ a1->LmGetIndex() + 1 ] = -180.0*(p1_cur.x - pc_cur.x)/(M_PI*(pow2(p1_cur.x - pc_cur.x) + pow2(p1_cur.y - pc_cur.y)));
            dx [ a2->LmGetIndex() ] = 180.0*(-p1_cur.y + pc_cur.y)/(M_PI*(pow2(p1_cur.x - pc_cur.x) + pow2(p1_cur.y - pc_cur.y)));
            dx [ a2->LmGetIndex() + 1 ] = 180.0*(p1_cur.x - pc_cur.x)/(M_PI*(pow2(p1_cur.x - pc_cur.x) + pow2(p1_cur.y - pc_cur.y)));

      } else if (equationIndex == 2) {
            dx [ a0->LmGetIndex() ] = -1.0;
            dx [ a0->LmGetIndex() + 1 ] = 0.0;
            dx [ a1->LmGetIndex() ] = 1.0;
            dx [ a1->LmGetIndex() + 1 ] = 0.0;
            dx [ a2->LmGetIndex() ] = 0.0;
            dx [ a2->LmGetIndex() + 1 ] = 0.0;
      } else {
            dx [ a0->LmGetIndex() ] = 0.0;
            dx [ a0->LmGetIndex() + 1 ] = -1.0;
            dx [ a1->LmGetIndex() ] = 0.0;
            dx [ a1->LmGetIndex() + 1 ] = 1.0;
            dx [ a2->LmGetIndex() ] = 0.0;
            dx [ a2->LmGetIndex() + 1 ] = 0.0;

      }
  }

};

#endif