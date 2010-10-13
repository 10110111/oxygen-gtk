/*
* this file is part of the oxygen gtk engine
* Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
* Copyright (c) 2010 Ruslan Kabatsayev <b7.10110111@gmail.com>
*
* based on the Null Theme Engine for Gtk+.
* Copyright (c) 2008 Robert Staudinger
*
* This  library is free  software; you can  redistribute it and/or
* modify it  under  the terms  of the  GNU Lesser  General  Public
* License  as published  by the Free  Software  Foundation; either
* version 2 of the License, or( at your option ) any later version.
*
* This library is distributed  in the hope that it will be useful,
* but  WITHOUT ANY WARRANTY; without even  the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License  along  with  this library;  if not,  write to  the Free
* Software Foundation, Inc., 51  Franklin St, Fifth Floor, Boston,
* MA 02110-1301, USA.
*/

#include "config.h"
#include "oxygenstylewrapper.h"
#include "oxygenrcstyle.h"

#include <gmodule.h>
#include <gtk/gtk.h>

#include <fstream>
#include <string>
#include <vector>

//_________________________________________________
extern "C" G_MODULE_EXPORT void theme_init( GTypeModule* );
extern "C" G_MODULE_EXPORT void theme_exit( void );
extern "C" G_MODULE_EXPORT GtkRcStyle* theme_create_rc_style( void );
extern "C" G_MODULE_EXPORT const gchar* g_module_check_init( GModule* );

//_________________________________________________
void theme_init( GTypeModule* module )
{
    oxygen_rc_style_register_type( module );
    oxygen_style_register_type( module );

    // read blacklist
    const std::string configFile( std::string( GTK_THEME_DIR ) + "/argb-apps.conf" );
    std::ifstream in( configFile.c_str() );
    if( !in )
    {
        #if OXYGEN_DEBUG
        fprintf(stderr,"Oxygen::theme_init - ARGB config file \"" << configFile << "\" not found" << std::endl;
        #endif

        return;
    }

    const char* progname = g_get_prgname();
    if( !progname ) return;

    // load options into a string
    std::string contents;
    std::getline( in, contents, '\n' );

    // split string using ":" as a delimiter
    std::vector<std::string> appNames;
    size_t position( std::string::npos );
    while( ( position = contents.find( ':' ) ) != std::string::npos )
    {
        std::string appName( contents.substr(0, position ) );
        if( !appName.empty() ) { appNames.push_back( appName ); }
        contents = contents.substr( position+1 );
    }

    if( !contents.empty() ) appNames.push_back( contents );
    if( appNames.empty() ) return;

    bool useRgba( false );
    const bool allBut( appNames.front() == "allbut" );
    if( allBut ) useRgba = true;

    for( std::vector<std::string>::const_iterator iter = appNames.begin(); iter != appNames.end(); ++iter )
    {
        if( *iter == progname )
        {
            useRgba = !allBut;
            break;
        }
    }

    #if OXYGEN_DEBUG
    std::cout << "Oxygen::init_theme - program: " << proganme << " ARGB visual is " << (useRgba ? "":"not ") << "used" << std::endl;
    #endif

    if(useRgba)
    {
        GdkColormap* cmap=gdk_screen_get_rgba_colormap(gdk_screen_get_default());
        gtk_widget_push_colormap(cmap);
        gtk_widget_set_default_colormap(cmap);
    }

}

//_________________________________________________
G_MODULE_EXPORT void theme_exit( void )
{}

//_________________________________________________
G_MODULE_EXPORT GtkRcStyle* theme_create_rc_style( void )
{ return GTK_RC_STYLE( g_object_new( OXYGEN_TYPE_RC_STYLE, NULL ) ); }

//_________________________________________________
G_MODULE_EXPORT const gchar* g_module_check_init( GModule *module )
{
    return gtk_check_version(
        GTK_MAJOR_VERSION,
        GTK_MINOR_VERSION,
        GTK_MICRO_VERSION - GTK_INTERFACE_AGE );
}
