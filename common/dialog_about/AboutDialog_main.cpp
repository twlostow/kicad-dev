/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2010 Rafael Sokolowski <Rafael.Sokolowski@web.de>
 * Copyright (C) 2010-2018 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <boost/version.hpp>
#include <wx/aboutdlg.h>
#include <wx/arrimpl.cpp>
#include <wx/textctrl.h>
#include <wx/utils.h>

/* Used icons:
 *  lang_xx_xpm;      // Icons of various national flags
 *  show_3d_xpm;      // 3D icon
 *  edit_module_xpm;
 *  icon_kicad_xpm;   // Icon of the application
 */
#include <bitmaps.h>
#include <build_version.h>
#include <common.h>
#include <pgm_base.h>
#include <eda_base_frame.h>

#include "aboutinfo.h"
#include "dialog_about.h"


WX_DEFINE_OBJARRAY( CONTRIBUTORS )

// Helper functions:
static wxString HtmlHyperlink( const wxString& url, const wxString& description = wxEmptyString );
static wxString HtmlNewline( const unsigned int amount = 1 );


/**
 * Initializes the <code>ABOUT_APP_INFO</code> object with application specific information.
 * This is the object which holds all information about the application
 */
void ABOUT_APP_INFO::Build( EDA_BASE_FRAME* aParent )
{
    // Set application specific icon

    if ( aParent )
        SetAppIcon( aParent->GetIcon() );

    /* Set title */
    SetAppName( Pgm().App().GetAppName() );

    /* Copyright information */
    SetCopyright( "(C) 1992-2018 KiCad Developers Team" );

    /* KiCad build version */
    wxString version;
    version << ::GetBuildVersion()
#ifdef DEBUG
            << ", debug"
#else
            << ", release"
#endif
            << " build";

    SetBuildVersion( version );

    /* wxWidgets version */
    wxString libVersion;
    libVersion << wxGetLibraryVersionInfo().GetVersionString();

    /* Unicode or ANSI version */
#if wxUSE_UNICODE
    libVersion << wxT( " Unicode " );
#else
    libVersion << wxT( " ANSI " );
#endif

    // Just in case someone builds KiCad with the platform native of Boost instead of
    // the version included with the KiCad source.
    libVersion << "and Boost " << ( BOOST_VERSION / 100000 ) << "."
               << ( BOOST_VERSION / 100 % 1000 ) << "." << ( BOOST_VERSION % 100 )
               << "\n";

    // Operating System Information

    wxPlatformInfo platformInfo;

    libVersion << "Platform: " << wxGetOsDescription() << ", " << platformInfo.GetArchName();

    SetLibVersion( libVersion );


    /* info/description part HTML formatted */

    wxString description;

    /* short description */
    description << "<p>";
    description << "<b><u>"
                << _( "Description" )
                << "</u></b>"; // bold & underlined font for caption

    description << "<p>"
                << _( "The KiCad EDA Suite is a set of open source applications for the "
                      "creation of electronic schematics and printed circuit boards." )
                << "</p>";

    description << "</p>";

    /* websites */
    description << "<p><b><u>"
                << _( "KiCad on the web" )
                << "</u></b>"; // bold & underlined font for caption

    // bullet-ed list with some http links
    description << "<ul>";
    description << "<li>"
                << _( "The official KiCad website - " )
                << HtmlHyperlink( "http://www.kicad-pcb.org" )
                << "</li>";
    description << "<li>"
                << _( "Developer website on Launchpad - " )
                << HtmlHyperlink( "https://launchpad.net/kicad" )
                << "</li>";

    description << "<li>"
                << _("Official KiCad library repositories - " )
                << HtmlHyperlink( "https://kicad.github.io" )
                << "</li>";

    description << "</ul></p>";

    description << "<p><b><u>"
                << _( "Bug tracker" )
                << "</u></b>"; // bold & underlined font caption

    // bullet-ed list with some http links
    description << "<ul>";
    description << "<li>"
                << _( "Report or examine bugs - " )
                << HtmlHyperlink( "https://bugs.launchpad.net/kicad/+bugs?orderby=-id&start=0",
                                  "https://bugs.launchpad.net/kicad" )
                << "</li>";
    description << "</ul></p>";

    description << "<p><b><u>"
                << _( "KiCad user's groups and community" )
                << "</u></b>"; // bold & underlined font caption

    description << "<ul>";
    description << "<li>"
                << _( "KiCad forum - " )
                << HtmlHyperlink( "https://forum.kicad.info" )
                << "</li>";

    description << "<li>"
                <<_(  "KiCad user's group - " )
                << HtmlHyperlink( "https://groups.yahoo.com/neo/groups/kicad-users/info" )
                << "</li>";

    description << "</ul></p>";

    SetDescription( description );


    // License information also HTML formatted:
    wxString license;
    license
        << "<div align='center'>"
        << HtmlNewline( 4 )
        << _( "The complete KiCad EDA Suite is released under the" ) << HtmlNewline( 2 )
        << HtmlHyperlink( "http://www.gnu.org/licenses",
                          _( "GNU Affero General Public License (AGPL) version 3 or any later version" ) )
        << "</div>";

    SetLicense( license );

    if( !aParent )
        return;


    /* A contributor consists of the following information:
     * Mandatory:
     * - Name
     * Optional:
     * - EMail address
     * - Category
     * - Category specific icon
     *
     * All contributors of the same category will be enumerated under this category
     * which should be represented by the same icon.
     */

    // The core developers
    AddDeveloper( new CONTRIBUTOR( "Jean-Pierre Charras" ) );
    AddDeveloper( new CONTRIBUTOR( "Dick Hollenbeck" ) );
    AddDeveloper( new CONTRIBUTOR( "Wayne Stambaugh" ) );

    // alphabetically by last name after main 3 above:
    AddDeveloper( new CONTRIBUTOR( "Frank Bennett" ) );
    AddDeveloper( new CONTRIBUTOR( "Cirilo Bernardo" ) );
    AddDeveloper( new CONTRIBUTOR( "Kevin Cozens" ) );
    AddDeveloper( new CONTRIBUTOR( "Jonas Diemer" ) );
    AddDeveloper( new CONTRIBUTOR( "Jon Evans" ) );
    AddDeveloper( new CONTRIBUTOR( "Seth Hillbrand" ) );
    AddDeveloper( new CONTRIBUTOR( wxT( "Torsten Hüter" ) ) );
    AddDeveloper( new CONTRIBUTOR( "Jerry Jacobs" ) );
    AddDeveloper( new CONTRIBUTOR( "Mario Luzeiro" ) );
    AddDeveloper( new CONTRIBUTOR( "Daniel Majewski" ) );
    AddDeveloper( new CONTRIBUTOR( "Lorenzo Marcantonio" ) );
    AddDeveloper( new CONTRIBUTOR( "Marco Mattila" ) );
    AddDeveloper( new CONTRIBUTOR( "Russell Oliver" ) );
    AddDeveloper( new CONTRIBUTOR( "Alexis Lockwood" ) );
    AddDeveloper( new CONTRIBUTOR( "Miguel Angel Ajo Pelayo" ) );
    AddDeveloper( new CONTRIBUTOR( "Jacobo Aragunde Perez" ) );
    AddDeveloper( new CONTRIBUTOR( "Simon Richter" ) );
    AddDeveloper( new CONTRIBUTOR( "Mark Roszko" ) );
    AddDeveloper( new CONTRIBUTOR( "Marco Serantoni" ) );
    AddDeveloper( new CONTRIBUTOR( "Brian Sidebotham" ) );
    AddDeveloper( new CONTRIBUTOR( wxT( "Mateusz Skowroński" ) ) );
    AddDeveloper( new CONTRIBUTOR( "Rafael Sokolowski" ) );
    AddDeveloper( new CONTRIBUTOR( "Vesa Solonen" ) );
    AddDeveloper( new CONTRIBUTOR( "Bernhard Stegmaier" ) );
    AddDeveloper( new CONTRIBUTOR( wxT( "Orson (Maciej Sumiński)" ) ) );
    AddDeveloper( new CONTRIBUTOR( "Oliver Walters" ) );
    AddDeveloper( new CONTRIBUTOR( "Tomasz Wlostowski" ) );
    AddDeveloper( new CONTRIBUTOR( "Adam Wolf" ) );
    AddDeveloper( new CONTRIBUTOR( "Jeff Young" ) );
    AddDeveloper( new CONTRIBUTOR( "Alexander Zakamaldin" ) );
    AddDeveloper( new CONTRIBUTOR( "Henner Zeller" ) );
    AddDeveloper( new CONTRIBUTOR( "Andrew Zonenberg" ) );
    AddDeveloper( new CONTRIBUTOR( wxT( "Nick Østergaard" ) ) );

    // The document writers
    AddDocWriter( new CONTRIBUTOR( "Jean-Pierre Charras" ) );
    AddDocWriter( new CONTRIBUTOR( "Marco Ciampa" ) );
    AddDocWriter( new CONTRIBUTOR( "Dick Hollenbeck" ) );
    AddDocWriter( new CONTRIBUTOR( "Igor Plyatov" ) );
    AddDocWriter( new CONTRIBUTOR( "Wayne Stambaugh" ) );
    AddDocWriter( new CONTRIBUTOR( "Fabrizio Tappero" ) );

    /* The translators
     * As category the language to which the translation was done is used
     * and as icon the national flag of the corresponding country.
     */
    AddTranslator( new CONTRIBUTOR( "Robert Buj",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Catalan (CA)",
                                          CreateKiBitmap( lang_ca_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( wxT( "Martin Kratoška" ),
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Czech (CZ)",
                                          CreateKiBitmap( lang_cs_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Jerry Jacobs",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Dutch (NL)",
                                          CreateKiBitmap( lang_nl_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Vesa Solonen",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Finnish (FI)",
                                          CreateKiBitmap( lang_fi_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Jean-Pierre Charras",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "French (FR)",
                                          CreateKiBitmap( lang_fr_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( wxT( "Mateusz Skowroński" ),
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Polish (PL)",
                                          CreateKiBitmap( lang_pl_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Kerusey Karyu",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Polish (PL)",
                                          CreateKiBitmap( lang_pl_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Renie Marquet",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Portuguese (PT)",
                                          CreateKiBitmap( lang_pt_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Igor Plyatov",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Russian (RU)",
                                          CreateKiBitmap( lang_ru_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Andrey Fedorushkov",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Russian (RU)",
                                          CreateKiBitmap( lang_ru_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Eldar Khayrullin",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Russian (RU)",
                                         CreateKiBitmap( lang_ru_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Pedro Martin del Valle",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Spanish (ES)",
                                          CreateKiBitmap( lang_es_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( wxT( "Iñigo Zuluaga" ),
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Spanish (ES)",
                                          CreateKiBitmap( lang_es_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( wxT( "Iñigo Figuero" ),
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Spanish (ES)",
                                          CreateKiBitmap( lang_es_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Rafael Sokolowski",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "German (DE)",
                                          CreateKiBitmap( lang_de_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Kenta Yonekura",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Japanese (JA)",
                                          CreateKiBitmap( lang_jp_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Manolis Stefanis",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Greek (el_GR)",
                                          CreateKiBitmap( lang_gr_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Athanasios Vlastos",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Greek (el_GR)",
                                          CreateKiBitmap( lang_gr_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Milonas Kostas",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Greek (el_GR)",
                                          CreateKiBitmap( lang_gr_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Michail Misirlis",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Greek (el_GR)",
                                          CreateKiBitmap( lang_gr_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Massimo Cioce",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Italian (IT)",
                                          CreateKiBitmap( lang_it_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Marco Ciampa",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Italian (IT)",
                                          CreateKiBitmap( lang_it_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Evgeniy Ivanov",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Bulgarian (BG)",
                                          CreateKiBitmap( lang_bg_xpm ) ) );
    AddTranslator( new CONTRIBUTOR( "Liu Guang",
                                          wxEmptyString,
                                          wxEmptyString,
                                          "Simplified Chinese (zh_CN)",
                                          CreateKiBitmap( lang_zh_xpm ) ) );

    // Maintainer who helper in translations, but not in a specific translation
    #define OTHERS_IN_TRANSLATION _( "Others" )
    AddTranslator( new CONTRIBUTOR( "Remy Halvick",
                                          wxEmptyString,
                                          wxEmptyString,
                                          OTHERS_IN_TRANSLATION ) );
    AddTranslator( new CONTRIBUTOR( "David Briscoe",
                                          wxEmptyString,
                                          wxEmptyString,
                                          OTHERS_IN_TRANSLATION ) );
    AddTranslator( new CONTRIBUTOR( "Dominique Laigle",
                                          wxEmptyString,
                                          wxEmptyString,
                                          OTHERS_IN_TRANSLATION ) );
    AddTranslator( new CONTRIBUTOR( "Paul Burke",
                                          wxEmptyString,
                                          wxEmptyString,
                                          OTHERS_IN_TRANSLATION ) );

    // Programm credits for icons
    #define ICON_CONTRIBUTION _( "Icons by" )
    AddArtist( new CONTRIBUTOR( wxT( "Iñigo Zuluaga" ),
                                      wxEmptyString,
                                      wxEmptyString,
                                      ICON_CONTRIBUTION,
                                      CreateKiBitmap( svg_file_xpm ) ) );
    AddArtist( new CONTRIBUTOR( "Konstantin Baranovskiy",
                                      wxEmptyString,
                                      wxEmptyString,
                                      ICON_CONTRIBUTION,
                                      CreateKiBitmap( svg_file_xpm ) ) );
    AddArtist( new CONTRIBUTOR( "Fabrizio Tappero",
                                      wxEmptyString,
                                      wxEmptyString,
                                      ICON_CONTRIBUTION,
                                      CreateKiBitmap( svg_file_xpm ) ) );

    // Program credits for 3d models
    #define MODELS_3D_CONTRIBUTION _( "3D models by" )
    AddArtist( new CONTRIBUTOR( "GitHub contributors",
                                      wxEmptyString,
                                      "https://github.com/KiCad/kicad-packages3D/graphs/contributors",
                                      MODELS_3D_CONTRIBUTION,
                                      CreateKiBitmap( three_d_xpm ) ) );
    AddArtist( new CONTRIBUTOR( "Christophe Boschat",
                                      wxEmptyString,
                                      wxEmptyString,
                                      MODELS_3D_CONTRIBUTION,
                                      CreateKiBitmap( three_d_xpm ) ) );
    AddArtist( new CONTRIBUTOR( "Renie Marquet",
                                      wxEmptyString,
                                      wxEmptyString,
                                      MODELS_3D_CONTRIBUTION,
                                      CreateKiBitmap( three_d_xpm ) ) );

    #define SYMBOL_LIB_CONTRIBUTION _( "Symbols by" )
    AddArtist( new CONTRIBUTOR( "GitHub contributors",
                                      wxEmptyString,
                                      "https://github.com/KiCad/kicad-symbols/graphs/contributors",
                                      SYMBOL_LIB_CONTRIBUTION,
                                      CreateKiBitmap( new_component_xpm ) ) );

    #define FOOTPRINT_LIB_CONTRIBUTION _( "Footprints by" )
    AddArtist( new CONTRIBUTOR( "GitHub contributors",
                                      wxEmptyString,
                                      "https://github.com/KiCad/kicad-footprints/graphs/contributors",
                                      FOOTPRINT_LIB_CONTRIBUTION,
                                      CreateKiBitmap( edit_module_xpm ) ) );

    // Program credits for package developers.
    AddPackager( new CONTRIBUTOR( "Jean-Samuel Reynaud" ) );
    AddPackager( new CONTRIBUTOR( "Bernhard Stegmaier" ) );
    AddPackager( new CONTRIBUTOR( "Adam Wolf" ) );
    AddPackager( new CONTRIBUTOR( wxT( "Nick Østergaard" ) ) );
}


void ShowAboutDialog( EDA_BASE_FRAME* aParent )
{
    ABOUT_APP_INFO info;
    info.Build( aParent );

    DIALOG_ABOUT dlg( aParent, info );
    dlg.ShowModal();
}


///////////////////////////////////////////////////////////////////////////////
/// Helper functions
///////////////////////////////////////////////////////////////////////////////

/**
 * Wrap \a aUrl with a HTML anchor tag containing a hyperlink text reference
 * to form a HTML hyperlink.
 *
 * @param aUrl the url that will be embedded in an anchor tag containing a hyperlink reference
 * @param aDescription the optional describing text that will be represented as a hyperlink.
 *  If not specified the url will be used as hyperlink.
 * @return a HTML conform hyperlink like <a href='url'>description</a>
 */
static wxString HtmlHyperlink( const wxString& aUrl, const wxString& aDescription )
{
    wxString hyperlink = wxEmptyString;

    if( aDescription.IsEmpty() )
        hyperlink << "<a href='" << aUrl << "'>" << aUrl << "</a>";
    else
        hyperlink << "<a href='" << aUrl << "'>" << aDescription << "</a>";

    return hyperlink;
}


/**
 * Create an HTML newline character sequence of \a aCount.
 *
 * @param aCount the number of HTML newline tags to concatenate, default is to return just
 *               one <br> tag.
 * @return the concatenated amount of HTML newline tag(s) <br>
 */
static wxString HtmlNewline( const unsigned int aCount )
{
    wxString newlineTags = wxEmptyString;

    for( size_t i = 0; i<aCount; ++i )
        newlineTags << "<br>";

    return newlineTags;
}
