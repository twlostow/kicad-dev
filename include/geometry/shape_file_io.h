/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 CERN
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


#ifndef __SHAPE_FILE_IO_H
#define __SHAPE_FILE_IO_H

#include <cstdio>
#include <string>
#include <vector>
#include <functional>

class SHAPE;

/**
 * Class SHAPE_FILE_IO
 *
 * Helper class for saving/loading shapes from a file.
 */
class SHAPE_FILE_IO
{
    public:
        class VARIANT
        {
        public:
            VARIANT( const std::string& aStr = "", bool aNull = false ) :
                m_str ( aStr ) {
                    if( aNull )
                    {
                        m_type = VT_NULL;
                    } else {
                        m_type = VT_STRING;
                    }
                };

            enum VariantType {
                VT_STRING = 0,
                VT_INT = 1,
                VT_NULL = 2
            };

            static VARIANT Null() { return VARIANT( "", true); }

            std::string AsString() const { return m_str; }
            int AsInt() const { return atoi(m_str.c_str()); }

            bool IsNull() const { return m_type == VT_NULL; }

        private:
            std::string m_str;
            VariantType m_type;
        };

        enum IO_MODE
        {
            IOM_READ = 0,
            IOM_APPEND,
            IOM_WRITE
        };

        typedef std::function< VARIANT() > TOKEN_FUNC;

        SHAPE_FILE_IO();
        SHAPE_FILE_IO( const std::string& aFilename, IO_MODE aMode = IOM_READ );
        ~SHAPE_FILE_IO();

        void BeginGroup( const std::string& aName = "<noname>");
        void EndGroup();

        std::vector<SHAPE*> ReadGroup();

        void Write( const SHAPE* aShape, const std::string& aName = "<noname>" );

        void Write( const SHAPE& aShape, const std::string aName = "<noname>" )
        {
            Write( &aShape, aName );
        }

        void Flush();

    private:

        typedef std::vector<std::string> TOKENS;

        void readNextLine();
        const VARIANT getNextToken();

        bool eof();

        FILE* m_file;
        bool m_groupActive;
        IO_MODE m_mode;

        std::vector<std::string> m_tokens;
        int m_tokenPos;
};

#endif
