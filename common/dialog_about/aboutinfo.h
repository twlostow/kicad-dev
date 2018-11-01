/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014 Rafael Sokolowski <Rafael.Sokolowski@web.de>
 * Copyright (C) 2014-2017 KiCad Developers, see CHANGELOG.TXT for contributors.
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

#ifndef ABOUTAPPINFO_H
#define ABOUTAPPINFO_H

#include <wx/aboutdlg.h>
#include <wx/bitmap.h>
#include <wx/dynarray.h>

#include "bitmap_types.h"

class CONTRIBUTOR;
class EDA_BASE_FRAME;

WX_DECLARE_OBJARRAY( CONTRIBUTOR, CONTRIBUTORS );


/**
 * An object of this class is meant to be used to store application specific information
 * like who has contributed in which area of the application, the license, copyright
 * and other descriptive information.
 */
class ABOUT_APP_INFO
{
public:
    ABOUT_APP_INFO() {};
    virtual ~ABOUT_APP_INFO() {};

    void Build( EDA_BASE_FRAME* aFrame );

    void AddDeveloper( const CONTRIBUTOR* developer )
    {
        if( developer != NULL )
            m_developers.Add( developer );
    }

    void AddDocWriter( const CONTRIBUTOR* docwriter )
    {
        if( docwriter != NULL )
            m_docwriters.Add( docwriter );
    }

    void AddArtist( const CONTRIBUTOR* artist )
    {
        if( artist != NULL )
            m_artists.Add( artist );
    }

    void AddTranslator( const CONTRIBUTOR* translator )
    {
        if( translator != NULL )
            m_translators.Add( translator );
    }

    void AddPackager( const CONTRIBUTOR* packager )
    {
        if( packager   != NULL )
            m_packagers.Add( packager );
    }

    CONTRIBUTORS GetDevelopers()  { return m_developers; }
    CONTRIBUTORS GetDocWriters()  { return m_docwriters; }
    CONTRIBUTORS GetArtists()     { return m_artists; }
    CONTRIBUTORS GetTranslators() { return m_translators; }
    CONTRIBUTORS GetPackagers()   { return m_packagers; }

    void SetDescription( const wxString& text ) { m_description = text; }
    wxString& GetDescription() { return m_description; }

    void SetLicense( const wxString& text ) { m_license = text; }
    wxString& GetLicense() { return m_license; }

    void SetCopyright( const wxString& text ) { m_copyright = text; }
    wxString GetCopyright() { return m_copyright; }

    void SetAppName( const wxString& name ) { m_appName = name; }
    wxString& GetAppName() { return m_appName; }

    void SetBuildVersion( const wxString& version ) { m_buildVersion = version; }
    wxString& GetBuildVersion() { return m_buildVersion; }

    void SetLibVersion( const wxString& version ) { m_libVersion = version; }
    wxString& GetLibVersion() { return m_libVersion; }

    void SetAppIcon( const wxIcon& aIcon ) { m_appIcon = aIcon; }
    wxIcon& GetAppIcon() { return m_appIcon; }

    ///> Wrapper to manage memory allocation for bitmaps
    wxBitmap* CreateKiBitmap( BITMAP_DEF aBitmap )
    {
        m_bitmaps.emplace_back( KiBitmapNew( aBitmap ) );
        return m_bitmaps.back().get();
    }

private:
    CONTRIBUTORS m_developers;
    CONTRIBUTORS m_docwriters;
    CONTRIBUTORS m_artists;
    CONTRIBUTORS m_translators;
    CONTRIBUTORS m_packagers;

    wxString     m_description;
    wxString     m_license;

    wxString     m_copyright;
    wxString     m_appName;
    wxString     m_buildVersion;
    wxString     m_libVersion;

    wxIcon       m_appIcon;

    ///> Bitmaps to be freed when the dialog is closed
    std::vector<std::unique_ptr<wxBitmap>> m_bitmaps;
};


/**
 * A contributor, a person which was involved in the development of the application
 * or which has contributed in any kind somehow to the project.
 *
 * A contributor consists of the following mandatory information:
 * - Name
 *
 * Each contributor can have optional information assigned like:
 * - EMail address
 * - A category
 * - A category specific icon
 */
class CONTRIBUTOR
{
public:
    CONTRIBUTOR( const wxString& aName,
                 const wxString& aEmail = wxEmptyString,
                 const wxString& aUrl = wxEmptyString,
                 const wxString& aCategory = wxEmptyString,
                 wxBitmap*       aIcon = NULL )
    {
        m_checked = false;
        m_name = aName;
        m_url = aUrl,
        m_email = aEmail;
        m_category = aCategory;
        m_icon = aIcon;
    }

    virtual ~CONTRIBUTOR() {}

    wxString& GetName()     { return m_name; }
    wxString& GetEMail()    { return m_email; }
    wxString& GetUrl()      { return m_url; }
    wxString& GetCategory() { return m_category; }
    wxBitmap* GetIcon()     { return m_icon; }
    void SetChecked( bool status ) { m_checked = status; }
    bool IsChecked() { return m_checked; }

private:
    wxString  m_name;
    wxString  m_email;
    wxString  m_url;
    wxString  m_category;
    wxBitmap* m_icon;
    bool      m_checked;
};

#endif // ABOUTAPPINFO_H
