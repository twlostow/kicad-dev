#ifndef __VIEW_OVERLAY_H
#define __VIEW_OVERLAY_H

#include <view/view.h>
#include <view/view_item.h>
#include <gal/graphics_abstraction_layer.h>


namespace KIGFX
{

class VIEW_OVERLAY : public VIEW_ITEM, public GAL_API_BASE
{
public:
    VIEW_OVERLAY();
    ~VIEW_OVERLAY();


    void Clear();
    void Begin();

    /**
     * Function ViewBBox()
     * Returns the bounding box for all stored items covering all its layers.
     *
     * @return The current bounding box
     */
    virtual const BOX2I ViewBBox() const;

    /**
     * Function ViewDraw()
     * Draws all the stored items in the group on the given layer.
     *
     * @param aLayer is the layer which should be drawn.
     * @param aGal is the GAL that should be used for drawing.
     */
    virtual void ViewDraw( int aLayer, GAL* aGal ) const;

    /**
     * Function ViewGetLayers()
     * Returns all the layers used by the stored items.
     *
     * @param aLayers[] is the output layer index array.
     * @param aCount is the number of layer indices in aLayers[].
     */
    virtual void ViewGetLayers( int aLayers[], int& aCount ) const;

    void End();

    void DrawLine( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint )
    {
        updateGroups();
        m_gal->DrawLine( aStartPoint, aEndPoint );
    }

    void DrawSegment( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint, double aWidth )
    {
        updateGroups();
        m_gal->DrawSegment( aStartPoint, aEndPoint, aWidth );
    }

    void DrawPolyline( std::deque<VECTOR2D>& aPointList )
    {
        updateGroups();
        m_gal->DrawPolyline( aPointList );
    }

    void DrawCircle( const VECTOR2D& aCenterPoint, double aRadius )
    {
        updateGroups();
        m_gal->DrawCircle( aCenterPoint, aRadius );
    }

    void DrawArc( const VECTOR2D& aCenterPoint, double aRadius, double aStartAngle, double aEndAngle )
    {
        updateGroups();
        m_gal->DrawArc( aCenterPoint, aRadius, aStartAngle, aEndAngle );
    }

    void DrawRectangle( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint )
    {
        updateGroups();
        m_gal->DrawRectangle( aStartPoint, aEndPoint );
    }

    void DrawPolygon( const std::deque<VECTOR2D>& aPointList )
    {
        updateGroups();
        m_gal->DrawPolygon( aPointList );
    }

    void DrawCurve( const VECTOR2D& startPoint,    const VECTOR2D& controlPointA,
                    const VECTOR2D& controlPointB, const VECTOR2D& endPoint )
    {
        updateGroups();
        m_gal->DrawCurve( startPoint, controlPointA, controlPointB, endPoint );
    }

    void Rotate( double aAngle )
    {
        m_gal->Rotate( aAngle );
    }

    void Translate( const VECTOR2D& aTranslation )
    {
        m_gal->Translate( aTranslation );
    }

    void Scale( const VECTOR2D& aScale )
    {
        m_gal->Scale( aScale );
    }

    void Transform( const MATRIX3x3D& aTransformation )
    {
        m_gal->Transform ( aTransformation );
    }

	void Save()
	{

	}

	void Restore()
	{

	}

	void TestLine ( double x1, double y1, double x2, double y2 )
	{
		m_gal->TestLine (x1, y1, x2, y2);
	}

void TestLine2 ( double x1, double y1, double x2, double y2 )
	{
		m_gal->TestLine2 (x1, y1, x2, y2);
	}

private:
	void viewAssign( VIEW* aView );
    void updateGroups();

	GAL *m_gal;

    std::vector<int> m_requestedGroups;
    int m_currentGroup;
};

} // namespace KIGFX

#endif // VIEW_GROUP_H_

