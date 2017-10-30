/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 1992-2017 KiCad Developers, see AUTHORS.txt for contributors.
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

#ifndef __CLASS_CONSTRAINT_H
#define __CLASS_CONSTRAINT_H


#include <class_board_item.h>
#include <class_pcb_text.h>

#include <geometry/shape_segment.h>

class LINE_READER;
class EDA_DRAW_PANEL;
class TEXTE_PCB;
class MSG_PANEL_ITEM;


class CONSTRAINT_BASE : public BOARD_ITEM
{
protected:
    int         m_width;        ///< Line width
    EDA_UNITS_T m_unit;         ///< 0 = inches, 1 = mm
    TEXTE_PCB   m_text;
    bool m_withDimension;
public:

    CONSTRAINT_BASE( BOARD_ITEM* aParent, KICAD_T aType );
    virtual ~CONSTRAINT_BASE();

    void SetTextSize( const wxSize& aTextSize )
    {
        m_text.SetTextSize( aTextSize );
    }

    void SetLayer( PCB_LAYER_ID aLayer ) override;


    int GetWidth() const                { return m_width; }
    void SetWidth( int aWidth )         { m_width = aWidth; }



    void            SetText( const wxString& NewText );
    const wxString  GetText() const;

    TEXTE_PCB&      Text()  { return m_text; }
    TEXTE_PCB&      Text() const  { return *(const_cast<TEXTE_PCB*> (&m_text)); }

    void            Draw( EDA_DRAW_PANEL* panel, wxDC* DC,
                          GR_DRAWMODE aColorMode, const wxPoint& offset = ZeroOffset ) override {};

};

class CONSTRAINT_LINEAR : public CONSTRAINT_BASE
{
    private:
        VECTOR2I m_p0, m_p1;
        VECTOR2I m_measureOrigin;
        int m_distance;
        double m_angle;
        bool m_freeAngle;

        void updateAngle();

    public:

        CONSTRAINT_LINEAR( BOARD_ITEM *aParent );
        CONSTRAINT_LINEAR( const CONSTRAINT_LINEAR& aOther );
        ~CONSTRAINT_LINEAR();

        void SetMeasureLineOrigin( const VECTOR2I& aP ) { m_measureOrigin = aP; }
        const VECTOR2I GetMeasureLineOrigin() { return m_measureOrigin; }

        const std::vector<SHAPE_SEGMENT> BuildShape( ) const;
        const VECTOR2I& GetP0() const { return m_p0; }
        const VECTOR2I& GetP1() const { return m_p1; }
        const VECTOR2I GetDisplacementVector() const;
        int GetDistance() const { return m_distance; }
        double GetAngle() const { return m_angle; }

        void SetP0( const VECTOR2I& aP ) { m_p0 = aP; updateAngle(); }
        void SetP1( const VECTOR2I& aP ) { m_p1 = aP; updateAngle(); }
        void SetDistance( int  aLength ) { m_distance = aLength; }

        void SetAngle( bool aFree, double aAngle = 0.0)
        {
            m_angle = aAngle;
            m_freeAngle = aFree;
            updateAngle();
        }

        bool IsFreeAngle() const
        {
             return m_freeAngle;
        }


        const SEG GetSeg() const
        {
            return SEG( m_p0, m_p1 );
        }



        void            Draw( EDA_DRAW_PANEL* panel, wxDC* DC,
                              GR_DRAWMODE aColorMode, const wxPoint& offset = ZeroOffset ) override;

                              const wxPoint&  GetPosition() const override;

                              void            SetPosition( const wxPoint& aPos ) override;

        /**
         * Function Move
         * @param offset : moving vector
         */
        void            Move( const wxPoint& offset ) override;

        void            Rotate( const wxPoint& aRotCentre, double aAngle ) override;

        void            Flip( const wxPoint& aCentre ) override;

        /**
         * Function Mirror
         * Mirror the Dimension , relative to a given horizontal axis
         * the text is not mirrored. only its position (and angle) is mirrored
         * the layer is not changed
         * @param axis_pos : vertical axis position
         */
        void            Mirror( const wxPoint& axis_pos );

        void            GetMsgPanelInfo( std::vector< MSG_PANEL_ITEM >& aList ) override;

        bool            HitTest( const wxPoint& aPosition ) const override;

        bool HitTest( const EDA_RECT& aRect, bool aContained = true, int aAccuracy = 0 ) const override;

        wxString GetClass() const override
        {
            return wxT( "CONSTRAINT_LINEAR" );
        }

        // Virtual function
        const EDA_RECT    GetBoundingBox() const override;

        wxString    GetSelectMenuText() const override;

        BITMAP_DEF GetMenuImage() const override;

        EDA_ITEM*   Clone() const override;

        virtual const BOX2I ViewBBox() const override;

    #if defined(DEBUG)
        virtual void Show( int nestLevel, std::ostream& os ) const override { ShowDummy( os ); }
    #endif
};


#endif    // DIMENSION_H_
