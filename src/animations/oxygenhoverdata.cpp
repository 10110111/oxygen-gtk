/*
* this file is part of the oxygen gtk engine
* Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
*
* the hover data code is largely inspired from the gtk redmond engine
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

#include "oxygenhoverdata.h"
#include "../oxygengtkutils.h"

#include <gtk/gtk.h>
#include <iostream>

namespace Oxygen
{

    //________________________________________________________________________________
    void HoverData::connect( GtkWidget* widget )
    {
        // on connection, needs to check whether mouse pointer is in widget or not
        // to have the proper initial value of the hover flag
        gint xPointer,yPointer;
        gdk_window_get_pointer(widget->window,&xPointer,&yPointer, 0L);
        setHovered( widget, Gtk::gdk_rectangle_contains( &widget->allocation, xPointer, yPointer ) );

        // register callbacks
        _enterId = g_signal_connect( G_OBJECT(widget), "enter-notify-event", G_CALLBACK( enterNotifyEvent ), this );
        _leaveId = g_signal_connect( G_OBJECT(widget), "leave-notify-event", G_CALLBACK( leaveNotifyEvent ), this );
    }

    //________________________________________________________________________________
    void HoverData::disconnect( GtkWidget* widget )
    {
        g_signal_handler_disconnect( G_OBJECT(widget), _enterId );
        g_signal_handler_disconnect( G_OBJECT(widget), _leaveId );
    }

    //________________________________________________________________________________
    gboolean HoverData::enterNotifyEvent(GtkWidget* widget, GdkEventCrossing*, gpointer data )
    {
        static_cast<HoverData*>( data )->setHovered( widget, true );
        return FALSE;
    }

    //________________________________________________________________________________
    gboolean HoverData::leaveNotifyEvent( GtkWidget* widget, GdkEventCrossing*, gpointer data )
    {
        static_cast<HoverData*>( data )->setHovered( widget, false );
        return FALSE;
    }

}
