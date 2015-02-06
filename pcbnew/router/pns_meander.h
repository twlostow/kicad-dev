/*
 * KiRouter - a push-and-(sometimes-)shove PCB router
 *
 * Copyright (C) 2013-2015 CERN
 * Author: Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __PNS_MEANDER_H
#define __PNS_MEANDER_H

#include <math/vector2d.h>

#include <geometry/shape.h>
#include <geometry/shape_line_chain.h>

#include "pns_node.h"
#include "pns_via.h"
#include "pns_line.h"
#include "pns_placement_algo.h"

class PNS_ROUTER;
class PNS_SHOVE;
class PNS_OPTIMIZER;
class PNS_ROUTER_BASE;
class PNS_MEANDER_PLACER_BASE;

///< Shapes of available meanders
enum PNS_MEANDER_TYPE {
        MT_SINGLE, // _|^|_, single-sided
        MT_START,  // _|^|
        MT_FINISH, // |^|_
        MT_TURN,   // |^| or |_|
        MT_CHECK_START, 
        MT_CHECK_FINISH,
        MT_CORNER, // line corner
        MT_EMPTY
};

/**
 * Class PNS_MEANDER_SETTINGS
 *
 * Holds dimensions for the meandering algorithm
 */
class PNS_MEANDER_SETTINGS
{
public:

    enum CornerType {
        ROUND = 1,
        CHAMFER
    };

    PNS_MEANDER_SETTINGS ()
    {
        m_minAmplitude = 500000;
        m_maxAmplitude = 1000000;
        m_step = 50000;
        m_spacing = 500000;
        m_targetLength = 85000000;
        m_cornerType = ROUND;
        m_cornerRadiusPercentage = 100;
    }
    
    int m_minAmplitude;
    int m_maxAmplitude;
            
    int m_spacing;
    int m_step;

    int m_targetLength;
    
    CornerType m_cornerType;
    int m_cornerRadiusPercentage;
};

class PNS_MEANDERED_LINE;

class PNS_MEANDER_SHAPE
{
    public:
        
        PNS_MEANDER_SHAPE ( PNS_MEANDER_PLACER_BASE *aPlacer, int aWidth, bool aIsDual = false) : 
            m_placer ( aPlacer ),
            m_dual ( aIsDual ),
            m_width ( aWidth ),
            m_baselineOffset ( 0 )
            
        {};

         void SetBaseIndex ( int aIndex )
        {
            m_baseIndex = aIndex;
        }

        int BaseIndex () const
        {
            return m_baseIndex;
        }

        void SetType ( PNS_MEANDER_TYPE aType ) { 
                m_type = aType; 
        }
        
        PNS_MEANDER_TYPE Type() const { 
            return m_type; 
        }

        int Amplitude() const {
            return m_amplitude; 
        }
        
        void MakeCorner ( VECTOR2I aP1, VECTOR2I aP2 = VECTOR2I (0, 0) );

        void Resize ( int aAmpl );
        void Recalculate ( );
        
        bool IsDual() const { return m_dual; }

        bool Side() const { return m_side; }

        VECTOR2I End() const
        {
            return m_clippedBaseSeg.B;
        }

        const SHAPE_LINE_CHAIN& CLine( int aShape ) const 
        {
            return m_shapes[aShape];
        }

        int CornerRadius ( ) const;
        void MakeEmpty();

        bool Fit ( PNS_MEANDER_TYPE aType, const SEG& aSeg, const VECTOR2I& aP, bool aSide );

        const SEG& BaseSegment() const {
            return m_clippedBaseSeg;
        }



        int BaselineLength() const;
        int MaxTunableLength() const;


        const PNS_MEANDER_SETTINGS& Settings() const;

        int Width() const
        {
            return m_width;
        }


        void SetBaselineOffset ( int aOffset )
        {
            m_baselineOffset = aOffset;
        }

    private:

        SHAPE_LINE_CHAIN circleQuad ( VECTOR2D aP, VECTOR2D aDir, bool side );
        VECTOR2I reflect ( VECTOR2I p, const SEG& line );
        
        SHAPE_LINE_CHAIN genUShape ( VECTOR2D aP, VECTOR2D aDir, int aAmpl, int aSpan, int aCornerRadius );
        SHAPE_LINE_CHAIN genMeanderShape ( VECTOR2D aP, VECTOR2D aDir, bool aSide, PNS_MEANDER_TYPE aType, int aAmpl, int aBaselineOffset = 0);
        
        void updateBaseSegment();

        PNS_MEANDER_TYPE m_type;
        PNS_MEANDER_PLACER_BASE *m_placer;
        bool m_dual;
        int m_width;
        int m_amplitude;
        int m_length;
        int m_index;
        int m_spacing;
        int m_baselineOffset;
        VECTOR2I m_p0;
        SEG m_baseSeg;
        SEG m_clippedBaseSeg;
        bool m_side;
        SHAPE_LINE_CHAIN m_shapes[2];
        int m_baseIndex;



};

class PNS_MEANDERED_LINE {
    public:
        PNS_MEANDERED_LINE () {};

        PNS_MEANDERED_LINE ( PNS_MEANDER_PLACER_BASE *aPlacer, bool aIsDual = false ) :
            m_placer ( aPlacer ),
            m_dual ( aIsDual ) {};
        
        void AddCorner ( const VECTOR2I& aA, const VECTOR2I& aB = VECTOR2I(0, 0) );
        void AddMeander ( PNS_MEANDER_SHAPE *aShape );
    
        void Clear();

        void SetWidth ( int aWidth )
        {
            m_width= aWidth;
        }
    
        
        void MeanderSegment ( const SEG& aSeg, int aBaseIndex = 0 );

        void SetBaselineOffset ( int aOffset )
        {
            m_baselineOffset = aOffset;
        }

        std::vector<PNS_MEANDER_SHAPE*> & Meanders() 
        {
            return m_meanders;
        }

        bool CheckSelfIntersections ( PNS_MEANDER_SHAPE *aShape, int aClearance );
        
        int TunableLength() const;

    const PNS_MEANDER_SETTINGS& Settings() const;

    private:
            
        VECTOR2I m_last;
        
        PNS_MEANDER_PLACER_BASE *m_placer;
        std::vector<PNS_MEANDER_SHAPE *> m_meanders;
          
        bool m_dual;
        int m_width, m_baselineOffset;
};



#endif    // __PNS_MEANDER_H
