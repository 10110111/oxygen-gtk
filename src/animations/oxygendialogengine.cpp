/*
* this file is part of the oxygen gtk engine
* Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
*
* This  library is free  software; you can  redistribute it and/or
* modify it  under  the terms  of the  GNU Lesser  General  Public
* License  as published  by the Free  Software  Foundation; either
* version 2 of the License, or(at your option ) any later version.
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

#include "oxygendialogengine.h"

#include <string>

namespace Oxygen
{

    //_________________________________________________________
    bool DialogEngine::registerWidget( GtkWidget* widget )
    {
        if( contains( widget ) ) return false;

        /*
        gtk_dialog_set_alternative_button_order will cause errors to be logged, but don't want these
        so register or own error handler, and then unregister afterwards...
        */
        const guint id( g_log_set_handler( "Gtk", G_LOG_LEVEL_CRITICAL, logHandler, 0L ) );

        // change order
        gtk_dialog_set_alternative_button_order( GTK_DIALOG( widget ),
            GTK_RESPONSE_HELP,
            GTK_RESPONSE_OK,
            GTK_RESPONSE_YES,
            GTK_RESPONSE_ACCEPT,
            GTK_RESPONSE_APPLY,
            GTK_RESPONSE_REJECT,
            GTK_RESPONSE_CLOSE,
            GTK_RESPONSE_NO,
            GTK_RESPONSE_CANCEL,
            -1 );

        // reset error handler
        g_log_remove_handler( "Gtk", id );
        g_log_set_handler( "Gtk", G_LOG_LEVEL_CRITICAL, g_log_default_handler, 0L );

        // insert in set
        _data.insert( widget );

        // call base class
        BaseEngine::registerWidget( widget );
        return true;

    }

    //_________________________________________________________
    void DialogEngine::logHandler( const gchar* domain, GLogLevelFlags flags, const gchar* message, gpointer data )
    {

        /*
        discard all messages containing "gtk_box_reorder_child:"
        and fallback to default handler otherwise
        */
        if( std::string( message ).find( "gtk_box_reorder_child" ) == std::string::npos )
        { g_log_default_handler( domain, flags, message, data ); }

    }

}
