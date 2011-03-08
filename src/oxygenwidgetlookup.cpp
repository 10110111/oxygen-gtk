/*
* this file is part of the oxygen gtk engine
* Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
* Copyright (c) 2010 Ruslan Kabatsayev <b7.10110111@gmail.com>
*
* Hook-setup code provided by Ruslan
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

#include "oxygenwidgetlookup.h"
#include "config.h"

#include <iostream>
#include <cairo.h>
#include <cairo-gobject.h>

namespace Oxygen
{

    //__________________________________________________________________
    WidgetLookup::WidgetLookup( void ):
        _hooksInitialized( false )
    {}


    //_____________________________________________________
    WidgetLookup::~WidgetLookup( void )
    {

        // disconnect hooks
        _drawHook.disconnect();

    }

    //_____________________________________________________
    void WidgetLookup::initializeHooks( void )
    {

        if( _hooksInitialized ) return;

        // lookup relevant signal
        const guint signalId( g_signal_lookup("draw", GTK_TYPE_WIDGET ) );
        if( signalId <= 0 ) return;

        // install hooks
        _drawHook.connect( "draw", (GSignalEmissionHook)drawHook, this );
        _hooksInitialized = true;

        return;

    }

    //_____________________________________________________
    gboolean WidgetLookup::drawHook( GSignalInvocationHint*, guint numParams, const GValue* params, gpointer )
    {

        // check number of parameters
        if( numParams < 2 ) return FALSE;

        // get widget from params
        GtkWidget* widget( GTK_WIDGET( g_value_get_object( params ) ) );

        // check type
        if( !GTK_IS_WIDGET( widget ) ) return FALSE;

        // check second parameter type.
        if( !G_VALUE_HOLDS( params+1, CAIRO_GOBJECT_TYPE_CONTEXT ) )
        { return FALSE; }

        // retrieve context and cast
        cairo_t* context( static_cast<cairo_t*>( g_value_get_boxed(params+1) ) );

        #if OXYGEN_DEBUG
        std::cerr
            << "Oxygen::WidgetLookup::drawHook -"
            << " widget: " << widget << " (" << G_OBJECT_TYPE_NAME( widget ) << ")"
            << " context: " << context
            << std::endl;
        #endif

        return TRUE;

    }

}
