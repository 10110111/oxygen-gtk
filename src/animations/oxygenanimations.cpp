/*
* this file is part of the oxygen gtk engine
* Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
* Copyright (c) 2010 Ruslan Kabatsayev <b7.10110111@gmail.com>
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
#include "../oxygenqtsettings.h"
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
        registerEngine( _scrollBarEngine = new ScrollBarEngine( this ) );
        registerEngine( _scrolledWindowEngine = new ScrolledWindowEngine( this ) );
        registerEngine( _tabWidgetEngine = new TabWidgetEngine( this ) );
        registerEngine( _treeViewEngine = new TreeViewEngine( this ) );
        registerEngine( _widgetSizeEngine = new WidgetSizeEngine( this ) );

        // animations specific engines
        registerEngine( _widgetStateEngine = new WidgetStateEngine( this ) );
        registerEngine( _arrowStateEngine = new ArrowStateEngine( this ) );
        registerEngine( _scrollBarStateEngine = new ScrollBarStateEngine( this ) );
        registerEngine( _tabWidgetStateEngine = new TabWidgetStateEngine( this ) );
        registerEngine( _treeViewStateEngine = new TreeViewStateEngine( this ) );
        registerEngine( _menuBarStateEngine = new MenuBarStateEngine( this ) );
        registerEngine( _menuStateEngine = new MenuStateEngine( this ) );
        registerEngine( _toolBarStateEngine = new ToolBarStateEngine( this ) );

    }

    //____________________________________________________________________________________________
    Animations::~Animations( void )
    {

        // delete all engines
        for( BaseEngine::List::iterator iter = _engines.begin(); iter != _engines.end(); ++iter )
        { delete *iter; }

        // clear hooks
        _comboBoxHook.disconnect();
        _backgroundHintHook.disconnect();

        // FIXME: should we also clear the allWidgets list ?

    }

    //_________________________________________
    void Animations::initialize( const QtSettings& settings )
    {

        const bool animationsEnabled( settings.animationsEnabled() );

        // pass animations configuration to engines
        widgetStateEngine().setEnabled( animationsEnabled && settings.genericAnimationsEnabled() );
        widgetStateEngine().setDuration( settings.genericAnimationsDuration() );

        arrowStateEngine().setEnabled( animationsEnabled && settings.genericAnimationsEnabled() );
        arrowStateEngine().setDuration( settings.genericAnimationsDuration() );

        scrollBarStateEngine().setEnabled( animationsEnabled && settings.genericAnimationsEnabled() );
        scrollBarStateEngine().setDuration( settings.genericAnimationsDuration() );

        tabWidgetStateEngine().setEnabled( animationsEnabled && settings.genericAnimationsEnabled() );
        tabWidgetStateEngine().setDuration( settings.genericAnimationsDuration() );

        treeViewStateEngine().setEnabled( animationsEnabled && settings.genericAnimationsEnabled() );
        treeViewStateEngine().setDuration( settings.genericAnimationsDuration() );

        menuBarStateEngine().setAnimationsEnabled( animationsEnabled && (settings.menuBarAnimationType() != None) );
        menuBarStateEngine().setFollowMouse( settings.menuBarAnimationType() == FollowMouse );
        menuBarStateEngine().setDuration( settings.menuBarAnimationsDuration() );
        menuBarStateEngine().setFollowMouseAnimationsDuration( settings.menuBarFollowMouseAnimationsDuration() );

        menuStateEngine().setEnabled( animationsEnabled && (settings.menuAnimationType() != None) );
        menuStateEngine().setFollowMouse( settings.menuAnimationType() == FollowMouse );
        menuStateEngine().setDuration( settings.menuAnimationsDuration() );
        menuStateEngine().setFollowMouseAnimationsDuration( settings.menuFollowMouseAnimationsDuration() );

        // for now, only Fade animations mode is supported for toolbar animations
        toolBarStateEngine().setEnabled( animationsEnabled && (settings.toolBarAnimationType() != None) );
        toolBarStateEngine().setFollowMouse(  settings.toolBarAnimationType() == FollowMouse );
        toolBarStateEngine().setDuration( settings.genericAnimationsDuration() );
        toolBarStateEngine().setFollowMouseAnimationsDuration( settings.toolBarAnimationsDuration() );

    }

    //____________________________________________________________________________________________
    void Animations::initializeHooks( void )
    {
        if( _hooksInitialized ) return;

        _comboBoxHook.connect( "size-allocate", (GSignalEmissionHook)comboBoxHook, this );
        _backgroundHintHook.connect( "realize", (GSignalEmissionHook)backgroundHintHook, this );

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
    gboolean Animations::comboBoxHook( GSignalInvocationHint*, guint, const GValue* params, gpointer data )
    {

        // get widget from params
        GtkWidget* widget( GTK_WIDGET( g_value_get_object( params ) ) );

        // check type
        if( !GTK_IS_WIDGET( widget ) ) return false;
        if( !GTK_IS_ENTRY( widget ) ) return true;

        // get parent combobox
        #if ENABLE_COMBOBOX_ENTRY_HACK
        GtkWidget* parent( Gtk::gtk_parent_combobox_entry( widget ) );
        if( !parent ) return true;

        // TODO: remove when bug is fixed upstream
        GtkAllocation rect( Gtk::gtk_widget_get_allocation( widget ) );
        GtkAllocation parentRect( Gtk::gtk_widget_get_allocation( parent ) );

        if( rect.y != parentRect.y )
        {
            GtkAllocation newRect( Gtk::gdk_rectangle( rect.x, parentRect.y, rect.width, parentRect.height ) );
            gtk_widget_size_allocate( widget, &newRect );
        }
        #endif

        return true;

    }

    //____________________________________________________________________________________________
    gboolean Animations::backgroundHintHook( GSignalInvocationHint*, guint, const GValue* params, gpointer data )
    {
        // get widget from params
        GtkWidget* widget( GTK_WIDGET( g_value_get_object( params ) ) );

        // check type
        if( !GTK_IS_WIDGET( widget ) ) return FALSE;
        if( !GTK_IS_WINDOW( widget ) ) return TRUE;

        Animations& animations( *static_cast<Animations*>(data) );
        animations.backgroundHintEngine().registerWidget( widget );

        return TRUE;
    }

}
