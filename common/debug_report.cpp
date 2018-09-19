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

#include <debug_report.h>

#include "wx/xml/xml.h"
#include <wx/debugrpt.h>

#if wxUSE_STACKWALKER
#include "wx/stackwalk.h"
#endif

#include <wx/datetime.h>
#include <wx/ffile.h>
#include <wx/filename.h>


void DEBUG_REPORT::GenerateReport( Context ctx )
{
    DEBUG_REPORT report;

    // Add all wx default reports
    report.AddAll( ctx );

    // Add our own reports
    report.AddTimestamp();

    // Show confirmation dialog and save report
    if( wxDebugReportPreviewStd().Show( report ) )
    {
        report.Process();
    }
}

bool DEBUG_REPORT::AddTimestamp()
{
    wxFileName fn( GetDirectory(), wxT( "timestamp.my" ) );
    wxFFile    file( fn.GetFullPath(), wxT( "w" ) );
    if( !file.IsOpened() )
        return false;

    wxDateTime dt = wxDateTime::Now();
    bool       ret = file.Write( dt.FormatISODate() + wxT( ' ' ) + dt.FormatISOTime() );
    ret &= file.Close();

    AddFile( fn.GetFullName(), wxT( "timestamp of this report" ) );

    return ret;
}

#if !wxCHECK_VERSION( 3, 1, 2 ) && wxUSE_STACKWALKER
class XmlStackWalker : public wxStackWalker
{
public:
    XmlStackWalker( wxXmlNode* nodeStack )
    {
        m_isOk = false;
        m_nodeStack = nodeStack;
    }

    bool IsOk() const
    {
        return m_isOk;
    }

protected:
    virtual void OnStackFrame( const wxStackFrame& frame ) override;

    wxXmlNode* m_nodeStack;
    bool       m_isOk;
};

// ----------------------------------------------------------------------------
// local functions
// ----------------------------------------------------------------------------

static inline void HexProperty( wxXmlNode* node, const wxChar* name, wxUIntPtr value )
{
    node->AddAttribute( name, wxString::Format( wxT( "%#zx" ), value ) );
}

static inline void NumProperty( wxXmlNode* node, const wxChar* name, unsigned long value )
{
    node->AddAttribute( name, wxString::Format( wxT( "%lu" ), value ) );
}

static inline void TextElement( wxXmlNode* node, const wxChar* name, const wxString& value )
{
    wxXmlNode* nodeChild = new wxXmlNode( wxXML_ELEMENT_NODE, name );
    node->AddChild( nodeChild );
    nodeChild->AddChild( new wxXmlNode( wxXML_TEXT_NODE, wxEmptyString, value ) );
}

#if wxUSE_CRASHREPORT && defined( __INTEL__ )

static inline void HexElement( wxXmlNode* node, const wxChar* name, unsigned long value )
{
    TextElement( node, name, wxString::Format( wxT( "%08lx" ), value ) );
}

#endif // wxUSE_CRASHREPORT

// ============================================================================
// XmlStackWalker implementation based on wx, but with additional information
// like offset, address and module of a stack item
// ============================================================================

void XmlStackWalker::OnStackFrame( const wxStackFrame& frame )
{
    m_isOk = true;

    wxXmlNode* nodeFrame = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "frame" ) );
    m_nodeStack->AddChild( nodeFrame );

    NumProperty( nodeFrame, wxT( "level" ), frame.GetLevel() );
    wxString func = frame.GetName();
    if( !func.empty() )
    {
        nodeFrame->AddAttribute( wxT( "function" ), func );
    }

    HexProperty( nodeFrame, wxT( "offset" ), frame.GetOffset() );
    HexProperty( nodeFrame, wxT( "address" ), wxPtrToUInt( frame.GetAddress() ) );

    wxString module = frame.GetModule();
    if( !module.empty() )
        nodeFrame->AddAttribute( wxT( "module" ), module );

    if( frame.HasSourceLocation() )
    {
        nodeFrame->AddAttribute( wxT( "file" ), frame.GetFileName() );
        NumProperty( nodeFrame, wxT( "line" ), frame.GetLine() );
    }

    const size_t nParams = frame.GetParamCount();
    if( nParams )
    {
        wxXmlNode* nodeParams = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "parameters" ) );
        nodeFrame->AddChild( nodeParams );

        for( size_t n = 0; n < nParams; n++ )
        {
            wxXmlNode* nodeParam = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "parameter" ) );
            nodeParams->AddChild( nodeParam );

            NumProperty( nodeParam, wxT( "number" ), n );

            wxString type, name, value;
            if( !frame.GetParam( n, &type, &name, &value ) )
                continue;

            if( !type.empty() )
                TextElement( nodeParam, wxT( "type" ), type );

            if( !name.empty() )
                TextElement( nodeParam, wxT( "name" ), name );

            if( !value.empty() )
                TextElement( nodeParam, wxT( "value" ), value );
        }
    }
}


bool DEBUG_REPORT::AddContext( Context ctx )
{
    wxCHECK_MSG( IsOk(), false, wxT( "use IsOk() first" ) );

    // create XML dump of current context
    wxXmlDocument xmldoc;
    wxXmlNode*    nodeRoot = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "report" ) );
    xmldoc.SetRoot( nodeRoot );
    nodeRoot->AddAttribute( wxT( "version" ), wxT( "1.0" ) );
    nodeRoot->AddAttribute(
            wxT( "kind" ), ctx == Context_Current ? wxT( "user" ) : wxT( "exception" ) );

    // add system information
    wxXmlNode* nodeSystemInfo = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "system" ) );
    if( DoAddSystemInfo( nodeSystemInfo ) )
        nodeRoot->AddChild( nodeSystemInfo );
    else
        delete nodeSystemInfo;

    // add information about the loaded modules
    wxXmlNode* nodeModules = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "modules" ) );
    if( DoAddLoadedModules( nodeModules ) )
        nodeRoot->AddChild( nodeModules );
    else
        delete nodeModules;

    // add CPU context information: this only makes sense for exceptions as our
    // current context is not very interesting otherwise
    if( ctx == Context_Exception )
    {
        wxXmlNode* nodeContext = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "context" ) );
        if( DoAddExceptionInfo( nodeContext ) )
            nodeRoot->AddChild( nodeContext );
        else
            delete nodeContext;
    }

    // add stack traceback
#if wxUSE_STACKWALKER
    wxXmlNode*     nodeStack = new wxXmlNode( wxXML_ELEMENT_NODE, wxT( "stack" ) );
    XmlStackWalker sw( nodeStack );
#if wxUSE_ON_FATAL_EXCEPTION
    if( ctx == Context_Exception )
    {
        sw.WalkFromException();
    }
    else // Context_Current
#endif   // wxUSE_ON_FATAL_EXCEPTION
    {
        sw.Walk();
    }

    if( sw.IsOk() )
        nodeRoot->AddChild( nodeStack );
    else
        delete nodeStack;
#endif // wxUSE_STACKWALKER

    // finally let the user add any extra information he needs
    DoAddCustomContext( nodeRoot );


    // save the entire context dump in a file
    wxFileName fn( GetDirectory(), GetReportName(), wxT( "xml" ) );

    if( !xmldoc.Save( fn.GetFullPath() ) )
        return false;

    AddFile( fn.GetFullName(), _( "process context description" ) );

    return true;
}
#endif // !wxCHECK_VERSION(3, 1, 2) && wxUSE_STACKWALKER