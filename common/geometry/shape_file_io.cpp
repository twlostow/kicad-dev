/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2013 CERN
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
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

#include <string>
#include <cassert>

#include <geometry/shape.h>
#include <geometry/shape_file_io.h>
#include <geometry/shape_line_chain.h>
#include <geometry/shape_poly_set.h>

SHAPE_FILE_IO::SHAPE_FILE_IO()
{
    m_groupActive = false;
    m_mode = IOM_WRITE;
    m_file = stdout;
    m_tokens.clear();
    m_tokenPos = 0;
}

SHAPE_FILE_IO::SHAPE_FILE_IO( const std::string& aFilename, SHAPE_FILE_IO::IO_MODE aMode )
{
    m_groupActive = false;

    if( aFilename.length() )
    {
        switch( aMode )
        {
            case IOM_READ: m_file = fopen( aFilename.c_str(), "rb" ); break;
            case IOM_WRITE: m_file = fopen( aFilename.c_str(), "wb" ); break;
            case IOM_APPEND: m_file = fopen( aFilename.c_str(), "ab" ); break;
            default:
                return;
        }
    }
    else
    {
        m_file = NULL;
    }

    m_mode = aMode;
    m_tokens.clear();
    m_tokenPos = 0;
    // fixme: exceptions
}


SHAPE_FILE_IO::~SHAPE_FILE_IO()
{
    if( !m_file )
        return;

    if( m_groupActive && m_mode != IOM_READ )
        fprintf( m_file, "endgroup\n" );

    if ( m_file != stdout )
    {
        fclose( m_file );
    }
}




bool parseLineChain( SHAPE_FILE_IO::TOKEN_FUNC getToken, SHAPE_LINE_CHAIN& aShape )
{
    int n_pts = getToken().AsInt();
    aShape.SetClosed ( getToken().AsInt() );

    // Rough sanity check, just make sure the loop bounds aren't absolutely outlandish
    if( n_pts < 0 )
        return false;

    for( int i = 0; i < n_pts; i++ )
    {
        int x = getToken().AsInt();
        int y = getToken().AsInt();
        printf("x %d y %d\n", x, y);
        aShape.Append( x, y );
    }

    return true;
}


bool parsePolySet( SHAPE_FILE_IO::TOKEN_FUNC getToken, SHAPE_POLY_SET& aShape )
{
    std::string tmp;

    tmp = getToken().AsString();

    if( tmp != "polyset" )
        return false;

    int n_polys = getToken().AsInt();

    if( n_polys < 0 )
        return false;


    for( int i = 0; i < n_polys; i++ )
    {
        tmp = getToken().AsString();

        if( tmp != "poly" )
            return false;

        int n_outlines = getToken().AsInt();

        if( n_outlines < 0 )
            return false;

        aShape.NewOutline();

        for( int j = 0; j < n_outlines; j++ )
        {
            SHAPE_LINE_CHAIN outline;

            outline.SetClosed( true );

            int n_pts = getToken().AsInt();

            // Rough sanity check, just make sure the loop bounds aren't absolutely outlandish
            if( n_pts < 0 )
                return false;

            for( int i = 0; i < n_pts; i++ )
            {
                int x = getToken().AsInt();
                int y = getToken().AsInt();
                outline.Append( x, y );
            }

            if (j == 0)
                aShape.Outline(i) = outline;
            else
                aShape.AddHole( outline );
        }
    }

    return true;
}

void SHAPE_FILE_IO::readNextLine()
{
    char tmp[1024];

    if( fgets(tmp, 1024, m_file ) == nullptr )
        return;

    std::string str(tmp);
    const std::string delim = " ";

    for ( int i = 0; i < str.length(); i++)
    if(str[i] == '\n' || str[i] == '\r' || str[i] == '\t')
        str[i] = ' ';

    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == std::string::npos)
            pos = str.length();

        std::string token = str.substr(prev, pos-prev);
        if (!token.empty())
            m_tokens.push_back(token);

        prev = pos + delim.length();
    } while (pos < str.length() && prev < str.length());

}

const SHAPE_FILE_IO::VARIANT SHAPE_FILE_IO::getNextToken()
{
    if ( m_tokens.size() == 0 )
        readNextLine();

    if ( m_tokenPos == m_tokens.size() )
    {
        m_tokens.clear();
        while( m_tokens.size() == 0)
        {
            readNextLine();
            if( eof() )
                return VARIANT::Null();

        }
        m_tokenPos = 0;
    }

    if ( m_tokens.size() == 0 )
        return VARIANT::Null();

    return VARIANT( m_tokens[m_tokenPos++] );
}

bool SHAPE_FILE_IO::eof()
{
    return feof(m_file);
}

std::vector<SHAPE*> SHAPE_FILE_IO::ReadGroup()
{
    std::vector<SHAPE*> rv;

    bool groupFound = false;

    auto tokenFunc= [this] () -> VARIANT { return getNextToken(); };

    while ( !eof() )
    {
        auto t = getNextToken().AsString();

        if( t.size() > 0 )
        {
            if( t == "group" )
            {
                groupFound = true;
            }
            else if( t == "endgroup" )
            {
                return rv;
            }
            else if (t == "shape")
            {

                int type = getNextToken().AsInt();
                std::string name = getNextToken().AsString();

                switch ( type )
                {
                    case SH_LINE_CHAIN:
                    {
                        SHAPE_LINE_CHAIN *s = new SHAPE_LINE_CHAIN;
                        parseLineChain( tokenFunc, *s );
                        rv.push_back(s);
                        break;
                    }

                    case SH_POLY_SET:
                    {
                        SHAPE_POLY_SET *s = new SHAPE_POLY_SET;
                        parsePolySet( tokenFunc, *s );
                        s->CacheTriangulation();

                        //if( !s->HasHoles() )
                            //s->Unfracture( SHAPE_POLY_SET::PM_STRICTLY_SIMPLE );

                        rv.push_back(s);
                        break;
                    }

                    default:
                     break;
                }
            }
        }
    }

void SHAPE_FILE_IO::BeginGroup( const std::string& aName )
{
    assert( m_mode != IOM_READ );

    if( !m_file )
        return;

    fprintf( m_file, "group %s\n", aName.c_str() );
    m_groupActive = true;
}


void SHAPE_FILE_IO::EndGroup()
{
    assert( m_mode != IOM_READ );

    if( !m_file || !m_groupActive )
        return;

    fprintf( m_file, "endgroup\n" );
    m_groupActive = false;
}


void SHAPE_FILE_IO::Write( const SHAPE* aShape, const std::string& aName )
{
    assert( m_mode != IOM_READ );

    if( !m_file )
        return;

    if( !m_groupActive )
        fprintf( m_file,"group default\n" );

    std::string sh = aShape->Format();

    fprintf( m_file, "shape %d %s %s\n", aShape->Type(), aName.c_str(), sh.c_str() );
    fflush( m_file );
}

void SHAPE_FILE_IO::Flush()
{
    fflush( m_file );
}
