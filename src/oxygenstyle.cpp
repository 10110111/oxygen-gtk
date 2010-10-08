/*
* this file is part of the oxygen gtk engine
* Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
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

#include "oxygenstyle.h"
#include "oxygencairocontext.h"
#include "oxygencairoutils.h"
#include "oxygencolorutils.h"
#include "oxygengtkutils.h"

#include <algorithm>
#include <cmath>
#include <gtk/gtk.h>

namespace Oxygen
{

    //__________________________________________________________________
    Style& Style::instance( void )
    {
        static Style singleton;
        return singleton;
    }

    //__________________________________________________________________
    Style::Style( void )
    { _settings.init(); }

    //__________________________________________________________________
    void Style::fill( GdkWindow* window, GdkRectangle* clipRect, gint x, gint y, gint w, gint h, const ColorUtils::Rgba& color ) const
    {

        // define colors
        Cairo::Context context( window, clipRect );
        cairo_rectangle( context, x, y, w, h );
        cairo_set_source( context, color );
        cairo_fill( context );

    }

    //__________________________________________________________________
    void Style::drawSeparator( GdkWindow* window, GdkRectangle* clipRect, gint x, gint y, gint w, gint h, StyleOptions options ) const
    {

        // define colors
        ColorUtils::Rgba base( settings().palette().color( Palette::Window ) );
        if( options & Blend )
        {

            gint wh, wy;
            Gtk::gdk_map_to_toplevel( window, 0L, &wy, 0L, &wh );
            if( wh > 0 )
            {
                if( options & Menu ) base = ColorUtils::menuBackgroundColor( settings().palette().color( Palette::Window ), wh, y+wy+h/2 );
                else base = ColorUtils::backgroundColor( settings().palette().color( Palette::Window ), wh, y+wy+h/2 );
            }

        }

        Cairo::Context context( window, clipRect );
        _helper.drawSeparator( context, base, x, y, w, h, (options&Vertical) );

    }

    //__________________________________________________________________
    bool Style::renderWindowBackground( GdkWindow* window, GtkWidget* widget, GdkRectangle* clipRect, gint x, gint y, gint w, gint h ) const
    {

        // define colors
        ColorUtils::Rgba base(settings().palette().color( Palette::Window ) );

        // get window dimension and position
        gint ww, wh;
        gint wx, wy;
        if( !Gtk::gdk_map_to_toplevel( window, widget, &wx, &wy, &ww, &wh, true ) )
        { return false; }

        // the hard-coded metrics are copied for
        // kdebase/workspace/libs/oxygen/oxygenhelper.cpp
        // vertical shift to account for window decoration
        const int yShift = 23;
        wy += yShift;

        // translate to toplevel coordinates
        x+=wx;
        y+=wy;

        // create context and translate to toplevel coordinates
        Cairo::Context context( window, clipRect );
        cairo_translate( context, -wx, -wy );

        // split
        const int splitY( std::min(300, 3*wh/4 ) );

        // store rectangle
        GdkRectangle rect = { x, y, w, h };

        /*
        if there is a valid clipRect,
        intersects it with painting Rect, for performances
        */
        if( clipRect )
        {

            GdkRectangle localClip( *clipRect );
            localClip.x += wx;
            localClip.y += wy;
            gdk_rectangle_intersect( &rect, &localClip, &rect );

        }

        // upper rect
        GdkRectangle upperRect = { 0, 0, ww, splitY };
        if( gdk_rectangle_intersect( &rect, &upperRect, &upperRect ) )
        {

            Cairo::Pattern pattern( verticalGradient( base, 0, splitY ) );

            gdk_cairo_rectangle( context, &upperRect );
            cairo_set_source( context, pattern );
            cairo_fill( context );

        }

        // fill lower rect
        GdkRectangle lowerRect = { 0, splitY, ww, wh - splitY + yShift };
        if( gdk_rectangle_intersect( &rect, &lowerRect, &lowerRect ) )
        {

            ColorUtils::Rgba bottom( ColorUtils::backgroundBottomColor( base ) );
            gdk_cairo_rectangle( context, &lowerRect );
            cairo_set_source( context, bottom );
            cairo_fill( context );

        }

        // radial pattern
        const int patternHeight = 64;
        const int radialW( std::min(600, ww ) );

        GdkRectangle radialRect = {  (ww - radialW)/2, 0, radialW, patternHeight };
        if( gdk_rectangle_intersect( &rect, &radialRect, &radialRect ) )
        {

            // get pattern
            Cairo::Pattern pattern( radialGradient( base, 64, patternHeight - 64, 64 ) );

            // add matrix transformation
            // TODO: possibly pass that also in ::radialGradient
            cairo_matrix_t transformation;
            cairo_matrix_init_identity( &transformation );
            cairo_matrix_scale( &transformation, 128.0/radialW, 1.0 );
            cairo_matrix_translate( &transformation, -(ww - radialW)/2, 0 );
            cairo_pattern_set_matrix( pattern, &transformation );

            // fill
            gdk_cairo_rectangle( context, &radialRect );
            cairo_set_source( context, pattern );
            cairo_fill( context );

        }

        return true;

    }

    //__________________________________________________________________
    void Style::renderMenuBackground( GdkWindow* window, GdkRectangle* clipRect, gint x, gint y, gint w, gint h, StyleOptions options ) const
    {
        // define colors
        ColorUtils::Rgba base(settings().palette().color( Palette::Window ) );
        ColorUtils::Rgba top( ColorUtils::backgroundTopColor( base ) );
        ColorUtils::Rgba bottom( ColorUtils::backgroundBottomColor( base ) );

        // create context and translate
        Cairo::Context context( window, clipRect );
        cairo_translate( context, x, y );
        const bool hasAlpha( options&Alpha );

        // paint translucent first
        if( hasAlpha )
        {
            cairo_rectangle( context, 0, 0, w, h );
            cairo_set_operator( context, CAIRO_OPERATOR_SOURCE );
            cairo_set_source( context, ColorUtils::alphaColor( base, 0 ) );
            cairo_fill( context );
        }

        const int splitY( std::min(200, 3*h/4 ) );

        {
            // upper rect
            GdkRectangle upperRect = { 0, 0, w, splitY };
            Cairo::Pattern pattern( cairo_pattern_create_linear( 0, 0, 0, splitY ) );
            cairo_pattern_add_color_stop( pattern, 0, top );
            cairo_pattern_add_color_stop( pattern, 1, bottom );

            gdk_cairo_rounded_rectangle( context, &upperRect, 3.5, hasAlpha ? CornersTop:CornersNone );
            cairo_set_source( context, pattern );
            cairo_fill( context );

        }

        {

            // lower part
            GdkRectangle lowerRect = { 0, splitY, w, h-splitY };
            gdk_cairo_rounded_rectangle( context, &lowerRect, 3.5, hasAlpha ? CornersBottom:CornersNone );
            cairo_set_source( context, bottom );
            cairo_fill( context );

        }

        return;

    }

    //__________________________________________________________________
    void Style::renderTooltipBackground( GdkWindow* window, GdkRectangle* clipRect, gint x, gint y, gint w, gint h, StyleOptions options ) const
    {

        // define colors
        ColorUtils::Rgba base(settings().palette().color( Palette::Tooltip ) );
        ColorUtils::Rgba top( ColorUtils::backgroundTopColor( base ) );
        ColorUtils::Rgba bottom( ColorUtils::backgroundBottomColor( base ) );

        // create context and translate
        Cairo::Context context( window, clipRect );
        cairo_translate( context, x, y );

        // paint translucent first
        const bool hasAlpha( (options&Alpha) );
        if( hasAlpha )
        {
            if( settings().tooltipTransparent() )
            {
                top.setAlpha( 0.86 );
                bottom.setAlpha( 0.86 );
            }

            cairo_rectangle( context, 0, 0, w, h );
            cairo_set_operator( context, CAIRO_OPERATOR_SOURCE );
            cairo_set_source( context, ColorUtils::alphaColor( base, 0 ) );
            cairo_fill( context );
        }

        // fill
        {
            GdkRectangle rect = { 0, 0, w, h };
            Cairo::Pattern pattern( cairo_pattern_create_linear( 0, 0, 0, h ) );
            cairo_pattern_add_color_stop( pattern, 0, top );
            cairo_pattern_add_color_stop( pattern, 1, bottom );

            gdk_cairo_rounded_rectangle( context, &rect, 4, hasAlpha ? CornersAll:CornersNone );
            cairo_set_source( context, pattern );
            cairo_fill( context );

        }

        // contrast pixel
        {
            Cairo::Pattern pattern( cairo_pattern_create_linear( 0, 0, 0, h ) );
            cairo_pattern_add_color_stop( pattern, 0.5, ColorUtils::lightColor( bottom ) );
            cairo_pattern_add_color_stop( pattern, 0.9, bottom );

            cairo_rounded_rectangle( context, 0.5, 0.5, w-1, h-1, 4, hasAlpha ? CornersAll:CornersNone );
            cairo_set_line_width( context, 1.0 );
            cairo_set_source( context, pattern );
            cairo_stroke( context );
        }

        return;

    }

    //__________________________________________________________________
    void Style::renderHeaderBackground( GdkWindow* window, GdkRectangle* clipRect, gint x, gint y, gint w, gint h ) const
    {

        // load color
        const ColorUtils::Rgba base( settings().palette().color( Palette::Window ) );

        // render normal window background
        if( !renderWindowBackground( window, clipRect, x, y, w, h ) )
        {
            // if failed, fallback to flat background
            Cairo::Context context( window, clipRect );
            cairo_rectangle( context, x, y, w, h );
            cairo_set_source( context, base );
            cairo_fill( context );
        }

        // render lines
        renderHeaderLines( window, clipRect, x, y, w, h );

        // render side dots
        Cairo::Context context( window, clipRect );
        int yCenter( y + h/2 );
        int xDots( x + w - 1 );
        _helper.renderDot( context, base, xDots, yCenter - 3 );
        _helper.renderDot( context, base, xDots, yCenter );
        _helper.renderDot( context, base, xDots, yCenter + 3 );

    }


    //__________________________________________________________________
    void Style::renderHeaderLines( GdkWindow* window, GdkRectangle* clipRect, gint x, gint y, gint w, gint h ) const
    {

        // add horizontal lines
        const ColorUtils::Rgba base( settings().palette().color( Palette::Window ) );
        const ColorUtils::Rgba dark( ColorUtils::darkColor( base ) );
        const ColorUtils::Rgba light( ColorUtils::lightColor( base ) );

        Cairo::Context context( window, clipRect );
        cairo_set_line_width( context, 1.0 );

        // dark line
        cairo_set_source( context, dark );
        cairo_move_to( context, x, y+h-0.5 );
        cairo_line_to( context, x+w, y+h-0.5 );
        cairo_stroke( context );

        // light line
        cairo_set_source( context, light );
        cairo_move_to( context, x, y+h-1.5 );
        cairo_line_to( context, x+w, y+h-1.5 );
        cairo_stroke( context );

    }

    //____________________________________________________________________________________
    bool Style::renderHoleBackground(
        GdkWindow* window,
        GdkRectangle* clipRect,
        gint x, gint y, gint w, gint h ) const
    {

        // do nothing if not enough room
        if( w < 14 )  return false;

        /*
        this code is the same as in renderWindowBackground
        except that a mask is added to each portion of the background
        TODO: make as much code in common as possible between
        the two, to avoid code duplication.
        */

        // define colors
        ColorUtils::Rgba base(settings().palette().color( Palette::Window ) );

        // get window size and height
        gint ww, wh;
        gint wx, wy;
        if( !Gtk::gdk_map_to_toplevel( window, &wx, &wy, &ww, &wh, true ) )
        { return false; }

        // do nothing if window dimensions could not be found
        if( ww < 0 || wh < 0 ) return false;

        // the hard-coded metrics are copied for
        // kdebase/workspace/libs/oxygen/oxygenhelper.cpp
        // vertical shift to account for window decoration
        const int yShift = 23;
        wy += yShift;

        // translate to toplevel coordinates
        x+=wx;
        y+=wy;

        // create context and translate to toplevel coordinates
        Cairo::Context context( window, clipRect );
        cairo_translate( context, -wx, -wy );

        //GdkRectangle rect = { x, y, w, h };
        GdkRectangle rect = { x, y, w, h+1 };
        GdkRectangle mask = {x+1, y+1, w-2, h-2 };
        const double maskRadius = 3.5;

        // the hard-coded metrics are copied for
        // kdebase/workspace/libs/oxygen/oxygenhelper.cpp
        const int splitY( std::min(300, 3*wh/4 ) );

        // upper rect
        GdkRectangle upperRect = { 0, 0, ww, splitY };
        if( gdk_rectangle_intersect( &rect, &upperRect, &upperRect ) )
        {

            cairo_save( context );
            gdk_cairo_rectangle( context, &upperRect );
            cairo_clip( context );

            Cairo::Pattern pattern( verticalGradient( base, -yShift, splitY-yShift ) );
            cairo_set_source( context, pattern );

            gdk_cairo_rectangle( context, &upperRect );
            gdk_cairo_rounded_rectangle_negative( context, &mask, maskRadius );
            cairo_fill( context );

            cairo_restore( context );
        }


        // lower rect
        GdkRectangle lowerRect = { 0, splitY, ww, wh - splitY + yShift };
        if( gdk_rectangle_intersect( &rect, &lowerRect, &lowerRect ) )
        {

            cairo_save( context );
            gdk_cairo_rectangle( context, &lowerRect );
            cairo_clip( context );

            ColorUtils::Rgba bottom( ColorUtils::backgroundBottomColor( base ) );
            cairo_set_source( context, bottom );
            gdk_cairo_rectangle( context, &lowerRect );
            gdk_cairo_rounded_rectangle_negative( context, &mask, maskRadius );
            cairo_fill( context );

            cairo_restore( context );
        }

        // radial pattern
        const int patternHeight = 64;
        const int radialW( std::min(600, ww ) );

        GdkRectangle radialRect = {  (ww - radialW)/2, 0, radialW, patternHeight };
        if( gdk_rectangle_intersect( &rect, &radialRect, &radialRect ) )
        {

            // get pattern
            Cairo::Pattern pattern( radialGradient( base, 64, patternHeight - 64, 64 ) );

            // add matrix transformation
            // TODO: possibly pass that also in ::radialGradient
            cairo_matrix_t transformation;
            cairo_matrix_init_identity( &transformation );
            cairo_matrix_scale( &transformation, 128.0/radialW, 1.0 );
            cairo_matrix_translate( &transformation, -(ww - radialW)/2, 0 );
            cairo_pattern_set_matrix( pattern, &transformation );

            cairo_save( context );
            GdkRectangle local = { (ww - radialW)/2, 0, radialW, patternHeight };
            gdk_cairo_rectangle( context, &local );
            cairo_clip( context );

            gdk_cairo_rectangle( context, &local );
            gdk_cairo_rounded_rectangle_negative( context, &mask, maskRadius );
            cairo_set_source( context, pattern );
            cairo_fill( context );

            cairo_restore( context );

        }

        return true;

    }

    //__________________________________________________________________
    void Style::renderSplitter( GdkWindow* window, GdkRectangle* clipRect, gint x, gint y, gint w, gint h, StyleOptions options ) const
    {

        // orientation
        const bool vertical( options&Vertical );

        // colors
        const ColorUtils::Rgba& base( settings().palette().color( Palette::Window ) );

        // context
        Cairo::Context context( window, clipRect );

        // hover rect
        if( options&Hover )
        {
            const ColorUtils::Rgba& highlight( ColorUtils::alphaColor( ColorUtils::lightColor( base ), 0.5 ) );

            Cairo::Context context( window, clipRect );
            Cairo::Pattern pattern;
            double a(0.1);
            if( vertical )
            {

                if( w > 30 ) a = 10/w;
                pattern.set( cairo_pattern_create_linear( x, 0, x+w, 0 ) );

            } else {

                if( h>30 ) a = 10/h;
                pattern.set( cairo_pattern_create_linear( 0, y, 0, y+h ) );

            }

            cairo_pattern_add_color_stop( pattern, 0, ColorUtils::alphaColor( highlight, 0 ) );
            cairo_pattern_add_color_stop( pattern, a, highlight );
            cairo_pattern_add_color_stop( pattern, 1.0-a, highlight );
            cairo_pattern_add_color_stop( pattern, 1.0, ColorUtils::alphaColor( highlight, 0 ) );

            cairo_set_source( context, pattern );
            cairo_rectangle( context, x, y, w, h );
            cairo_fill( context );
        }

        // dots
        if( vertical )
        {

            y += h/2;
            const int ngroups( std::max( 1, w/250 ) );
            int center = ( w-( ngroups-1 )*250 )/2 + x;
            for( int k = 0; k < ngroups; k++, center += 250 )
            {
                _helper.renderDot( context, base, center-3, y-1 );
                _helper.renderDot( context, base, center, y-1 );
                _helper.renderDot( context, base, center+3, y-1 );
            }

        } else {

            x += w/2;
            const int ngroups( std::max( 1, h/250 ) );
            int center = ( h-( ngroups-1 )*250 )/2 + y;
            for( int k = 0; k < ngroups; k++, center += 250 )
            {
                _helper.renderDot( context, base, x, center-3 );
                _helper.renderDot( context, base, x, center );
                _helper.renderDot( context, base, x, center+3 );
            }

        }

    }

    //____________________________________________________________________________________
    void Style::renderProgressBarHole(
        GdkWindow* window,
        GdkRectangle* clipRect,
        gint x, gint y, gint w, gint h, StyleOptions options ) const
    {

        // context
        const ColorUtils::Rgba base(settings().palette().color( Palette::Window ) );

        Cairo::Context context( window, clipRect );
        renderScrollBarHole( context, x, y, w, h, base, (options&Vertical) );

    }

    //____________________________________________________________________________________
    void Style::renderProgressBarHandle(
        GdkWindow* window,
        GdkRectangle* clipRect,
        gint x, gint y, gint w, gint h,
        GtkProgressBarOrientation orientation ) const
    {

        // colors
        const ColorUtils::Rgba base( settings().palette().color( Palette::Active, Palette::Window ) );
        const ColorUtils::Rgba glow( settings().palette().color( Palette::Focus ) );

        // context
        Cairo::Context context( window );

        // get pixbug
        GdkPixbuf* pixbuf( _helper.progressBarIndicator( base, glow, w, h ) );
        cairo_translate( context, -1, -2 );
        cairo_translate( context, x, y );
        switch( orientation )
        {
            case GTK_PROGRESS_TOP_TO_BOTTOM:
            {
                cairo_matrix_t transformation;
                cairo_matrix_init( &transformation, 1,0,0,-1,0,h+4 );
                cairo_transform( context, &transformation );
                break;
            }

            case GTK_PROGRESS_RIGHT_TO_LEFT:
            {
                cairo_matrix_t transformation;
                cairo_matrix_init( &transformation, -1,0,0,1,w+2,0 );
                cairo_transform( context, &transformation );
                break;
            }

            default: break;
        }

        cairo_rectangle( context, 0, 0, gdk_pixbuf_get_width( pixbuf ), gdk_pixbuf_get_height( pixbuf ) );
        gdk_cairo_set_source_pixbuf( context, pixbuf, 0, 0 );
        cairo_fill( context );

        g_object_unref( pixbuf );

    }

    //____________________________________________________________________________________
    void Style::renderScrollBarHole(
        GdkWindow* window,
        GdkRectangle* clipRect,
        gint x, gint y, gint w, gint h, StyleOptions options ) const
    {

        // colors
        const ColorUtils::Rgba base(settings().palette().color( Palette::Window ) );

        // context
        Cairo::Context context( window, clipRect );
        renderScrollBarHole( context, x, y, w, h, base, options&Vertical );

    }

    //____________________________________________________________________________________
    void Style::renderScrollBarHandle(
        GdkWindow* window,
        GdkRectangle* clipRect,
        gint x, gint y, gint w, gint h, StyleOptions options ) const
    {

        // store colors
        const ColorUtils::Rgba color( settings().palette().color( Palette::Button ) );
        const ColorUtils::Rgba light( ColorUtils::lightColor( color ) );
        const ColorUtils::Rgba mid( ColorUtils::midColor( color ) );
        const ColorUtils::Rgba dark( ColorUtils::darkColor( color ) );
        const ColorUtils::Rgba shadow( ColorUtils::shadowColor( color ) );
        const ColorUtils::Rgba base( ColorUtils::mix( dark, shadow, 0.5 ) );

        // glow color
        ColorUtils::Rgba glow;
        if( settings().scrollBarColored() ) glow = ColorUtils::mix( dark, shadow, 0.5 );
        else if( options&Hover ) glow = settings().palette().color( Palette::Hover );
        else glow = base;

        // vertical
        const bool vertical( options&Vertical );

        const double xf( vertical ? x+2 : x+3 );
        const double yf( vertical ? y+2 : y+1 );
        const double wf( vertical ? w-4 : w-6 );
        const double hf( vertical ? h-4 : h-3 );

        if( wf <= 0 || hf <= 0 ) return;

        // context
        Cairo::Context context( window, clipRect );

        // glow, shadow
        {
            cairo_rounded_rectangle( context, xf-0.8, yf-0.8, wf+1.6, hf+1.6, 3 );
            cairo_set_source( context, ColorUtils::alphaColor( glow, 0.6 ) );
            cairo_fill( context );

            cairo_rounded_rectangle( context, xf-1.2, yf-0.8, wf+2.4, hf+1.6, 3 );
            cairo_set_source( context, ColorUtils::alphaColor( glow, 0.3 ) );
            cairo_set_line_width( context, 1.5 );
            cairo_stroke( context );

        }

        // colored background
        if( settings().scrollBarColored() )
        {
            if( options&Hover ) cairo_set_source( context, settings().palette().color( Palette::Hover ) );
            else cairo_set_source( context, color );

            cairo_rounded_rectangle( context, xf, yf, wf, hf, 2 );
            cairo_fill( context );
        }


        // slider pattern
        {

            Cairo::Pattern pattern;
            if( vertical ) pattern.set( cairo_pattern_create_linear( xf, 0, xf+wf, 0 ) );
            else pattern.set( cairo_pattern_create_linear( 0, yf, 0, yf+hf ) );

            if( settings().scrollBarColored() )
            {

                cairo_pattern_add_color_stop( pattern, 0, ColorUtils::alphaColor( light, 0.6 ) );
                cairo_pattern_add_color_stop( pattern, 0.3, ColorUtils::alphaColor( dark, 0.3 ) );
                cairo_pattern_add_color_stop( pattern, 1.0, ColorUtils::alphaColor( light, 0.8 ) );

            } else {

                cairo_pattern_add_color_stop( pattern, 0, color );
                cairo_pattern_add_color_stop( pattern, 1, mid );

            }

            cairo_set_source( context, pattern );
            cairo_rounded_rectangle( context, xf, yf, wf, hf, 2 );
            cairo_fill( context );

        }

        // pattern
        if( settings().scrollBarBevel() )
        {

            Cairo::Pattern pattern;
            if( vertical ) pattern.set( cairo_pattern_create_linear( 0, 0, 0, 30 ) );
            else pattern.set( cairo_pattern_create_linear( 0, 0, 30, 0 ) );
            cairo_pattern_set_extend( pattern, CAIRO_EXTEND_REFLECT );
            if( settings().scrollBarColored() )
            {

                cairo_pattern_add_color_stop( pattern, 0, ColorUtils::alphaColor( shadow, 0.15 ) );
                cairo_pattern_add_color_stop( pattern, 1.0, ColorUtils::alphaColor( light, 0.15 ) );

            } else {

                cairo_pattern_add_color_stop( pattern, 0, ColorUtils::alphaColor( shadow, 0.1 ) );
                cairo_pattern_add_color_stop( pattern, 1.0, ColorUtils::alphaColor( light, 0.1 ) );

            }

            cairo_set_source( context, pattern );
            cairo_rounded_rectangle( context, xf, yf, wf, hf, 2 );
            cairo_fill( context );

        }

        // bevel
        if( !settings().scrollBarColored() )
        {
            Cairo::Pattern pattern;
            if( vertical ) pattern.set( cairo_pattern_create_linear( 0, yf, 0, yf+hf ) );
            else pattern.set( cairo_pattern_create_linear( xf, 0, xf+wf, 0 ) );

            cairo_pattern_add_color_stop( pattern, 0, ColorUtils::alphaColor( light, 0 ) );
            cairo_pattern_add_color_stop( pattern, 0.5, light );
            cairo_pattern_add_color_stop( pattern, 1.0, ColorUtils::alphaColor( light, 0 ) );
            cairo_set_source( context, pattern );
            cairo_set_line_width( context, 1 );

            if( vertical )
            {
                cairo_move_to( context, xf + 0.5, yf + 0.5 );
                cairo_line_to( context, xf + 0.5, yf + hf -1.5 );
                cairo_stroke( context );

                cairo_move_to( context, xf + wf - 0.5, yf + 0.5 );
                cairo_line_to( context, xf + wf - 0.5, yf + hf - 0.5 );
                cairo_stroke( context );

            } else {

                cairo_move_to( context, xf + 0.5, yf + 0.5 );
                cairo_line_to( context, xf + wf - 0.5, yf + 0.5 );
                cairo_stroke( context );

                cairo_move_to( context, xf + 0.5, yf + hf - 0.5 );
                cairo_line_to( context, xf + wf - 0.5, yf + hf - 0.5 );
                cairo_stroke( context );

            }

        }

    }

    //____________________________________________________________________________________
    void Style::renderToolBarHandle(
        GdkWindow* window,
        GdkRectangle* clipRect,
        gint x, gint y, gint w, gint h, StyleOptions options ) const
    {

        const bool vertical( options & Vertical );
        const ColorUtils::Rgba base( settings().palette().color( Palette::Window ) );

        Cairo::Context context( window, clipRect );
        int counter(0);
        if( vertical )
        {

            const int xcenter( x+w/2 );
            for( int ycenter = y+2; ycenter <= y+h-3; ycenter+=3, ++counter )
            {
                if( counter%2 == 0 ) _helper.renderDot( context, base, xcenter+1, ycenter );
                else _helper.renderDot( context, base, xcenter-2, ycenter );
            }

        } else {

            const int ycenter( y + h/2 );
            for( int xcenter = x+2; xcenter < x+w-3; xcenter+=3, ++counter )
            {
                if( counter%2 == 0 ) _helper.renderDot( context, base, xcenter, ycenter+1 );
                else _helper.renderDot( context, base, xcenter, ycenter-2 );
            }

        }

        return;

    }

    //__________________________________________________________________
    void Style::drawFloatFrame( GdkWindow* window, GdkRectangle* clipRect, gint x, gint y, gint w, gint h, StyleOptions options ) const
    {

        // define colors
        ColorUtils::Rgba base(settings().palette().color( Palette::Window ) );
        ColorUtils::Rgba light( ColorUtils::lightColor( base ) );
        ColorUtils::Rgba dark( ColorUtils::darkColor( ColorUtils::backgroundBottomColor( base ) ) );

        // create context and translate
        Cairo::Context context( window, clipRect );
        cairo_translate( context, x, y );
        Cairo::Pattern pattern( cairo_pattern_create_linear( 0, 0.5, 0, h-1 ) );
        cairo_pattern_add_color_stop( pattern, 0, light );

        const bool hasAlpha( options & Alpha );
        if( hasAlpha )
        {
            cairo_pattern_add_color_stop( pattern, std::max( 0.8, 1.0 - 10/h), light );
            cairo_pattern_add_color_stop( pattern, 1, ColorUtils::alphaColor( light, 0 ) );
            cairo_rounded_rectangle( context, 0.5, 0.5, w-1, h-1, 3.5 );
        } else {
            cairo_pattern_add_color_stop( pattern, 1, ColorUtils::alphaColor( dark, 0 ) );
            cairo_rectangle( context, 0.5, 0.5, w-1, h-1 );
        }

        cairo_set_source( context, pattern );
        cairo_set_line_width( context, 1.0 );
        cairo_stroke( context );

    }

    //__________________________________________________________________
    void Style::renderButtonSlab(
        GdkWindow* window,
        GdkRectangle* clipRect,
        gint x, gint y, gint w, gint h, StyleOptions options ) const
    {

        // flat buttons are only rendered with a simple Rect, and only when either focused or sunken
        if( options & Flat )
        {
            if( options & Sunken )
            {

                Cairo::Context context( window, clipRect );
                ColorUtils::Rgba base( settings().palette().color( Palette::Window ) );

                if( options & Hover )
                {

                    const ColorUtils::Rgba glow( settings().palette().color( Palette::Hover ) );
                    _helper.holeFocused( base, glow, 0, 7, false ).render( context, x, y, w, h );

                } else _helper.hole( base, 0, 7, false ).render( context, x, y, w, h );

            } else if( options&Hover ) {

                const ColorUtils::Rgba glow( settings().palette().color( Palette::Focus ) );
                Cairo::Context context( window, clipRect );
                _helper.slitFocused( glow ).render( context, x, y, w, h );

            }

            return;

        }

        // do nothing if not enough room
        if( w < 14 ) return;

        // define colors
        ColorUtils::Rgba base;
        if( options&Blend )
        {

            gint wh, wy;
            Gtk::gdk_map_to_toplevel( window, 0L, &wy, 0L, &wh );
            base = ColorUtils::backgroundColor( settings().palette().color( Palette::Button ), wh, y+wy+h/2 );

        } else {

            base = settings().palette().color( Palette::Button );

        }

        const ColorUtils::Rgba light( ColorUtils::lightColor( base ) );

        // create context
        Cairo::Context context( window, clipRect );

        // for edgeOut buttons create slab first
        if( !(options&Sunken) )
        {
            if( options&Hover )
            {

                ColorUtils::Rgba glow( settings().palette().color( Palette::Hover ) );
                _helper.slabFocused( base, glow, 0 ).render( context, x, y, w, h );

            } else if( options&Focus ) {

                ColorUtils::Rgba glow( settings().palette().color( Palette::Focus) );
                _helper.slabFocused( base, glow, 0 ).render( context, x, y, w, h );

            } else {

                _helper.slab( base, 0 ).render( context, x, y, w, h );

            }
        }

        // fill with appropriate pattern
        Cairo::Pattern pattern;
        if( options&Sunken )
        {

            pattern.set( cairo_pattern_create_linear( 0, y-h, 0, y+h ) );
            cairo_pattern_add_color_stop( pattern, 0, light );
            cairo_pattern_add_color_stop( pattern, 1.0, base );

        } else {

            pattern.set( cairo_pattern_create_linear( 0, double(y)-0.2*h, 0, double(y) + h + 0.4*h ) );
            cairo_pattern_add_color_stop( pattern, 0, light );
            cairo_pattern_add_color_stop( pattern, 0.6, base );

        }

        cairo_set_source( context, pattern );
        _helper.fillSlab( context, x, y, w, h );

        if( options&Sunken )
        { _helper.slabSunken( base, 0 ).render( context, x, y, w, h ); }

    }

    //__________________________________________________________________
    void Style::renderSlab(
        GdkWindow* window,
        GdkRectangle* clipRect,
        gint x, gint y, gint w, gint h, const Gtk::Gap& gap, StyleOptions options ) const
    {

        // define colors
        ColorUtils::Rgba base;
        if( options&Blend )
        {

            gint wh, wy;
            Gtk::gdk_map_to_toplevel( window, 0L, &wy, 0L, &wh );
            base = ColorUtils::backgroundColor( settings().palette().color( Palette::Button ), wh, y+wy+h/2 );

        } else {

            base = settings().palette().color( Palette::Button );

        }

        // create context
        Cairo::Context context( window, clipRect );
        generateGapMask( context, x, y, w, h, gap );
        renderSlab( context, x, y, w, h, base, options, TileSet::Ring );

    }

    //__________________________________________________________________
    void Style::renderCheckBox(
        GdkWindow* window,
        GdkRectangle* clipRect,
        gint x, gint y, gint w, gint h, StyleOptions options ) const
    {

        // define checkbox rect
        gint cbw = CheckBox_Size;
        if( options & Flat ) cbw -= 5;
        GdkRectangle parent = {x, y, w, h };
        GdkRectangle child = {0, 0, cbw, cbw };
        centerRect( &parent, &child );

        // define colors
        ColorUtils::Rgba base;
        const Palette::Role role( options&Flat ? Palette::Window : Palette::Button );
        if( options&Blend )
        {

            gint wh, wy;
            Gtk::gdk_map_to_toplevel( window, 0L, &wy, 0L, &wh );
            base = ColorUtils::backgroundColor( settings().palette().color( role ), wh, y+wy+h/2 );

        } else {

            base = settings().palette().color( role );

        }

        // draw slab
        Cairo::Context context( window, clipRect );

        if( options & Flat )
        {

            _helper.holeFlat( base, 0 ).render( context, child.x+1, child.y-1, child.width, child.height, TileSet::Full );
            cairo_translate( context, 0, -2 );

        } else {

            StyleOptions localOptions( options );
            localOptions &= ~Sunken;
            renderSlab( context, child.x, child.y, child.width, child.height, base, localOptions );

        }

        // draw mark
        x = int( double(child.x + child.width/2) - 3.5 );
        y = int( double(child.y + child.height/2) - 2.5 );

        if( options&Sunken )
        {

            cairo_set_line_width( context, 2.0 );
            cairo_set_line_cap( context, CAIRO_LINE_CAP_ROUND );
            cairo_set_line_join( context, CAIRO_LINE_JOIN_ROUND );

            Palette::Group group( (options&Disabled) ? Palette::Disabled : Palette::Active );
            const ColorUtils::Rgba& color( settings().palette().color( group, ( options&Sunken ) ? Palette::WindowText : Palette::ButtonText ) );
            const ColorUtils::Rgba& background( settings().palette().color( ( options&Sunken ) ? Palette::Window : Palette::Button ) );
            const ColorUtils::Rgba& base( ColorUtils::decoColor( background, color ) );
            const ColorUtils::Rgba& contrast( ColorUtils::lightColor( background ) );

            cairo_translate( context, 0.5, 1.5 );
            if( !( options&Flat ) ) cairo_translate( context, 0, -1 );

            const double offset( 1.0 );
            if( settings().checkBoxStyle() == QtSettings::CS_CHECK )
            {

                cairo_save( context );
                cairo_translate( context, 0, offset );
                cairo_move_to( context, x+9, y );
                cairo_line_to( context, x+3, y+7 );
                cairo_line_to( context, x, y+4 );
                cairo_restore( context );
                cairo_set_source( context, contrast );
                cairo_stroke( context );

                cairo_move_to( context, x+9, y );
                cairo_line_to( context, x+3, y+7 );
                cairo_line_to( context, x, y+4 );
                cairo_set_source( context, base );
                cairo_stroke( context );

            } else {

                if( options&Sunken )
                {

                    cairo_save( context );
                    cairo_translate( context, 0, offset );
                    cairo_move_to( context, x+8, y ); cairo_line_to( context, x+1, y+7 );
                    cairo_move_to( context, x+8, y+7 ); cairo_line_to( context, x+1, y );
                    cairo_restore( context );
                    cairo_set_source( context, contrast );
                    cairo_stroke( context );

                    cairo_move_to( context, x+8, y ); cairo_line_to( context, x+1, y+7 );
                    cairo_move_to( context, x+8, y+7 ); cairo_line_to( context, x+1, y );
                    cairo_set_source( context, base );
                    cairo_stroke( context );

                } else {

                    cairo_save( context );
                    cairo_translate( context, 0, offset );
                    cairo_move_to( context, x+8, y-1 ); cairo_line_to( context, x, y+7 );
                    cairo_move_to( context, x+8, y+7 ); cairo_line_to( context, x, y-1 );
                    cairo_restore( context );
                    cairo_set_source( context, contrast );
                    cairo_stroke( context );

                    cairo_move_to( context, x+8, y-1 ); cairo_line_to( context, x, y+7 );
                    cairo_move_to( context, x+8, y+7 ); cairo_line_to( context, x, y-1 );
                    cairo_set_source( context, base );
                    cairo_stroke( context );

                }
            }


        }
    }
    //___________________________________________________________________
    void Style::renderRadioButton(
        GdkWindow* window,
        GdkRectangle* clipRect,
        gint x, gint y, gint w, gint h, StyleOptions options ) const
    {

        // define checkbox rect
        const gint cbw = CheckBox_Size;
        GdkRectangle parent = {x, y, w, h };
        GdkRectangle child = {0, 0, cbw, cbw };
        centerRect( &parent, &child );
        x = child.x;
        y = child.y;

        // define colors
        ColorUtils::Rgba base;
        if( options&Blend )
        {

            gint wh, wy;
            Gtk::gdk_map_to_toplevel( window, 0L, &wy, 0L, &wh );

            if( options & Menu ) base = ColorUtils::menuBackgroundColor( settings().palette().color( Palette::Button ), wh, y+wy+h/2 );
            else base = ColorUtils::backgroundColor( settings().palette().color( Palette::Button ), wh, y+wy+h/2 );

        } else {

            base = settings().palette().color( Palette::Button );

        }

        const ColorUtils::Rgba glow( slabShadowColor( options ) );

        // get the pixmap
        GdkPixbuf* pixbuf( glow.isValid() ? _helper.roundSlabFocused( base, glow, 0 ):_helper.roundSlab( base, 0 ) );

        // create context
        Cairo::Context context( window, clipRect );
        cairo_save( context );
        cairo_translate( context, x, y );
        cairo_rectangle( context, 0, 0, child.width, child.height );
        gdk_cairo_set_source_pixbuf( context, pixbuf, 0, 0 );
        cairo_fill( context );
        cairo_restore( context );
        g_object_unref( pixbuf );

        if( options&Sunken )
        {
            const double radius( 2.6 );
            const double dx( 0.5*child.width - radius );
            const double dy( 0.5*child.height - radius );

            Palette::Group group( (options&Disabled) ? Palette::Disabled : Palette::Active );
            const ColorUtils::Rgba background( settings().palette().color( Palette::Button ) );
            const ColorUtils::Rgba color( settings().palette().color( group, Palette::ButtonText ) );

            cairo_save( context );
            cairo_translate( context, 0, radius/2 );
            cairo_ellipse( context, x+dx, y+dy, child.width - 2*dx, child.height -2*dy );
            cairo_restore( context );
            cairo_set_source( context, ColorUtils::lightColor( background ) );
            cairo_fill( context );

            cairo_set_source( context, ColorUtils::decoColor( background, color ) );
            cairo_ellipse( context, x+dx, y+dy, child.width - 2*dx, child.height -2*dy );
            cairo_fill( context );

        }

        return;
    }

    //____________________________________________________________________________________
    void Style::renderHole(
        GdkWindow* window,
        GdkRectangle* clipRect,
        gint x, gint y, gint w, gint h, const Gtk::Gap& gap, StyleOptions options,
        TileSet::Tiles tiles ) const
    {

        // do nothing if not enough room
        if( w < 14 )  return;

        // load color
        const ColorUtils::Rgba base( settings().palette().color( Palette::Window ) );
        const bool fill( !( options&NoFill ) );
        if( fill ) tiles |= TileSet::Center;

        // create context, add mask, and render hole
        Cairo::Context context( window, clipRect );
        generateGapMask( context, x, y, w, h, gap );

        if( options & Focus )
        {

            const ColorUtils::Rgba glow( settings().palette().color( Palette::Focus ) );
            _helper.holeFocused( base, glow, 0, 7, fill ).render( context, x, y, w, h, tiles );

        } else if( options & Hover ) {

            const ColorUtils::Rgba glow( settings().palette().color( Palette::Hover ) );
            _helper.holeFocused( base, glow, 0, 7, fill ).render( context, x, y, w, h, tiles );

        } else {

            _helper.hole( base, 0, 7, fill ).render( context, x, y, w, h, tiles );

        }

    }

    //____________________________________________________________________________________
    void Style::renderDockFrame(
        GdkWindow* window,
        GdkRectangle* clipRect,
        gint x, gint y, gint w, gint h, const Gtk::Gap& gap, StyleOptions options ) const
    {

        // define colors
        ColorUtils::Rgba base;
        if( options&Blend )
        {

            gint wh, wy;
            Gtk::gdk_map_to_toplevel( window, 0L, &wy, 0L, &wh );
            base = ColorUtils::backgroundColor( settings().palette().color( Palette::Button ), wh, y+wy+h/2 );

        } else {

            base = settings().palette().color( Palette::Button );

        }

        // create context, add mask, and render
        Cairo::Context context( window, clipRect );
        generateGapMask( context, x, y, w, h, gap );
        _helper.dockFrame( base, w ).render( context, x, y, w, h );
    }

    //____________________________________________________________________________________
    void Style::renderMenuItemRect(
        GdkWindow* window,
        GdkRectangle* clipRect,
        GtkWidget* widget,
        gint x, gint y, gint w, gint h, StyleOptions options ) const
    {

        ColorUtils::Rgba base;
        gint wh, wy;
        Gtk::gdk_map_to_toplevel( window, 0L, &wy, 0L, &wh );
        const bool isInMenu( Gtk::gtk_parent_menu( widget ) );
        if( isInMenu )
        {

            if( wh > 0 ) base = ColorUtils::menuBackgroundColor( settings().palette().color( Palette::Window ), wh, y+wy+h/2 );
            else base = ColorUtils::alphaColor( ColorUtils::darkColor( settings().palette().color( Palette::Window ) ), 0.3 );

        } else if( wh > 0 ) {

            base = ColorUtils::backgroundColor( settings().palette().color( Palette::Window ), wh, y+wy+h/2 );

        } else {

            base = ColorUtils::backgroundTopColor( settings().palette().color( Palette::Window ) );

        }

        // more color customization, based on menuHighlight mode
        ColorUtils::Rgba color( base );
        if( settings().menuHighlightMode() == QtSettings::MM_STRONG )
        {

            if( (options & Sunken) || isInMenu ) color = settings().palette().color( Palette::Focus );
            else color = ColorUtils::tint( color, settings().palette().color( Palette::Hover ) );

        } else if( settings().menuHighlightMode() == QtSettings::MM_SUBTLE ) {

            if( (options & Sunken) || isInMenu ) color = ColorUtils::mix( color, ColorUtils::tint( color, settings().palette().color( Palette::Focus ), 0.6 ) );
            else color = ColorUtils::mix( color, ColorUtils::tint( color, settings().palette().color( Palette::Hover ), 0.6 ) );

        } else if( !( isInMenu && wh<= 0 ) ) color = ColorUtils::midColor( color );


        // adjust vertical extent of the rect in menus
        if( isInMenu )
        {
            y+=1;
            h-=2;
        }

        bool hasSubMenu( isInMenu && GTK_IS_MENU_ITEM( widget ) && gtk_menu_item_get_submenu( GTK_MENU_ITEM( widget ) ) );
        if( hasSubMenu )
        {

            // draw item rect in a pixbuf
            GdkPixbuf* pixbuf( gdk_pixbuf_new( GDK_COLORSPACE_RGB, true, 8, w, h ) );
            gdk_pixbuf_fill( pixbuf, ColorUtils::Rgba::transparent( color ).toInt() );
            {
                Cairo::Context context( pixbuf );
                _helper.holeFlat( color, 0 ).render( context, 0, 0, w, h, TileSet::Full  );
                context.updateGdkPixbuf();
            }

            Cairo::Context context( window, clipRect );
            cairo_translate( context, x, y );
            gdk_cairo_set_source_pixbuf( context, pixbuf, 0, 0 );

            // generate linear gradient for masking
            Cairo::Pattern mask( cairo_pattern_create_linear( w-40, 0, w-4, 0 ) );
            cairo_pattern_add_color_stop( mask, 0,  ColorUtils::Rgba::black() );
            cairo_pattern_add_color_stop( mask, 1,  ColorUtils::Rgba::transparent() );
            cairo_mask( context, mask );

            g_object_unref( pixbuf );

        } else {

            Cairo::Context context( window, clipRect );
            _helper.holeFlat( color, 0 ).render( context, x, y, w, h, TileSet::Full  );
        }
    }

    //____________________________________________________________________________________
    void Style::renderSelection(
        GdkWindow* window,
        GdkRectangle* clipRect,
        gint x, gint y, gint w, gint h,
        TileSet::Tiles tiles,
        StyleOptions options
        ) const
    {

        ColorUtils::Rgba base;
        if( options & Selected  ) base = settings().palette().color( Palette::Focus );
        else if( options & Hover ) base = settings().palette().color( Palette::Hover );
        else return;

        // create context
        Cairo::Context context( window, clipRect );
        if( !(tiles&TileSet::Left) ) { x -= 8; w+=8; }
        if( !(tiles&TileSet::Right) ) { w += 8; }
        _helper.selection( base, h, false ).render( context, x, y, w, h, tiles );

    }

    //____________________________________________________________________________________
    void Style::renderArrow(
        GdkWindow* window,
        GdkRectangle* clipRect,
        GtkArrowType orientation,
        gint x, gint y, gint w, gint h,
        QtSettings::ArrowSize arrowSize, StyleOptions options ) const
    {

        // get polygon
        const Polygon arrow( genericArrow( orientation, arrowSize ) );

        // retrieve colors
        ColorUtils::Rgba base;
        if( options&Disabled ) base = settings().palette().color( Palette::Disabled, Palette::ButtonText );
        else if( options&Hover ) base = settings().palette().color( Palette::Hover );
        else base = settings().palette().color( Palette::Active, Palette::ButtonText );

        const int xcenter = x + w/2;
        const int ycenter = y + h/2;

        // create context and translate to center
        Cairo::Context context( window, clipRect );
        cairo_translate( context, xcenter, ycenter );

        switch( orientation )
        {
            case GTK_ARROW_UP:
            //cairo_translate( context, 0., -0.5 );
            break;

            case GTK_ARROW_DOWN:
            if( arrowSize == QtSettings::ArrowSmall ) cairo_translate( context, 0, 0.5 );
            else cairo_translate( context, 0, 1 );
            break;

            case GTK_ARROW_LEFT:
            case GTK_ARROW_RIGHT:
            if( arrowSize == QtSettings::ArrowSmall )
            { cairo_translate( context, 0.5, 0 ); }
            break;

            default:
            break;
        }

        switch( arrowSize )
        {
            case QtSettings::ArrowTiny:
            case QtSettings::ArrowSmall:
            cairo_set_line_width( context, 1.2 );
            break;

            default:
            case QtSettings::ArrowNormal:
            cairo_set_line_width( context, 1.6 );
            break;
        }

        cairo_set_line_cap( context, CAIRO_LINE_CAP_ROUND );
        cairo_set_line_join( context, CAIRO_LINE_JOIN_ROUND );

        // contrast
        if( options&Contrast )
        {

            // contrast color
            const ColorUtils::Rgba contrast( ColorUtils::lightColor( settings().palette().color( Palette::Window ) ) );

            cairo_save( context );
            cairo_translate( context, 0, 1 );
            cairo_polygon( context, arrow );
            cairo_restore( context );

            cairo_set_source( context, contrast );
            cairo_stroke( context );
        }

        // base
        cairo_polygon( context, arrow );
        cairo_set_source( context, base );
        cairo_stroke( context );

    }

    //____________________________________________________________________________________
    void Style::renderSliderGroove(
        GdkWindow* window,
        GdkRectangle* clipRect,
        gint x, gint y, gint w, gint h, StyleOptions options ) const
    {

        const ColorUtils::Rgba base( settings().palette().color( Palette::Window ) );

        const bool vertical( options & Vertical );
        GdkRectangle parent = { x, y, w, h };

        if( vertical ) h-=30;
        else w-=30;
        GdkRectangle child = { 0, 0, vertical ? 5:w, vertical ? h:5 };
        centerRect( &parent, &child );

        Cairo::Context context( window, clipRect );
        _helper.groove( base, 0 ).render( context, child.x, child.y, child.width, child.height );

    }

    //____________________________________________________________________________________
    void Style::renderSliderHandle(
        GdkWindow* window,
        GdkRectangle* clipRect,
        gint x, gint y, gint w, gint h, StyleOptions options ) const
    {

        // sclider dimension
        int SliderThickness = 23;
        int SliderLength = 15;

        const bool vertical( options&Vertical );
        GdkRectangle parent = { x, y, w, h };
        GdkRectangle child = { 0, 0, vertical ? SliderThickness:SliderLength, vertical ? SliderLength:SliderThickness };

        centerRect( &parent, &child );

        // define colors
        ColorUtils::Rgba base;
        if( options&Blend )
        {

            gint wh, wy;
            Gtk::gdk_map_to_toplevel( window, 0L, &wy, 0L, &wh );
            base = ColorUtils::backgroundColor( settings().palette().color( Palette::Button ), wh, y+wy+h/2 );

        } else {

            base = settings().palette().color( Palette::Button );

        }

        // render slab
        options &= ~Sunken;
        Cairo::Context context( window, clipRect );
        renderSlab( context, child.x, child.y, child.width, child.height, base, options );

    }

    //____________________________________________________________________________________
    void Style::renderSizeGrip(
        GdkWindow* window,
        GdkRectangle* clipRect,
        GdkWindowEdge edge,
        gint x, gint y, gint w, gint h ) const
    {

        gint dimension = std::min( w, h );

        // edges
        Polygon a;
        switch( edge )
        {

            case GDK_WINDOW_EDGE_NORTH_WEST:
            w = dimension;
            h = dimension;
            a << Point( double(x)+0.5, double(y)+0.5 ) << Point( double(x+w)-0.5, double(y)+0.5 ) << Point( double(x)+0.5, double(y+h)-0.5);
            break;

            case GDK_WINDOW_EDGE_NORTH_EAST:
            x+= w-dimension;
            w = dimension;
            h = dimension;
            a << Point( double(x)+0.5, double(y)+0.5 ) << Point( double(x+w)-0.5, double(y)+0.5 ) << Point( double(x+w)-0.5, double(y+h)-0.5 );
            break;

            case GDK_WINDOW_EDGE_SOUTH_WEST:
            y+= h-dimension;
            w = dimension;
            h = dimension;
            a << Point( double(x)+0.5, double(y)+0.5 ) << Point( double(x+w)-0.5, double(y+h)-0.5 ) << Point( double(x)+0.5, double(y+h)-0.5 );
            break;

            case GDK_WINDOW_EDGE_SOUTH_EAST:
            x+= w-dimension;
            y+= h-dimension;
            w = dimension;
            h = dimension;
            a << Point( double(x)+0.5, double(y+h)-0.5 ) << Point( double(x+w)-0.5, double(y)+0.5 ) << Point( double(x+w)-0.5, double(y+h)-0.5 );
            break;

            default: return;

        }


        // colors
        const ColorUtils::Rgba base( settings().palette().color( Palette::Window ) );
        const ColorUtils::Rgba dark( ColorUtils::darkColor( base ) );
        const ColorUtils::Rgba light( ColorUtils::lightColor( base ) );

        // context
        Cairo::Context context( window, clipRect );
        cairo_set_line_width( context, 1.0 );

        // fill
        cairo_polygon( context, a );
        cairo_set_source( context, base );
        cairo_fill( context );

        // draw edges
        cairo_move_to( context, a[0].x(), a[0].y() );
        cairo_line_to( context, a[1].x(), a[1].y() );
        cairo_set_source( context, dark );
        cairo_stroke( context );

        cairo_move_to( context, a[1].x(), a[1].y() );
        cairo_line_to( context, a[2].x(), a[2].y() );
        cairo_line_to( context, a[0].x(), a[0].y() );
        cairo_set_source( context, light );
        cairo_stroke( context );

    }

    //____________________________________________________________________________________
    void Style::renderTab(
        GdkWindow* window,
        GdkRectangle* clipRect,
        gint x, gint y, gint w, gint h,
        GtkPositionType side,
        StyleOptions options,
        TabOptions tabOptions
        ) const
    {

        const bool isCurrentTab( tabOptions & CurrentTab );
        const bool isFirstTab( tabOptions & FirstTab );

        // get color
        const ColorUtils::Rgba base( settings().palette().color( Palette::Window ) );
        const ColorUtils::Rgba light( ColorUtils::lightColor( base ) );
        const ColorUtils::Rgba dark( ColorUtils::darkColor( base ) );

        // create context
        Cairo::Context context( window, clipRect );

        // borders and connections to tabs
        // this is quite painfull and slipery code.
        // the same is true with oxygen-qt
        int offset = 2;
        int adjust = settings().applicationName().isFirefox() ? 0:2;

        SlabRect tabSlab;
        SlabRect::List slabs;
        switch( side )
        {
            case GTK_POS_BOTTOM:
            {
                // main slab
                y += adjust; h -= 2*adjust;
                tabSlab = SlabRect( x, y-offset, w, h+10 + offset, TileSet::Ring&(~TileSet::Bottom ) );
                if( isFirstTab ) { tabSlab._x-=1; tabSlab._w+=1; }

                // connections to frame
                if( isCurrentTab )
                {

                    if( isFirstTab ) slabs.push_back( SlabRect( x-1, y+h+offset-6, 8, 18, TileSet::Left ) );
                    else slabs.push_back( SlabRect( x-7, y+h-1, 3+14, 10, TileSet::Top ) );
                    slabs.push_back( SlabRect( x+w-10, y+h-1, 3+14, 10, TileSet::Top ) );

                } else if( isFirstTab ) {

                    slabs.push_back( SlabRect( x-1, y+h-1, w+4+2, 12, TileSet::Top|TileSet::Left ) );
                    slabs.push_back( SlabRect( x-1, y+h+offset-6, 8, 16, TileSet::Left ) );

                } else {

                    slabs.push_back( SlabRect( x-4-1, y+h-1, w+8+2, 10, TileSet::Top ) );

                }

                break;
            }

            case GTK_POS_TOP:
            {

                // main slab
                y += adjust; h -= 2*adjust;
                tabSlab = SlabRect( x, y-10, w, h+10+offset, TileSet::Ring&(~TileSet::Top ) );
                if( isFirstTab ) { tabSlab._x-=1; tabSlab._w+=1; }

                // connections to frame
                if( isCurrentTab )
                {

                    if( isFirstTab ) slabs.push_back( SlabRect( x-1, y-7-7, 8, 18, TileSet::Left ) );
                    else slabs.push_back( SlabRect( x-7, y-9, 3+14, 10, TileSet::Bottom ) );
                    slabs.push_back( SlabRect( x+w-10, y-9, 3+14, 10, TileSet::Bottom ) );

                } else if( isFirstTab ) {

                    slabs.push_back( SlabRect( x-1, y-10+1, w+4+2, 10, TileSet::Bottom|TileSet::Left ) );

                } else {

                    slabs.push_back( SlabRect( x-4-1, y-10+1, w+8+2, 10, TileSet::Bottom ) );

                }

                break;
            }

            case GTK_POS_RIGHT:
            {

                // main slab
                x += adjust; w -= 2*adjust;
                tabSlab = SlabRect( x-offset, y, w+10+offset, h, TileSet::Ring&(~TileSet::Right ) );
                if( isFirstTab ) { tabSlab._y-=1; tabSlab._h+=1; }

                // connections to frame
                if( isCurrentTab )
                {

                    if( isFirstTab ) slabs.push_back( SlabRect( x+w+offset-6, y-1, 18, 8, TileSet::Top ) );
                    else slabs.push_back( SlabRect( x+w-1, y-7, 10, 3+14, TileSet::Left ) );
                    slabs.push_back( SlabRect( x+w-1, y+h-10, 10, 3+14, TileSet::Left ) );

                } else if( isFirstTab ) {

                    slabs.push_back( SlabRect( x+w-1, y-1, 12, h+4+3, TileSet::Top|TileSet::Left ) );
                    slabs.push_back( SlabRect( x+w+offset-6, y-1, 16, 8, TileSet::Top ) );

                } else {

                    slabs.push_back( SlabRect( x+w-1, y-4-1, 10, h+8+3, TileSet::Left ) );

                }
                break;
            }


            case GTK_POS_LEFT:
            {

                // main slab
                x += adjust; w -= 2*adjust;
                tabSlab = SlabRect( x-10, y, w+10+offset, h, TileSet::Ring&(~TileSet::Left ) );
                if( isFirstTab ) { tabSlab._y-=1; tabSlab._h+=1; }

                // connections to frame
                if( isCurrentTab )
                {

                    if( isFirstTab ) slabs.push_back( SlabRect( x-7-7, y-1, 18, 8, TileSet::Top ) );
                    else slabs.push_back( SlabRect( x-9, y-7, 10, 3+14, TileSet::Right ) );
                    slabs.push_back( SlabRect( x-9, y+h-10, 10, 3+14, TileSet::Right ) );

                } else if( isFirstTab ) {

                    slabs.push_back( SlabRect( x-10+1, y-1, 10, h+4+3, TileSet::Top|TileSet::Right ) );
                    slabs.push_back( SlabRect( x-10+1 + offset-5, y-1, 16, 8, TileSet::Top ) );

                } else {

                    slabs.push_back( SlabRect( x-10+1, y-4-1, 10, h+8+3, TileSet::Right ) );

                }

                break;

            }

            default: return;
        }

        // adjust rect for filling
        SlabRect fillSlab( tabSlab );
        fillSlab._x += 4;
        fillSlab._y += 4;
        fillSlab._w -= 8;
        fillSlab._h -= 8;

        // fill
        Cairo::Pattern pattern;
        int dimension = 0;
        switch( side )
        {
            case GTK_POS_BOTTOM:
            dimension = fillSlab._h;
            if( isCurrentTab ) fillSlab._h -= 2;
            else fillSlab._h -= 3;
            pattern.set( cairo_pattern_create_linear( 0, y-4, 0, y+h+10 ) );
            break;

            case GTK_POS_TOP:
            dimension = fillSlab._h;
            if( isCurrentTab )
            {

                fillSlab._y += 2;
                fillSlab._h -= 2;

            } else {

                fillSlab._y += 3;
                fillSlab._h -= 3;

            }
            pattern.set( cairo_pattern_create_linear( 0, y+h+2, 0, y-10 ) );
            break;

            case GTK_POS_RIGHT:
            dimension = fillSlab._w;
            if( isCurrentTab ) fillSlab._w -= 2;
            else fillSlab._w -= 3;
            pattern.set( cairo_pattern_create_linear( x-4, 0, x+w+10, 0 ) );
            break;

            case GTK_POS_LEFT:
            dimension = fillSlab._w;
            if( isCurrentTab )
            {
                fillSlab._x += 2;
                fillSlab._w -= 2;
            } else {
                fillSlab._x += 3;
                fillSlab._w -= 3;
            }
            pattern.set( cairo_pattern_create_linear( x+w+2, 0, x-10, 0 ) );
            break;

            default: return;

        }

        if( isCurrentTab )
        {

            cairo_pattern_add_color_stop( pattern, 0.0, ColorUtils::alphaColor( light, 0.5 ) );
            cairo_pattern_add_color_stop( pattern, 0.1, ColorUtils::alphaColor( light, 0.5 ) );
            cairo_pattern_add_color_stop( pattern, 0.25, ColorUtils::alphaColor( light, 0.3 ) );
            cairo_pattern_add_color_stop( pattern, 0.5, ColorUtils::alphaColor( light, 0.2 ) );
            cairo_pattern_add_color_stop( pattern, 0.75, ColorUtils::alphaColor( light, 0.1 ) );
            cairo_pattern_add_color_stop( pattern, 0.9, ColorUtils::Rgba::transparent( light ) );

        } else {

            cairo_pattern_add_color_stop( pattern, 0.0, ColorUtils::alphaColor( light, 0.1 ) );
            cairo_pattern_add_color_stop( pattern, 0.4, ColorUtils::alphaColor( dark, 0.5 ) );
            cairo_pattern_add_color_stop( pattern, std::max( 0.8, 1.0 - 10.0/dimension ), ColorUtils::alphaColor( dark, 0.4 ) );
            cairo_pattern_add_color_stop( pattern, std::max( 0.9, 1.0 - 5.0/dimension ), ColorUtils::Rgba::transparent( dark ) );

        }

        // in firefox a solid background must be filled
        if( isCurrentTab && settings().applicationName().isFirefox() )
        {
            cairo_set_source( context, base );
            cairo_rectangle( context, fillSlab._x, fillSlab._y, fillSlab._w, fillSlab._h );
            cairo_fill( context );
        }

        // draw pattern
        cairo_set_source( context, pattern );
        cairo_rectangle( context, fillSlab._x, fillSlab._y, fillSlab._w, fillSlab._h );
        cairo_fill( context );

        // render tab
        if( (options&Hover) && !isCurrentTab )
        {

            const ColorUtils::Rgba glow( settings().palette().color( Palette::Hover ) );
            _helper.slabFocused( base, glow, 0 ).render( context, tabSlab._x, tabSlab._y, tabSlab._w, tabSlab._h, tabSlab._tiles );

        } else {

            _helper.slab( base, 0 ).render( context, tabSlab._x, tabSlab._y, tabSlab._w, tabSlab._h, tabSlab._tiles );

        }

        // render connections to frame
        for( SlabRect::List::const_iterator iter = slabs.begin(); iter != slabs.end(); iter++ )
        { _helper.slab(base, 0).render( context, iter->_x, iter->_y, iter->_w, iter->_h, iter->_tiles ); }

    }

     //____________________________________________________________________________________
    void Style::renderTreeExpander(
        GdkWindow* window,
        GdkRectangle* clipRect,
        gint x, gint y, gint w, gint h,
        GtkExpanderStyle style,
        StyleOptions options
        ) const
    {

        // retrieve colors
        ColorUtils::Rgba base;
        if( options&Disabled ) base = settings().palette().color( Palette::Disabled, Palette::ButtonText );
        else if( options&Hover ) base = settings().palette().color( Palette::Hover );
        else base = settings().palette().color( Palette::Active, Palette::ButtonText );

        const int xcenter = x + w/2;
        const int ycenter = y + h/2;

        // expander 'radius' (copied from oxygen-qt)
        const int radius( ( 9 - 4 ) / 2 );

        // create context and translate to center
        Cairo::Context context( window, clipRect );
        cairo_translate( context, xcenter, ycenter );

        cairo_set_line_width( context, 1.0 );
        cairo_set_source( context, base );

        // horizontal line
        cairo_move_to( context, 0.5-radius, 0.5 );
        cairo_line_to( context, 0.5+radius, 0.5 );

        // vertical line
        if( style == GTK_EXPANDER_COLLAPSED || style == GTK_EXPANDER_SEMI_COLLAPSED )
        {
            cairo_move_to( context, 0.5, 0.5-radius );
            cairo_line_to( context, 0.5, 0.5+radius );
        }

        cairo_stroke( context );

    }

    //__________________________________________________________________
    void Style::sanitizeSize( GdkWindow* window, gint& w, gint& h ) const
    {
        if( w < 0 && h < 0 ) gdk_drawable_get_size( window, &w, &h );
        else if( w < 0 ) gdk_drawable_get_size( window, &w, 0L );
        else if( h < 0 ) gdk_drawable_get_size( window, 0L, &h );
    }

    //____________________________________________________________________________________
    ColorUtils::Rgba Style::slabShadowColor( StyleOptions options ) const
    {

        ColorUtils::Rgba glow;
        if( options & Hover ) glow = settings().palette().color( Palette::Hover );
        else if( options & Focus ) glow =  settings().palette().color( Palette::Focus );
        return glow;

    }

    //__________________________________________________________________
    void Style::renderSlab( Cairo::Context& context, gint x, gint y, gint w, gint h, const ColorUtils::Rgba& base, StyleOptions options, TileSet::Tiles tiles ) const
    {

        // check rect
        if( !( w>=0 && h>=0 ) ) return;

        // additional adjustment for sunken frames
        // TODO: double check
        if( options & Sunken)
        {

            x -= 1;
            w += 2;
            h += 2;

        }

        // fill
        if( !(options & NoFill))
        {

            Cairo::Pattern pattern;
            const ColorUtils::Rgba shadow( ColorUtils::shadowColor( base ) );
            if( shadow.value() > base.value() && (options & Sunken) )
            {

                pattern.set( cairo_pattern_create_linear( 0, y, 0, y+2*h ) );
                cairo_pattern_add_color_stop( pattern, 0, base );
                cairo_pattern_add_color_stop( pattern, 1.0, ColorUtils::lightColor( base ) );

            } else {

                pattern.set( cairo_pattern_create_linear( 0, y-h, 0, y+h ) );
                cairo_pattern_add_color_stop( pattern, 0, ColorUtils::lightColor( base ) );
                cairo_pattern_add_color_stop( pattern, 1.0, base );

            }

            cairo_set_source( context, pattern );
            _helper.fillSlab( context, x, y, w, h );

        }

        if( !(options&Sunken) ) {

            // calculate glow color
            TileSet tile;
            ColorUtils::Rgba glow( slabShadowColor( options ) );
            if( glow.isValid() ) tile = _helper.slabFocused( base, glow , 0);
            else if( base.isValid() ) tile = _helper.slab( base, 0 );
            else return;
            tile.render( context, x, y, w, h );

        } else if( base.isValid() ) {

            _helper.slabSunken( base, 0 ).render( context, x, y, w, h );

        }

    }

    //__________________________________________________________________
    void Style::renderScrollBarHole( Cairo::Context& context, gint x, gint y, gint w, gint h, const ColorUtils::Rgba& base, bool vertical, TileSet::Tiles tiles ) const
    {

        // use tileset from helper
        _helper.scrollHole( base, vertical ).render( context, x, y, w, h, tiles );

    }

    //__________________________________________________________________
    Polygon Style::genericArrow( GtkArrowType orientation, QtSettings::ArrowSize size ) const
    {

        Polygon a;
        switch( orientation )
        {

            case GTK_ARROW_UP:
            {
                if( size == QtSettings::ArrowTiny ) a << Point( -1.75, 1.125 ) << Point( 0.5, -1.125 ) << Point( 2.75, 1.125 );
                else if( size == QtSettings::ArrowSmall ) a << Point( -2,1.5 ) << Point( 0.5, -1.5 ) << Point( 3,1.5 );
                else a << Point( -3,2.5 ) << Point( 0.5, -1.5 ) << Point( 4,2.5 );
                break;
            }

            case GTK_ARROW_DOWN:
            {
                if( size == QtSettings::ArrowTiny ) a << Point( -1.75, -1.125 ) << Point( 0.5, 1.125 ) << Point( 2.75, -1.125 );
                else if( size == QtSettings::ArrowSmall ) a << Point( -2,-1.5 ) << Point( 0.5, 1.5 ) << Point( 3,-1.5 );
                else a << Point( -3,-1.5 ) << Point( 0.5, 2.5 ) << Point( 4,-1.5 );
                break;
            }

            case GTK_ARROW_LEFT:
            {
                if( size == QtSettings::ArrowTiny ) a << Point( 1.125, -1.75 ) << Point( -1.125, 0.5 ) << Point( 1.125, 2.75 );
                else if( size == QtSettings::ArrowSmall ) a << Point( 1.5,-2 ) << Point( -1.5, 0.5 ) << Point( 1.5,3 );
                else a << Point( 2.5,-3 ) << Point( -1.5, 0.5 ) << Point( 2.5,4 );
                break;
            }

            case GTK_ARROW_RIGHT:
            {
                if( size == QtSettings::ArrowTiny ) a << Point( -1.125, -1.75 ) << Point( 1.125, 0.5 ) << Point( -1.125, 2.75 );
                else if( size == QtSettings::ArrowSmall ) a << Point( -1.5,-2 ) << Point( 1.5, 0.5 ) << Point( -1.5,3 );
                else a << Point( -1.5,-3 ) << Point( 2.5, 0.5 ) << Point( -1.5,4 );
                break;
            }

            default: break;

        }

        return a;



    }

    //__________________________________________________________________
    cairo_pattern_t* Style::verticalGradient( const ColorUtils::Rgba& base, int y1, int y2 ) const
    {

        ColorUtils::Rgba top( ColorUtils::backgroundTopColor( base ) );
        ColorUtils::Rgba bottom( ColorUtils::backgroundBottomColor( base ) );
        cairo_pattern_t* pattern = cairo_pattern_create_linear( 0, y1, 0, y2 );
        cairo_pattern_add_color_stop( pattern, 0, top );
        cairo_pattern_add_color_stop( pattern, 0.5, base );
        cairo_pattern_add_color_stop( pattern, 1, bottom );
        return pattern;
    }

    //__________________________________________________________________
    cairo_pattern_t* Style::radialGradient( const ColorUtils::Rgba& base, int x, int y, int r ) const
    {
        // create radial pattern
        ColorUtils::Rgba radial( ColorUtils::backgroundRadialColor( base ) );
        cairo_pattern_t* pattern( cairo_pattern_create_radial( x, y, r ) );
        cairo_pattern_add_color_stop( pattern, 0, radial );
        cairo_pattern_add_color_stop( pattern, 0.5, ColorUtils::alphaColor( radial, 101.0/255 ) );
        cairo_pattern_add_color_stop( pattern, 0.75, ColorUtils::alphaColor( radial, 37.0/255 ) );
        cairo_pattern_add_color_stop( pattern, 1.0, ColorUtils::alphaColor( radial, 0 ) );
        return pattern;
    }

    //__________________________________________________________________
    void Style::centerRect( GdkRectangle* parent, GdkRectangle* child ) const
    {
        if( !( parent && child ) ) return;
        child->x = parent->x + (parent->width - child->width)/2;
        child->y = parent->y + (parent->height - child->height)/2;
        return;
    }

    //__________________________________________________________________
    void Style::generateGapMask( Cairo::Context& context, gint x, gint y, gint w, gint h, const Gtk::Gap& gap ) const
    {

        if( gap.width() <= 0 ) return;

        // store current rect in
        GdkRectangle r = { x, y, w, h };
        GdkRegion* region = gdk_region_rectangle( &r );

        GdkRegion* mask = 0L;
        switch( gap.position() )
        {
            case GTK_POS_TOP:
            {
                GdkRectangle mask_r = { x+gap.x(), y, gap.width(), gap.height() };
                mask = gdk_region_rectangle( &mask_r );
                break;
            }

            case GTK_POS_BOTTOM:
            {
                GdkRectangle mask_r = { x+gap.x(), y+h-gap.height(), gap.width(), gap.height() };
                mask = gdk_region_rectangle( &mask_r );
                break;
            }

            case GTK_POS_LEFT:
            {
                GdkRectangle mask_r = { x, y+gap.x(), gap.height(), gap.width() };
                mask = gdk_region_rectangle( &mask_r );
                break;
            }

            case GTK_POS_RIGHT:
            {
                GdkRectangle mask_r = { x + w - gap.height(), y+gap.x(), gap.height(), gap.width() };
                mask = gdk_region_rectangle( &mask_r );
                break;
            }

            default: return;
        }

        gdk_region_subtract( region, mask );
        gdk_cairo_region( context, region );
        cairo_clip( context );

        gdk_region_destroy( region );
        gdk_region_destroy( mask );

        return;

    }

}
