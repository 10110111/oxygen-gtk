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

#include "oxygenanimations.h"
#include "../oxygengtkutils.h"
#include "../config.h"

#include <cassert>
#include <iostream>

namespace Oxygen
{

    //_________________________________________
    Animations::Animations( void ):
        _enabled( true ),
        _hooksInitialized( false )
    {

        // create engines
        registerEngine( _backgroundHintEngine = new BackgroundHintEngine( this ) );
        registerEngine( _comboBoxEngine = new ComboBoxEngine( this ) );
        registerEngine( _comboBoxEntryEngine = new ComboBoxEntryEngine( this ) );
        registerEngine( _dialogEngine = new DialogEngine( this ) );
        registerEngine( _hoverEngine = new HoverEngine( this ) );
        registerEngine( _mainWindowEngine = new MainWindowEngine( this ) );
        registerEngine( _menuShellEngine = new MenuShellEngine( this ) );
        registerEngine( _scrollBarEngine = new ScrollBarEngine( this ) );
        registerEngine( _scrolledWindowEngine = new ScrolledWindowEngine( this ) );
        registerEngine( _tabWidgetEngine = new TabWidgetEngine( this ) );
        registerEngine( _treeViewEngine = new TreeViewEngine( this ) );
        registerEngine( _widgetSizeEngine = new WidgetSizeEngine( this ) );

    }

    //____________________________________________________________________________________________
    Animations::~Animations( void )
    {

        // delete all engines
        for( BaseEngine::List::iterator iter = _engines.begin(); iter != _engines.end(); ++iter )
        { delete *iter; }

        // clear hooks
        _comboBoxHook.disconnect();

        // FIXME: should we also clear the allWidgets list ?

    }

    //____________________________________________________________________________________________
    void Animations::initializeHooks( void )
    {
        if( _hooksInitialized ) return;

        # if ENABLE_COMBOBOX_LIST_RESIZE
        _comboBoxHook.connect( "size-allocate", (GSignalEmissionHook)comboBoxHook, this );
        #endif

        _hooksInitialized = true;
    }

    //____________________________________________________________________________________________
    bool Animations::registerWidget( GtkWidget* widget )
    {

        if( _allWidgets.find( widget ) != _allWidgets.end() ) return false;

        #if OXYGEN_DEBUG
        std::cerr << "Oxygen::Animations::registerWidget - " << widget << " (" << G_OBJECT_TYPE_NAME( widget ) << ")" << std::endl;
        #endif

        WidgetData data;
        data._destroyId.connect( G_OBJECT( widget ), "destroy", G_CALLBACK( destroyNotifyEvent ), this );
        data._styleChangeId.connect( G_OBJECT( widget ), "style-set", G_CALLBACK( styleChangeNotifyEvent ), this );
        _allWidgets.insert( std::make_pair( widget, data ) );
        return true;

    }

    //____________________________________________________________________________________________
    void Animations::unregisterWidget( GtkWidget* widget )
    {

        #if OXYGEN_DEBUG
        std::cerr << "Oxygen::Animations::unregisterWidget - " << widget << " (" << G_OBJECT_TYPE_NAME( widget ) << ")" << std::endl;
        #endif

        // find in map
        WidgetMap::iterator iter( _allWidgets.find( widget ) );
        assert( iter != _allWidgets.end() );

        // disconnect signal
        iter->second._destroyId.disconnect();
        iter->second._styleChangeId.disconnect();

        // erase from map
        _allWidgets.erase( widget );

        // erase from all maps
        for( BaseEngine::List::iterator iter = _engines.begin(); iter != _engines.end(); ++iter )
        { (*iter)->unregisterWidget( widget ); }

    }

    //____________________________________________________________________________________________
    void Animations::setEnabled( bool value )
    {
        if( value == _enabled ) return;
        _enabled = value;
        for( BaseEngine::List::iterator iter = _engines.begin(); iter != _engines.end(); ++iter )
        { (*iter)->setEnabled( value ); }
    }

    //____________________________________________________________________________________________
    gboolean Animations::destroyNotifyEvent( GtkWidget* widget, gpointer data )
    {
        static_cast<Animations*>(data)->unregisterWidget( widget );
        return FALSE;
    }

    //____________________________________________________________________________________________
    void Animations::styleChangeNotifyEvent( GtkWidget* widget, GtkStyle*, gpointer data )
    { static_cast<Animations*>(data)->unregisterWidget( widget ); }

    //____________________________________________________________________________________________
    gboolean Animations::comboBoxHook( GSignalInvocationHint*, guint, const GValue* params, gpointer data )
    {

        // get widget from params
        GtkWidget* widget( GTK_WIDGET( g_value_get_object( params ) ) );

        // check type
        if( !GTK_IS_WIDGET( widget ) ) return FALSE;
        if( !GTK_IS_WINDOW( widget ) ) return TRUE;

        GtkWindow* window( GTK_WINDOW( widget ) );
        if( gtk_window_get_type_hint( window ) != GDK_WINDOW_TYPE_HINT_COMBO ) return TRUE;

        Animations& animations( *static_cast<Animations*>(data) );
        GtkWidget* combobox = animations.comboBoxEngine().find( widget );
        if( !combobox ) combobox = animations.comboBoxEntryEngine().find( widget );
        if( !combobox ) return true;

        int w, h;
        gtk_window_get_size( window, &w, &h );

        gint targetX, dummy, y;
        gtk_window_get_position( window, &dummy, &y );
        gdk_window_get_origin(combobox->window, &targetX, &dummy);
        gtk_window_move( window, targetX+combobox->allocation.x+3, y );

        gtk_widget_set_size_request( widget, combobox->allocation.width - 6, widget->allocation.height );

        return TRUE;

    }

}
