/*
 * Copyright (C) 2018 CERN
 * Author: Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CAIRO_PRINT_H_
#define _CAIRO_PRINT_H_

#include <gal/cairo/cairo_gal.h>

class wxDC;
class wxGCDC;

namespace KIGFX
{
/**
 * CAIRO_PRINT_CTX provides a Cairo context created from wxPrintDC.
 * It allows one to prepare printouts using the Cairo library and let wxWidgets handle the rest.
 */
class CAIRO_PRINT_CTX
{
public:
    CAIRO_PRINT_CTX( wxDC* aDC );
    ~CAIRO_PRINT_CTX();

    cairo_t* GetContext() const
    {
        return m_ctx;
    }

    cairo_surface_t* GetSurface() const
    {
        return m_surface;
    }

private:
    wxGCDC* m_gcdc;
    cairo_t* m_ctx;
    cairo_surface_t* m_surface;

#ifdef __WXMSW__
    void* m_hdc;
#endif /* __WXMSW__ */
};


class CAIRO_PRINT_GAL : public CAIRO_GAL_BASE
{
public:
    CAIRO_PRINT_GAL( GAL_DISPLAY_OPTIONS& aDisplayOptions,
            cairo_t* aContext, cairo_surface_t* aSurface );
};
} // namespace KIGFX

#endif /* _CAIRO_PRINT_H_ */


