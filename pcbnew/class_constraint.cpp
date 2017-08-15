#include <fctsys.h>
#include <confirm.h>
#include <gr_basic.h>
#include <class_drawpanel.h>
#include <wxPcbStruct.h>
#include <drawtxt.h>
#include <dialog_helpers.h>
#include <macros.h>
#include <base_units.h>
#include <board_commit.h>

#include <class_board.h>
#include <class_pcb_text.h>
#include <class_dimension.h>

#include <pcbnew.h>
#include <class_pcb_layer_box_selector.h>

#include <class_constraint.h>

CONSTRAINT_BASE::CONSTRAINT_BASE( BOARD_ITEM* aParent, KICAD_T aType ) :
    BOARD_ITEM ( aParent, aType ),
    m_text( this ),
    m_width( 10000 )
{

}

CONSTRAINT_BASE::~CONSTRAINT_BASE()
{

}

void CONSTRAINT_BASE::SetLayer( PCB_LAYER_ID aLayer )
{
    m_Layer = aLayer;
}

 CONSTRAINT_LINEAR::CONSTRAINT_LINEAR( BOARD_ITEM *aParent )
 : CONSTRAINT_BASE( aParent, PCB_CONSTRAINT_LINEAR_T )
 {

 }

 CONSTRAINT_LINEAR::CONSTRAINT_LINEAR( const CONSTRAINT_LINEAR& aOther )
 : CONSTRAINT_BASE( aOther.GetParent(), PCB_CONSTRAINT_LINEAR_T )
 {
     m_Layer = aOther.m_Layer;
     m_delta = aOther.m_delta;
     m_p0 =aOther.m_p0;
     m_p1 = aOther.m_p1;
     m_length=aOther.m_length;
     m_measureOrigin = aOther.m_measureOrigin;
     m_angle = aOther.m_angle;
     m_freeAngle = aOther.m_freeAngle;
 }

 CONSTRAINT_LINEAR::~CONSTRAINT_LINEAR()
 {

 }

void CONSTRAINT_LINEAR::Draw( EDA_DRAW_PANEL* panel, wxDC* DC,
        GR_DRAWMODE aColorMode, const wxPoint& offset )
{
}

const std::vector<SHAPE_SEGMENT> CONSTRAINT_LINEAR::BuildShape( ) const
{
    std::vector<SHAPE_SEGMENT> rv;

    const int arrowSize = 1000000;

    int dist = 2000000;

    auto dir = VECTOR2D(2000000 , 0).Rotate(m_angle * M_PI / 180.0);



    SEG m0 ( m_p0, SEG(m_p0, m_p0 + dir).LineProject ( m_measureOrigin ) );
    SEG m1 ( m_p1, SEG(m_p1, m_p1 + dir).LineProject ( m_measureOrigin ) );


    auto m0e = m0.B + (m0.B - m0.A).Resize( arrowSize );
    auto m1e = m1.B + (m1.B - m1.A).Resize( arrowSize );

    rv.reserve( 10 );

    rv.push_back( SHAPE_SEGMENT ( m0.B, m1.B, m_width ) );
    rv.push_back( SHAPE_SEGMENT ( m0.A, m0.B, m_width ) );
    rv.push_back( SHAPE_SEGMENT ( m1.A, m1.B, m_width ) );
    rv.push_back( SHAPE_SEGMENT ( m0.B, m0e, m_width ) );
    rv.push_back( SHAPE_SEGMENT ( m1.B, m1e, m_width ) );

    auto da0 = dir.Rotate((90.0-30.0) * M_PI / 180.0).Resize( arrowSize );
    auto da1 = dir.Rotate((90.0+30.0) * M_PI / 180.0).Resize( arrowSize );

    SEG base (  m0.B, m1.B  );

    auto p = base.LineProject( m0.B + da0 );

    if ( !base.Contains ( p  ) )
    {
        da0 = -da0;
        da1 = -da1;
    }

    rv.push_back( SHAPE_SEGMENT ( m0.B + da0, m0.B, m_width ) );
    rv.push_back( SHAPE_SEGMENT ( m0.B + da1, m0.B, m_width ) );
    rv.push_back( SHAPE_SEGMENT ( m1.B - da0, m1.B, m_width ) );
    rv.push_back( SHAPE_SEGMENT ( m1.B - da1, m1.B, m_width ) );

    return rv;
};


void CONSTRAINT_LINEAR::Move( const wxPoint& offset )
{
    m_p0 += VECTOR2I( offset );
    m_p1 += VECTOR2I( offset );
    m_measureOrigin += VECTOR2I( offset );
}

void CONSTRAINT_LINEAR::Rotate( const wxPoint& aRotCentre, double aAngle )
{
}


void CONSTRAINT_LINEAR::Flip( const wxPoint& aCentre )
{
}


void CONSTRAINT_LINEAR::Mirror( const wxPoint& axis_pos )
{
}


void CONSTRAINT_LINEAR::GetMsgPanelInfo( std::vector<MSG_PANEL_ITEM>& aList )
{
}


bool CONSTRAINT_LINEAR::HitTest( const wxPoint& aPosition ) const
{
    printf("CL HitTest %d %d\n", aPosition.x, aPosition.y );

    for ( const auto s : BuildShape() )
        if ( s.Collide( VECTOR2I( aPosition ) ) )
        {
            printf("OK!\n");
            return true;
        }

    return false;
}


bool CONSTRAINT_LINEAR::HitTest( const EDA_RECT& aRect, bool aContained,
        int aAccuracy ) const
{
    return aRect.Contains( GetBoundingBox() );
}


// Virtual function
const EDA_RECT CONSTRAINT_LINEAR::GetBoundingBox() const
{
    int         xmin = std::numeric_limits<int>::max();
    int         xmax = std::numeric_limits<int>::min();
    int         ymin = std::numeric_limits<int>::max();
    int         ymax = std::numeric_limits<int>::min();

    EDA_RECT    bBox;

    //, xmax, ymin, ymax;

    for ( const auto s : BuildShape() )
    {
        xmin = std::min(xmin, s.GetSeg().A.x );
        xmin = std::min(xmin, s.GetSeg().B.x );
        ymin = std::min(ymin, s.GetSeg().A.y );
        ymin = std::min(ymin, s.GetSeg().B.y );
        xmax = std::max(xmax, s.GetSeg().A.x );
        xmax = std::max(xmax, s.GetSeg().B.x );
        ymax = std::max(ymax, s.GetSeg().A.y );
        ymax = std::max(ymax, s.GetSeg().B.y );
    }

    xmin -= m_width / 2;
    ymin -= m_width / 2;
    xmax += m_width / 2;
    ymax += m_width / 2;

    bBox.SetX( xmin );
    bBox.SetY( ymin );
    bBox.SetWidth( xmax - xmin + 1 );
    bBox.SetHeight( ymax - ymin + 1 );

    bBox.Normalize();

    printf("cbb %d %d %d %d\n", xmin, ymin, xmax, ymax );

    return bBox;
}


wxString CONSTRAINT_LINEAR::GetSelectMenuText() const
{
    return wxString("Linear constraint");
}


BITMAP_DEF CONSTRAINT_LINEAR::GetMenuImage() const
{
}


EDA_ITEM* CONSTRAINT_LINEAR::Clone() const
{
    return new CONSTRAINT_LINEAR( *this );
}


const BOX2I CONSTRAINT_LINEAR::ViewBBox() const
{
    BOX2I dimBBox = BOX2I( VECTOR2I( GetBoundingBox().GetPosition() ),
                           VECTOR2I( GetBoundingBox().GetSize() ) );

    return dimBBox;
}

const wxPoint&  CONSTRAINT_LINEAR::GetPosition() const
{
    return (wxPoint) m_p0;
}

void            CONSTRAINT_LINEAR::SetPosition( const wxPoint& aPos )
{

}

void CONSTRAINT_LINEAR::updateAngle()
{
    if( m_freeAngle )
    {
        VECTOR2I d = m_p1 - m_p0;

        m_angle = RAD2DEG( atan2 (d.y, d.x) ) + 90.0;
        printf("angle %.1f\n", m_angle );
        m_length = d.EuclideanNorm();
    }
}
