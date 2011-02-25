/*
* this file is part of the oxygen gtk engine
* Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
* Copyright (c) 2010 Ruslan Kabatsayev <b7.10110111@gmail.com>
*
* MenuState prelight effect is based on
* Redmond95 - a cairo based GTK+ engine
* Copyright (C) 2001 Red Hat, Inc. <@redhat.com>
* Copyright (C) 2006 Andrew Johnson <acjgenius@earthlink.net>
* Copyright (C) 2006-2007 Benjamin Berg <benjamin@sipsolutions.net>
*
* the menushell data code is largely inspired from the gtk redmond engine
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

#include "oxygenmenustatedata.h"
#include "../oxygengtkutils.h"

#include <gtk/gtk.h>

namespace Oxygen
{

    //________________________________________________________________________________
    void MenuStateData::connect( GtkWidget* widget )
    {

        _target = widget;
        _motionId.connect( G_OBJECT(widget), "motion-notify-event", G_CALLBACK( motionNotifyEvent ), this );
        _leaveId.connect( G_OBJECT(widget), "leave-notify-event", G_CALLBACK( leaveNotifyEvent ), this );

        // connect timeLines
        _current._timeLine.connect( (GSourceFunc)delayedUpdate, this );
        _previous._timeLine.connect( (GSourceFunc)delayedUpdate, this );

        // set directions
        _current._timeLine.setDirection( TimeLine::Forward );
        _previous._timeLine.setDirection( TimeLine::Backward );

    }

    //________________________________________________________________________________
    void MenuStateData::disconnect( GtkWidget* widget )
    {

        _target = 0L;

        // disconnect signal
        _motionId.disconnect();
        _leaveId.disconnect();

        // disconnect timelines
        _current._timeLine.disconnect();
        _previous._timeLine.disconnect();

    }

    //________________________________________________________________________________
    void MenuStateData::updateItems( GdkEventType type )
    {

        if( !_target ) return;

        const bool isLeaveEvent( type == GDK_LEAVE_NOTIFY );

        gint xPointer, yPointer;
        gdk_window_get_pointer( gtk_widget_get_window( _target ), &xPointer, &yPointer, 0L );

        GdkWindow* window( gtk_widget_get_window( _target ) );
        GdkWindow* childWindow( 0L );
        int xOffset(0), yOffset(0);

        bool activeFound( false );
        GtkWidget *activeWidget( 0L );
        GList *children( gtk_container_get_children( GTK_CONTAINER( _target ) ) );
        for( GList* child = g_list_first(children); child; child = g_list_next(child) )
        {

            if( !( child->data && GTK_IS_MENU_ITEM( child->data ) ) ) continue;

            GtkWidget* childWidget( GTK_WIDGET( child->data ) );
            const GtkStateType state( gtk_widget_get_state( childWidget ) );

            // do nothing for disabled child
            if( state == GTK_STATE_INSENSITIVE ) continue;

            if( childWindow != gtk_widget_get_window( childWidget ) )
            {
                childWindow = gtk_widget_get_window( childWidget );
                updateOffsets( window, childWindow, xOffset, yOffset );
            }

            // get allocation and offsets
            GtkAllocation allocation( childWidget->allocation );
            allocation.x += xOffset;
            allocation.y += yOffset;

            if( Gtk::gdk_rectangle_contains( &allocation, xPointer, yPointer ) )
            {

                activeFound = true;
                if( state != GTK_STATE_PRELIGHT )
                {
                    updateState( childWidget, allocation, true );
                    if( !isLeaveEvent ) gtk_widget_set_state( childWidget, GTK_STATE_PRELIGHT );
                }

            } else if( state != GTK_STATE_NORMAL ) {

                activeWidget = childWidget;

            }
        }

        if( children ) g_list_free( children );

        // fade-out current
        if( _current.isValid() && !activeFound && !menuItemIsActive( _current._widget ) )
        { updateState( _current._widget, _current._rect, false ); }

        // disable previous active widget, if either another active widget was found, or this one is not active
        if( activeWidget && (activeFound || !menuItemIsActive( activeWidget ) ) )
        { gtk_widget_set_state( activeWidget, GTK_STATE_NORMAL ); }

        return;

    }

    //________________________________________________________________________________
    void MenuStateData::updateOffsets( GdkWindow* parent, GdkWindow* child, int& x, int& y ) const
    {
        x = 0;
        y = 0;
        if( !( parent && child ) ) return;

        while( child && GDK_IS_WINDOW( child ) &&
            child != parent &&
            gdk_window_get_window_type( child ) != GDK_WINDOW_TOPLEVEL &&
            gdk_window_get_window_type( child ) != GDK_WINDOW_TEMP
            )
        {
            gint xloc;
            gint yloc;
            gdk_window_get_position( child, &xloc, &yloc );
            x += xloc;
            y += yloc;
            child = gdk_window_get_parent( child );
        }

        return;

    }

    //________________________________________________________________________________
    bool MenuStateData::menuItemIsActive( GtkWidget* widget ) const
    {

        // check argument
        if( !GTK_IS_MENU_ITEM( widget ) ) return false;

        // check menu
        GtkWidget* menu( GTK_MENU_ITEM( widget )->submenu );
        if( !GTK_IS_MENU( menu ) ) return false;

        GtkWidget* topLevel( gtk_widget_get_toplevel( menu ) );
        if( !topLevel ) return false;

        return
            GTK_WIDGET_VISIBLE( menu ) &&
            GTK_WIDGET_REALIZED( topLevel ) &&
            GTK_WIDGET_VISIBLE( topLevel );
    }

    //________________________________________________________________________________
    bool MenuStateData::updateState( GtkWidget* widget, const GdkRectangle& rect, bool state )
    {

        // do nothing if animations are disabled
        if( !_animationsEnabled ) return true;

        if( state && widget != _current._widget )
        {

            // stop current animation if running
            if( _current._timeLine.isRunning() ) _current._timeLine.stop();

            // stop previous animation if running
            if( _current.isValid() )
            {
                if( _previous._timeLine.isRunning() ) _previous._timeLine.stop();

                if( _previous.isValid() )
                { _dirtyRect = _previous._rect; }

                // move current to previous
                _previous.copy( _current );
            }

            // assign new widget to current and start animation
            const bool animate( !_current.isValid() );
            _current.update( widget, rect );
            if( _current.isValid() )
            {
                if( animate ) _current._timeLine.start();
                else delayedUpdate( this );
            }

            return true;

        } else if( (!state) && widget == _current._widget ) {

            // stop current animation if running
            if( _current._timeLine.isRunning() ) _current._timeLine.stop();

            // stop previous animation if running
            if( _previous._timeLine.isRunning() ) _previous._timeLine.stop();

            if( _previous.isValid() )
            { _dirtyRect = _previous._rect; }

            // move current to previous; clear current, and animate
            _previous.copy( _current );
            _current.clear();
            if( _previous.isValid() && gtk_widget_get_state( _previous._widget ) == GTK_STATE_PRELIGHT ) _previous._timeLine.start();

            return true;

        } else return false;

    }

    //_____________________________________________
    GdkRectangle MenuStateData::dirtyRect( void )
    {

        GdkRectangle rect( Gtk::gdk_rectangle() );

        const GdkRectangle previousRect( _previous._rect );
        const GdkRectangle currentRect( _current._rect );

        if( Gtk::gdk_rectangle_is_valid( &previousRect ) && Gtk::gdk_rectangle_is_valid( &currentRect ) )
        {

            gdk_rectangle_union( &previousRect, &currentRect, &rect );

        } else if( Gtk::gdk_rectangle_is_valid( &previousRect ) ) {

            rect = previousRect;

        } else if( Gtk::gdk_rectangle_is_valid( &currentRect ) ) {

            rect = currentRect;

        }

        // also union with dirty rect
        if( Gtk::gdk_rectangle_is_valid( &_dirtyRect ) )
        {
            if( Gtk::gdk_rectangle_is_valid( &rect ) ) gdk_rectangle_union( &_dirtyRect, &rect, &rect );
            else rect = _dirtyRect;

            _dirtyRect = Gtk::gdk_rectangle();

        }

        return rect;

    }

    //________________________________________________________________________________
    gboolean MenuStateData::motionNotifyEvent(GtkWidget*, GdkEventMotion*, gpointer pointer )
    {
        static_cast<MenuStateData*>( pointer )->updateItems( GDK_MOTION_NOTIFY );
        return FALSE;
    }

    //________________________________________________________________________________
    gboolean MenuStateData::leaveNotifyEvent( GtkWidget*, GdkEventCrossing*, gpointer pointer )
    {
        static_cast<MenuStateData*>( pointer )->updateItems( GDK_LEAVE_NOTIFY );
        return FALSE;
    }

    //_____________________________________________
    gboolean MenuStateData::delayedUpdate( gpointer pointer )
    {

        MenuStateData& data( *static_cast<MenuStateData*>( pointer ) );

        if( data._target )
        {
            const GdkRectangle rect( data.dirtyRect() );
            Gtk::gtk_widget_queue_draw( data._target, &rect );
        }

        return FALSE;

    }

}
