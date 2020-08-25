/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2019 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <advanced_config.h>

#include <common.h>
#include <config_params.h>
#include <settings/settings_manager.h>

#include <wx/config.h>
#include <wx/filename.h>
#include <wx/log.h>

/*
 * Flag to enable advanced config debugging
 *
 * Use "KICAD_ADVANCED_CONFIG" to enable.
 *
 * @ingroup trace_env_vars
 */
static const wxChar AdvancedConfigMask[] = wxT( "KICAD_ADVANCED_CONFIG" );

/**
 * Limits and default settings for the coroutine stack size allowed.
 * Warning! Setting the stack size below the default may lead to unexplained crashes
 * This configuration setting is intended for developers only.
 */
namespace AC_STACK
{
    static constexpr int min_stack = 32 * 4096;
    static constexpr int default_stack = 256 * 4096;
    static constexpr int max_stack = 4096 * 4096;
}

/**
 * List of known keys for advanced configuration options.
 *
 * Set these options in the file `kicad_advanced` in the
 * KiCad config directory.
 */
namespace AC_KEYS
{

/**
 * When filling zones, we add an extra amount of clearance to each zone to ensure that rounding
 * errors do not overrun minimum clearance distances.  This is the extra in mm.
 */
static const wxChar ExtraFillMargin[] = wxT( "ExtraFillMargin" );

/**
 * Testing mode for new connectivity algorithm.  Setting this to on will cause all modifications
 * to the netlist to be recalculated on the fly.  This may be slower than the standard process
 * at the moment
 */
static const wxChar RealtimeConnectivity[] = wxT( "RealtimeConnectivity" );

/**
 * Configure the coroutine stack size in bytes.  This should be allocated in multiples of
 * the system page size (n*4096 is generally safe)
 */
static const wxChar CoroutineStackSize[] = wxT( "CoroutineStackSize" );

/**
 * Show PNS router debug graphics while routing
 */
static const wxChar ShowRouterDebugGraphics[] = wxT( "ShowRouterDebugGraphics" );

/**
 * When set to true, this will wrap polygon point sets at 4 points per line rather
 * than a single point per line.  Single point per line helps with version control systems
 */
static const wxChar CompactFileSave[] = wxT( "CompactSave" );

/**
 * For drawsegments - arcs.
 * Distance from an arc end point and the estimated end point,
 * when rotating from the start point to the end point.
 * 0 will not allow any approximate result, and the arc will not show.
 * Squared value for performances, in system unit.
 */
static const wxChar DrawArcAccuracy[] = wxT( "DrawArcAccuracy" );

/**
 * For drawsegments - arcs.
 * When drawing an arc, the angle ( center - start ) - ( start - end ),
 * can be limited to avoid extremely high radii.
 * The value is the tan( angle )
 */
static const wxChar DrawArcCenterStartEndMaxAngle[] = wxT( "DrawArcCenterStartEndMaxAngle" );

/**
 * When true, GAL will stroke the triangulations (only used in OpenGL) with a visible color
 */
static const wxChar StrokeTriangulation[] = wxT( "StrokeTriangulation" );

/**
 * When true, enable Altium Schematic import (*.SchDoc)
 */
static const wxChar PluginAltiumSch[] = wxT( "PluginAltiumSch" );

} // namespace KEYS


/*
 * Get a simple string for common parameters.
 *
 * This isn't exhaustive, but it covers most common types that might be
 * used in the advance config
 */
wxString dumpParamCfg( const PARAM_CFG& aParam )
{
    wxString s = aParam.m_Ident + ": ";

    /*
     * This implementation is rather simplistic, but it is
     * effective enough for simple uses. A better implementation would be
     * some kind of visitor, but that's somewhat more work.
     */
    switch( aParam.m_Type )
    {
    case paramcfg_id::PARAM_INT:
    case paramcfg_id::PARAM_INT_WITH_SCALE:
        s << *static_cast<const PARAM_CFG_INT&>( aParam ).m_Pt_param;
        break;
    case paramcfg_id::PARAM_DOUBLE:
        s << *static_cast<const PARAM_CFG_DOUBLE&>( aParam ).m_Pt_param;
        break;
    case paramcfg_id::PARAM_WXSTRING:
        s << *static_cast<const PARAM_CFG_WXSTRING&>( aParam ).m_Pt_param;
        break;
    case paramcfg_id::PARAM_FILENAME:
        s << *static_cast<const PARAM_CFG_FILENAME&>( aParam ).m_Pt_param;
        break;
    case paramcfg_id::PARAM_BOOL:
        s << ( *static_cast<const PARAM_CFG_BOOL&>( aParam ).m_Pt_param ? "true" : "false" );
        break;
    default: s << "Unsupported PARAM_CFG variant: " << aParam.m_Type;
    }

    return s;
}


/**
 * Dump the configs in the given array to trace.
 */
static void dumpCfg( const std::vector<PARAM_CFG*>& aArray )
{
    // only dump if we need to
    if( !wxLog::IsAllowedTraceMask( AdvancedConfigMask ) )
        return;

    for( const PARAM_CFG* param : aArray )
    {
        wxLogTrace( AdvancedConfigMask, dumpParamCfg( *param ) );
    }
}


/**
 * Get the filename for the advanced config file
 *
 * The user must check the file exists if they care.
 */
static wxFileName getAdvancedCfgFilename()
{
    const static wxString cfg_filename{ "kicad_advanced" };
    return wxFileName( SETTINGS_MANAGER::GetUserSettingsPath(), cfg_filename );
}


ADVANCED_CFG::ADVANCED_CFG()
{
    wxLogTrace( AdvancedConfigMask, "Init advanced config" );

    // Init defaults - this is done in case the config doesn't exist,
    // then the values will remain as set here.
    m_realTimeConnectivity      = true;
    m_coroutineStackSize        = AC_STACK::default_stack;
    m_ShowRouterDebugGraphics   = false;
    m_drawArcAccuracy           = 10.0;
    m_drawArcCenterMaxAngle     = 50.0;
    m_DrawTriangulationOutlines = false;
    m_PluginAltiumSch           = false;

    loadFromConfigFile();
}


const ADVANCED_CFG& ADVANCED_CFG::GetCfg()
{
    static ADVANCED_CFG instance;
    return instance;
}


void ADVANCED_CFG::loadFromConfigFile()
{
    const wxFileName k_advanced = getAdvancedCfgFilename();

    if( !k_advanced.FileExists() )
    {
        wxLogTrace( AdvancedConfigMask, "File does not exist %s", k_advanced.GetFullPath() );

        // load the defaults
        wxConfig emptyConfig;
        loadSettings( emptyConfig );

        return;
    }

    wxLogTrace( AdvancedConfigMask, "Loading advanced config from: %s", k_advanced.GetFullPath() );

    wxFileConfig file_cfg( "", "", k_advanced.GetFullPath() );
    loadSettings( file_cfg );
}


void ADVANCED_CFG::loadSettings( wxConfigBase& aCfg )
{
    std::vector<PARAM_CFG*> configParams;

    configParams.push_back( new PARAM_CFG_BOOL( true, AC_KEYS::RealtimeConnectivity,
                                                &m_realTimeConnectivity, true ) );

    configParams.push_back( new PARAM_CFG_DOUBLE( true, AC_KEYS::ExtraFillMargin,
                                                  &m_extraClearance, 0.001, 0.0, 1.0 ) );

    configParams.push_back( new PARAM_CFG_INT( true, AC_KEYS::CoroutineStackSize,
                                               &m_coroutineStackSize, AC_STACK::default_stack,
                                               AC_STACK::min_stack, AC_STACK::max_stack ) );

    configParams.push_back( new PARAM_CFG_BOOL( true, AC_KEYS::ShowRouterDebugGraphics,
                                                &m_ShowRouterDebugGraphics, false ) );

    configParams.push_back( new PARAM_CFG_BOOL( true, AC_KEYS::CompactFileSave,
                                                &m_CompactSave, false ) );

    configParams.push_back( new PARAM_CFG_DOUBLE( true, AC_KEYS::DrawArcAccuracy,
                                                  &m_drawArcAccuracy, 10.0, 0.0, 100000.0 ) );

    configParams.push_back( new PARAM_CFG_DOUBLE( true, AC_KEYS::DrawArcCenterStartEndMaxAngle,
                                                  &m_drawArcCenterMaxAngle, 50.0, 0.0, 100000.0 ) );

    configParams.push_back( new PARAM_CFG_BOOL( true, AC_KEYS::StrokeTriangulation,
                                                &m_DrawTriangulationOutlines, false ) );

    configParams.push_back(
            new PARAM_CFG_BOOL( true, AC_KEYS::PluginAltiumSch, &m_PluginAltiumSch, false ) );

    wxConfigLoadSetups( &aCfg, configParams );

    for( auto param : configParams )
        delete param;

    dumpCfg( configParams );
}


