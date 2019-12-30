/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 KiCad Developers, see CHANGELOG.TXT for contributors.
 * @author Kristoffer Ödmark
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


#ifndef __PCB_STRING_IO_H
#define __PCB_STRING_IO_H

#include <kicad_plugin.h>
#include <class_board_item.h>
#include <class_module.h>
#include <class_board.h>
#include <pcb_parser.h>
#include <memory.h>
#include <build_version.h>


class PCB_STRING_IO : public PCB_IO
{

public:
    /* Saves the entire board to the clipboard formatted using the PCB_IO formatting */
    void Save( const wxString& aFileName, BOARD* aBoard,
                const PROPERTIES* aProperties = NULL ) override;
    /* Writes all the settings of the BOARD* set by setBoard() and then adds all
     * the BOARD_ITEM* found in selection formatted by PCB_IO to clipboard as a text
     */

    PCB_STRING_IO();
    ~PCB_STRING_IO();

    void SetBoard( BOARD* aBoard );
    STRING_FORMATTER* GetFormatter();

    static std::string FormatBoard( BOARD* aBoard )
    {
        std::unique_ptr<PCB_STRING_IO> io (new PCB_STRING_IO);
        io->SetBoard( aBoard );
        io->Save( "", aBoard, nullptr );
        return io->m_str;
    }

private:
    void writeHeader( BOARD* aBoard );

    std::string m_str;
    STRING_FORMATTER m_formatter;
};

#endif
