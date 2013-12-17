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

#include "oxygenmenustatedata.h"
#include "../oxygengtkutils.h"

#include <cassert>
#include <gtk/gtk.h>

namespace Oxygen
{

    //________________________________________________________________________________
    const int MenuStateData::_timeOut = 50;
    void MenuStateData::connect( GtkWidget* widget )
    {

        _target = widget;

        // save paddings
        if( GTK_IS_MENU( widget ) )
        {
            gtk_widget_style_get( _target,
                "vertical-padding", &_yPadding,
                "horizontal-padding", &_xPadding,
                NULL );
        }

        // connect timeLines
        _current._timeLine.connect( (GSourceFunc)delayedUpdate, this );
        _previous._timeLine.connect( (GSourceFunc)delayedUpdate, this );

        // set directions
        _current._timeLine.setDirection( TimeLine::Forward );
        _previous._timeLine.setDirection( TimeLine::Backward );

        // follow mouse animation
        FollowMouseData::connect( (GSourceFunc)followMouseUpdate, this );

    }

    //________________________________________________________________________________
    void MenuStateData::disconnect( GtkWidget* widget )
    {

        _target = 0L;

        // disconnect timelines
        _current._timeLine.disconnect();
        _previous._timeLine.disconnect();
        _timer.stop();

        // base class
        FollowMouseData::disconnect();

   }

    //________________________________________________________________________________
    bool MenuStateData::updateState( GtkWidget* widget, bool state, bool delayed )
    {

        if( state && widget != _current._widget )
        {

            // get window offset
            GdkWindow* window( gtk_widget_get_window( _target ) );
            GdkWindow* childWindow( gtk_widget_get_window( widget ) );
            int xOffset(0);
            int yOffset(0);
            Gtk::gdk_window_translate_origin( window, childWindow, &xOffset, &yOffset );

            // get allocation
            const GdkRectangle rect( Gtk::gtk_widget_get_allocation( widget ) );

            // stop timer
            if( _timer.isRunning() ) _timer.stop();

            // stop current animation if running
            if( _current._timeLine.isRunning() ) _current._timeLine.stop();

            // stop previous animation if running
            if( _current.isValid() )
            {
                if( _previous._timeLine.isRunning() ) _previous._timeLine.stop();

                if( _previous.isValid() )
                {
                    _dirtyRect = _previous._rect;
                    _dirtyRect.x += _previous._xOffset;
                    _dirtyRect.y += _previous._yOffset;
                }

                // move current to previous
                _previous.copy( _current );
            }

            // assign new widget to current and start animation
            const bool animate( !_current.isValid() );
            const GdkRectangle startRect( _current._rect );
            const int startOffset( _current._yOffset );
            _current.update( widget, rect, xOffset, yOffset );

            if( _current.isValid() )
            {
                if( animate ) _current._timeLine.start();
                else if( followMouse() && (startOffset == _current._yOffset ) ) startAnimation( startRect, _current._rect );
                else delayedUpdate( this );
            }

            return false;

        } else if( !state && widget == _current._widget ) {

            // stop current animation if running
            if( _current._timeLine.isRunning() ) _current._timeLine.stop();

            // stop previous animation if running
            if( _previous._timeLine.isRunning() ) _previous._timeLine.stop();

            if( _previous.isValid() )
            {
                _dirtyRect = _previous._rect;
                _dirtyRect.x += _previous._xOffset;
                _dirtyRect.y += _previous._yOffset;
            }

            // move current to previous; clear current, and animate
            if( followMouse() && delayed ) {

                if( !_timer.isRunning() )
                { _timer.start( _timeOut, (GSourceFunc)delayedAnimate, this ); }

                return true;

            } else {

                if( _timer.isRunning() ) _timer.stop();
                _previous.copy( _current );
                _current.clear();
                if( _previous.isValid() && gtk_widget_get_state_flags( _previous._widget )&GTK_STATE_FLAG_PRELIGHT )
                { _previous._timeLine.start(); }

                return false;

            }


        } else return false;

    }

    //_____________________________________________
    GdkRectangle MenuStateData::dirtyRect( void )
    {

        GdkRectangle rect( Gtk::gdk_rectangle() );
        const GdkRectangle previousRect( _previous.dirtyRect() );
        const GdkRectangle currentRect( _current.dirtyRect() );
        Gtk::gdk_rectangle_union( &previousRect, &currentRect, &rect );

        // add _dirtyRect
        if( Gtk::gdk_rectangle_is_valid( &_dirtyRect ) )
        {
            Gtk::gdk_rectangle_union( &_dirtyRect, &rect, &rect );
            _dirtyRect = Gtk::gdk_rectangle();
        }

        // add followMouse dirtyRect
        if( followMouse() )
        {

            // retrieve dirty rect and add relevant offsets
            GdkRectangle followMouseRect( FollowMouseData::dirtyRect() );
            if( Gtk::gdk_rectangle_is_valid( &_current._rect ) )
            {

                followMouseRect.x += _current._xOffset;
                followMouseRect.y += _current._yOffset;

            } else if( Gtk::gdk_rectangle_is_valid( &_previous._rect ) ) {

                followMouseRect.x += _previous._xOffset;
                followMouseRect.y += _previous._yOffset;

            } else if( Gtk::gdk_rectangle_is_valid( &followMouseRect ) && _target ) {

                // no valid offset found. Add full allocation
                followMouseRect = Gtk::gtk_widget_get_allocation( _target );
                followMouseRect.x += _xPadding;
                followMouseRect.y += _yPadding;
                followMouseRect.width -= 2*_xPadding;
                followMouseRect.height -= 2*_yPadding;

            }

            Gtk::gdk_rectangle_union( &followMouseRect, &rect, &rect );
        }

        // extend rect by some arbitrary number to prevent glitches
        if( Gtk::gdk_rectangle_is_valid( &rect ) ) rect.height += 1;

        return rect;

    }

    //_____________________________________________
    gboolean MenuStateData::delayedUpdate( gpointer pointer )
    {

        MenuStateData& data( *static_cast<MenuStateData*>( pointer ) );

        if( data._target )
        {
            GdkRectangle rect( data.dirtyRect() );

            const int margin( 3 );
            rect.x -= margin;
            rect.y -= margin;
            rect.width += 2*margin;
            rect.height += 2*margin;

            Gtk::gtk_widget_queue_draw( data._target, &rect );
        }

        return FALSE;

    }

    //_____________________________________________
    gboolean MenuStateData::followMouseUpdate( gpointer pointer )
    {

        MenuStateData& data( *static_cast<MenuStateData*>( pointer ) );

        if( data._target && data.followMouse() )
        {

            data.updateAnimatedRect();
            GdkRectangle rect( data.dirtyRect() );

            const int margin( 3 );
            rect.x -= margin;
            rect.y -= margin;
            rect.width += 2*margin;
            rect.height += 2*margin;

            Gtk::gtk_widget_queue_draw( data._target, &rect );

        }

        return FALSE;

    }

    //_____________________________________________
    gboolean MenuStateData::delayedAnimate( gpointer pointer )
    {

        MenuStateData& data( *static_cast<MenuStateData*>( pointer ) );
        data._previous.copy( data._current );
        data._current.clear();

        if( data._previous.isValid() )
        { data._previous._timeLine.start(); }

        return FALSE;

    }

}
