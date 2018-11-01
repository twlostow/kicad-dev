/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 KiCad Developers, see CHANGELOG.txt for contributors.
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

#include <thread>

#include <debug_report.h>

#include <wx/debugrpt.h>
#include <wx/base64.h>
#include <wx/progdlg.h>

#include <dialogs/dialog_crash_report_base.h>
#include <dialogs/dialog_crash_report_preview_base.h>
#include <kicad_curl/kicad_curl_easy.h>

#if wxUSE_STACKWALKER
#include "wx/stackwalk.h"
#endif

#include <aboutinfo.h>

#include <wx/datetime.h>
#include <wx/ffile.h>
#include <wx/dynlib.h>
#include <wx/filename.h>

extern std::string GetKicadCurlVersion();
extern std::string GetCurlLibVersion();

class DIALOG_CRASH_REPORT_PREVIEW : public DIALOG_CRASH_REPORT_PREVIEW_BASE
{
public:
	DIALOG_CRASH_REPORT_PREVIEW( wxWindow* parent, const wxString& aText ) :
        DIALOG_CRASH_REPORT_PREVIEW_BASE( parent )
    {
        m_text->ChangeValue( aText );
    }
};


class DIALOG_CRASH_REPORT : public DIALOG_CRASH_REPORT_BASE
{
public:
    DIALOG_CRASH_REPORT( DEBUG_REPORT* aReport = nullptr ) :
        DIALOG_CRASH_REPORT_BASE( nullptr )
    {
        m_report = aReport;
    }

	~DIALOG_CRASH_REPORT()
    {

    }

    DEBUG_REPORT *GetReport() const{ return m_report ;}

    virtual void OnViewReport( wxCommandEvent& event ) override
    {
        DIALOG_CRASH_REPORT_PREVIEW preview( nullptr, m_report->GetReportText() );

        preview.ShowModal();
    }

    virtual void OnSendReport( wxCommandEvent& event ) override
    {
        auto rpt = m_report->GetReportText();
        bool ok = true, done = false;

        wxString encoded = wxBase64Encode( (const char*) rpt.c_str(), rpt.length() ); 

        auto senderThreadFunc = [&]( ) {
            KICAD_CURL_EASY curl;
            curl.SetPostData( (const char *) encoded.c_str() );
            curl.SetURL( "http://pcbe15262:8051" );
            
            try
            {
                printf("Uploading...\n");
                curl.Perform();
                printf("Done\n");
            }
            catch ( ... )
            {
                ok = false;
            }
            done = true;
        };

        std::thread senderThread ( senderThreadFunc );

        wxProgressDialog dlg( _("Sending report..." ), _("The crash report is being uploaded. Please wait..."),
                            100,    // range
                            this,   // parent 
                            wxPD_APP_MODAL |
                            wxPD_SMOOTH // - makes indeterminate mode bar on WinXP very small
                            );

        while( !done && senderThread.joinable() )
        {
            dlg.Pulse();
            wxMilliSleep( 100 );
        }

        senderThread.join();

        if( !ok )
        {
            wxMessageBox( wxT("Error sending cebug report.") );
            return;
        }
        m_btnSendReport->SetLabel( _("Report sent") );
        m_btnSendReport->Enable( false );
    }

	virtual void OnExitKicad( wxCommandEvent& event ) override
    {
        auto app = wxApp::GetInstance();
        EndModal( false );
        app->ExitMainLoop();
    }


private:
    DEBUG_REPORT* m_report;
};

static const wxString formatHex( uint64_t addr )
{
    char tmp[1024];
    snprintf(tmp, 1024, "%p", (size_t) addr);
    return wxString( tmp );
}

void DEBUG_REPORT::GenerateReport( wxDebugReport::Context ctx )
{
    DEBUG_REPORT report;

    // Add all wx default reports
    report.AddContext( ctx );

    DIALOG_CRASH_REPORT crashDialog( &report );

    //crashDialog.SetVisible( true );
    crashDialog.ShowModal( );
}

#if !wxCHECK_VERSION( 3, 1, 2 ) && wxUSE_STACKWALKER
class YAML_STACK_WALKER : public wxStackWalker
{
public:
    YAML_STACK_WALKER( )
    {
        m_isOk = false;
        m_msg << "stack-trace:\n";
    }

    bool IsOk() const
    {
        return m_isOk;
    }

    const wxString& GetMessage() const
    {
        return m_msg;
    }

protected:
    virtual void OnStackFrame( const wxStackFrame& frame ) override;

    bool       m_isOk;
    wxString   m_msg;
};

// ============================================================================
// Stack walker implementation based on wx, but with additional information
// like offset, address and module of a stack item
// ============================================================================

void YAML_STACK_WALKER::OnStackFrame( const wxStackFrame& frame )
{
    m_isOk = true;

    wxString eol = "\n";
    wxString indent4 = "    ";

    auto func = frame.GetName();

    m_msg << indent4 << "- frame " <<  frame.GetLevel() << ": " << eol;
    if( !func.empty() )
    {
        m_msg << indent4 << indent4 << "function: " << func << eol;
    }

    m_msg << indent4 << indent4 << "offset:  " << formatHex(frame.GetOffset()) << eol;
    m_msg << indent4 << indent4 << "address: " << formatHex(wxPtrToUInt( frame.GetAddress() )) << eol;
}
#endif

void DEBUG_REPORT::buildVersionInfo(wxString& aMsg)
{
    ABOUT_APP_INFO info;
    info.Build( nullptr );

    // DO NOT translate information in the msg_version string

    wxString eol = "\n";
    wxString indent4 = "    ";

    #define ON "ON" << eol
    #define OFF "OFF" << eol

    wxPlatformInfo platform;
    aMsg << "application: " << info.GetAppName() << eol;
    aMsg << "version:     " << info.GetBuildVersion() << eol;
    aMsg << eol;

    aMsg << "libraries:" << eol;
    aMsg << indent4 << "- " << wxGetLibraryVersionInfo().GetVersionString() << eol;

#ifdef BUILD_GITHUB_PLUGIN
    aMsg << indent4 << "- " << GetKicadCurlVersion() << eol;
#endif
    aMsg << eol;

    aMsg << "platform: " << eol;

    aMsg << indent4 << "os: " << wxGetOsDescription() << eol;
    aMsg << indent4 << "arch: " << platform.GetArchName() << eol;
    aMsg << indent4 << "endian: " << platform.GetEndiannessName() << eol;
    aMsg << indent4 << "wx-port: " << platform.GetPortIdName() << eol;
    aMsg << eol;

    aMsg << "build-info:" << eol;
    aMsg << indent4 << "wx-widgets: " << wxVERSION_NUM_DOT_STRING << " (";
    aMsg << __WX_BO_UNICODE __WX_BO_STL __WX_BO_WXWIN_COMPAT_2_8 ")";

    // Get the GTK+ version where possible.
#ifdef __WXGTK__
    int major, minor;

    major = wxPlatformInfo().Get().GetToolkitMajorVersion();
    minor = wxPlatformInfo().Get().GetToolkitMinorVersion();
    aMsg << " GTK+ " <<  major << "." << minor;
#endif

    aMsg << eol;

    aMsg << indent4 << "boost: " << ( BOOST_VERSION / 100000 ) << wxT( "." )
                      << ( BOOST_VERSION / 100 % 1000 ) << wxT( "." )
                      << ( BOOST_VERSION % 100 ) << eol;

#ifdef KICAD_USE_OCC
    aMsg << indent4 << "opencascade-technology: " << OCC_VERSION_COMPLETE << eol;
#endif

#ifdef KICAD_USE_OCE
    aMsg << indent4 << "opencascade-community-edition: " << OCC_VERSION_COMPLETE << eol;
#endif

#ifdef BUILD_GITHUB_PLUGIN
    aMsg << indent4 << "curl: " << GetCurlLibVersion() << eol;
#endif

    aMsg << indent4 << "compiler: ";
#if defined(__clang__)
    aMsg << "Clang " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__;
#elif defined(__GNUG__)
    aMsg << "GCC " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__;
#elif defined(_MSC_VER)
    aMsg << "Visual C++ " << _MSC_VER;
#elif defined(__INTEL_COMPILER)
    aMsg << "Intel C++ " << __INTEL_COMPILER;
#else
    aMsg << "Other Compiler ";
#endif

#if defined(__GXX_ABI_VERSION)
    aMsg << " with C++ ABI " << __GXX_ABI_VERSION << eol;
#else
    aMsg << " without C++ ABI";
#endif

    aMsg << eol;

    // Add build settings config (build options):
    aMsg << "build-settings:" << eol;

    aMsg << indent4 << "- USE_WX_GRAPHICS_CONTEXT=";
#ifdef USE_WX_GRAPHICS_CONTEXT
    aMsg << ON;
#else
    aMsg << OFF;
#endif

    aMsg << indent4 << "- USE_WX_OVERLAY=";
#ifdef USE_WX_OVERLAY
    aMsg << ON;
#else
    aMsg << OFF;
#endif

    aMsg << indent4 << "- KICAD_CRASH_REPORTER=";
#ifdef KICAD_CRASH_REPORTER
    aMsg << ON;
#else
    aMsg << OFF;
#endif

    aMsg << indent4 << "- KICAD_SCRIPTING=";
#ifdef KICAD_SCRIPTING
    aMsg << ON;
#else
    aMsg << OFF;
#endif

    aMsg << indent4 << "- KICAD_SCRIPTING_MODULES=";
#ifdef KICAD_SCRIPTING_MODULES
    aMsg << ON;
#else
    aMsg << OFF;
#endif

    aMsg << indent4 << "- KICAD_SCRIPTING_WXPYTHON=";
#ifdef KICAD_SCRIPTING_WXPYTHON
    aMsg << ON;
#else
    aMsg << OFF;
#endif

    aMsg << indent4 << "- KICAD_SCRIPTING_ACTION_MENU=";
#ifdef KICAD_SCRIPTING_ACTION_MENU
    aMsg << ON;
#else
    aMsg << OFF;
#endif

    aMsg << indent4 << "- BUILD_GITHUB_PLUGIN=";
#ifdef BUILD_GITHUB_PLUGIN
    aMsg << ON;
#else
    aMsg << OFF;
#endif

    aMsg << indent4 << "- KICAD_USE_OCE=";
#ifdef KICAD_USE_OCE
    aMsg << ON;
#else
    aMsg << OFF;
#endif

    aMsg << indent4 << "- KICAD_USE_OCC=";
#ifdef KICAD_USE_OCC
    aMsg << ON;
#else
    aMsg << OFF;
#endif

    aMsg << indent4 << "- KICAD_SPICE=";
#ifdef KICAD_SPICE
    aMsg << ON;
#else
    aMsg << OFF;
#endif

    aMsg << eol;
}

void DEBUG_REPORT::buildModulesInfo( wxString& aMsg )
{
    wxDynamicLibraryDetailsArray modules(wxDynamicLibrary::ListLoaded());
    const size_t count = modules.GetCount();

    if ( !count )
        return;

    wxString eol = "\n";
    wxString indent4 = "    ";

    aMsg << "modules:" << eol;

    for ( size_t n = 0; n < count; n++ )
    {
        const wxDynamicLibraryDetails& info = modules[n];
        void *addr = NULL;
        size_t len = 0;

        wxString path = info.GetPath();
        if ( path.empty() )
            path = info.GetName();

        aMsg << indent4 << "- " << path << ": " << eol;
        if ( info.GetAddress(&addr, &len) )
        {
            aMsg << indent4 << indent4 << "base:    " << formatHex( (uint64_t) addr ) << eol;
            aMsg << indent4 << indent4 << "size:    " << formatHex( (uint64_t) len ) << eol;
        }

        wxString ver = info.GetVersion();
        if ( !ver.empty() )
        {
            aMsg << indent4 << indent4 << "version: " << ver << eol;
        }
    }
    aMsg << eol;
}

void DEBUG_REPORT::buildExceptionContextInfo ( wxString& aMsg )
{
#if wxUSE_CRASHREPORT
    //printf("BuildExceptInfo\n");
    wxCrashContext c;
    if ( !c.code )
        return;

    wxString eol = "\n";
    wxString indent4 = "    ";

    aMsg << "exception-context:" << eol;

    aMsg << indent4 << "code:     " << formatHex(c.code) << eol; 
    aMsg << indent4 << "name:     " << c.GetExceptionString() << eol; 
    aMsg << indent4 << "address:  " << formatHex(c.addr) << eol; -

#ifdef __INTEL__
    aMsg << indent4 << "x86-registers:  " << eol;
    aMsg << indent4 << indent4 << "- eax:   " << formatHex( c.regs.eax ) << eol;
    aMsg << indent4 << indent4 << "- ebx:   " << formatHex( c.regs.ebx ) << eol;
    aMsg << indent4 << indent4 << "- ecx:   " << formatHex( c.regs.ecx ) << eol;
    aMsg << indent4 << indent4 << "- edx:   " << formatHex( c.regs.edx ) << eol;
    aMsg << indent4 << indent4 << "- esi:   " << formatHex( c.regs.esi ) << eol;
    aMsg << indent4 << indent4 << "- edi:   " << formatHex( c.regs.edi ) << eol;
    aMsg << indent4 << indent4 << "- ebp:   " << formatHex( c.regs.ebp ) << eol;
    aMsg << indent4 << indent4 << "- esp:   " << formatHex( c.regs.esp ) << eol;
    aMsg << indent4 << indent4 << "- eip:   " << formatHex( c.regs.eip ) << eol;
    aMsg << indent4 << indent4 << "- cs:    " << formatHex( c.regs.cs ) << eol;
    aMsg << indent4 << indent4 << "- ds:    " << formatHex( c.regs.ds ) << eol;
    aMsg << indent4 << indent4 << "- es:    " << formatHex( c.regs.es ) << eol;
    aMsg << indent4 << indent4 << "- fs:    " << formatHex( c.regs.fs ) << eol;
    aMsg << indent4 << indent4 << "- gs:    " << formatHex( c.regs.gs ) << eol;
    aMsg << indent4 << indent4 << "- ss:    " << formatHex( c.regs.ss ) << eol;
    aMsg << indent4 << indent4 << "- flags: " << formatHex( c.regs.flags ) << eol;
#endif // __INTEL__
#endif
}



bool DEBUG_REPORT::AddContext( wxDebugReport::Context ctx )
{
    wxString reportText;

    reportText += wxString::Format("KiCad crash report, version 1.0\n");
    reportText += wxT("--------------------------------------------------\n\n");
    
    wxString verInfo;
    wxString modInfo;
    wxString exceptInfo;

    buildVersionInfo( verInfo );
    buildModulesInfo( modInfo );
    buildExceptionContextInfo( exceptInfo );
    
#if wxUSE_STACKWALKER
    YAML_STACK_WALKER sw;

#if wxUSE_ON_FATAL_EXCEPTION
    if( ctx == wxDebugReport::Context_Exception )
    {
        sw.WalkFromException();
    }
    else // Context_Current
#endif   // wxUSE_ON_FATAL_EXCEPTION
    {
        sw.Walk();
    }

    reportText += verInfo;
    reportText += modInfo;
    reportText += exceptInfo;

    if( sw.IsOk() )
    {
        reportText += sw.GetMessage();
    }

    //printf("%s", (const char *) reportText.c_str() );

    m_reportText = reportText;

    return true;
}
#endif // !wxCHECK_VERSION(3, 1, 2) && wxUSE_STACKWALKER