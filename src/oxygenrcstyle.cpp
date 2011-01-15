/*
* this file is part of the oxygen gtk engine
* Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
*
* based on the 0L Theme Engine for Gtk+.
* Copyright (c) 2008 Robert Staudinger <robert.staudinger@gmail.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or( at your option ) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free
* Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
* MA 02110-1301, USA.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "oxygenrcstyle.h"
#include "oxygenstyle.h"
#include "oxygenstylewrapper.h"

#include <gtk/gtk.h>

namespace Oxygen
{

    //______________________________________________________________________
    GType RCStyle::_type = 0L;
    GTypeInfo RCStyle::_typeInfo;

    //______________________________________________________________________
    static GtkStyle* create_style( GtkRcStyle *rc_style )
    { return GTK_STYLE( g_object_new( OXYGEN_TYPE_STYLE, 0L ) ); }

    //______________________________________________________________________
    static guint parse(
        GtkRcStyle *rc_style,
        GtkSettings *settings,
        GScanner *scanner )
    {
        static GQuark scope_id = 0;
        guint old_scope;
        guint token;

        if( !scope_id )
        { scope_id = g_quark_from_string( "oxygen_engine" ); }

        old_scope = g_scanner_set_scope( scanner, scope_id );

        token = g_scanner_peek_next_token( scanner );
        while( token != G_TOKEN_RIGHT_CURLY )
        {
            token = g_scanner_peek_next_token( scanner );
            if( token != G_TOKEN_NONE ) return token;
        }

        g_scanner_get_next_token( scanner );
        g_scanner_set_scope( scanner, old_scope );

        return G_TOKEN_NONE;
    }

    //______________________________________________________________________
    static GtkRcStyleClass *oxygen_rc_style_parent_class = 0L;
    static void merge( GtkRcStyle *dst, GtkRcStyle *src )
    {
        if( oxygen_rc_style_parent_class )
        { oxygen_rc_style_parent_class->merge( dst, src ); }
    }

    //______________________________________________________________________
    void RCStyle::instanceInit( OxygenRcStyle* )
    {}

    //______________________________________________________________________
    void RCStyle::classInit( OxygenRcStyleClass *klass )
    {
        GtkRcStyleClass *rc_style_class = GTK_RC_STYLE_CLASS( klass );

        oxygen_rc_style_parent_class = static_cast<GtkRcStyleClass*>(g_type_class_peek_parent( klass ) );

        rc_style_class->create_style = create_style;
        rc_style_class->parse = parse;
        rc_style_class->merge = merge;
    }

    //______________________________________________________________________
    void RCStyle::registerType( GTypeModule *module )
    {

        if( !_type )
        {
            GTypeInfo info =
            {
                (guint16)sizeof(OxygenRcStyleClass ),
                (GBaseInitFunc) NULL,
                (GBaseFinalizeFunc) NULL,
                (GClassInitFunc) classInit,
                (GClassFinalizeFunc) NULL,
                NULL,
                (guint16)sizeof( OxygenRcStyle ),
                0,
                (GInstanceInitFunc) instanceInit,
                NULL
            };

            _typeInfo = info;
            _type = g_type_module_register_type( module, GTK_TYPE_RC_STYLE, "OxygenRcStyle", &_typeInfo, GTypeFlags(0 ) );
        }
    }

    //______________________________________________________________________
    GType RCStyle::type( void )
    { return _type; }

}
