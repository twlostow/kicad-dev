#if 0

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * Copyright (C) 2016-2017 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * @author Wayne Stambaugh <stambaughw@gmail.com>
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
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ctype.h>
#include <algorithm>

#include <wx/mstream.h>
#include <wx/filename.h>
#include <wx/tokenzr.h>

#include <draw_graphic_text.h>
#include <kiway.h>
#include <kicad_string.h>
#include <richio.h>
#include <core/typeinfo.h>
#include <properties.h>

#include <general.h>
#include <lib_field.h>
#include <sch_bus_entry.h>
#include <sch_marker.h>
#include <sch_junction.h>
#include <sch_line.h>
#include <sch_no_connect.h>
#include <sch_component.h>
#include <sch_text.h>
#include <sch_sheet.h>
#include <sch_bitmap.h>
#include <sch_legacy_plugin.h>
#include <template_fieldnames.h>
#include <sch_screen.h>
#include <class_libentry.h>
#include <class_library.h>
#include <lib_arc.h>
#include <lib_bezier.h>
#include <lib_circle.h>
#include <lib_pin.h>
#include <lib_polyline.h>
#include <lib_rectangle.h>
#include <lib_text.h>
#include <eeschema_id.h>    // for MAX_UNIT_COUNT_PER_PACKAGE definition


// Must be the first line of part library document (.dcm) files.
#define DOCFILE_IDENT     "EESchema-DOCLIB  Version 2.0"

#define SCH_PARSE_ERROR( text, reader, pos )                         \
    THROW_PARSE_ERROR( text, reader.GetSource(), reader.Line(),      \
                       reader.LineNumber(), pos - reader.Line() )


// Token delimiters.
const char* delims = " \t\r\n";


/**
 * @ingroup trace_env_vars
 *
 * Flag to enable legacy schematic plugin debug output.
 */
const wxChar traceSchLegacyPlugin[] = wxT( "KICAD_TRACE_SCH_LEGACY_PLUGIN" );


static bool is_eol( char c )
{
    //        The default file eol character used internally by KiCad.
    //        |
    //        |            Possible eol if someone edited the file by hand on certain platforms.
    //        |            |
    //        |            |           May have gone past eol with strtok().
    //        |            |           |
    if( c == '\n' || c == '\r' || c == 0 )
        return true;

    return false;
}


/**
 * Function strCompare
 *
 * compares \a aString to the string starting at \a aLine and advances the character point to
 * the end of \a String and returns the new pointer position in \a aOutput if it is not NULL.
 *
 * @param aString - A pointer to the string to compare.
 * @param aLine - A pointer to string to begin the comparison.
 * @param aOutput - A pointer to a string pointer to the end of the comparison if not NULL.
 * @return True if \a aString was found starting at \a aLine.  Otherwise false.
 */
static bool strCompare( const char* aString, const char* aLine, const char** aOutput = NULL )
{
    size_t len = strlen( aString );
    bool retv = ( strncasecmp( aLine, aString, len ) == 0 ) &&
                ( isspace( aLine[ len ] ) || aLine[ len ] == 0 );

    if( retv && aOutput )
    {
        const char* tmp = aLine;

        // Move past the end of the token.
        tmp += len;

        // Move to the beginning of the next token.
        while( *tmp && isspace( *tmp ) )
            tmp++;

        *aOutput = tmp;
    }

    return retv;
}


/**
 * Function parseInt
 *
 * parses an ASCII integer string with possible leading whitespace into
 * an integer and updates the pointer at \a aOutput if it is not NULL, just
 * like "man strtol()".
 *
 * @param aReader - The line reader used to generate exception throw information.
 * @param aLine - A pointer the current position in a string.
 * @param aOutput - The pointer to a string pointer to copy the string pointer position when
 *                  the parsing is complete.
 * @return A valid integer value.
 * @throws An #IO_ERROR on an unexpected end of line.
 * @throws A #PARSE_ERROR if the parsed token is not a valid integer.
 */
static int parseInt( FILE_LINE_READER& aReader, const char* aLine, const char** aOutput = NULL )
{
    if( !*aLine )
        SCH_PARSE_ERROR( _( "unexpected end of line" ), aReader, aLine );

    // Clear errno before calling strtol() in case some other crt call set it.
    errno = 0;

    long retv = strtol( aLine, (char**) aOutput, 10 );

    // Make sure no error occurred when calling strtol().
    if( errno == ERANGE )
        SCH_PARSE_ERROR( "invalid integer value", aReader, aLine );

    // strtol does not strip off whitespace before the next token.
    if( aOutput )
    {
        const char* next = *aOutput;

        while( *next && isspace( *next ) )
            next++;

        *aOutput = next;
    }

    return (int) retv;
}


/**
 * Function parseHex
 *
 * parses an ASCII hex integer string with possible leading whitespace into
 * a long integer and updates the pointer at \a aOutput if it is not NULL, just
 * like "man strtol".
 *
 * @param aReader - The line reader used to generate exception throw information.
 * @param aLine - A pointer the current position in a string.
 * @param aOutput - The pointer to a string pointer to copy the string pointer position when
 *                  the parsing is complete.
 * @return A valid integer value.
 * @throws An #IO_ERROR on an unexpected end of line.
 * @throws A #PARSE_ERROR if the parsed token is not a valid integer.
 */
static unsigned long parseHex( FILE_LINE_READER& aReader, const char* aLine,
                               const char** aOutput = NULL )
{
    if( !*aLine )
        SCH_PARSE_ERROR( _( "unexpected end of line" ), aReader, aLine );

    unsigned long retv;

    // Clear errno before calling strtoul() in case some other crt call set it.
    errno = 0;
    retv = strtoul( aLine, (char**) aOutput, 16 );

    // Make sure no error occurred when calling strtoul().
    if( errno == ERANGE )
        SCH_PARSE_ERROR( "invalid hexadecimal number", aReader, aLine );

    // Strip off whitespace before the next token.
    if( aOutput )
    {
        // const char* next = aLine + strlen( token );

        const char* next = *aOutput;

        while( *next && isspace( *next ) )
            next++;

        *aOutput = next;
    }

    return retv;
}


/**
 * Function parseDouble
 *
 * parses an ASCII point string with possible leading whitespace into a double precision
 * floating point number and  updates the pointer at \a aOutput if it is not NULL, just
 * like "man strtod".
 *
 * @param aReader - The line reader used to generate exception throw information.
 * @param aLine - A pointer the current position in a string.
 * @param aOutput - The pointer to a string pointer to copy the string pointer position when
 *                  the parsing is complete.
 * @return A valid double value.
 * @throws An #IO_ERROR on an unexpected end of line.
 * @throws A #PARSE_ERROR if the parsed token is not a valid integer.
 */
static double parseDouble( FILE_LINE_READER& aReader, const char* aLine,
                           const char** aOutput = NULL )
{
    if( !*aLine )
        SCH_PARSE_ERROR( _( "unexpected end of line" ), aReader, aLine );

    // Clear errno before calling strtod() in case some other crt call set it.
    errno = 0;

    double retv = strtod( aLine, (char**) aOutput );

    // Make sure no error occurred when calling strtod().
    if( errno == ERANGE )
        SCH_PARSE_ERROR( "invalid floating point number", aReader, aLine );

    // strtod does not strip off whitespace before the next token.
    if( aOutput )
    {
        const char* next = *aOutput;

        while( *next && isspace( *next ) )
            next++;

        *aOutput = next;
    }

    return retv;
}


/**
 * Function parseChar
 *
 * parses a single ASCII character and updates the pointer at \a aOutput if it is not NULL.
 *
 * @param aReader - The line reader used to generate exception throw information.
 * @param aCurrentToken - A pointer the current position in a string.
 * @param aNextToken - The pointer to a string pointer to copy the string pointer position when
 *                     the parsing is complete.
 * @return A valid ASCII character.
 * @throws An #IO_ERROR on an unexpected end of line.
 * @throws A #PARSE_ERROR if the parsed token is not a a single character token.
 */
static char parseChar( FILE_LINE_READER& aReader, const char* aCurrentToken,
                       const char** aNextToken = NULL )
{
    while( *aCurrentToken && isspace( *aCurrentToken ) )
        aCurrentToken++;

    if( !*aCurrentToken )
        SCH_PARSE_ERROR( _( "unexpected end of line" ), aReader, aCurrentToken );

    if( !isspace( *( aCurrentToken + 1 ) ) )
        SCH_PARSE_ERROR( "expected single character token", aReader, aCurrentToken );

    if( aNextToken )
    {
        const char* next = aCurrentToken + 2;

        while( *next && isspace( *next ) )
            next++;

        *aNextToken = next;
    }

    return *aCurrentToken;
}


/**
 * Function parseUnquotedString.
 *
 * parses an unquoted utf8 string and updates the pointer at \a aOutput if it is not NULL.
 *
 * The parsed string must be a continuous string with no white space.
 *
 * @param aString - A reference to the parsed string.
 * @param aReader - The line reader used to generate exception throw information.
 * @param aCurrentToken - A pointer the current position in a string.
 * @param aNextToken - The pointer to a string pointer to copy the string pointer position when
 *                     the parsing is complete.
 * @param aCanBeEmpty - True if the parsed string is optional.  False if it is mandatory.
 * @throws An #IO_ERROR on an unexpected end of line.
 * @throws A #PARSE_ERROR if the \a aCanBeEmpty is false and no string was parsed.
 */
static void parseUnquotedString( wxString& aString, FILE_LINE_READER& aReader,
                                 const char* aCurrentToken, const char** aNextToken = NULL,
                                 bool aCanBeEmpty = false )
{
    if( !*aCurrentToken )
    {
        if( aCanBeEmpty )
            return;
        else
            SCH_PARSE_ERROR( _( "unexpected end of line" ), aReader, aCurrentToken );
    }

    const char* tmp = aCurrentToken;

    while( *tmp && isspace( *tmp ) )
        tmp++;

    if( !*tmp )
    {
        if( aCanBeEmpty )
            return;
        else
            SCH_PARSE_ERROR( _( "unexpected end of line" ), aReader, aCurrentToken );
    }

    std::string utf8;

    while( *tmp && !isspace( *tmp ) )
        utf8 += *tmp++;

    aString = FROM_UTF8( utf8.c_str() );

    if( aString.IsEmpty() && !aCanBeEmpty )
        SCH_PARSE_ERROR( _( "expected unquoted string" ), aReader, aCurrentToken );

    if( aNextToken )
    {
        const char* next = tmp;

        while( *next && isspace( *next ) )
            next++;

        *aNextToken = next;
    }
}


/**
 * Function parseQuotedString.
 *
 * parses an quoted ASCII utf8 and updates the pointer at \a aOutput if it is not NULL.
 *
 * The parsed string must be contained within a single line.  There are no multi-line
 * quoted strings in the legacy schematic file format.
 *
 * @param aString - A reference to the parsed string.
 * @param aReader - The line reader used to generate exception throw information.
 * @param aCurrentToken - A pointer the current position in a string.
 * @param aNextToken - The pointer to a string pointer to copy the string pointer position when
 *                     the parsing is complete.
 * @param aCanBeEmpty - True if the parsed string is optional.  False if it is mandatory.
 * @throws An #IO_ERROR on an unexpected end of line.
 * @throws A #PARSE_ERROR if the \a aCanBeEmpty is false and no string was parsed.
 */
static void parseQuotedString( wxString& aString, FILE_LINE_READER& aReader,
                               const char* aCurrentToken, const char** aNextToken = NULL,
                               bool aCanBeEmpty = false )
{
    if( !*aCurrentToken )
    {
        if( aCanBeEmpty )
            return;
        else
            SCH_PARSE_ERROR( _( "unexpected end of line" ), aReader, aCurrentToken );
    }

    const char* tmp = aCurrentToken;

    while( *tmp && isspace( *tmp ) )
        tmp++;

    if( !*tmp )
    {
        if( aCanBeEmpty )
            return;
        else
            SCH_PARSE_ERROR( _( "unexpected end of line" ), aReader, aCurrentToken );
    }

    // Verify opening quote.
    if( *tmp != '"' )
        SCH_PARSE_ERROR( "expecting opening quote", aReader, aCurrentToken );

    tmp++;

    std::string utf8;     // utf8 without escapes and quotes.

    // Fetch everything up to closing quote.
    while( *tmp )
    {
        if( *tmp == '\\' )
        {
            tmp++;

            if( !*tmp )
                SCH_PARSE_ERROR( _( "unexpected end of line" ), aReader, aCurrentToken );

            // Do not copy the escape byte if it is followed by \ or "
            if( *tmp != '"' && *tmp != '\\' )
                    utf8 += '\\';

            utf8 += *tmp;
        }
        else if( *tmp == '"' )  // Closing double quote.
        {
            break;
        }
        else
        {
            utf8 += *tmp;
        }

        tmp++;
    }

    aString = FROM_UTF8( utf8.c_str() );

    if( aString.IsEmpty() && !aCanBeEmpty )
        SCH_PARSE_ERROR( "expected quoted string", aReader, aCurrentToken );

    if( *tmp && *tmp != '"' )
        SCH_PARSE_ERROR( "no closing quote for string found", aReader, tmp );

    // Move past the closing quote.
    tmp++;

    if( aNextToken )
    {
        const char* next = tmp;

        while( *next && *next == ' ' )
            next++;

        *aNextToken = next;
    }
}


/**
 * Class SCH_LEGACY_PLUGIN_CACHE
 * is a cache assistant for the part library portion of the #SCH_PLUGIN API, and only for the
 * #SCH_LEGACY_PLUGIN, so therefore is private to this implementation file, i.e. not placed
 * into a header.
 */
class SCH_LEGACY_PLUGIN_CACHE
{
    wxFileName      m_libFileName;  // Absolute path and file name is required here.
    wxDateTime      m_fileModTime;
    LIB_ALIAS_MAP   m_aliases;      // Map of names of LIB_ALIAS pointers.
    bool            m_isWritable;
    bool            m_isModified;
    int             m_modHash;      // Keep track of the modification status of the library.
    int             m_versionMajor;
    int             m_versionMinor;
    int             m_libType;      // Is this cache a component or symbol library.

    LIB_PART*       loadPart( FILE_LINE_READER& aReader );
    void            loadHeader( FILE_LINE_READER& aReader );
    void            loadAliases( std::unique_ptr< LIB_PART >& aPart, FILE_LINE_READER& aReader );
    void            loadField( std::unique_ptr< LIB_PART >& aPart, FILE_LINE_READER& aReader );
    void            loadDrawEntries( std::unique_ptr< LIB_PART >& aPart,
                                     FILE_LINE_READER&            aReader );
    void            loadFootprintFilters( std::unique_ptr< LIB_PART >& aPart,
                                          FILE_LINE_READER&            aReader );
    void            loadDocs();
    LIB_ARC*        loadArc( std::unique_ptr< LIB_PART >& aPart, FILE_LINE_READER& aReader );
    LIB_CIRCLE*     loadCircle( std::unique_ptr< LIB_PART >& aPart, FILE_LINE_READER& aReader );
    LIB_TEXT*       loadText( std::unique_ptr< LIB_PART >& aPart, FILE_LINE_READER& aReader );
    LIB_RECTANGLE*  loadRectangle( std::unique_ptr< LIB_PART >& aPart, FILE_LINE_READER& aReader );
    LIB_PIN*        loadPin( std::unique_ptr< LIB_PART >& aPart, FILE_LINE_READER& aReader );
    LIB_POLYLINE*   loadPolyLine( std::unique_ptr< LIB_PART >& aPart, FILE_LINE_READER& aReader );
    LIB_BEZIER*     loadBezier( std::unique_ptr< LIB_PART >& aPart, FILE_LINE_READER& aReader );

    FILL_T          parseFillMode( FILE_LINE_READER& aReader, const char* aLine,
                                   const char** aOutput );
    bool            checkForDuplicates( wxString& aAliasName );
    LIB_ALIAS*      removeAlias( LIB_ALIAS* aAlias );

    void            saveDocFile();

    friend SCH_LEGACY_PLUGIN;

public:
    SCH_LEGACY_PLUGIN_CACHE( const wxString& aLibraryPath );
    ~SCH_LEGACY_PLUGIN_CACHE();

    int GetModifyHash() const { return m_modHash; }

    // Most all functions in this class throw IO_ERROR exceptions.  There are no
    // error codes nor user interface calls from here, nor in any SCH_PLUGIN objects.
    // Catch these exceptions higher up please.

    /// Save the entire library to file m_libFileName;
    void Save( bool aSaveDocFile = true );

    void Load();

    void AddSymbol( const LIB_PART* aPart );

    void DeleteAlias( const wxString& aAliasName );

    void DeleteSymbol( const wxString& aAliasName );

    wxDateTime GetLibModificationTime();

    bool IsFile( const wxString& aFullPathAndFileName ) const;

    bool IsFileChanged() const;

    void SetModified( bool aModified = true ) { m_isModified = aModified; }

    wxString GetLogicalName() const { return m_libFileName.GetName(); }

    void SetFileName( const wxString& aFileName ) { m_libFileName = aFileName; }

    wxString GetFileName() const { return m_libFileName.GetFullPath(); }
};


SCH_LEGACY_PLUGIN::SCH_LEGACY_PLUGIN()
{
    init( NULL );
}


SCH_LEGACY_PLUGIN::~SCH_LEGACY_PLUGIN()
{
    delete m_cache;
}


void SCH_LEGACY_PLUGIN::init( KIWAY* aKiway, const PROPERTIES* aProperties )
{
    m_version = 0;
    m_rootSheet = NULL;
    m_props = aProperties;
    m_kiway = aKiway;
    m_cache = NULL;
    m_out = NULL;
}


SCH_SHEET* SCH_LEGACY_PLUGIN::Load( const wxString& aFileName, KIWAY* aKiway,
                                    SCH_SHEET* aAppendToMe, const PROPERTIES* aProperties )
{
}


void SCH_LEGACY_PLUGIN::Save( const wxString& aFileName, SCH_SCREEN* aScreen, KIWAY* aKiway,
                              const PROPERTIES* aProperties )
{
}


void SCH_LEGACY_PLUGIN::Format( SCH_SCREEN* aScreen )
{
}



void SCH_LEGACY_PLUGIN::cacheLib( const wxString& aLibraryFileName )
{
    if( !m_cache || !m_cache->IsFile( aLibraryFileName ) || m_cache->IsFileChanged() )
    {
        // a spectacular episode in memory management:
        delete m_cache;
        m_cache = new SCH_LEGACY_PLUGIN_CACHE( aLibraryFileName );

        // Because m_cache is rebuilt, increment PART_LIBS::s_modify_generation
        // to modify the hash value that indicate component to symbol links
        // must be updated.
        PART_LIBS::s_modify_generation++;

        if( !isBuffering( m_props ) )
            m_cache->Load();
    }
}


int SCH_LEGACY_PLUGIN::GetModifyHash() const
{
    if( m_cache )
        return m_cache->GetModifyHash();

    // If the cache hasn't been loaded, it hasn't been modified.
    return 0;
}


size_t SCH_LEGACY_PLUGIN::GetSymbolLibCount( const wxString&   aLibraryPath,
                                             const PROPERTIES* aProperties )
{
    LOCALE_IO toggle;

    m_props = aProperties;

    cacheLib( aLibraryPath );

    return m_cache->m_aliases.size();
}


void SCH_LEGACY_PLUGIN::EnumerateSymbolLib( wxArrayString&    aAliasNameList,
                                            const wxString&   aLibraryPath,
                                            const PROPERTIES* aProperties )
{
    LOCALE_IO   toggle;     // toggles on, then off, the C locale.

    m_props = aProperties;

    cacheLib( aLibraryPath );

    const LIB_ALIAS_MAP& aliases = m_cache->m_aliases;

    for( LIB_ALIAS_MAP::const_iterator it = aliases.begin();  it != aliases.end();  ++it )
        aAliasNameList.Add( it->first );
}


void SCH_LEGACY_PLUGIN::EnumerateSymbolLib( std::vector<LIB_ALIAS*>& aAliasList,
                                            const wxString&   aLibraryPath,
                                            const PROPERTIES* aProperties )
{
    LOCALE_IO   toggle;     // toggles on, then off, the C locale.

    m_props = aProperties;

    cacheLib( aLibraryPath );

    const LIB_ALIAS_MAP& aliases = m_cache->m_aliases;

    for( LIB_ALIAS_MAP::const_iterator it = aliases.begin();  it != aliases.end();  ++it )
        aAliasList.push_back( it->second );
}


LIB_ALIAS* SCH_LEGACY_PLUGIN::LoadSymbol( const wxString& aLibraryPath, const wxString& aAliasName,
                                          const PROPERTIES* aProperties )
{
    LOCALE_IO toggle;     // toggles on, then off, the C locale.

    m_props = aProperties;

    cacheLib( aLibraryPath );

    LIB_ALIAS_MAP::const_iterator it = m_cache->m_aliases.find( aAliasName );

    if( it == m_cache->m_aliases.end() )
        return NULL;

    return it->second;
}


void SCH_LEGACY_PLUGIN::SaveSymbol( const wxString& aLibraryPath, const LIB_PART* aSymbol,
                                    const PROPERTIES* aProperties )
{
    m_props = aProperties;

    cacheLib( aLibraryPath );

    m_cache->AddSymbol( aSymbol );

    if( !isBuffering( aProperties ) )
        m_cache->Save( writeDocFile( aProperties ) );
}


void SCH_LEGACY_PLUGIN::DeleteAlias( const wxString& aLibraryPath, const wxString& aAliasName,
                                     const PROPERTIES* aProperties )
{
    m_props = aProperties;

    cacheLib( aLibraryPath );

    m_cache->DeleteAlias( aAliasName );

    if( !isBuffering( aProperties ) )
        m_cache->Save( writeDocFile( aProperties ) );
}


void SCH_LEGACY_PLUGIN::DeleteSymbol( const wxString& aLibraryPath, const wxString& aAliasName,
                                      const PROPERTIES* aProperties )
{
    m_props = aProperties;

    cacheLib( aLibraryPath );

    m_cache->DeleteSymbol( aAliasName );

    if( !isBuffering( aProperties ) )
        m_cache->Save( writeDocFile( aProperties ) );
}


void SCH_LEGACY_PLUGIN::CreateSymbolLib( const wxString& aLibraryPath,
                                         const PROPERTIES* aProperties )
{
    if( wxFileExists( aLibraryPath ) )
    {
        THROW_IO_ERROR( wxString::Format(
            _( "symbol library '%s' already exists, cannot create a new library" ),
            aLibraryPath.GetData() ) );
    }

    LOCALE_IO toggle;

    m_props = aProperties;

    delete m_cache;
    m_cache = new SCH_LEGACY_PLUGIN_CACHE( aLibraryPath );
    m_cache->SetModified();
    m_cache->Save( writeDocFile( aProperties ) );
    m_cache->Load();    // update m_writable and m_mod_time
}


bool SCH_LEGACY_PLUGIN::DeleteSymbolLib( const wxString& aLibraryPath,
                                         const PROPERTIES* aProperties )
{
    wxFileName fn = aLibraryPath;

    if( !fn.FileExists() )
        return false;

    // Some of the more elaborate wxRemoveFile() crap puts up its own wxLog dialog
    // we don't want that.  we want bare metal portability with no UI here.
    if( wxRemove( aLibraryPath ) )
    {
        THROW_IO_ERROR( wxString::Format( _( "library '%s' cannot be deleted" ),
                                          aLibraryPath.GetData() ) );
    }

    if( m_cache && m_cache->IsFile( aLibraryPath ) )
    {
        delete m_cache;
        m_cache = 0;
    }

    return true;
}


void SCH_LEGACY_PLUGIN::SaveLibrary( const wxString& aLibraryPath, const PROPERTIES* aProperties )
{
    if( !m_cache )
        m_cache = new SCH_LEGACY_PLUGIN_CACHE( aLibraryPath );

    wxString oldFileName = m_cache->GetFileName();

    if( !m_cache->IsFile( aLibraryPath ) )
    {
        m_cache->SetFileName( aLibraryPath );
    }

    // This is a forced save.
    m_cache->SetModified();
    m_cache->Save( writeDocFile( aProperties ) );
    m_cache->SetFileName( oldFileName );
}


bool SCH_LEGACY_PLUGIN::CheckHeader( const wxString& aFileName )
{
    // Open file and check first line
    wxTextFile tempFile;

    tempFile.Open( aFileName );
    wxString firstline;
    // read the first line
    firstline = tempFile.GetFirstLine();
    tempFile.Close();

    return firstline.StartsWith( "EESchema" );
}

const char* SCH_LEGACY_PLUGIN::PropBuffering = "buffering";
const char* SCH_LEGACY_PLUGIN::PropNoDocFile = "no_doc_file";

SCH_LEGACY_PLUGIN_CACHE::SCH_LEGACY_PLUGIN_CACHE( const wxString& aFullPathAndFileName ) :
    m_libFileName( aFullPathAndFileName ),
    m_isWritable( true ),
    m_isModified( false ),
    m_modHash( 1 )
{
    m_versionMajor = -1;
    m_versionMinor = -1;
    m_libType = LIBRARY_TYPE_EESCHEMA;
}


SCH_LEGACY_PLUGIN_CACHE::~SCH_LEGACY_PLUGIN_CACHE()
{
    // When the cache is destroyed, all of the alias objects on the heap should be deleted.
    for( LIB_ALIAS_MAP::iterator it = m_aliases.begin();  it != m_aliases.end();  ++it )
    {
        wxLogTrace( traceSchLegacyPlugin, wxT( "Removing alias %s from library %s." ),
                    GetChars( it->second->GetName() ), GetChars( GetLogicalName() ) );
        LIB_PART* part = it->second->GetPart();
        LIB_ALIAS* alias = it->second;
        delete alias;

        // When the last alias of a part is destroyed, the part is no longer required and it
        // too is destroyed.
        if( part && part->GetAliasCount() == 0 )
            delete part;
    }

    m_aliases.clear();
}


wxDateTime SCH_LEGACY_PLUGIN_CACHE::GetLibModificationTime()
{
    // update the writable flag while we have a wxFileName, in a network this
    // is possibly quite dynamic anyway.
    m_isWritable = m_libFileName.IsFileWritable();

    return m_libFileName.GetModificationTime();
}


bool SCH_LEGACY_PLUGIN_CACHE::IsFile( const wxString& aFullPathAndFileName ) const
{
    return m_libFileName == aFullPathAndFileName;
}


bool SCH_LEGACY_PLUGIN_CACHE::IsFileChanged() const
{
    if( m_fileModTime.IsValid() && m_libFileName.IsOk() && m_libFileName.FileExists() )
        return m_libFileName.GetModificationTime() != m_fileModTime;

    return false;
}


LIB_ALIAS* SCH_LEGACY_PLUGIN_CACHE::removeAlias( LIB_ALIAS* aAlias )
{
    wxCHECK_MSG( aAlias != NULL, NULL, "NULL pointer cannot be removed from library." );

    LIB_ALIAS_MAP::iterator it = m_aliases.find( aAlias->GetName() );

    if( it == m_aliases.end() )
        return NULL;

    // If the entry pointer doesn't match the name it is mapped to in the library, we
    // have done something terribly wrong.
    wxCHECK_MSG( *it->second == aAlias, NULL,
                 "Pointer mismatch while attempting to remove alias entry <" + aAlias->GetName() +
                 "> from library cache <" + m_libFileName.GetName() + ">." );

    LIB_ALIAS*  alias = aAlias;
    LIB_PART*   part = alias->GetPart();

    alias = part->RemoveAlias( alias );

    if( !alias )
    {
        delete part;

        if( m_aliases.size() > 1 )
        {
            LIB_ALIAS_MAP::iterator next = it;
            next++;

            if( next == m_aliases.end() )
                next = m_aliases.begin();

            alias = next->second;
        }
    }

    m_aliases.erase( it );
    m_isModified = true;
    ++m_modHash;
    return alias;
}


void SCH_LEGACY_PLUGIN_CACHE::AddSymbol( const LIB_PART* aPart )
{
    // aPart is cloned in PART_LIB::AddPart().  The cache takes ownership of aPart.
    wxArrayString aliasNames = aPart->GetAliasNames();

    for( size_t i = 0; i < aliasNames.size(); i++ )
    {
        LIB_ALIAS_MAP::iterator it = m_aliases.find( aliasNames[i] );

        if( it != m_aliases.end() )
            removeAlias( it->second );

        LIB_ALIAS* alias = const_cast< LIB_PART* >( aPart )->GetAlias( aliasNames[i] );

        wxASSERT_MSG( alias != NULL, "No alias <" + aliasNames[i] + "> found in symbol <" +
                      aPart->GetName() +">." );

        m_aliases[ aliasNames[i] ] = alias;
    }

    m_isModified = true;
    ++m_modHash;
}


void SCH_LEGACY_PLUGIN_CACHE::Load()
{
    //wxCHECK_RET( m_libFileName.IsAbsolute(),
    //             wxString::Format( "Cannot use relative file paths in legacy plugin to "
    //                               "open library '%s'.", m_libFileName.GetFullPath() ) );

    wxLogTrace( traceSchLegacyPlugin, "Loading legacy symbol file '%s'",
                m_libFileName.GetFullPath() );

    FILE_LINE_READER reader( m_libFileName.GetFullPath() );

    if( !reader.ReadLine() )
        THROW_IO_ERROR( _( "unexpected end of file" ) );

    const char* line = reader.Line();

    if( !strCompare( "EESchema-LIBRARY Version", line, &line ) )
    {
        // Old .sym files (which are libraries with only one symbol, used to store and reuse shapes)
        // EESchema-LIB Version x.x SYMBOL. They are valid files.
        if( !strCompare( "EESchema-LIB Version", line, &line ) )
            SCH_PARSE_ERROR( "file is not a valid component or symbol library file", reader, line );
    }

    m_versionMajor = parseInt( reader, line, &line );

    if( *line != '.' )
        SCH_PARSE_ERROR( "invalid file version formatting in header", reader, line );

    line++;

    m_versionMinor = parseInt( reader, line, &line );

    if( m_versionMajor < 1 || m_versionMinor < 0 || m_versionMinor > 99 )
        SCH_PARSE_ERROR( "invalid file version in header", reader, line );

    // Check if this is a symbol library which is the same as a component library but without
    // any alias, documentation, footprint filters, etc.
    if( strCompare( "SYMBOL", line, &line ) )
    {
        // Symbol files add date and time stamp info to the header.
        m_libType = LIBRARY_TYPE_SYMBOL;

        /// @todo Probably should check for a valid date and time stamp even though it's not used.
    }
    else
    {
        m_libType = LIBRARY_TYPE_EESCHEMA;
    }

    while( reader.ReadLine() )
    {
        line = reader.Line();

        if( *line == '#' || isspace( *line ) )  // Skip comments and blank lines.
            continue;

        // Headers where only supported in older library file formats.
        if( m_libType == LIBRARY_TYPE_EESCHEMA && strCompare( "$HEADER", line ) )
            loadHeader( reader );

        if( strCompare( "DEF", line ) )
        {
            // Read one DEF/ENDDEF part entry from library:
            loadPart( reader );

        }
    }

    ++m_modHash;

    // Remember the file modification time of library file when the
    // cache snapshot was made, so that in a networked environment we will
    // reload the cache as needed.
    m_fileModTime = GetLibModificationTime();

    if( USE_OLD_DOC_FILE_FORMAT( m_versionMajor, m_versionMinor ) )
        loadDocs();
}


void SCH_LEGACY_PLUGIN_CACHE::loadDocs()
{
    const char* line;
    wxString    text;
    wxString    aliasName;
    wxFileName  fn = m_libFileName;
    LIB_ALIAS*  alias = NULL;;

    fn.SetExt( DOC_EXT );

    // Not all libraries will have a document file.
    if( !fn.FileExists() )
        return;

    if( !fn.IsFileReadable() )
        THROW_IO_ERROR( wxString::Format( _( "user does not have permission to read library "
                                             "document file '%s'" ), fn.GetFullPath() ) );

    FILE_LINE_READER reader( fn.GetFullPath() );

    line = reader.ReadLine();

    if( !line )
        THROW_IO_ERROR( _( "symbol document library file is empty" ) );

    if( !strCompare( DOCFILE_IDENT, line, &line ) )
        SCH_PARSE_ERROR( "invalid document library file version formatting in header",
                         reader, line );

    while( reader.ReadLine() )
    {
        line = reader.Line();

        if( *line == '#' )    // Comment line.
            continue;

        if( !strCompare( "$CMP", line, &line ) != 0 )
            SCH_PARSE_ERROR( "$CMP command expected", reader, line );

        parseUnquotedString( aliasName, reader, line, &line );    // Alias name.

        LIB_ALIAS_MAP::iterator it = m_aliases.find( aliasName );

        if( it == m_aliases.end() )
            wxLogWarning( "Alias '%s' not found in library:\n\n"
                          "'%s'\n\nat line %d offset %d", aliasName, fn.GetFullPath(),
                          reader.LineNumber(), (int) (line - reader.Line() ) );
        else
            alias = it->second;

        // Read the curent alias associated doc.
        // if the alias does not exist, just skip the description
        // (Can happen if a .dcm is not synchronized with the corresponding .lib file)
        while( reader.ReadLine() )
        {
            line = reader.Line();

            if( !line )
                SCH_PARSE_ERROR( "unexpected end of file", reader, line );

            if( strCompare( "$ENDCMP", line, &line ) )
                break;

            text = FROM_UTF8( line + 2 );
            text = text.Trim();

            switch( line[0] )
            {
            case 'D':
                if( alias )
                    alias->SetDescription( text );
                break;

            case 'K':
                if( alias )
                    alias->SetKeyWords( text );
                break;

            case 'F':
                if( alias )
                    alias->SetDocFileName( text );
                break;

            case '#':
                break;

            default:
                SCH_PARSE_ERROR( "expected token in symbol definition", reader, line );
            }
        }
    }
}


void SCH_LEGACY_PLUGIN_CACHE::loadHeader( FILE_LINE_READER& aReader )
{
    const char* line = aReader.Line();

    wxASSERT( strCompare( "$HEADER", line, &line ) );

    while( aReader.ReadLine() )
    {
        line = (char*) aReader;

        // The time stamp saved in old library files is not used or saved in the latest
        // library file version.
        if( strCompare( "TimeStamp", line, &line ) )
            continue;
        else if( strCompare( "$ENDHEADER", line, &line ) )
            return;
    }

    SCH_PARSE_ERROR( "$ENDHEADER not found", aReader, line );
}


LIB_PART* SCH_LEGACY_PLUGIN_CACHE::loadPart( FILE_LINE_READER& aReader )
{
    const char* line = aReader.Line();

    wxCHECK( strCompare( "DEF", line, &line ), NULL );

    // Read DEF line:
    char yes_no = 0;

    std::unique_ptr< LIB_PART > part( new LIB_PART( wxEmptyString ) );

    wxString name, prefix;

    parseUnquotedString( name, aReader, line, &line );           // Part name.
    parseUnquotedString( prefix, aReader, line, &line );         // Prefix name
    parseInt( aReader, line, &line );                            // NumOfPins, unused.
    part->SetPinNameOffset( parseInt( aReader, line, &line ) );  // Pin name offset.
    yes_no = parseChar( aReader, line, &line );                  // Show pin numbers.

    if( !( yes_no == 'Y' || yes_no == 'N') )
        SCH_PARSE_ERROR( "expected Y or N", aReader, line );

    part->SetShowPinNumbers( ( yes_no == 'N' ) ? false : true );

    yes_no = parseChar( aReader, line, &line );                  // Show pin numbers.

    if( !( yes_no == 'Y' || yes_no == 'N') )
        SCH_PARSE_ERROR( "expected Y or N", aReader, line );

    part->SetShowPinNames( ( yes_no == 'N' ) ? false : true );   // Show pin names.

    part->SetUnitCount( parseInt( aReader, line, &line ) );      // Number of units.

    // Ensure m_unitCount is >= 1.  Could be read as 0 in old libraries.
    if( part->GetUnitCount() < 1 )
        part->SetUnitCount( 1 );

    // Copy part name and prefix.
    LIB_FIELD& value = part->GetValueField();

    // The root alias is added to the alias list by SetName() which is called by SetText().
    if( name.IsEmpty() )
    {
        part->m_name = "~";
        value.SetText( "~" );
    }
    else if( name[0] != '~' )
    {
        part->m_name = name;
        value.SetText( name );
    }
    else
    {
        name = name.Right( name.Length() - 1 );
        part->m_name = name;
        value.SetText( name );
        value.SetVisible( false );
    }

    // There are some code paths in SetText() that do not set the root alias to the
    // alias list so add it here if it didn't get added by SetText().
    if( !part->HasAlias( part->GetName() ) )
        part->AddAlias( part->GetName() );

    LIB_FIELD& reference = part->GetReferenceField();

    if( prefix == "~" )
    {
        reference.Empty();
        reference.SetVisible( false );
    }
    else
    {
        reference.SetText( prefix );
    }

    // In version 2.2 and earlier, this parameter was a '0' which was just a place holder.
    // The was no concept of interchangeable multiple unit symbols.
    if( LIB_VERSION( m_versionMajor, m_versionMinor ) <= LIB_VERSION( 2, 2 ) )
    {
        // Nothing needs to be set since the default setting for symbols with multiple
        // units were never interchangeable.  Just parse the 0 an move on.
        parseInt( aReader, line, &line );
    }
    else
    {
        char locked = parseChar( aReader, line, &line );

        if( locked == 'L' )
            part->LockUnits( true );
        else if( locked == 'F' || locked == '0' )
            part->LockUnits( false );
        else
            SCH_PARSE_ERROR( "expected L, F, or 0", aReader, line );
    }


    // There is the optional power component flag.
    if( *line )
    {
        char power = parseChar( aReader, line, &line );

        if( power == 'P' )
            part->SetPower();
        else if( power == 'N' )
            part->SetNormal();
        else
            SCH_PARSE_ERROR( "expected P or N", aReader, line );
    }

    line = aReader.ReadLine();

    // Read lines until "ENDDEF" is found.
    while( line )
    {
        if( *line == '#' )                               // Comment
            ;
        else if( strCompare( "Ti", line, &line ) )       // Modification date is ignored.
            continue;
        else if( strCompare( "ALIAS", line, &line ) )    // Aliases
            loadAliases( part, aReader );
        else if( *line == 'F' )                          // Fields
            loadField( part, aReader );
        else if( strCompare( "DRAW", line, &line ) )     // Drawing objects.
            loadDrawEntries( part, aReader );
        else if( strCompare( "$FPLIST", line, &line ) )  // Footprint filter list
            loadFootprintFilters( part, aReader );
        else if( strCompare( "ENDDEF", line, &line ) )   // End of part description
        {
            // Now all is good, Add the root alias to the cache alias list.
            m_aliases[ part->GetName() ] = part->GetAlias( part->GetName() );

            // Add aliases when exist
            for( size_t ii = 0; ii < part->GetAliasCount(); ++ii )
                m_aliases[ part->GetAlias( ii )->GetName() ] = part->GetAlias( ii );

            return part.release();
        }

        line = aReader.ReadLine();
    }

    SCH_PARSE_ERROR( "missing ENDDEF", aReader, line );
}


bool SCH_LEGACY_PLUGIN_CACHE::checkForDuplicates( wxString& aAliasName )
{
    wxCHECK_MSG( !aAliasName.IsEmpty(), false, "alias name cannot be empty" );

    // The alias name is not a duplicate so don't change it.
    if( m_aliases.find( aAliasName ) == m_aliases.end() )
        return false;

    int dupCounter = 1;
    wxString newAlias = aAliasName;

    // If the alias is already loaded, the library is broken.  It may have been possible in
    // the past that this could happen so we assign a new alias name to prevent any conflicts
    // rather than throw an exception.
    while( m_aliases.find( newAlias ) != m_aliases.end() )
    {
        newAlias = aAliasName << dupCounter;
        dupCounter++;
    }

    aAliasName = newAlias;

    return true;
}


void SCH_LEGACY_PLUGIN_CACHE::loadAliases( std::unique_ptr< LIB_PART >& aPart,
                                           FILE_LINE_READER&            aReader )
{
    wxString newAlias;
    const char* line = aReader.Line();

    wxCHECK_RET( strCompare( "ALIAS", line, &line ), "Invalid ALIAS section" );

    // Parse the ALIAS list.
    wxString alias;
    parseUnquotedString( alias, aReader, line, &line );

    while( !alias.IsEmpty() )
    {
        newAlias = alias;
        checkForDuplicates( newAlias );
        aPart->AddAlias( newAlias );
        alias.clear();
        parseUnquotedString( alias, aReader, line, &line, true );
    }
}


void SCH_LEGACY_PLUGIN_CACHE::loadField( std::unique_ptr< LIB_PART >& aPart,
                                         FILE_LINE_READER&            aReader )
{
    const char* line = aReader.Line();

    wxCHECK_RET( *line == 'F', "Invalid field line" );

    int         id;

    if( sscanf( line + 1, "%d", &id ) != 1 || id < 0 )
        SCH_PARSE_ERROR( "invalid field ID", aReader, line + 1 );

    std::unique_ptr< LIB_FIELD > field( new LIB_FIELD( aPart.get(), id ) );

    // Skip to the first double quote.
    while( *line != '"' && *line != 0 )
        line++;

    if( *line == 0 )
        SCH_PARSE_ERROR( _( "unexpected end of line" ), aReader, line );

    wxString text;
    parseQuotedString( text, aReader, line, &line, true );

    // Doctor the *.lib file field which has a "~" in blank fields.  New saves will
    // not save like this.
    if( text.size() == 1 && text[0] == '~' )
        text.clear();

    field->m_Text = text;

    wxPoint pos;

    pos.x = parseInt( aReader, line, &line );
    pos.y = parseInt( aReader, line, &line );
    field->SetPosition( pos );

    wxSize textSize;

    textSize.x = textSize.y = parseInt( aReader, line, &line );
    field->SetTextSize( textSize );

    char textOrient = parseChar( aReader, line, &line );

    if( textOrient == 'H' )
        field->SetTextAngle( TEXT_ANGLE_HORIZ );
    else if( textOrient == 'V' )
        field->SetTextAngle( TEXT_ANGLE_VERT );
    else
        SCH_PARSE_ERROR( "invalid field text orientation parameter", aReader, line );

    char textVisible = parseChar( aReader, line, &line );

    if( textVisible == 'V' )
        field->SetVisible( true );
    else if ( textVisible == 'I' )
        field->SetVisible( false );
    else
        SCH_PARSE_ERROR( "invalid field text visibility parameter", aReader, line );

    // It may be technically correct to use the library version to determine if the field text
    // attributes are present.  If anyone knows if that is valid and what version that would be,
    // please change this to test the library version rather than an EOL or the quoted string
    // of the field name.
    if( *line != 0 && *line != '"' )
    {
        char textHJustify = parseChar( aReader, line, &line );

        if( textHJustify == 'C' )
            field->SetHorizJustify( GR_TEXT_HJUSTIFY_CENTER );
        else if( textHJustify == 'L' )
            field->SetHorizJustify( GR_TEXT_HJUSTIFY_LEFT );
        else if( textHJustify == 'R' )
            field->SetHorizJustify( GR_TEXT_HJUSTIFY_RIGHT );
        else
            SCH_PARSE_ERROR( "invalid field text horizontal justification parameter",
                             aReader, line );

        wxString attributes;

        parseUnquotedString( attributes, aReader, line, &line );

        if( !(attributes.size() == 3 || attributes.size() == 1 ) )
            SCH_PARSE_ERROR( "invalid field text attributes size",
                             aReader, line );

        if( attributes[0] == 'C' )
            field->SetVertJustify( GR_TEXT_VJUSTIFY_CENTER );
        else if( attributes[0] == 'B' )
            field->SetVertJustify( GR_TEXT_VJUSTIFY_BOTTOM );
        else if( attributes[0] == 'T' )
            field->SetVertJustify( GR_TEXT_VJUSTIFY_TOP );
        else
            SCH_PARSE_ERROR( "invalid field text vertical justification parameter",
                             aReader, line );

        if( attributes.size() == 3 )
        {
            if( attributes[1] == 'I' )        // Italic
                field->SetItalic( true );
            else if( attributes[1] != 'N' )   // No italics is default, check for error.
                SCH_PARSE_ERROR( "invalid field text italic parameter", aReader, line );

            if ( attributes[2] == 'B' )       // Bold
                field->SetBold( true );
            else if( attributes[2] != 'N' )   // No bold is default, check for error.
                SCH_PARSE_ERROR( "invalid field text bold parameter", aReader, line );
        }
    }

    // Fields in RAM must always have names.
    if( id < MANDATORY_FIELDS )
    {
        // Fields in RAM must always have names, because we are trying to get
        // less dependent on field ids and more dependent on names.
        // Plus assumptions are made in the field editors.
        field->m_name = TEMPLATE_FIELDNAME::GetDefaultFieldName( id );

        LIB_FIELD* fixedField = aPart->GetField( field->GetId() );

        // this will fire only if somebody broke a constructor or editor.
        // MANDATORY_FIELDS are always present in ram resident components, no
        // exceptions, and they always have their names set, even fixed fields.
        wxASSERT( fixedField );

        *fixedField = *field;

        // Ensure the VALUE field = the part name (can be not the case
        // with malformed libraries: edited by hand, or converted from other tools)
        if( fixedField->GetId() == VALUE )
            fixedField->m_Text = aPart->m_name;
    }
    else
    {
        wxString name;

        parseQuotedString( name, aReader, line, &line, true );  // Optional.

        if( !name.IsEmpty() )
            field->m_name = name;

        aPart->AddDrawItem( field.release() );    // LIB_FIELD* is now owned by the LIB_PART.
    }
}


void SCH_LEGACY_PLUGIN_CACHE::loadDrawEntries( std::unique_ptr< LIB_PART >& aPart,
                                               FILE_LINE_READER&            aReader )
{
    const char* line = aReader.Line();

    wxCHECK_RET( strCompare( "DRAW", line, &line ), "Invalid DRAW section" );

    line = aReader.ReadLine();

    while( line )
    {
        if( strCompare( "ENDDRAW", line, &line ) )
            return;

        switch( line[0] )
        {
        case 'A':    // Arc
            aPart->AddDrawItem( loadArc( aPart, aReader ) );
            break;

        case 'C':    // Circle
            aPart->AddDrawItem( loadCircle( aPart, aReader ) );
            break;

        case 'T':    // Text
            aPart->AddDrawItem( loadText( aPart, aReader ) );
            break;

        case 'S':    // Square
            aPart->AddDrawItem( loadRectangle( aPart, aReader ) );
            break;

        case 'X':    // Pin Description
            aPart->AddDrawItem( loadPin( aPart, aReader ) );
            break;

        case 'P':    // Polyline
            aPart->AddDrawItem( loadPolyLine( aPart, aReader ) );
            break;

        case 'B':    // Bezier Curves
            aPart->AddDrawItem( loadBezier( aPart, aReader ) );
            break;

        case '#':    // Comment
        case '\n':   // Empty line
        case '\r':
        case 0:
            break;

        default:
            SCH_PARSE_ERROR( "undefined DRAW entry", aReader, line );
        }

        line = aReader.ReadLine();
    }

    SCH_PARSE_ERROR( "file ended prematurely loading component draw element", aReader, line );
}


FILL_T SCH_LEGACY_PLUGIN_CACHE::parseFillMode( FILE_LINE_READER& aReader, const char* aLine,
                                               const char** aOutput )
{
    FILL_T mode;

    switch( parseChar( aReader, aLine, aOutput ) )
    {
    case 'F':
        mode = FILLED_SHAPE;
        break;

    case 'f':
        mode = FILLED_WITH_BG_BODYCOLOR;
        break;

    case 'N':
        mode = NO_FILL;
        break;

    default:
        SCH_PARSE_ERROR( "invalid fill type, expected f, F, or N", aReader, aLine );
    }

    return mode;
}


LIB_ARC* SCH_LEGACY_PLUGIN_CACHE::loadArc( std::unique_ptr< LIB_PART >& aPart,
                                           FILE_LINE_READER&            aReader )
{
    const char* line = aReader.Line();

    wxCHECK_MSG( strCompare( "A", line, &line ), NULL, "Invalid LIB_ARC definition" );

    std::unique_ptr< LIB_ARC > arc( new LIB_ARC( aPart.get() ) );

    wxPoint center;

    center.x = parseInt( aReader, line, &line );
    center.y = parseInt( aReader, line, &line );

    arc->SetPosition( center );
    arc->SetRadius( parseInt( aReader, line, &line ) );

    int angle1 = parseInt( aReader, line, &line );
    int angle2 = parseInt( aReader, line, &line );

    NORMALIZE_ANGLE_POS( angle1 );
    NORMALIZE_ANGLE_POS( angle2 );
    arc->SetFirstRadiusAngle( angle1 );
    arc->SetSecondRadiusAngle( angle2 );

    arc->SetUnit( parseInt( aReader, line, &line ) );
    arc->SetConvert( parseInt( aReader, line, &line ) );
    arc->SetWidth( parseInt( aReader, line, &line ) );

    // Old libraries (version <= 2.2) do not have always this FILL MODE param
    // when fill mode is no fill (default mode).
    if( *line != 0 )
        arc->SetFillMode( parseFillMode( aReader, line, &line ) );

    // Actual Coordinates of arc ends are read from file
    if( *line != 0 )
    {
        wxPoint arcStart, arcEnd;

        arcStart.x = parseInt( aReader, line, &line );
        arcStart.y = parseInt( aReader, line, &line );
        arcEnd.x = parseInt( aReader, line, &line );
        arcEnd.y = parseInt( aReader, line, &line );

        arc->SetStart( arcStart );
        arc->SetEnd( arcEnd );
    }
    else
    {
        // Actual Coordinates of arc ends are not read from file
        // (old library), calculate them
        wxPoint arcStart( arc->GetRadius(), 0 );
        wxPoint arcEnd( arc->GetRadius(), 0 );

        RotatePoint( &arcStart.x, &arcStart.y, -angle1 );
        arcStart += arc->GetPosition();
        arc->SetStart( arcStart );
        RotatePoint( &arcEnd.x, &arcEnd.y, -angle2 );
        arcEnd += arc->GetPosition();
        arc->SetEnd( arcEnd );
    }

    return arc.release();
}


LIB_CIRCLE* SCH_LEGACY_PLUGIN_CACHE::loadCircle( std::unique_ptr< LIB_PART >& aPart,
                                                 FILE_LINE_READER&            aReader )
{
    const char* line = aReader.Line();

    wxCHECK_MSG( strCompare( "C", line, &line ), NULL, "Invalid LIB_CIRCLE definition" );

    std::unique_ptr< LIB_CIRCLE > circle( new LIB_CIRCLE( aPart.get() ) );

    wxPoint center;

    center.x = parseInt( aReader, line, &line );
    center.y = parseInt( aReader, line, &line );

    circle->SetPosition( center );
    circle->SetRadius( parseInt( aReader, line, &line ) );
    circle->SetUnit( parseInt( aReader, line, &line ) );
    circle->SetConvert( parseInt( aReader, line, &line ) );
    circle->SetWidth( parseInt( aReader, line, &line ) );

    if( *line != 0 )
        circle->SetFillMode( parseFillMode( aReader, line, &line ) );

    return circle.release();
}


LIB_TEXT* SCH_LEGACY_PLUGIN_CACHE::loadText( std::unique_ptr< LIB_PART >& aPart,
                                             FILE_LINE_READER&            aReader )
{
    const char* line = aReader.Line();

    wxCHECK_MSG( strCompare( "T", line, &line ), NULL, "Invalid LIB_TEXT definition" );

    std::unique_ptr< LIB_TEXT > text( new LIB_TEXT( aPart.get() ) );

    text->SetTextAngle( (double) parseInt( aReader, line, &line ) );

    wxPoint center;

    center.x = parseInt( aReader, line, &line );
    center.y = parseInt( aReader, line, &line );
    text->SetPosition( center );

    wxSize size;

    size.x = size.y = parseInt( aReader, line, &line );
    text->SetTextSize( size );
    text->SetVisible( !parseInt( aReader, line, &line ) );
    text->SetUnit( parseInt( aReader, line, &line ) );
    text->SetConvert( parseInt( aReader, line, &line ) );

    wxString str;

    // If quoted string loading fails, load as not quoted string.
    if( *line == '"' )
        parseQuotedString( str, aReader, line, &line );
    else
        parseUnquotedString( str, aReader, line, &line );

    if( !str.IsEmpty() )
    {
        // convert two apostrophes back to double quote
        str.Replace( "''", "\"" );
        str.Replace( wxT( "~" ), wxT( " " ) );
    }

    text->SetText( str );

    // Here things are murky and not well defined.  At some point it appears the format
    // was changed to add text properties.  However rather than add the token to the end of
    // the text definition, it was added after the string and no mention if the file
    // verion was bumped or not so this code make break on very old component libraries.
    //
    // Update: apparently even in the latest version this can be different so added a test
    //         for end of line before checking for the text properties.
    if( LIB_VERSION( m_versionMajor, m_versionMinor ) > LIB_VERSION( 2, 0 ) && !is_eol( *line ) )
    {
        if( strCompare( "Italic", line, &line ) )
            text->SetItalic( true );
        else if( !strCompare( "Normal", line, &line ) )
            SCH_PARSE_ERROR( "invalid text stype, expected 'Normal' or 'Italic'",
                             aReader, line );

        if( parseInt( aReader, line, &line ) > 0 )
            text->SetBold( true );

        // Some old libaries version > 2.0 do not have these options for text justification:
        if( !is_eol( *line ) )
        {
            switch( parseChar( aReader, line, &line ) )
            {
            case 'L':
                text->SetHorizJustify( GR_TEXT_HJUSTIFY_LEFT );
                break;

            case 'C':
                text->SetHorizJustify( GR_TEXT_HJUSTIFY_CENTER );
                break;

            case 'R':
                text->SetHorizJustify( GR_TEXT_HJUSTIFY_RIGHT );
                break;

            default:
                SCH_PARSE_ERROR( "invalid horizontal text justication parameter, expected L, C, or R",
                                 aReader, line );
            }

            switch( parseChar( aReader, line, &line ) )
            {
            case 'T':
                text->SetVertJustify( GR_TEXT_VJUSTIFY_TOP );
                break;

            case 'C':
                text->SetVertJustify( GR_TEXT_VJUSTIFY_CENTER );
                break;

            case 'B':
                text->SetVertJustify( GR_TEXT_VJUSTIFY_BOTTOM );
                break;

            default:
                SCH_PARSE_ERROR( "invalid vertical text justication parameter, expected T, C, or B",
                                 aReader, line );
            }
        }
    }

    return text.release();
}


LIB_RECTANGLE* SCH_LEGACY_PLUGIN_CACHE::loadRectangle( std::unique_ptr< LIB_PART >& aPart,
                                                       FILE_LINE_READER&            aReader )
{
    const char* line = aReader.Line();

    wxCHECK_MSG( strCompare( "S", line, &line ), NULL, "Invalid LIB_RECTANGLE definition" );

    std::unique_ptr< LIB_RECTANGLE > rectangle( new LIB_RECTANGLE( aPart.get() ) );

    wxPoint pos;

    pos.x = parseInt( aReader, line, &line );
    pos.y = parseInt( aReader, line, &line );
    rectangle->SetPosition( pos );

    wxPoint end;

    end.x = parseInt( aReader, line, &line );
    end.y = parseInt( aReader, line, &line );
    rectangle->SetEnd( end );

    rectangle->SetUnit( parseInt( aReader, line, &line ) );
    rectangle->SetConvert( parseInt( aReader, line, &line ) );
    rectangle->SetWidth( parseInt( aReader, line, &line ) );

    if( *line != 0 )
        rectangle->SetFillMode( parseFillMode( aReader, line, &line ) );

    return rectangle.release();
}


LIB_PIN* SCH_LEGACY_PLUGIN_CACHE::loadPin( std::unique_ptr< LIB_PART >& aPart,
                                           FILE_LINE_READER&            aReader )
{
    const char* line = aReader.Line();

    wxCHECK_MSG( strCompare( "X", line, &line ), NULL, "Invalid LIB_PIN definition" );

    std::unique_ptr< LIB_PIN > pin( new LIB_PIN( aPart.get() ) );

    wxString name, number;

    parseUnquotedString( name, aReader, line, &line );
    parseUnquotedString( number, aReader, line, &line );

    pin->SetName( name, false );
    pin->SetNumber( number );

    wxPoint pos;

    pos.x = parseInt( aReader, line, &line );
    pos.y = parseInt( aReader, line, &line );
    pin->SetPosition( pos );
    pin->SetLength( parseInt( aReader, line, &line ), false );
    pin->SetOrientation( parseChar( aReader, line, &line ), false );
    pin->SetNumberTextSize( parseInt( aReader, line, &line ), false );
    pin->SetNameTextSize( parseInt( aReader, line, &line ), false );
    pin->SetUnit( parseInt( aReader, line, &line ) );
    pin->SetConvert( parseInt( aReader, line, &line ) );

    char type = parseChar( aReader, line, &line );

    wxString attributes;

    // Optional
    parseUnquotedString( attributes, aReader, line, &line, true );

    switch( type )
    {
    case 'I':
        pin->SetType( PIN_INPUT, false );
        break;

    case 'O':
        pin->SetType( PIN_OUTPUT, false );
        break;

    case 'B':
        pin->SetType( PIN_BIDI, false );
        break;

    case 'T':
        pin->SetType( PIN_TRISTATE, false );
        break;

    case 'P':
        pin->SetType( PIN_PASSIVE, false );
        break;

    case 'U':
        pin->SetType( PIN_UNSPECIFIED, false );
        break;

    case 'W':
        pin->SetType( PIN_POWER_IN, false );
        break;

    case 'w':
        pin->SetType( PIN_POWER_OUT, false );
        break;

    case 'C':
        pin->SetType( PIN_OPENCOLLECTOR, false );
        break;

    case 'E':
        pin->SetType( PIN_OPENEMITTER, false );
        break;

    case 'N':
        pin->SetType( PIN_NC, false );
        break;

    default:
        SCH_PARSE_ERROR( "unknown pin type", aReader, line );
    }

    if( !attributes.IsEmpty() )       /* Special Symbol defined */
    {
        enum
        {
            INVERTED        = 1 << 0,
            CLOCK           = 1 << 1,
            LOWLEVEL_IN     = 1 << 2,
            LOWLEVEL_OUT    = 1 << 3,
            FALLING_EDGE    = 1 << 4,
            NONLOGIC        = 1 << 5
        };

        int flags = 0;

        for( int j = attributes.size(); j > 0; )
        {
            switch( attributes[--j].GetValue() )
            {
            case '~':
                break;

            case 'N':
                pin->SetVisible( false );
                break;

            case 'I':
                flags |= INVERTED;
                break;

            case 'C':
                flags |= CLOCK;
                break;

            case 'L':
                flags |= LOWLEVEL_IN;
                break;

            case 'V':
                flags |= LOWLEVEL_OUT;
                break;

            case 'F':
                flags |= FALLING_EDGE;
                break;

            case 'X':
                flags |= NONLOGIC;
                break;

            default:
                SCH_PARSE_ERROR( "unknown pin attribute", aReader, line );
            }
        }

        switch( flags )
        {
        case 0:
            pin->SetShape( PINSHAPE_LINE );
            break;

        case INVERTED:
            pin->SetShape( PINSHAPE_INVERTED );
            break;

        case CLOCK:
            pin->SetShape( PINSHAPE_CLOCK );
            break;

        case INVERTED | CLOCK:
            pin->SetShape( PINSHAPE_INVERTED_CLOCK );
            break;

        case LOWLEVEL_IN:
            pin->SetShape( PINSHAPE_INPUT_LOW );
            break;

        case LOWLEVEL_IN | CLOCK:
            pin->SetShape( PINSHAPE_CLOCK_LOW );
            break;

        case LOWLEVEL_OUT:
            pin->SetShape( PINSHAPE_OUTPUT_LOW );
            break;

        case FALLING_EDGE:
            pin->SetShape( PINSHAPE_FALLING_EDGE_CLOCK );
            break;

        case NONLOGIC:
            pin->SetShape( PINSHAPE_NONLOGIC );
            break;

        default:
            SCH_PARSE_ERROR( "pin attributes do not define a valid pin shape", aReader, line );
        }
    }

    return pin.release();
}


LIB_POLYLINE* SCH_LEGACY_PLUGIN_CACHE::loadPolyLine( std::unique_ptr< LIB_PART >& aPart,
                                                     FILE_LINE_READER&            aReader )
{
    const char* line = aReader.Line();

    wxCHECK_MSG( strCompare( "P", line, &line ), NULL, "Invalid LIB_POLYLINE definition" );

    std::unique_ptr< LIB_POLYLINE > polyLine( new LIB_POLYLINE( aPart.get() ) );

    int points = parseInt( aReader, line, &line );
    polyLine->SetUnit( parseInt( aReader, line, &line ) );
    polyLine->SetConvert( parseInt( aReader, line, &line ) );
    polyLine->SetWidth( parseInt( aReader, line, &line ) );

    wxPoint pt;

    for( int i = 0; i < points; i++ )
    {
        pt.x = parseInt( aReader, line, &line );
        pt.y = parseInt( aReader, line, &line );
        polyLine->AddPoint( pt );
    }

    if( *line != 0 )
        polyLine->SetFillMode( parseFillMode( aReader, line, &line ) );

    return polyLine.release();
}


LIB_BEZIER* SCH_LEGACY_PLUGIN_CACHE::loadBezier( std::unique_ptr< LIB_PART >& aPart,
                                                 FILE_LINE_READER&            aReader )
{
    const char* line = aReader.Line();

    wxCHECK_MSG( strCompare( "B", line, &line ), NULL, "Invalid LIB_BEZIER definition" );

    std::unique_ptr< LIB_BEZIER > bezier( new LIB_BEZIER( aPart.get() ) );

    int points = parseInt( aReader, line, &line );
    bezier->SetUnit( parseInt( aReader, line, &line ) );
    bezier->SetConvert( parseInt( aReader, line, &line ) );
    bezier->SetWidth( parseInt( aReader, line, &line ) );

    wxPoint pt;

    for( int i = 0; i < points; i++ )
    {
        pt.x = parseInt( aReader, line, &line );
        pt.y = parseInt( aReader, line, &line );
        bezier->AddPoint( pt );
    }

    if( *line != 0 )
        bezier->SetFillMode( parseFillMode( aReader, line, &line ) );

    return bezier.release();
}


void SCH_LEGACY_PLUGIN_CACHE::loadFootprintFilters( std::unique_ptr< LIB_PART >& aPart,
                                                    FILE_LINE_READER&            aReader )
{
    const char* line = aReader.Line();

    wxCHECK_RET( strCompare( "$FPLIST", line, &line ), "Invalid footprint filter list" );

    line = aReader.ReadLine();

    while( line )
    {
        if( strCompare( "$ENDFPLIST", line, &line ) )
            return;

        wxString footprint;

        parseUnquotedString( footprint, aReader, line, &line );
        aPart->GetFootPrints().Add( footprint );
        line = aReader.ReadLine();
    }

    SCH_PARSE_ERROR( "file ended prematurely while loading footprint filters", aReader, line );
}


void SCH_LEGACY_PLUGIN_CACHE::Save( bool aSaveDocFile )
{
    if( !m_isModified )
        return;

    std::unique_ptr< FILE_OUTPUTFORMATTER > formatter( new FILE_OUTPUTFORMATTER( m_libFileName.GetFullPath() ) );
    formatter->Print( 0, "%s %d.%d\n", LIBFILE_IDENT, LIB_VERSION_MAJOR, LIB_VERSION_MINOR );
    formatter->Print( 0, "#encoding utf-8\n");

    for( LIB_ALIAS_MAP::iterator it = m_aliases.begin();  it != m_aliases.end();  it++ )
    {
        if( !it->second->IsRoot() )
            continue;

        it->second->GetPart()->Save( *formatter.get() );
    }

    formatter->Print( 0, "#\n#End Library\n" );
    formatter.reset();

    m_fileModTime = m_libFileName.GetModificationTime();
    m_isModified = false;

    if( aSaveDocFile )
        saveDocFile();
}


void SCH_LEGACY_PLUGIN_CACHE::saveDocFile()
{
    wxFileName docFileName = m_libFileName;

    docFileName.SetExt( DOC_EXT );
    FILE_OUTPUTFORMATTER formatter( docFileName.GetFullPath() );

    formatter.Print( 0, "%s\n", DOCFILE_IDENT );

    for( LIB_ALIAS_MAP::iterator it = m_aliases.begin();  it != m_aliases.end();  it++ )
    {
        it->second->SaveDoc( formatter );
    }

    formatter.Print( 0, "#\n#End Doc Library\n" );
}


void SCH_LEGACY_PLUGIN_CACHE::DeleteAlias( const wxString& aAliasName )
{
    LIB_ALIAS_MAP::iterator it = m_aliases.find( aAliasName );

    if( it == m_aliases.end() )
        THROW_IO_ERROR( wxString::Format( _( "library %s does not contain an alias %s" ),
                                          m_libFileName.GetFullName(), aAliasName ) );

    LIB_ALIAS*  alias = it->second;
    LIB_PART*   part = alias->GetPart();

    alias = part->RemoveAlias( alias );

    if( !alias )
    {
        delete part;

        if( m_aliases.size() > 1 )
        {
            LIB_ALIAS_MAP::iterator next = it;
            next++;

            if( next == m_aliases.end() )
                next = m_aliases.begin();

            alias = next->second;
        }
    }

    m_aliases.erase( it );
    ++m_modHash;
    m_isModified = true;
}


void SCH_LEGACY_PLUGIN_CACHE::DeleteSymbol( const wxString& aAliasName )
{
    LIB_ALIAS_MAP::iterator it = m_aliases.find( aAliasName );

    if( it == m_aliases.end() )
        THROW_IO_ERROR( wxString::Format( _( "library %s does not contain an alias %s" ),
                                          m_libFileName.GetFullName(), aAliasName ) );

    LIB_ALIAS*  alias = it->second;
    LIB_PART*   part = alias->GetPart();

    wxArrayString aliasNames = part->GetAliasNames();

    // Deleting all of the aliases deletes the symbol from the library.
    for( size_t i = 0;  i < aliasNames.Count(); i++ )
        DeleteAlias( aliasNames[i] );
}




bool SCH_LEGACY_PLUGIN::writeDocFile( const PROPERTIES* aProperties )
{
    std::string propName( SCH_LEGACY_PLUGIN::PropNoDocFile );

    if( aProperties && aProperties->find( propName ) != aProperties->end() )
        return false;

    return true;
}


bool SCH_LEGACY_PLUGIN::isBuffering( const PROPERTIES* aProperties )
{
    return ( aProperties && aProperties->Exists( SCH_LEGACY_PLUGIN::PropBuffering ) );
}

#endif
