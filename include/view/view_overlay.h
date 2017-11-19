#ifndef __VIEW_OVERLAY_H
#define __VIEW_OVERLAY_H

#include <view/view.h>
#include <view/view_item.h>
#include <gal/graphics_abstraction_layer.h>

#include <vector>
#include <deque>

class SEG;

namespace KIGFX
{

class VIEW_OVERLAY : public VIEW_ITEM
{
public:
    VIEW_OVERLAY();
    virtual ~VIEW_OVERLAY();

    struct COMMAND;
    struct COMMAND_ARC;
    struct COMMAND_LINE;
    struct COMMAND_CIRCLE;
    struct COMMAND_SET_STROKE;
    struct COMMAND_SET_FILL;
    struct COMMAND_SET_COLOR;
    struct COMMAND_SET_WIDTH;

    void Clear();

    virtual const BOX2I ViewBBox() const override;
    virtual void ViewDraw( int aLayer, VIEW *aView ) const override;
    virtual void ViewGetLayers( int aLayers[], int& aCount ) const override;

    void Line( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint );
    void Line( const SEG& aSeg );
    void Segment( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint, double aWidth );
    void Polyline( std::deque<VECTOR2D>& aPointList );
    void Circle( const VECTOR2D& aCenterPoint, double aRadius );
    void Arc( const VECTOR2D& aCenterPoint, double aRadius, double aStartAngle, double aEndAngle );
    void Rectangle( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint );
    void Polygon( const std::deque<VECTOR2D>& aPointList );
    void SetIsFill( bool aIsFillEnabled );
    void SetIsStroke( bool aIsStrokeEnabled );
    void SetFillColor( const COLOR4D& aColor );
    void SetStrokeColor( const COLOR4D& aColor );

    void SetLineWidth( double aLineWidth );

private:
    void releaseCommands();

    std::vector<COMMAND*> m_commands;
};
} // namespace KIGFX


#endif
