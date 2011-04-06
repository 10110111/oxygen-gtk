/*
* this file is part of the oxygen gtk engine
* Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
*
* based on the Null Theme Engine for Gtk+.
* Copyright (c) 2008 Robert Staudinger <robert.staudinger@gmail.com>
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

#include "oxygensliderdemowidget.h"

#include <iostream>
#include <string>

namespace Oxygen
{

    //____________________________________________________
    SliderDemoWidget::SliderDemoWidget( void )
    {

        // main widget
        GtkWidget* mainWidget( gtk_hbox_new( false, 0 ) );
        gtk_box_set_spacing( GTK_BOX( mainWidget ), 5 );
        setWidget( mainWidget );

        // setup
        setName( "Sliders" );
        setComments( "Shows the appearance of sliders, progress bars and scrollbars" );
        setIconName( "measure" );
        realize();

        // horizontal sliders
        {

            // frame
            GtkWidget* frame( gtk_frame_new( "Horizontal" ) );
            gtk_box_pack_start( GTK_BOX( mainWidget ), frame, true, true, 0 );
            gtk_widget_show( frame );

            // container
            GtkWidget* box( gtk_vbox_new( false, 0 ) );
            gtk_container_set_border_width( GTK_CONTAINER( box ), 5 );
            gtk_box_set_spacing( GTK_BOX( box ), 2 );
            gtk_widget_show( box );
            gtk_container_add( GTK_CONTAINER( frame ), box );

            // scale
            _horizontalSliders._scale = gtk_hscale_new_with_range( 0, 100, 1 );
            gtk_scale_set_draw_value( GTK_SCALE( _horizontalSliders._scale ), false );
            gtk_range_set_value( GTK_RANGE( _horizontalSliders._scale ), 25 );
            gtk_box_pack_start( GTK_BOX( box ), _horizontalSliders._scale, false, true, 0 );
            gtk_widget_show( _horizontalSliders._scale );

            // progress bar
            _horizontalSliders._progressBar = gtk_progress_bar_new();
            gtk_progress_bar_set_orientation( GTK_PROGRESS_BAR( _horizontalSliders._progressBar ), GTK_PROGRESS_LEFT_TO_RIGHT );
            gtk_progress_bar_set_fraction( GTK_PROGRESS_BAR( _horizontalSliders._progressBar ), 0.25 );
            gtk_box_pack_start( GTK_BOX( box ), _horizontalSliders._progressBar, false, true, 0 );
            gtk_widget_show( _horizontalSliders._progressBar );

            // scrollbar
            GtkObject* adjustment( gtk_adjustment_new( 25, 0, 100, 1, 1, 10 ) );
            _horizontalSliders._scrollBar = gtk_hscrollbar_new( GTK_ADJUSTMENT( adjustment ) );
            gtk_box_pack_start( GTK_BOX( box ), _horizontalSliders._scrollBar, false, true, 0 );
            gtk_widget_show( _horizontalSliders._scrollBar );

        }

        // vertical sliders
        {

            // frame
            GtkWidget* frame( gtk_frame_new( "Vertical" ) );
            gtk_box_pack_start( GTK_BOX( mainWidget ), frame, false, true, 0 );
            gtk_widget_show( frame );

            // container
            GtkWidget* box( gtk_hbox_new( false, 0 ) );
            gtk_container_set_border_width( GTK_CONTAINER( box ), 5 );
            gtk_box_set_spacing( GTK_BOX( box ), 5 );
            gtk_widget_show( box );
            gtk_container_add( GTK_CONTAINER( frame ), box );

            // scale
            _verticalSliders._scale = gtk_vscale_new_with_range( 0, 100, 1 );
            gtk_scale_set_draw_value( GTK_SCALE( _verticalSliders._scale ), false );
            gtk_range_set_value( GTK_RANGE( _verticalSliders._scale ), 25 );
            gtk_box_pack_start( GTK_BOX( box ), _verticalSliders._scale, false, true, 0 );
            gtk_widget_show( _verticalSliders._scale );

            // progress bar
            _verticalSliders._progressBar = gtk_progress_bar_new();
            gtk_progress_bar_set_orientation( GTK_PROGRESS_BAR( _verticalSliders._progressBar ), GTK_PROGRESS_BOTTOM_TO_TOP );
            gtk_progress_bar_set_fraction( GTK_PROGRESS_BAR( _verticalSliders._progressBar ), 0.25 );
            gtk_box_pack_start( GTK_BOX( box ), _verticalSliders._progressBar, false, true, 0 );
            gtk_widget_show( _verticalSliders._progressBar );

            // scrollbar
            GtkObject* adjustment( gtk_adjustment_new( 25, 0, 100, 1, 1, 10 ) );
            _verticalSliders._scrollBar = gtk_vscrollbar_new( GTK_ADJUSTMENT( adjustment ) );
            gtk_box_pack_start( GTK_BOX( box ), _verticalSliders._scrollBar, false, true, 0 );
            gtk_widget_show( _verticalSliders._scrollBar );

        }

        _horizontalSliders.connect( GCallback( valueChanged ), this );
        _verticalSliders.connect( GCallback( valueChanged ), this );

    }

    //____________________________________________________
    SliderDemoWidget::~SliderDemoWidget( void )
    {}

    //____________________________________________________
    void SliderDemoWidget::valueChanged( GtkRange* range, gpointer pointer )
    {
        const double value( gtk_range_get_value( range ) );
        SliderDemoWidget& data( *static_cast<SliderDemoWidget*>( pointer ) );
        data._horizontalSliders.setValue( value );
        data._verticalSliders.setValue( value );
    }

    //____________________________________________________
    void SliderDemoWidget::Sliders::setValue( const double& value ) const
    {
        gtk_range_set_value( GTK_RANGE( _scale ), value );
        gtk_progress_bar_set_fraction( GTK_PROGRESS_BAR( _progressBar ), value/100 );
        gtk_range_set_value( GTK_RANGE( _scrollBar ), value );
    }

}
