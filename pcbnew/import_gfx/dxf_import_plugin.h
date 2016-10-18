/*
 * This file comes from the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2011 Rallaz, rallazz@gmail.com
 * Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
 *
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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

#ifndef DXF_IMPORT_PLUGIN_H
#define DXF_IMPORT_PLUGIN_H

#include "graphics_import_plugin.h"
#include "drw_interface.h"

class wxRealPoint;

class DXF_IMPORT_PLUGIN : public GRAPHICS_IMPORT_PLUGIN, public DRW_Interface
{
public:
    DXF_IMPORT_PLUGIN();

    const wxString GetName() const override
    {
        return "AutoCAD DXF";
    }

    const wxArrayString GetFileExtensions() const override
    {
        return wxArrayString( 1, { "dxf" } );
    }

    bool Load( const wxString& aFileName ) override;

private:
    // coordinate conversions from dxf to internal units
    int mapX( double aDxfCoordX );
    int mapY( double aDxfCoordY );
    int mapDim( double aDxfValue );

    // Functions to aid in the creation of a LWPolyline
    void insertLine( const wxRealPoint& aSegStart, const wxRealPoint& aSegEnd );
    void insertArc( const wxRealPoint& aSegStart, const wxRealPoint& aSegEnd, double aBulge );

    // Methods from DRW_CreationInterface:
    // They are "call back" fonctions, called when the corresponding object
    // is read in dxf file
    void addHeader( const DRW_Header* aData ) override;
    void addLine( const DRW_Line& aData) override;
    void addCircle( const DRW_Circle& aData ) override;
    void addArc( const DRW_Arc& aData ) override;
    void addLWPolyline( const DRW_LWPolyline& aData ) override;
    void addPolyline( const DRW_Polyline& aData ) override;
    void addText( const DRW_Text& aData ) override;
    void addMText( const DRW_MText& aData) override;

    /**
     * Converts a native unicode string into a DXF encoded string.
     *
     * DXF endoding includes the following special sequences:
     * - %%%c for a diameter sign
     * - %%%d for a degree sign
     * - %%%p for a plus/minus sign
     */
    static wxString toDxfString( const wxString& aStr );

    /**
     * Converts a DXF encoded string into a native Unicode string.
     */
    static wxString toNativeString( const wxString& aData );

    void writeLine();
    void writeMtext();

    double m_DXF2mm;        // The scale factor to convert DXF units to mm

    // These functions are not used in Kicad.
    // But because they are virtual pure in DRW_Interface, they should be defined
    void addLayer( const DRW_Layer& aData ) override {}
    void addDimStyle( const DRW_Dimstyle& aData ) override {}
    void addLType( const DRW_LType& aData ) override {}
    void addBlock( const DRW_Block& aData ) override {}
    void endBlock() override {}
    void addPoint( const DRW_Point& aData ) override {}
    void addRay( const DRW_Ray& aData ) override {}
    void addXline( const DRW_Xline& aData ) override {}
    void addEllipse( const DRW_Ellipse& aData ) override {}
    void addSpline( const DRW_Spline* aData ) override {}
    void addKnot( const DRW_Entity&) override {}
    void addInsert( const DRW_Insert& aData ) override {}
    void addTrace( const DRW_Trace& aData ) override {}
    void addSolid( const DRW_Solid& aData ) override {}
    void addDimAlign( const DRW_DimAligned* aData ) override {}
    void addDimLinear( const DRW_DimLinear* aData ) override {}
    void addDimRadial( const DRW_DimRadial* aData ) override {}
    void addDimDiametric( const DRW_DimDiametric* aData ) override {}
    void addDimAngular( const DRW_DimAngular* aData ) override {}
    void addDimAngular3P( const DRW_DimAngular3p* aData ) override {}
    void addDimOrdinate( const DRW_DimOrdinate* aData ) override {}
    void addLeader( const DRW_Leader* aData ) override {}
    void addHatch( const DRW_Hatch* aData ) override {}
    void addImage( const DRW_Image* aData ) override {}
    void add3dFace( const DRW_3Dface& aData ) override {}
    void addComment( const char*) override {}
    void addVport( const DRW_Vport& aData ) override {}
    void addViewport( const DRW_Viewport& aData ) override {}
    void addTextStyle( const DRW_Textstyle& aData ) override {}
    void linkImage( const DRW_ImageDef* aData ) override {}
    void setBlock( const int aHandle ) override {}
    void writeTextstyles() override {}
    void writeVports() override {}
    void writeHeader( DRW_Header& aData ) override {}
    void writeEntities() override {}
    void writeLTypes() override {}
    void writeLayers() override {}
    void writeBlockRecords() override {}
    void writeBlocks() override {}
    void writeDimstyles() override {}

    void addAppId( const DRW_AppId& data ) override {}
    void writeAppId() override {}
};

#endif /* DXF_IMPORT_PLUGIN_H */
