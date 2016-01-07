/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 1992-2011 KiCad Developers, see AUTHORS.txt for contributors.
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

/**
 * @file class_dimension.h
 * @brief DIMENSION class definition.
 */

#ifndef DIMENSION_H_
#define DIMENSION_H_


#include <class_board_item.h>
#include <class_pcb_text.h>


class LINE_READER;
class EDA_DRAW_PANEL;
class TEXTE_PCB;
class MSG_PANEL_ITEM;


/**
 * Class DIMENSION
 *
 * For better understanding of the points that make a dimension:
 *
 *            m_featureLineGO  m_featureLineDO
 *            |                              |
 *            |                              |
 *            |                              |
 *            |  m_arrowG2F      m_arrowD2F  |
 *            | /                          \ |
 * m_crossBarO|/____________________________\|m_crossBarF
 *            |\           m_Text           /|
 *            | \                          / |
 *            |  m_arrowG1F      m_arrowD1F  |
 *            |                              |
 *            m_featureLineGF  m_featureLineDF
 */
class DIMENSION : public BOARD_ITEM
{
    int         m_Width;        ///< Line width
    int         m_Shape;        ///< Currently always 0.
    EDA_UNITS_T m_Unit;         ///< 0 = inches, 1 = mm
    int         m_Value;        ///< value of PCB dimensions.
    int         m_Height;       ///< length of feature lines
    TEXTE_PCB   m_Text;

public:
// TODO private: These member should be private. they are public only due to legacy code
    wxPoint     m_crossBarO, m_crossBarF;
    wxPoint     m_featureLineGO, m_featureLineGF;
    wxPoint     m_featureLineDO, m_featureLineDF;
    wxPoint     m_arrowD1F, m_arrowD2F;
    wxPoint     m_arrowG1F, m_arrowG2F;

    DIMENSION( BOARD_ITEM* aParent );

    // Do not create a copy constructor.  The one generated by the compiler is adequate.

    ~DIMENSION();

    void SetValue( int aValue ) { m_Value = aValue; }

    int GetValue() const { return m_Value; }

    const wxPoint&  GetPosition() const;

    void            SetPosition( const wxPoint& aPos ); // override, sets m_Text's position too

    void SetTextSize( const wxSize& aTextSize )
    {
        m_Text.SetSize( aTextSize );
    }

    void SetLayer( LAYER_ID aLayer );

    void SetShape( int aShape )         { m_Shape = aShape; }
    int GetShape() const { return m_Shape; }

    int GetWidth() const { return m_Width; }
    void SetWidth( int aWidth )         { m_Width = aWidth; }

    /**
     * Function SetOrigin
     * Sets a new origin of the crossbar line. All remaining lines are adjusted after that.
     * @param aOrigin is the new point to be used as the new origin of the crossbar line.
     */
    void SetOrigin( const wxPoint& aOrigin );

    /**
     * Function GetOrigin
     * @return Origin of the crossbar line.
     */
    const wxPoint& GetOrigin() const
    {
        return m_featureLineGO;
    }

    /**
     * Function SetEnd
     * Sets a new end of the crossbar line. All remaining lines are adjusted after that.
     * @param aEnd is the new point to be used as the new end of the crossbar line.
     */
    void SetEnd( const wxPoint& aEnd );

    /**
     * Function GetEnd
     * @return End of the crossbar line.
     */
    const wxPoint& GetEnd()
    {
        return m_featureLineDO;
    }

    /**
     * Function SetHeight
     * Sets the length of feature lines.
     * @param aHeight is the new height.
     */
    void SetHeight( int aHeight );

    /**
     * Function GetHeight
     * Returns the length of feature lines.
     */
    int GetHeight() const
    {
        return m_Height;
    }

    /**
     * Function UpdateHeight
     * Updates stored height basing on points coordinates.
     */
    void UpdateHeight();

    /**
     * Function GetAngle
     * Returns angle of the crossbar.
     * @return Angle of the crossbar line expressed in radians.
     */
    double GetAngle() const
    {
        wxPoint delta( m_featureLineDO - m_featureLineGO );

        return atan2( delta.y, delta.x );
    }

    /**
     * Function AdjustDimensionDetails
     * Calculate coordinates of segments used to draw the dimension.
     * @param aDoNotChangeText (bool) if false, the dimension text is initialized
     */
    void            AdjustDimensionDetails( bool aDoNotChangeText = false );

    void            SetText( const wxString& NewText );
    const wxString  GetText() const;

    TEXTE_PCB&      Text()  { return m_Text; }
    TEXTE_PCB&      Text() const  { return *(const_cast<TEXTE_PCB*> (&m_Text)); }

    void            Copy( DIMENSION* source );

    void            Draw( EDA_DRAW_PANEL* panel, wxDC* DC,
                          GR_DRAWMODE aColorMode, const wxPoint& offset = ZeroOffset );

    /**
     * Function Move
     * @param offset : moving vector
     */
    void            Move( const wxPoint& offset );

    void            Rotate( const wxPoint& aRotCentre, double aAngle );

    void            Flip( const wxPoint& aCentre );

    /**
     * Function Mirror
     * Mirror the Dimension , relative to a given horizontal axis
     * the text is not mirrored. only its position (and angle) is mirrored
     * the layer is not changed
     * @param axis_pos : vertical axis position
     */
    void            Mirror( const wxPoint& axis_pos );

    void            GetMsgPanelInfo( std::vector< MSG_PANEL_ITEM >& aList );

    bool            HitTest( const wxPoint& aPosition ) const;

    /** @copydoc BOARD_ITEM::HitTest(const EDA_RECT& aRect,
     *                               bool aContained = true, int aAccuracy ) const
     */
    bool HitTest( const EDA_RECT& aRect, bool aContained = true, int aAccuracy = 0 ) const;

    wxString GetClass() const
    {
        return wxT( "DIMENSION" );
    }

    // Virtual function
    const EDA_RECT    GetBoundingBox() const;

    wxString    GetSelectMenuText() const;

    BITMAP_DEF GetMenuImage() const { return add_dimension_xpm; }

    EDA_ITEM*   Clone() const;

    /// @copydoc VIEW_ITEM::ngViewBBox()
    virtual const BOX2I ngViewBBox() const;

#if defined(DEBUG)
    virtual void Show( int nestLevel, std::ostream& os ) const { ShowDummy( os ); }    // override
#endif
};

#endif    // DIMENSION_H_
