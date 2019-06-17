/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013-2016 CERN
 * Copyright (C) 2016-2019 KiCad Developers, see AUTHORS.txt for contributors.
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

#include "pcb_actions.h"
#include <pcbnew_id.h>
#include <bitmaps.h>
#include <layers_id_colors_and_visibility.h>
#include <tool/tool_manager.h>
#include <tools/pcb_actions.h>


OPT<TOOL_EVENT> PCB_ACTIONS::TranslateLegacyId( int aId )
{
    switch( aId )
    {
    case ID_GEN_IMPORT_GRAPHICS_FILE:
        return PCB_ACTIONS::placeImportedGraphics.MakeEvent();

    case ID_NO_TOOL_SELECTED:
        return PCB_ACTIONS::selectionTool.MakeEvent();

    case ID_PCB_MUWAVE_TOOL_GAP_CMD:
        return PCB_ACTIONS::microwaveCreateGap.MakeEvent();

    case ID_PCB_MUWAVE_TOOL_STUB_CMD:
        return PCB_ACTIONS::microwaveCreateStub.MakeEvent();

    case ID_PCB_MUWAVE_TOOL_STUB_ARC_CMD:
        return PCB_ACTIONS::microwaveCreateStubArc.MakeEvent();

    case ID_PCB_MUWAVE_TOOL_FUNCTION_SHAPE_CMD:
        return PCB_ACTIONS::microwaveCreateFunctionShape.MakeEvent();

    case ID_PCB_MUWAVE_TOOL_SELF_CMD:
        return PCB_ACTIONS::microwaveCreateLine.MakeEvent();
    }

    return OPT<TOOL_EVENT>();
}


// Actions, being statically-defined, require specialized I18N handling.  We continue to
// use the _() macro so that string harvesting by the I18N framework doesn't have to be
// specialized, but we don't translate on initialization and instead do it in the getters.

#undef _
#define _(s) s


// DRAWING_TOOL
//
TOOL_ACTION PCB_ACTIONS::drawLine( "pcbnew.InteractiveDrawing.line",
        AS_GLOBAL,
        MD_SHIFT + MD_CTRL + 'L', LEGACY_HK_NAME( "Draw Line" ),
        _( "Draw Line" ), _( "Draw a line" ),
        add_graphical_segments_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::drawPolygon( "pcbnew.InteractiveDrawing.graphicPolygon",
        AS_GLOBAL,
        MD_SHIFT + MD_CTRL + 'P', LEGACY_HK_NAME( "Draw Graphic Polygon" ),
        _( "Draw Graphic Polygon" ), _( "Draw a graphic polygon" ),
        add_graphical_polygon_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::drawCircle( "pcbnew.InteractiveDrawing.circle",
        AS_GLOBAL,
        MD_SHIFT + MD_CTRL + 'C', LEGACY_HK_NAME( "Draw Circle" ),
        _( "Draw Circle" ), _( "Draw a circle" ),
        add_circle_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::drawArc( "pcbnew.InteractiveDrawing.arc",
        AS_GLOBAL,
        MD_SHIFT + MD_CTRL + 'A', LEGACY_HK_NAME( "Draw Arc" ),
        _( "Draw Arc" ), _( "Draw an arc" ),
        add_arc_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::placeText( "pcbnew.InteractiveDrawing.text",
        AS_GLOBAL,
        MD_SHIFT + MD_CTRL + 'T', LEGACY_HK_NAME( "Add Text" ),
        _( "Add Text" ), _( "Add a text item" ),
        text_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::drawDimension( "pcbnew.InteractiveDrawing.dimension",
        AS_GLOBAL,
        MD_SHIFT + MD_CTRL + 'H', LEGACY_HK_NAME(  "Add Dimension" ),
        _( "Add Dimension" ), _( "Add a dimension" ),
        add_dimension_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::drawZone( "pcbnew.InteractiveDrawing.zone",
        AS_GLOBAL,
#ifdef __WXOSX_MAC__
        MD_ALT + 'Z',
#else
        MD_SHIFT + MD_CTRL + 'Z',
#endif
        LEGACY_HK_NAME( "Add Filled Zone" ),
        _( "Add Filled Zone" ), _( "Add a filled zone" ),
        add_zone_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::drawVia( "pcbnew.InteractiveDrawing.via",
        AS_GLOBAL,
        MD_SHIFT + MD_CTRL + 'V', LEGACY_HK_NAME( "Add Vias" ),
        _( "Add Vias" ), _( "Add free-standing vias" ),
        add_via_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::drawZoneKeepout( "pcbnew.InteractiveDrawing.keepout",
        AS_GLOBAL,
        MD_SHIFT + MD_CTRL + 'K', LEGACY_HK_NAME( "Add Keepout Area" ),
        _( "Add Keepout Area" ), _( "Add a keepout area" ),
        add_keepout_area_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::drawZoneCutout( "pcbnew.InteractiveDrawing.zoneCutout",
        AS_GLOBAL,
        MD_SHIFT + 'C', LEGACY_HK_NAME( "Add a Zone Cutout" ),
        _( "Add a Zone Cutout" ), _( "Add a cutout area of an existing zone" ),
        add_zone_cutout_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::drawSimilarZone( "pcbnew.InteractiveDrawing.similarZone",
        AS_GLOBAL,
        MD_SHIFT + MD_CTRL + '.', LEGACY_HK_NAME( "Add a Similar Zone" ),
        _( "Add a Similar Zone" ), _( "Add a zone with the same settings as an existing zone" ),
        add_zone_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::placeImportedGraphics( "pcbnew.InteractiveDrawing.placeImportedGraphics",
        AS_GLOBAL,
        MD_SHIFT + MD_CTRL + 'F', LEGACY_HK_NAME( "Place DXF" ),
        _( "Place Imported Graphics" ), "",
        import_vector_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::setAnchor( "pcbnew.InteractiveDrawing.setAnchor",
        AS_GLOBAL,
        MD_SHIFT + MD_CTRL + 'N', LEGACY_HK_NAME( "Place the Footprint Anchor" ),
        _( "Place the Footprint Anchor" ), "",
        anchor_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::incWidth( "pcbnew.InteractiveDrawing.incWidth",
        AS_CONTEXT,
        MD_CTRL + '+', LEGACY_HK_NAME( "Increase Line Width" ),
        _( "Increase Line Width" ), _( "Increase the line width" ) );

TOOL_ACTION PCB_ACTIONS::decWidth( "pcbnew.InteractiveDrawing.decWidth",
        AS_CONTEXT,
        MD_CTRL + '-', LEGACY_HK_NAME( "Decrease Line Width" ),
        _( "Decrease Line Width" ), _( "Decrease the line width" ) );

TOOL_ACTION PCB_ACTIONS::arcPosture( "pcbnew.InteractiveDrawing.arcPosture",
        AS_CONTEXT,
        '/', LEGACY_HK_NAME( "Switch Track Posture" ),
        _( "Switch Arc Posture" ), _( "Switch the arc posture" ) );

TOOL_ACTION PCB_ACTIONS::deleteLastPoint( "pcbnew.InteractiveDrawing.deleteLastPoint",
        AS_CONTEXT,
        WXK_BACK, "",
        _( "Delete Last Point" ), _( "Delete the last point added to the current item" ),
        undo_xpm );

TOOL_ACTION PCB_ACTIONS::closeZoneOutline( "pcbnew.InteractiveDrawing.closeZoneOutline",
        AS_CONTEXT, 0, "",
        _( "Close Zone Outline" ), _( "Close the outline of a zone in progress" ),
        checked_ok_xpm );


// DRC
//
TOOL_ACTION PCB_ACTIONS::runDRC( "pcbnew.DRCTool.runDRC",
        AS_GLOBAL, 0, "",
        _( "Design Rules Checker" ), _( "Show the design rules checker window" ),
        erc_xpm );


// EDIT_TOOL
//
TOOL_ACTION PCB_ACTIONS::editFootprintInFpEditor( "pcbnew.InteractiveEdit.EditFpInFpEditor",
        AS_GLOBAL,
        MD_CTRL + 'E', LEGACY_HK_NAME( "Edit with Footprint Editor" ),
        _( "Open in Footprint Editor" ),
        _( "Opens the selected footprint in the Footprint Editor" ),
        module_editor_xpm );

TOOL_ACTION PCB_ACTIONS::editActivate( "pcbnew.InteractiveEdit",
        AS_GLOBAL, 0, "",
        _( "Edit Activate" ), "",
        move_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::move( "pcbnew.InteractiveEdit.move",
        AS_GLOBAL,
        'M', LEGACY_HK_NAME( "Move Item" ),
        _( "Move" ), _( "Moves the selected item(s)" ),
        move_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::duplicate( "pcbnew.InteractiveEdit.duplicate",
        AS_GLOBAL,
        MD_CTRL + 'D', LEGACY_HK_NAME( "Duplicate Item" ),
        _( "Duplicate" ), _( "Duplicates the selected item(s)" ),
        duplicate_xpm );

TOOL_ACTION PCB_ACTIONS::duplicateIncrement( "pcbnew.InteractiveEdit.duplicateIncrementPads",
        AS_GLOBAL,
        MD_SHIFT + MD_CTRL + 'D', LEGACY_HK_NAME( "Duplicate Item and Increment" ),
        _( "Duplicate" ), _( "Duplicates the selected item(s), incrementing pad numbers" ),
        duplicate_xpm );

TOOL_ACTION PCB_ACTIONS::moveExact( "pcbnew.InteractiveEdit.moveExact",
        AS_GLOBAL,
        MD_CTRL + 'M', LEGACY_HK_NAME( "Move Item Exactly" ),
        _( "Move Exactly..." ), _( "Moves the selected item(s) by an exact amount" ),
        move_exactly_xpm );

TOOL_ACTION PCB_ACTIONS::createArray( "pcbnew.InteractiveEdit.createArray",
        AS_GLOBAL,
        MD_CTRL + 'T', LEGACY_HK_NAME( "Create Array" ),
        _( "Create Array..." ), _( "Create array" ),
        array_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::rotateCw( "pcbnew.InteractiveEdit.rotateCw",
        AS_GLOBAL,
        MD_SHIFT + 'R', LEGACY_HK_NAME( "Rotate Item Clockwise (Modern Toolset only)" ),
        _( "Rotate Clockwise" ), _( "Rotates selected item(s) clockwise" ),
        rotate_cw_xpm, AF_NONE, (void*) -1 );

TOOL_ACTION PCB_ACTIONS::rotateCcw( "pcbnew.InteractiveEdit.rotateCcw",
        AS_GLOBAL,
        'R', LEGACY_HK_NAME( "Rotate Item" ),
        _( "Rotate Counterclockwise" ), _( "Rotates selected item(s) counterclockwise" ),
        rotate_ccw_xpm, AF_NONE, (void*) 1 );

TOOL_ACTION PCB_ACTIONS::flip( "pcbnew.InteractiveEdit.flip",
        AS_GLOBAL,
        'F', LEGACY_HK_NAME( "Flip Item" ),
        _( "Flip" ), _( "Flips selected item(s) to opposite side of board" ),
        swap_layer_xpm );

TOOL_ACTION PCB_ACTIONS::mirror( "pcbnew.InteractiveEdit.mirror",
        AS_GLOBAL, 0, "",
        _( "Mirror" ), _( "Mirrors selected item" ),
        mirror_h_xpm );

TOOL_ACTION PCB_ACTIONS::remove( "pcbnew.InteractiveEdit.remove",
        AS_GLOBAL,
        WXK_BACK, "",
        _( "Delete" ), _( "Deletes selected item(s)" ),
        delete_xpm, AF_NONE, (void*) REMOVE_FLAGS::NORMAL );

TOOL_ACTION PCB_ACTIONS::deleteFull( "pcbnew.InteractiveEdit.deleteFull",
        AS_GLOBAL,
        WXK_DELETE, LEGACY_HK_NAME( "Delete Full Track" ),
        _( "Delete Full Track" ), _( "Deletes selected item(s) and copper connections" ),
        delete_xpm, AF_NONE, (void*) REMOVE_FLAGS::ALT );

TOOL_ACTION PCB_ACTIONS::properties( "pcbnew.InteractiveEdit.properties",
        AS_GLOBAL,
        'E', LEGACY_HK_NAME( "Edit Item" ),
        _( "Properties..." ), _( "Displays item properties dialog" ),
        config_xpm );

TOOL_ACTION PCB_ACTIONS::updateUnits( "pcbnew.InteractiveEdit.updateUnits",
        AS_GLOBAL );


// FOOTPRINT_EDITOR_TOOLS
//
TOOL_ACTION PCB_ACTIONS::toggleFootprintTree( "pcbnew.ModuleEditor.toggleFootprintTree",
        AS_GLOBAL, 0, "",
        _( "Show Footprint Tree" ), _( "Toggles the footprint tree visibility" ),
        search_tree_xpm );

TOOL_ACTION PCB_ACTIONS::newFootprint( "pcbnew.ModuleEditor.newFootprint",
        AS_GLOBAL,
        MD_CTRL + 'N', LEGACY_HK_NAME( "New" ),
        _( "New Footprint..." ), _( "Create a new, empty footprint" ),
        new_footprint_xpm );

TOOL_ACTION PCB_ACTIONS::createFootprint( "pcbnew.ModuleEditor.createFootprint",
        AS_GLOBAL, 0, "",
        _( "Create Footprint..." ), _( "Create a new footprint using the Footprint Wizard" ),
        module_wizard_xpm );

TOOL_ACTION PCB_ACTIONS::saveToBoard( "pcbnew.ModuleEditor.saveToBoard",
        AS_GLOBAL, 0, "",
        _( "Save to Board" ), _( "Update footprint on board" ),
        save_fp_to_board_xpm );

TOOL_ACTION PCB_ACTIONS::saveToLibrary( "pcbnew.ModuleEditor.saveToLibrary",
        AS_GLOBAL, 0, "",
        _( "Save to Library" ), _( "Save changes to library" ),
        save_xpm );

TOOL_ACTION PCB_ACTIONS::editFootprint( "pcbnew.ModuleEditor.editFootprint",
        AS_GLOBAL, 0, "",
        _( "Edit Footprint" ), _( "Show selected footprint on editor canvas" ),
        edit_xpm );

TOOL_ACTION PCB_ACTIONS::deleteFootprint( "pcbnew.ModuleEditor.deleteFootprint",
        AS_GLOBAL, 0, "",
        _( "Delete Footprint from Library" ), "",
        delete_xpm );

TOOL_ACTION PCB_ACTIONS::cutFootprint( "pcbnew.ModuleEditor.cutFootprint",
        AS_GLOBAL, 0, "",
        _( "Cut Footprint" ), "",
        cut_xpm );

TOOL_ACTION PCB_ACTIONS::copyFootprint( "pcbnew.ModuleEditor.copyFootprint",
        AS_GLOBAL, 0, "",
        _( "Copy Footprint" ), "",
        copy_xpm );

TOOL_ACTION PCB_ACTIONS::pasteFootprint( "pcbnew.ModuleEditor.pasteFootprint",
        AS_GLOBAL, 0, "",
        _( "Paste Footprint" ), "",
        paste_xpm );

TOOL_ACTION PCB_ACTIONS::importFootprint( "pcbnew.ModuleEditor.importFootprint",
        AS_GLOBAL, 0, "",
        _( "Import Footprint..." ), "",
        import_module_xpm );

TOOL_ACTION PCB_ACTIONS::exportFootprint( "pcbnew.ModuleEditor.exportFootprint",
        AS_GLOBAL, 0, "",
        _( "Export Footprint..." ), "",
        export_module_xpm );

TOOL_ACTION PCB_ACTIONS::footprintProperties( "pcbnew.ModuleEditor.footprintProperties",
        AS_GLOBAL, 0, "",
        _( "Footprint Properties..." ), "",
        module_options_xpm );

TOOL_ACTION PCB_ACTIONS::placePad( "pcbnew.ModuleEditor.placePad",
        AS_GLOBAL, 0, "",
        _( "Add Pad" ), _( "Add a pad" ),
        pad_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::createPadFromShapes( "pcbnew.ModuleEditor.createPadFromShapes",
        AS_CONTEXT, 0, "",
        _( "Create Pad from Selected Shapes" ),
        _( "Creates a custom-shaped pads from a set of selected shapes" ),
        primitives_to_custom_pad_xpm );

TOOL_ACTION PCB_ACTIONS::explodePadToShapes( "pcbnew.ModuleEditor.explodePadToShapes",
        AS_CONTEXT, 0, "",
        _( "Explode Pad to Graphic Shapes" ),
        _( "Converts a custom-shaped pads to a set of graphical shapes" ),
        custom_pad_to_primitives_xpm );

TOOL_ACTION PCB_ACTIONS::defaultPadProperties( "pcbnew.ModuleEditor.defaultPadProperties",
        AS_GLOBAL, 0, "",
        _( "Default Pad Properties..." ), _( "Edit the pad properties used when creating new pads" ),
        options_pad_xpm );


// GLOBAL_EDIT_TOOL
//
TOOL_ACTION PCB_ACTIONS::updateFootprint( "pcbnew.GlobalEdit.updateFootprint",
        AS_GLOBAL, 0, "",
        _( "Update Footprint..." ),
        _( "Update footprint to include any changes from the library" ),
        reload_xpm );

TOOL_ACTION PCB_ACTIONS::updateFootprints( "pcbnew.GlobalEdit.updateFootprints",
        AS_GLOBAL, 0, "",
        _( "Update Footprints from Library..." ),
        _( "Update footprints to include any changes from the library" ),
        reload_xpm );

TOOL_ACTION PCB_ACTIONS::changeFootprint( "pcbnew.GlobalEdit.changeFootprint",
        AS_GLOBAL, 0, "",
        _( "Change Footprint..." ), _( "Assign a different footprint from the library" ),
        exchange_xpm );

TOOL_ACTION PCB_ACTIONS::changeFootprints( "pcbnew.GlobalEdit.changeFootprints",
        AS_GLOBAL, 0, "",
        _( "Change Footprints..." ), _( "Assign different footprints from the library" ),
        exchange_xpm );

TOOL_ACTION PCB_ACTIONS::swapLayers( "pcbnew.GlobalEdit.swapLayers",
        AS_GLOBAL, 0, "",
        _( "Swap Layers..." ), _( "Move tracks or drawings from one layer to another" ),
        swap_layer_xpm );

TOOL_ACTION PCB_ACTIONS::editTracksAndVias( "pcbnew.GlobalEdit.editTracksAndVias",
        AS_GLOBAL, 0, "",
        _( "Edit Track && Via Properties..." ), "",
        width_track_via_xpm );

TOOL_ACTION PCB_ACTIONS::editTextAndGraphics( "pcbnew.GlobalEdit.editTextAndGraphics",
        AS_GLOBAL, 0, "",
        _( "Edit Text && Graphic Properties..." ), "",
        reset_text_xpm );

TOOL_ACTION PCB_ACTIONS::globalDeletions( "pcbnew.GlobalEdit.globalDeletions",
        AS_GLOBAL, 0, "",
        _( "Global Deletions..." ),
        _( "Delete tracks, footprints and graphic items from board" ),
        general_deletions_xpm );

TOOL_ACTION PCB_ACTIONS::cleanupTracksAndVias( "pcbnew.GlobalEdit.cleanupTracksAndVias",
        AS_GLOBAL, 0, "",
        _( "Cleanup Tracks && Vias..." ),
        _( "Clean stubs, vias, delete break points or unconnected tracks" ),
        delete_xpm );


// MICROWAVE_TOOL
//
TOOL_ACTION PCB_ACTIONS::microwaveCreateGap( "pcbnew.MicrowaveTool.createGap",
        AS_GLOBAL, 0, "",
        _( "Add Gap" ), _( "Create gap of specified length for microwave applications" ),
        mw_add_gap_xpm, AF_ACTIVATE, (void*) MWAVE_TOOL_SIMPLE_ID::GAP );

TOOL_ACTION PCB_ACTIONS::microwaveCreateStub( "pcbnew.MicrowaveTool.createStub",
        AS_GLOBAL, 0, "",
        _( "Add Stub" ), _( "Create stub of specified length for microwave applications" ),
        mw_add_stub_xpm, AF_ACTIVATE, (void*) MWAVE_TOOL_SIMPLE_ID::STUB );

TOOL_ACTION PCB_ACTIONS::microwaveCreateStubArc( "pcbnew.MicrowaveTool.createStubArc",
        AS_GLOBAL, 0, "",
        _( "Add Arc Stub" ), _( "Create stub (arc) of specified length for microwave applications" ),
        mw_add_stub_arc_xpm, AF_ACTIVATE, (void*) MWAVE_TOOL_SIMPLE_ID::STUB_ARC );

TOOL_ACTION PCB_ACTIONS::microwaveCreateFunctionShape( "pcbnew.MicrowaveTool.createFunctionShape",
        AS_GLOBAL, 0, "",
        _( "Add Polynomial Shape" ), _( "Create polynomial shape for microwave applications" ),
        mw_add_gap_xpm, AF_ACTIVATE, (void*) MWAVE_TOOL_SIMPLE_ID::FUNCTION_SHAPE );

TOOL_ACTION PCB_ACTIONS::microwaveCreateLine( "pcbnew.MicrowaveTool.createLine",
        AS_GLOBAL, 0, "",
        _( "Add Microwave Line" ), _( "Create line of specified length for microwave applications" ),
        mw_add_line_xpm, AF_ACTIVATE );


// PAD_TOOL
//
TOOL_ACTION PCB_ACTIONS::copyPadSettings( "pcbnew.PadTool.CopyPadSettings",
        AS_GLOBAL, 0, "",
        _( "Copy Pad Properties" ), _( "Copy current pad's properties" ),
        copy_pad_settings_xpm );

TOOL_ACTION PCB_ACTIONS::applyPadSettings( "pcbnew.PadTool.ApplyPadSettings",
        AS_GLOBAL, 0, "",
        _( "Paste Pad Properties" ), _( "Replace the current pad's properties with those copied earlier" ),
        apply_pad_settings_xpm );

TOOL_ACTION PCB_ACTIONS::pushPadSettings( "pcbnew.PadTool.PushPadSettings",
        AS_GLOBAL, 0, "",
        _( "Push Pad Properties..." ), _( "Copy the current pad's properties to other pads" ),
        push_pad_settings_xpm );

TOOL_ACTION PCB_ACTIONS::enumeratePads( "pcbnew.PadTool.enumeratePads",
        AS_GLOBAL, 0, "",
        _( "Renumber Pads..." ), _( "Renumber pads by clicking on them in the desired order" ),
        pad_enumerate_xpm, AF_ACTIVATE );


// PCB_EDITOR_CONTROL
//
TOOL_ACTION PCB_ACTIONS::boardSetup( "pcbnew.EditorControl.boardSetup",
        AS_GLOBAL, 0, "",
        _( "Board Setup..." ),
        _( "Edit board setup including layers, design rules and various defaults" ),
        options_board_xpm );

TOOL_ACTION PCB_ACTIONS::importNetlist( "pcbnew.EditorControl.importNetlist",
        AS_GLOBAL, 0, "",
        _( "Netlist..." ), _( "Read netlist and update board connectivity" ),
        netlist_xpm );

TOOL_ACTION PCB_ACTIONS::importSpecctraSession( "pcbnew.EditorControl.importSpecctraSession",
        AS_GLOBAL, 0, "",
        _( "Specctra Session..." ), _( "Import routed Specctra session (*.ses) file" ),
        import_xpm );

TOOL_ACTION PCB_ACTIONS::exportSpecctraDSN( "pcbnew.EditorControl.exportSpecctraDSN",
        AS_GLOBAL, 0, "",
        _( "Specctra DSN..." ), _( "Export Specctra DSN routing info" ),
        export_dsn_xpm );

TOOL_ACTION PCB_ACTIONS::generateGerbers( "pcbnew.EditorControl.generateGerbers",
        AS_GLOBAL, 0, "",
        _( "&Gerbers (.gbr)..." ), _( "Generate Gerbers for fabrication" ),
        post_compo_xpm );

TOOL_ACTION PCB_ACTIONS::generateDrillFiles( "pcbnew.EditorControl.generateDrillFiles",
        AS_GLOBAL, 0, "",
        _( "&Drill Files (.drl)..." ), _( "Generate Excellon drill file(s)" ),
        post_drill_xpm );

TOOL_ACTION PCB_ACTIONS::generatePosFile( "pcbnew.EditorControl.generatePosFile",
        AS_GLOBAL, 0, "",
        _( "Footprint &Positions (.pos)..." ),
        _( "Generate footprint position file for pick and place" ),
        post_compo_xpm );

TOOL_ACTION PCB_ACTIONS::generateReportFile( "pcbnew.EditorControl.generateReportFile",
        AS_GLOBAL, 0, "",
        _( "&Footprint Report (.rpt)..." ),
        _( "Create report of all footprints from current board" ),
        tools_xpm );

TOOL_ACTION PCB_ACTIONS::generateD356File( "pcbnew.EditorControl.generateD356File",
        AS_GLOBAL, 0, "",
        _( "IPC-D-356 Netlist File..." ), _( "Generate IPC-D-356 netlist file" ),
        netlist_xpm );

TOOL_ACTION PCB_ACTIONS::generateBOM( "pcbnew.EditorControl.generateBOM",
        AS_GLOBAL, 0, "",
        _( "&BOM..." ), _( "Create bill of materials from current schematic" ),
        bom_xpm );

// Track & via size control
TOOL_ACTION PCB_ACTIONS::trackWidthInc( "pcbnew.EditorControl.trackWidthInc",
        AS_GLOBAL,
        'W', LEGACY_HK_NAME( "Switch Track Width To Next" ),
        "", "" );

TOOL_ACTION PCB_ACTIONS::trackWidthDec( "pcbnew.EditorControl.trackWidthDec",
        AS_GLOBAL,
        MD_SHIFT + 'W', LEGACY_HK_NAME( "Switch Track Width To Previous" ),
        "", "" );

TOOL_ACTION PCB_ACTIONS::viaSizeInc( "pcbnew.EditorControl.viaSizeInc",
        AS_GLOBAL,
        '\'', LEGACY_HK_NAME( "Increase Via Size" ),
        "", "" );

TOOL_ACTION PCB_ACTIONS::viaSizeDec( "pcbnew.EditorControl.viaSizeDec",
        AS_GLOBAL,
        '\\', LEGACY_HK_NAME( "Decrease Via Size" ),
        "", "" );

TOOL_ACTION PCB_ACTIONS::trackViaSizeChanged( "pcbnew.EditorControl.trackViaSizeChanged",
        AS_GLOBAL, 0, "",
        "", "",
        nullptr, AF_NOTIFY );

TOOL_ACTION PCB_ACTIONS::zoneMerge( "pcbnew.EditorControl.zoneMerge",
        AS_GLOBAL, 0, "",
        _( "Merge Zones" ), _( "Merge zones" ) );

TOOL_ACTION PCB_ACTIONS::zoneDuplicate( "pcbnew.EditorControl.zoneDuplicate",
        AS_GLOBAL, 0, "",
        _( "Duplicate Zone onto Layer..." ), _( "Duplicate zone outline onto a different layer" ),
        zone_duplicate_xpm );

TOOL_ACTION PCB_ACTIONS::placeTarget( "pcbnew.EditorControl.placeTarget",
        AS_GLOBAL, 0, "",
        _( "Add Layer Alignment Target" ), _( "Add a layer alignment target" ),
        add_pcb_target_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::placeModule( "pcbnew.EditorControl.placeModule",
        AS_GLOBAL,
        'O', LEGACY_HK_NAME( "Add Footprint" ),
        _( "Add Footprint" ), _( "Add a footprint" ),
        module_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::drillOrigin( "pcbnew.EditorControl.drillOrigin",
        AS_GLOBAL, 0, "",
        _( "Drill and Place Offset" ), _( "Place origin point for drill and place files" ),
        pcb_offset_xpm );

TOOL_ACTION PCB_ACTIONS::crossProbeSchToPcb( "pcbnew.EditorControl.crossProbSchToPcb",
        AS_GLOBAL );

TOOL_ACTION PCB_ACTIONS::toggleLock( "pcbnew.EditorControl.toggleLock",
        AS_GLOBAL,
        'L', LEGACY_HK_NAME( "Lock/Unlock Footprint" ),
        "Toggle Lock", "",
        lock_unlock_xpm );

TOOL_ACTION PCB_ACTIONS::lock( "pcbnew.EditorControl.lock",
        AS_GLOBAL, 0, "",
        _( "Lock" ), "",
        locked_xpm );

TOOL_ACTION PCB_ACTIONS::unlock( "pcbnew.EditorControl.unlock",
        AS_GLOBAL, 0, "",
        _( "Unlock" ), "",
        unlocked_xpm );

TOOL_ACTION PCB_ACTIONS::appendBoard( "pcbnew.EditorControl.appendBoard",
        AS_GLOBAL, 0, "",
        "Append Board...", "",
        add_board_xpm );

TOOL_ACTION PCB_ACTIONS::highlightNet( "pcbnew.EditorControl.highlightNet",
        AS_GLOBAL );

TOOL_ACTION PCB_ACTIONS::toggleLastNetHighlight( "pcbnew.EditorControl.toggleLastNetHighlight",
        AS_GLOBAL, 0, "",
        _( "Toggle Last Net Highlight" ), _( "Toggle between last two highlighted nets" ) );

TOOL_ACTION PCB_ACTIONS::clearHighlight( "pcbnew.EditorControl.clearHighlight",
        AS_GLOBAL, 0, "",
        _( "Clear Net Highlighting" ), "" );

TOOL_ACTION PCB_ACTIONS::highlightNetTool( "pcbnew.EditorControl.highlightNetTool",
        AS_GLOBAL, 0, "",
        _( "Highlight Nets" ), _( "Highlight all copper items of a net" ),
        net_highlight_xpm );

TOOL_ACTION PCB_ACTIONS::highlightNetSelection( "pcbnew.EditorControl.highlightNetSelection",
        AS_GLOBAL,
        '`', LEGACY_HK_NAME( "Toggle Highlight of Selected Net (Modern Toolset only)" ),
        _( "Highlight Net" ), _( "Highlight all copper items of a net" ),
        net_highlight_xpm );

TOOL_ACTION PCB_ACTIONS::localRatsnestTool( "pcbnew.Control.localRatsnestTool",
        AS_GLOBAL, 0, "",
        _( "Highlight Ratsnest" ), "",
        tool_ratsnest_xpm );

TOOL_ACTION PCB_ACTIONS::hideDynamicRatsnest( "pcbnew.Control.hideDynamicRatsnest",
        AS_GLOBAL );

TOOL_ACTION PCB_ACTIONS::updateLocalRatsnest( "pcbnew.Control.updateLocalRatsnest",
        AS_GLOBAL );

TOOL_ACTION PCB_ACTIONS::listNets( "pcbnew.Control.listNets",
        AS_GLOBAL, 0, "",
        _( "List Nets" ), _( "Show a list of nets with names and IDs" ),
        list_nets_xpm );

TOOL_ACTION PCB_ACTIONS::showPythonConsole( "pcbnew.Control.showPythonConsole",
        AS_GLOBAL, 0, "",
        _( "Scripting Console" ), _( "Show the Python scripting console" ),
        py_script_xpm );

TOOL_ACTION PCB_ACTIONS::showLayersManager( "pcbnew.Control.showLayersManager",
        AS_GLOBAL, 0, "",
        _( "Show Layers Manager" ), _( "Show/hide the layers manager" ),
        layers_manager_xpm );

TOOL_ACTION PCB_ACTIONS::showMicrowaveToolbar( "pcbnew.Control.showMicrowaveToolbar",
        AS_GLOBAL, 0, "",
        _( "Show Microwave Toolbar" ), _( "Show/hide microwave toolbar\n(Experimental feature)" ),
        mw_toolbar_xpm );

TOOL_ACTION PCB_ACTIONS::flipBoard( "pcbnew.Control.flipBoard",
        AS_GLOBAL, 0, "",
        _( "Flip Board View" ), _( "Flip (mirror) the board view" ),
        flip_board_xpm );


// PCBNEW_CONTROL
//

// Display modes
TOOL_ACTION PCB_ACTIONS::showRatsnest( "pcbnew.Control.showRatsnest",
        AS_GLOBAL, 0, "",
        _( "Show Ratsnest" ), _( "Show board ratsnest" ),
        general_ratsnest_xpm );

TOOL_ACTION PCB_ACTIONS::ratsnestLineMode( "pcbnew.Control.ratsnestLineMode",
        AS_GLOBAL, 0, "",
        _( "Curved Ratsnest Lines" ), _( "Show ratsnest with curved lines" ),
        curved_ratsnest_xpm );

TOOL_ACTION PCB_ACTIONS::trackDisplayMode( "pcbnew.Control.trackDisplayMode",
        AS_GLOBAL,
        'K', LEGACY_HK_NAME( "Track Display Mode" ),
        _( "Sketch Tracks" ), _( "Show tracks in outline mode" ),
        showtrack_xpm );

TOOL_ACTION PCB_ACTIONS::padDisplayMode( "pcbnew.Control.padDisplayMode",
        AS_GLOBAL, 0, "",
        _( "Sketch Pads" ), _( "Show pads in outline mode" ),
        pad_sketch_xpm );

TOOL_ACTION PCB_ACTIONS::viaDisplayMode( "pcbnew.Control.viaDisplayMode",
        AS_GLOBAL, 0, "",
        _( "Sketch Vias" ), _( "Show vias in outline mode" ),
        via_sketch_xpm );

TOOL_ACTION PCB_ACTIONS::graphicDisplayMode( "pcbnew.Control.graphicDisplayMode",
        AS_GLOBAL );

TOOL_ACTION PCB_ACTIONS::moduleEdgeOutlines( "pcbnew.Control.graphicOutlines",
        AS_GLOBAL, 0, "",
        _( "Sketch Graphics" ), _( "Show footprint graphic items in outline mode" ),
        show_mod_edge_xpm );

TOOL_ACTION PCB_ACTIONS::zoneDisplayEnable( "pcbnew.Control.zoneDisplayEnable",
        AS_GLOBAL, 0, "",
        _( "Fill Zones" ), _( "Show filled areas of zones" ),
        show_zone_xpm);

TOOL_ACTION PCB_ACTIONS::zoneDisplayDisable( "pcbnew.Control.zoneDisplayDisable",
        AS_GLOBAL, 0, "",
        _( "Wireframe Zones" ), _( "Show only zone boundaries" ),
        show_zone_disable_xpm );

TOOL_ACTION PCB_ACTIONS::zoneDisplayOutlines( "pcbnew.Control.zoneDisplayOutlines",
        AS_GLOBAL, 0, "",
        _( "Sketch Zones" ), _( "Show solid areas of zones in outline mode" ),
        show_zone_outline_only_xpm);


// Layer control
TOOL_ACTION PCB_ACTIONS::layerTop( "pcbnew.Control.layerTop",
        AS_GLOBAL,
        WXK_PAGEUP, LEGACY_HK_NAME( "Switch to Component (F.Cu) layer" ),
        "Switch to Component (F.Cu) layer", "",
        nullptr, AF_NONE, (void*) F_Cu );

TOOL_ACTION PCB_ACTIONS::layerInner1( "pcbnew.Control.layerInner1",
        AS_GLOBAL,
        WXK_F5, LEGACY_HK_NAME( "Switch to Inner layer 1" ),
        "Switch to Inner layer 1", "",
        nullptr, AF_NONE, (void*) In1_Cu );

TOOL_ACTION PCB_ACTIONS::layerInner2( "pcbnew.Control.layerInner2",
        AS_GLOBAL,
        WXK_F6, LEGACY_HK_NAME( "Switch to Inner layer 2" ),
        "Switch to Inner layer 2", "",
        nullptr, AF_NONE, (void*) In2_Cu );

TOOL_ACTION PCB_ACTIONS::layerInner3( "pcbnew.Control.layerInner3",
        AS_GLOBAL,
        WXK_F7, LEGACY_HK_NAME( "Switch to Inner layer 3" ),
        "Switch to Inner layer 3", "",
        nullptr, AF_NONE, (void*) In3_Cu );

TOOL_ACTION PCB_ACTIONS::layerInner4( "pcbnew.Control.layerInner4",
        AS_GLOBAL,
        WXK_F8, LEGACY_HK_NAME( "Switch to Inner layer 4" ),
        "Switch to Inner layer 4", "",
        nullptr, AF_NONE, (void*) In4_Cu );

TOOL_ACTION PCB_ACTIONS::layerInner5( "pcbnew.Control.layerInner5",
        AS_GLOBAL,
        MD_SHIFT + WXK_F5, LEGACY_HK_NAME( "Switch to Inner layer 5" ),
        "Switch to Inner layer 5", "",
        nullptr, AF_NONE, (void*) In5_Cu );

TOOL_ACTION PCB_ACTIONS::layerInner6( "pcbnew.Control.layerInner6",
        AS_GLOBAL,
        MD_SHIFT + WXK_F6, LEGACY_HK_NAME( "Switch to Inner layer 6" ),
        "Switch to Inner layer 6", "",
        nullptr, AF_NONE, (void*) In6_Cu );

TOOL_ACTION PCB_ACTIONS::layerBottom( "pcbnew.Control.layerBottom",
        AS_GLOBAL,
        WXK_PAGEDOWN, LEGACY_HK_NAME( "Switch to Copper (B.Cu) layer" ),
        "Switch to Copper (B.Cu) layer", "",
        nullptr, AF_NONE, (void*) B_Cu );

TOOL_ACTION PCB_ACTIONS::layerNext( "pcbnew.Control.layerNext",
        AS_GLOBAL,
        '+', LEGACY_HK_NAME( "Switch to Next Layer" ),
        "Switch to Next Layer", "" );

TOOL_ACTION PCB_ACTIONS::layerPrev( "pcbnew.Control.layerPrev",
        AS_GLOBAL,
        '-', LEGACY_HK_NAME( "Switch to Previous Layer" ),
        "Switch to Previous Layer", "" );

TOOL_ACTION PCB_ACTIONS::layerToggle( "pcbnew.Control.layerToggle",
        AS_GLOBAL,
        'V', LEGACY_HK_NAME( "Add Through Via" ),
        "Add Through Via", "" );

TOOL_ACTION PCB_ACTIONS::layerAlphaInc( "pcbnew.Control.layerAlphaInc",
        AS_GLOBAL,
        '}', LEGACY_HK_NAME( "Increment Layer Transparency (Modern Toolset only)" ),
        _( "Increase Layer Opacity" ), _( "Make the current layer more transparent" ),
        contrast_mode_xpm );

TOOL_ACTION PCB_ACTIONS::layerAlphaDec( "pcbnew.Control.layerAlphaDec",
        AS_GLOBAL,
        '{', LEGACY_HK_NAME( "Decrement Layer Transparency (Modern Toolset only)" ),
        _( "Decrease Layer Opacity" ), _( "Make the current layer more transparent" ),
        contrast_mode_xpm );

TOOL_ACTION PCB_ACTIONS::layerChanged( "pcbnew.Control.layerChanged",
        AS_GLOBAL, 0, "",
        "", "",
        nullptr, AF_NOTIFY );

// Miscellaneous
TOOL_ACTION PCB_ACTIONS::selectionTool( "pcbnew.Control.selectionTool",
        AS_GLOBAL, 0, "",
        _( "Select item(s)" ), "",
        cursor_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::deleteTool( "pcbnew.Control.deleteTool",
        AS_GLOBAL, 0, "",
        _( "Delete Items Tool" ), _( "Click on items to delete them" ),
        delete_xpm );


// PCBNEW_PICKER_TOOL
//
TOOL_ACTION PCB_ACTIONS::pickerTool( "pcbnew.InteractivePicker",
        AS_GLOBAL, 0, "", "", "", nullptr, AF_ACTIVATE );


// PLACEMENT_TOOL
//
TOOL_ACTION PCB_ACTIONS::alignTop( "pcbnew.AlignAndDistribute.alignTop",
        AS_GLOBAL, 0, "",
        _( "Align to Top" ),
        _( "Aligns selected items to the top edge" ), align_items_top_xpm );

TOOL_ACTION PCB_ACTIONS::alignBottom( "pcbnew.AlignAndDistribute.alignBottom",
        AS_GLOBAL, 0, "",
        _( "Align to Bottom" ),
        _( "Aligns selected items to the bottom edge" ), align_items_bottom_xpm );

TOOL_ACTION PCB_ACTIONS::alignLeft( "pcbnew.AlignAndDistribute.alignLeft",
        AS_GLOBAL, 0, "",
        _( "Align to Left" ),
        _( "Aligns selected items to the left edge" ), align_items_left_xpm );

TOOL_ACTION PCB_ACTIONS::alignRight( "pcbnew.AlignAndDistribute.alignRight",
        AS_GLOBAL, 0, "",
        _( "Align to Right" ),
        _( "Aligns selected items to the right edge" ), align_items_right_xpm );

TOOL_ACTION PCB_ACTIONS::alignCenterX( "pcbnew.AlignAndDistribute.alignCenterX",
        AS_GLOBAL, 0, "",
        _( "Align to Middle" ),
        _( "Aligns selected items to the vertical center" ), align_items_middle_xpm );

TOOL_ACTION PCB_ACTIONS::alignCenterY( "pcbnew.AlignAndDistribute.alignCenterY",
        AS_GLOBAL, 0, "",
        _( "Align to Center" ),
        _( "Aligns selected items to the horizontal center" ), align_items_center_xpm );

TOOL_ACTION PCB_ACTIONS::distributeHorizontally( "pcbnew.AlignAndDistribute.distributeHorizontally",
        AS_GLOBAL, 0, "",
        _( "Distribute Horizontally" ),
        _( "Distributes selected items along the horizontal axis" ), distribute_horizontal_xpm );

TOOL_ACTION PCB_ACTIONS::distributeVertically( "pcbnew.AlignAndDistribute.distributeVertically",
        AS_GLOBAL, 0, "",
        _( "Distribute Vertically" ),
        _( "Distributes selected items along the vertical axis" ), distribute_vertical_xpm );


// POINT_EDITOR
//
TOOL_ACTION PCB_ACTIONS::pointEditorAddCorner( "pcbnew.PointEditor.addCorner",
        AS_GLOBAL,
        WXK_INSERT, "",
        _( "Create Corner" ), _( "Create a corner" ), add_corner_xpm );

TOOL_ACTION PCB_ACTIONS::pointEditorRemoveCorner( "pcbnew.PointEditor.removeCorner",
        AS_GLOBAL, 0, "",
        _( "Remove Corner" ), _( "Remove corner" ), delete_xpm );


// POSITION_RELATIVE_TOOL
//
TOOL_ACTION PCB_ACTIONS::positionRelative( "pcbnew.PositionRelative.positionRelative",
        AS_GLOBAL,
        MD_SHIFT + 'P', LEGACY_HK_NAME( "Position Item Relative" ),
        _( "Position Relative To..." ),
        _( "Positions the selected item(s) by an exact amount relative to another" ),
        move_relative_xpm );

TOOL_ACTION PCB_ACTIONS::selectpositionRelativeItem( "pcbnew.PositionRelative.selectpositionRelativeItem",
        AS_GLOBAL );


// SELECTION_TOOL
//
TOOL_ACTION PCB_ACTIONS::selectionActivate( "pcbnew.InteractiveSelection",
        AS_GLOBAL, 0, "", "", "", nullptr, AF_ACTIVATE );   // No description, not shown anywhere

TOOL_ACTION PCB_ACTIONS::selectionCursor( "pcbnew.InteractiveSelection.Cursor",
        AS_GLOBAL );

TOOL_ACTION PCB_ACTIONS::selectItem( "pcbnew.InteractiveSelection.SelectItem",
        AS_GLOBAL );

TOOL_ACTION PCB_ACTIONS::selectItems( "pcbnew.InteractiveSelection.SelectItems",
        AS_GLOBAL );

TOOL_ACTION PCB_ACTIONS::unselectItem( "pcbnew.InteractiveSelection.UnselectItem",
        AS_GLOBAL );

TOOL_ACTION PCB_ACTIONS::unselectItems( "pcbnew.InteractiveSelection.UnselectItems",
        AS_GLOBAL );

TOOL_ACTION PCB_ACTIONS::selectionClear( "pcbnew.InteractiveSelection.Clear",
        AS_GLOBAL );

TOOL_ACTION PCB_ACTIONS::selectionMenu( "pcbnew.InteractiveSelection.SelectionMenu",
        AS_GLOBAL );

TOOL_ACTION PCB_ACTIONS::selectConnection( "pcbnew.InteractiveSelection.SelectConnection",
        AS_GLOBAL,
        'U', LEGACY_HK_NAME( "Select Single Track" ),
        _( "Select Single Track" ),
        _( "Selects all track segments & vias between two junctions." ),
        add_tracks_xpm );

TOOL_ACTION PCB_ACTIONS::selectCopper( "pcbnew.InteractiveSelection.SelectCopper",
        AS_GLOBAL,
        'I', LEGACY_HK_NAME( "Select Connected Tracks" ),
        _( "Select Connected Tracks" ),
        _( "Selects all connected tracks & vias." ),
        net_highlight_xpm );

TOOL_ACTION PCB_ACTIONS::expandSelectedConnection( "pcbnew.InteractiveSelection.ExpandConnection",
        AS_GLOBAL, 0, "",
        _( "Expand Selected Connection" ),
        _( "Expands the current selection to select a connection between two junctions." ) );

TOOL_ACTION PCB_ACTIONS::selectNet( "pcbnew.InteractiveSelection.SelectNet",
        AS_GLOBAL, 0, "",
        _( "Select All Tracks in Net" ),
        _( "Selects all tracks & vias belonging to the same net." ),
        mode_track_xpm );

TOOL_ACTION PCB_ACTIONS::selectOnSheetFromEeschema( "pcbnew.InteractiveSelection.SelectOnSheet",
        AS_GLOBAL, 0, "",
        _( "Sheet" ),
        _( "Selects all modules and tracks in the schematic sheet" ),
        select_same_sheet_xpm );

TOOL_ACTION PCB_ACTIONS::selectSameSheet( "pcbnew.InteractiveSelection.SelectSameSheet",
        AS_GLOBAL,  0, "",
        _( "Items in Same Hierarchical Sheet" ),
        _( "Selects all modules and tracks in the same schematic sheet" ),
        select_same_sheet_xpm );

TOOL_ACTION PCB_ACTIONS::findMove( "pcbnew.InteractiveSelection.FindMove",
        AS_GLOBAL,
        'T', LEGACY_HK_NAME( "Get and Move Footprint" ),
        _( "Get and Move Footprint" ),
        _( "Selects a footprint by reference and places it under the cursor for moving"),
        move_xpm );

TOOL_ACTION PCB_ACTIONS::filterSelection( "pcbnew.InteractiveSelection.FilterSelection",
        AS_GLOBAL, 0, "",
        _( "Filter Selection..." ), _( "Filter the types of items in the selection" ),
        options_generic_xpm );


// ZONE_FILLER_TOOL
//
TOOL_ACTION PCB_ACTIONS::zoneFill( "pcbnew.ZoneFiller.zoneFill",
        AS_GLOBAL, 0, "",
        _( "Fill" ), _( "Fill zone(s)" ),
        fill_zone_xpm );

TOOL_ACTION PCB_ACTIONS::zoneFillAll( "pcbnew.ZoneFiller.zoneFillAll",
        AS_GLOBAL,
        'B', LEGACY_HK_NAME( "Fill or Refill All Zones" ),
        _( "Fill All" ), _( "Fill all zones" ),
        fill_zone_xpm );

TOOL_ACTION PCB_ACTIONS::zoneUnfill( "pcbnew.ZoneFiller.zoneUnfill",
        AS_GLOBAL, 0, "",
        _( "Unfill" ), _( "Unfill zone(s)" ),
        zone_unfill_xpm );

TOOL_ACTION PCB_ACTIONS::zoneUnfillAll( "pcbnew.ZoneFiller.zoneUnfillAll",
        AS_GLOBAL,
        MD_CTRL + 'B', LEGACY_HK_NAME( "Remove Filled Areas in All Zones" ),
        _( "Unfill All" ), _( "Unfill all zones" ),
        zone_unfill_xpm );


// AUTOPLACER_TOOL
//
TOOL_ACTION PCB_ACTIONS::autoplaceSelectedComponents( "pcbnew.Autoplacer.autoplaceSelected",
        AS_GLOBAL, 0, "",
        _( "Place Selected Footprints" ),
        _( "Performs automatic placement of selected components" ),
        module_check_xpm );

TOOL_ACTION PCB_ACTIONS::autoplaceOffboardComponents( "pcbnew.Autoplacer.autoplaceOffboard",
        AS_GLOBAL, 0, "",
        _( "Place Off-Board Footprints" ),
        _( "Performs automatic placement of components outside board area" ),
        module_xpm );


// ROUTER_TOOL
//
TOOL_ACTION PCB_ACTIONS::routerActivateSingle( "pcbnew.InteractiveRouter.SingleTrack",
        AS_GLOBAL,
        'X', LEGACY_HK_NAME( "Add New Track" ),
        _( "Interactive Router (Single Tracks)" ), _( "Run push & shove router (single tracks)" ),
        add_tracks_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::routerActivateDiffPair( "pcbnew.InteractiveRouter.DiffPair",
        AS_GLOBAL,
        '6', LEGACY_HK_NAME( "Route Differential Pair (Modern Toolset only)" ),
        _( "Interactive Router (Differential Pairs)" ), _( "Run push & shove router (differential pairs)" ),
        ps_diff_pair_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::routerSettingsDialog( "pcbnew.InteractiveRouter.SettingsDialog",
        AS_GLOBAL,
        MD_CTRL + MD_SHIFT + ',', LEGACY_HK_NAME( "Routing Options" ),
        _( "Interactive Router Settings..." ), _( "Open Interactive Router settings" ),
        tools_xpm );

TOOL_ACTION PCB_ACTIONS::routerDiffPairDialog( "pcbnew.InteractiveRouter.DiffPairDialog",
        AS_GLOBAL, 0, "",
        _( "Differential Pair Dimensions..." ), _( "Open Differential Pair Dimension settings" ),
        ps_diff_pair_gap_xpm );

TOOL_ACTION PCB_ACTIONS::selectLayerPair( "pcbnew.InteractiveRouter.SelectLayerPair",
        AS_GLOBAL, 0, "",
        _( "Set Layer Pair..." ), _( "Change active layer pair for routing" ),
        select_layer_pair_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::routerTuneSingleTrace( "pcbnew.LengthTuner.TuneSingleTrack",
        AS_GLOBAL,
        '7', LEGACY_HK_NAME( "Tune Single Track (Modern Toolset only)" ),
        _( "Tune length of a single track" ), "",
        ps_tune_length_xpm, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::routerTuneDiffPair( "pcbnew.LengthTuner.TuneDiffPair",
        AS_GLOBAL,
        '8', LEGACY_HK_NAME( "Tune Differential Pair Length (Modern Toolset only)" ),
        _( "Tune length of a differential pair" ), "",
        nullptr, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::routerTuneDiffPairSkew( "pcbnew.LengthTuner.TuneDiffPairSkew",
        AS_GLOBAL,
        '9', LEGACY_HK_NAME( "Tune Differential Pair Skew (Modern Toolset only)" ),
        _( "Tune skew of a differential pair" ), "",
        nullptr, AF_ACTIVATE );

TOOL_ACTION PCB_ACTIONS::routerInlineDrag( "pcbnew.InteractiveRouter.InlineDrag",
        AS_CONTEXT, 0, "",
        _( "Drag Track/Via" ), _( "Drags tracks and vias without breaking connections" ),
        drag_xpm );

TOOL_ACTION PCB_ACTIONS::inlineBreakTrack( "pcbnew.InteractiveRouter.InlineBreakTrack",
        AS_GLOBAL, 0, "",
        _( "Break Track" ),
        _( "Splits the track segment into two segments connected at the cursor position." ),
        break_line_xpm );

TOOL_ACTION PCB_ACTIONS::breakTrack( "pcbnew.InteractiveRouter.BreakTrack",
        AS_GLOBAL, 0, "",
        _( "Break Track" ),
        _( "Splits the track segment into two segments connected at the cursor position." ),
        break_line_xpm );

TOOL_ACTION PCB_ACTIONS::drag45Degree( "pcbnew.InteractiveRouter.Drag45Degree",
        AS_GLOBAL,
        'D', LEGACY_HK_NAME( "Drag Track Keep Slope" ),
        _( "Drag (45 degree mode)" ),
        _( "Drags the track segment while keeping connected tracks at 45 degrees." ),
        drag_segment_withslope_xpm );

TOOL_ACTION PCB_ACTIONS::dragFreeAngle( "pcbnew.InteractiveRouter.DragFreeAngle",
        AS_GLOBAL,
        'G', LEGACY_HK_NAME( "Drag Item" ),
        _( "Drag (free angle)" ),
        _( "Drags the nearest joint in the track without restricting the track angle." ),
        move_xpm );


