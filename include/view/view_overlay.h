#ifndef __VIEW_OVERLAY_H
#define __VIEW_OVERLAY_H

#include <view/view.h>
#include <view/view_item.h>
#include <gal/graphics_abstraction_layer.h>


namespace KIGFX
{

struct VIEW_OVERLAY {};

#if 0
class VIEW_OVERLAY : public VIEW_ITEM, public GAL_API_BASE
{
public:
    VIEW_OVERLAY( VIEW_BASE *aView );
    ~VIEW_OVERLAY();


    void Clear();
    void Begin();

    /**
     * Function ViewBBox()
     * Returns the bounding box for all stored items covering all its layers.
     *
     * @return The current bounding box
     */
    virtual const BOX2I ngViewBBox() const;

    /**
     * Function ViewDraw()
     * Draws all the stored items in the group on the given layer.
     *
     * @param aLayer is the layer which should be drawn.
     * @param aGal is the GAL that should be used for drawing.
     */
    virtual void ngViewDraw( int aLayer, VIEW_BASE *aView ) const;

    void End();

    virtual void DrawLine( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint )
    {
        updateGroups();
        m_gal->DrawLine( aStartPoint, aEndPoint );
    }

    virtual void DrawSegment( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint, double aWidth )
    {
        updateGroups();
        m_gal->DrawSegment( aStartPoint, aEndPoint, aWidth );
    }

    virtual void DrawPolyline( std::deque<VECTOR2D>& aPointList )
    {
        updateGroups();
        m_gal->DrawPolyline( aPointList );
    }

    virtual void DrawCircle( const VECTOR2D& aCenterPoint, double aRadius )
    {
        updateGroups();
        //m_gal->DrawPolyline( aCount, aRadius );
    }

    virtual void DrawArc( const VECTOR2D& aCenterPoint, double aRadius, double aStartAngle, double aEndAngle )
    {
        updateGroups();
//        m_gal->DrawArc( aCenterPoint, aRadius, aStartAngle, aEndAngle );
    }

    virtual void DrawRectangle( const VECTOR2D& aStartPoint, const VECTOR2D& aEndPoint )
    {
        updateGroups();
        m_gal->DrawRectangle( aStartPoint, aEndPoint );
    }

    virtual void DrawPolygon( const std::deque<VECTOR2D>& aPointList )
    {
        updateGroups();
//        m_gal->DrawPolygon( aPointList );
    }

    virtual void DrawCurve( const VECTOR2D& startPoint,    const VECTOR2D& controlPointA,
                    const VECTOR2D& controlPointB, const VECTOR2D& endPoint )
    {
        updateGroups();
//        m_gal->DrawCurve( aStartPoint, controlPointA, controlPointB, endPoint );
    }

    /**
     * @brief Enable/disable fill.
     *
     * @param aIsFillEnabled is true, when the graphics objects should be filled, else false.
     */
    virtual void SetIsFill( bool aIsFillEnabled )
    {
        m_gal->SetIsFill ( aIsFillEnabled );
    }

    /**
     * @brief Enable/disable stroked outlines.
     *
     * @param aIsStrokeEnabled is true, if the outline of an object should be stroked.
     */
    virtual void SetIsStroke( bool aIsStrokeEnabled )
    {
        m_gal->SetIsStroke ( aIsStrokeEnabled );
    }

    /**
     * @brief Set the fill color.
     *
     * @param aColor is the color for filling.
     */
    virtual void SetFillColor( const COLOR4D& aColor )
    {
        m_gal->SetFillColor ( aColor );
    }

    /**
     * @brief Set the stroke color.
     *
     * @param aColor is the color for stroking the outline.
     */
    virtual void SetStrokeColor( const COLOR4D& aColor )
    {
        m_gal->SetStrokeColor ( aColor );
    }

    /**
     * @brief Get the stroke color.
     *
     * @return the color for stroking the outline.
     */
    virtual const COLOR4D& GetStrokeColor() const
    {
        return m_gal->GetStrokeColor();
    }

    /**
     * @brief Set the line width.
     *
     * @param aLineWidth is the line width.
     */
    virtual void SetLineWidth( double aLineWidth )
    {
        m_gal->SetLineWidth( aLineWidth );
    }

    /**
     * @brief Get the line width.
     *
     * @return the actual line width.
     */
    virtual double GetLineWidth() const
    {
        return m_gal->GetLineWidth();
    }

    // ----
    // Text
    // ----
    /**
     * @brief Draws a vector type text using preloaded Newstroke font.
     *
     * @param aText is the text to be drawn.
     * @param aPosition is the text position in world coordinates.
     * @param aRotationAngle is the text rotation angle.
     */
    virtual void StrokeText( const wxString& aText, const VECTOR2D& aPosition,
            double aRotationAngle )
    {
        m_gal->StrokeText(aText, aPosition, aRotationAngle );
    }

    /**
     * @brief Loads attributes of the given text (bold/italic/underline/mirrored and so on).
     *
     * @param aText is the text item.
     */
    virtual void SetTextAttributes( const EDA_TEXT* aText )
    {
        m_gal->SetTextAttributes(aText);
    }

    /// @copydoc STROKE_FONT::SetGlyphSize()
    virtual void SetGlyphSize( const VECTOR2D aGlyphSize )
    {
        m_gal->SetGlyphSize(aGlyphSize);
    }

    /// @copydoc STROKE_FONT::SetBold()
    virtual void SetBold( const bool aBold )
    {
        m_gal->SetBold(aBold);
    }

    /// @copydoc STROKE_FONT::SetItalic()
    virtual void SetItalic( const bool aItalic )
    {
        m_gal->SetItalic(aItalic);
    }

    /// @copydoc STROKE_FONT::SetMirrored()
    virtual void SetMirrored( const bool aMirrored )
    {
        m_gal->SetMirrored(aMirrored);
    }

    /// @copydoc STROKE_FONT::SetHorizontalJustify()
    virtual void SetHorizontalJustify( const EDA_TEXT_HJUSTIFY_T aHorizontalJustify )
    {
        m_gal->SetHorizontalJustify(aHorizontalJustify);
    }

    /// @copydoc STROKE_FONT::SetVerticalJustify
    virtual void SetVerticalJustify( const EDA_TEXT_VJUSTIFY_T aVerticalJustify )
    {
        m_gal->SetVerticalJustify(aVerticalJustify);
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

    void DrawSingleItem ( const EDA_ITEM *aItem );

    virtual void ngViewGetLayers( int aLayers[], int& aCount ) const;

private:

	void updateGroups();

	VIEW_BASE *m_view;
    GAL *m_gal;

    std::vector<int> m_requestedGroups;
    int m_currentGroup;
};
#endif
} // namespace KIGFX


#endif
