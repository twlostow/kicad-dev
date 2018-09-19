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

#ifndef DEBUG_REPORT_H_
#define DEBUG_REPORT_H_

#include <wx/debugrpt.h>

/**
 * Class DEBUG_REPORT
 *
 * Based on wxDebugReportCompress but with a improved context
 * saver which gives us useful stack-traces. Furthermore it include
 * additional information which are helpful for debugging a crash.
 */
class DEBUG_REPORT : public wxDebugReportCompress
{
public:
    DEBUG_REPORT()
    {
    }

    /**
     * Function GenerateReport
     *
     * generate a KiCad debug report and displays it to the user
     *
     * @param ctx Context for which the report should be generated
     */
    static void GenerateReport( Context ctx );

    bool AddTimestamp();

#if !wxCHECK_VERSION( 3, 1, 2 ) && wxUSE_STACKWALKER
    // in case of wx <= 3.1.1 important stack information were not saved
    virtual bool AddContext( Context ctx ) override;
#endif
};

#endif