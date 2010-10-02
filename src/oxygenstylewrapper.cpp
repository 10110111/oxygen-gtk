/*
* this file is part of the oxygen gtk engine
* Copyright( c ) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
*
* based on the Null Theme Engine for Gtk+.
* Copyright( C ) 2008 Robert Staudinger
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

#include <gtk/gtk.h>
#include <iostream>

#include "oxygen.h"
#include "oxygengtkdetails.h"
#include "oxygengtkutils.h"
#include "oxygenmaps.h"
#include "oxygenrcstyle.h"
#include "oxygenstyle.h"
#include "oxygenstylewrapper.h"

//______________________________________________________________________
struct _OxygenStyle
{ GtkStyle parent; };

//______________________________________________________________________
struct _OxygenStyleClass
{  GtkStyleClass parent; };

//___________________________________________________________
static GType oxygen_style_type = 0;
static GtkStyleClass* oxygen_style_parent_class = 0L;

//___________________________________________________________
static void draw_flat_box(
    GtkStyle* style,
    GdkWindow* window,
    GtkStateType state,
    GtkShadowType shadow,
    GdkRectangle* clipRect,
    GtkWidget* widget,
    const char* detail,
    gint x,
    gint y,
    gint w,
    gint h )
{
    g_return_if_fail( style && window );
    Oxygen::Style::instance().sanitizeSize( window, w, h );
    g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
        "widget=%s, primitive=flat_box, state=%s, shadow=%s, detail=%s",
        G_OBJECT_TYPE_NAME( widget ),
        Oxygen::Maps::getState( state ),
        Oxygen::Maps::getShadow( shadow ),
        detail );

    const Gtk::Detail d( detail );
    bool accepted( false );
    if( d.isBase() || d.isEventBox() )
    {

        Oxygen::Style::instance().renderWindowBackground( window, clipRect, x, y, w, h );
        return;

    } else if( d.isViewportBin() ) {

        GtkShadowType shadow = gtk_viewport_get_shadow_type( GTK_VIEWPORT( widget ) );
        if( shadow == GTK_SHADOW_NONE )
        {
            Oxygen::Style::instance().renderWindowBackground( window, clipRect, x, y, w, h );
            accepted = true;
        }

    } else if( d.isTrough() ) {

        if( GTK_IS_PROGRESS_BAR( widget ) )
        { return; }

    } else if( d.isTooltip() ) {

        Oxygen::StyleOptions options( Gtk::gtk_widget_has_rgba( widget ) ? Oxygen::Alpha : Oxygen::None );
        Oxygen::Style::instance().renderTooltipBackground( window, clipRect, x, y, w, h, options );
        return;

    } else if(
       d.isCheckButton() ||
       d.isRadioButton() ||
       d.isExpander() ) {

        return;

    } else if( d.isCell() ) {

        if( state == GTK_STATE_SELECTED  || state == GTK_STATE_PRELIGHT )
        {

            unsigned int tiles( Oxygen::TileSet::Center );
            if( d.isCellStart() ) tiles |= Oxygen::TileSet::Left;
            else if( d.isCellEnd() ) tiles |= Oxygen::TileSet::Right;
            else if( !d.isCellMiddle() ) tiles = Oxygen::TileSet::Horizontal;

            Oxygen::StyleOptions options( Oxygen::styleOptions( widget, state ) );
            Oxygen::Style::instance().renderSelection( window, clipRect, x, y, w, h, tiles, options );
        }

        // TODO: handle non selected alternate background,
        // when enabled in options

        return;

    }

    if( !accepted )
    {

        // fallback to parent style
        oxygen_style_parent_class->draw_flat_box( style, window, state,
            shadow, clipRect, widget, detail,
            x, y, w, h );

    }

}

//___________________________________________________________
static void draw_box( GtkStyle* style,
    GdkWindow* window,
    GtkStateType state,
    GtkShadowType shadow,
    GdkRectangle* clipRect,
    GtkWidget* widget,
    const gchar* detail,
    gint x,
    gint y,
    gint w,
    gint h )
{
    g_return_if_fail( style && window );

    g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
        "widget=%s, primitive=box, state=%s, shadow=%s, detail=%s",
        G_OBJECT_TYPE_NAME( widget ),
        Oxygen::Maps::getState( state ),
        Oxygen::Maps::getShadow( shadow ),
        detail );

    Oxygen::Style::instance().sanitizeSize( window, w, h );
    const Gtk::Detail d( detail );
    if( d.isButton() || d.isOptionMenu() )
    {
        if( Gtk::gtk_parent_treeview( widget ) )
        {

            Oxygen::Style::instance().renderHeaderBackground( window, clipRect, x, y, w, h );

        } else {

            Oxygen::StyleOptions options( Oxygen::Blend );
            options |= Oxygen::styleOptions( widget, state, shadow );
            Oxygen::Style::instance().renderButtonSlab( window, clipRect, x, y, w, h, options );

        }

    } else if( d.isMenuBar() || d.isToolbar() ) {

        Oxygen::Style::instance().renderWindowBackground( window, clipRect, x, y, w, h );
        return;

    } else if( d.isMenu() ) {

        Oxygen::StyleOptions options( Gtk::gtk_widget_has_rgba( widget ) ? Oxygen::Alpha : Oxygen::None );
        Oxygen::Style::instance().renderMenuBackground( window, clipRect, x, y, w, h, options );
        Oxygen::Style::instance().drawFloatFrame( window, clipRect, x, y, w, h, options );

    } else if( d.isDefaultButton() || d.isScrollBar() || d.isPaned() || d.isHandleBox() ) {

        return;

    } else if( d.isMenuItem() ) {

        Oxygen::Style::instance().renderMenuItemRect( window, clipRect, widget, x, y, w, h );

    } else if( d.isTrough() && shadow == GTK_SHADOW_IN ) {

        if( GTK_IS_PROGRESS_BAR( widget ) )
        {

            if( !Oxygen::Style::instance().settings().applicationName().isFirefox() )
            {
                /*
                need to call the parent style implementation here,
                otherwise some uninitialized pixels are present.
                Not sure why
                */
                oxygen_style_parent_class->draw_box( style, window, state, shadow, clipRect, widget, detail, x, y, w, h );
                Oxygen::Style::instance().renderWindowBackground( window, widget, clipRect, x, y, w, h );
            }

            Oxygen::StyleOptions options( Oxygen::None );
            if( !Gtk::gtk_progress_bar_is_horizontal( widget ) ) options |= Oxygen::Vertical;
            Oxygen::Style::instance().renderProgressBarHole( window, clipRect, x, y, w, h, options );

        } else if( GTK_IS_VSCROLLBAR( widget ) ) {

            // TODO: handle arrow button configuration
            // TODO: handle arrow button size when calculating rect size
            Oxygen::Style::instance().renderScrollBarHole( window, clipRect, x, y+15, w-1, h-30, Oxygen::Vertical );

        } else if( GTK_IS_HSCROLLBAR( widget ) ) {

            // TODO: handle arrow button configuration
            // TODO: handle arrow button size when calculating rect size
            Oxygen::Style::instance().renderScrollBarHole( window, clipRect, x+15, y, w-30, h-1, Oxygen::None );

        } else if( GTK_IS_VSCALE( widget ) ) {

            Oxygen::Style::instance().renderSliderGroove( window, clipRect, x, y, w, h, Oxygen::Vertical );

        } else if( GTK_IS_HSCALE( widget ) ) {

            Oxygen::Style::instance().renderSliderGroove( window, clipRect, x, y, w, h, Oxygen::None );

        }

    } else if( d.isSpinButton()) {

        Oxygen::StyleOptions options( Oxygen::Blend | Oxygen::NoFill );
        options |= Oxygen::styleOptions( widget, state, shadow );

        // TODO: make sure the offsets are robust enough
        // Oxygen::Style::instance().outline(window,clipRect, x-5, y-1 ,w+7, h+1 );
        Oxygen::Style::instance().renderHoleBackground(window,clipRect, x-5, y-1 ,w+7, h+1 );
        Oxygen::Style::instance().renderHole(window,clipRect,x-5,y-1,w+7,h+1, options);

    } else if( d.isSpinButtonArrow() ) {

        return;

    } else if( d.isBar() && GTK_IS_PROGRESS_BAR( widget ) ) {

        Oxygen::Style::instance().renderProgressBarHandle( window, clipRect, x, y, w, h );
        return;

    } else if( d.isScale() ) {

        Oxygen::StyleOptions options( Oxygen::Blend );
        options |= Oxygen::styleOptions( widget, state, shadow );
        if( GTK_IS_VSCALE( widget ) ) options |= Oxygen::Vertical;
        Oxygen::Style::instance().renderSliderHandle( window, clipRect, x, y, w, h, options );

    } else {

        oxygen_style_parent_class->draw_box( style, window, state,
            shadow, clipRect, widget, detail,
            x, y, w, h );
    }

}

//___________________________________________________________
static void draw_shadow( GtkStyle* style,
    GdkWindow* window,
    GtkStateType state,
    GtkShadowType shadow,
    GdkRectangle* clipRect,
    GtkWidget* widget,
    const gchar* detail,
    gint x,
    gint y,
    gint w,
    gint h )
{
    g_return_if_fail( style && window );
    Oxygen::Style::instance().sanitizeSize( window, w, h );

    g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
        "widget=%s, primitive=shadow, state=%s, shadow=%s, detail=%s",
        G_OBJECT_TYPE_NAME( widget ),
        Oxygen::Maps::getState( state ),
        Oxygen::Maps::getShadow( shadow ),
        detail );

    const Gtk::Detail d( detail );
    if( ( d.isEntry() || d.isViewport() || d.isScrolledWindow() ) && shadow == GTK_SHADOW_IN ) {

        Oxygen::Style::instance().renderHoleBackground( window, clipRect, x-1, y-1, w+2, h+1 );
        Oxygen::Style::instance().renderHole( window, clipRect, x-1, y-1, w+2, h+1, Oxygen::NoFill );

        return;

    } else if( d.isFrame() ) {

        if( shadow == GTK_SHADOW_IN ) {

            Oxygen::Style::instance().renderHoleBackground( window, clipRect, x-1, y-1, w+2, h+1 );
            Oxygen::Style::instance().renderHole( window, clipRect, x-1, y-1, w+2, h+1, Oxygen::NoFill );

        } else if( (shadow == GTK_SHADOW_ETCHED_IN || shadow == GTK_SHADOW_ETCHED_OUT) && !Gtk::gtk_parent_button( widget )) {

            Oxygen::Style::instance().renderDockFrame( window, clipRect, x, y+1, w, h-2, Oxygen::Blend );

        }

        return;

    } else if( ( d.isTrough() || d.isBar() ) && GTK_IS_PROGRESS_BAR( widget ) ) {

        return;

    } else {

        oxygen_style_parent_class->draw_shadow(
            style, window, state,
            shadow, clipRect, widget, detail,
            x, y, w, h );
    }

}

//___________________________________________________________
static void draw_check( GtkStyle* style,
    GdkWindow* window,
    GtkStateType state,
    GtkShadowType shadow,
    GdkRectangle* clipRect,
    GtkWidget* widget,
    const gchar* detail,
    gint x,
    gint y,
    gint w,
    gint h )
{
    g_return_if_fail( style && window );

    Oxygen::Style::instance().sanitizeSize( window, w, h );

    g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
        "widget=%s, primitive=check, state=%s, shadow=%s, detail=%s",
        G_OBJECT_TYPE_NAME( widget ),
        Oxygen::Maps::getState( state ),
        Oxygen::Maps::getShadow( shadow ),
        detail );

    const Gtk::Detail d( detail );
    if( d.isCheckButton() || d.isCellCheck() )
    {

        Oxygen::StyleOptions options( Oxygen::styleOptions( widget, state, shadow ) );
        if( !(d.isCellCheck() || Gtk::gtk_parent_treeview( widget ) ) )
        {
            // enable blending
            options |= Oxygen::Blend;
        }

        if( d.isCellCheck() )
        {
            // disable hover and focus in cellcheck because all cells would
            // get it when view is hovered/focused otherwise
            options &= ~(Oxygen::Hover|Oxygen::Focus);
        }

        Oxygen::Style::instance().renderCheckBox( window, clipRect, x, y, w, h, options );

    } else if( d.isCheck() && GTK_IS_CHECK_MENU_ITEM( widget ) ) {

        Oxygen::StyleOptions options( Oxygen::styleOptions( widget, state, shadow ) );
        options |= (Oxygen::Blend|Oxygen::Flat|Oxygen::NoFill );
        Oxygen::Style::instance().renderCheckBox( window, clipRect, x, y, w, h, options );

    } else {

        oxygen_style_parent_class->draw_check( style, window, state,
            shadow, clipRect, widget, detail,
            x, y, w, h );
    }

}

//___________________________________________________________
static void draw_option( GtkStyle* style,
    GdkWindow* window,
    GtkStateType state,
    GtkShadowType shadow,
    GdkRectangle* clipRect,
    GtkWidget* widget,
    const gchar* detail,
    gint x,
    gint y,
    gint w,
    gint h )
{
    g_return_if_fail( style && window );

    Oxygen::Style::instance().sanitizeSize( window, w, h );


    g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
        "widget=%s, primitive=option, state=%s, shadow=%s, detail=%s",
        G_OBJECT_TYPE_NAME( widget ),
        Oxygen::Maps::getState( state ),
        Oxygen::Maps::getShadow( shadow ),
        detail );

    if( Gtk::Detail( detail ).isRadioButton() )
    {

        Oxygen::StyleOptions options( Oxygen::styleOptions( widget, state, shadow ) );
        if( !Gtk::gtk_parent_treeview( widget ) ) options |= Oxygen::Blend;
        Oxygen::Style::instance().renderRadioButton( window, clipRect, x, y, w, h, options );

    } else {

        oxygen_style_parent_class->draw_option( style, window, state,
            shadow, clipRect, widget, detail,
            x, y, w, h );

    }
}

//______________________________________________
static void draw_hline(
    GtkStyle* style,
    GdkWindow* window,
    GtkStateType state,
    GdkRectangle* clipRect,
    GtkWidget* widget,
    const gchar* detail,
    gint x1,
    gint x2,
    gint y )
{
    g_return_if_fail( style && window );

    g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
        "widget=%s, primitive=hline, state=%s, detail=%s",
        G_OBJECT_TYPE_NAME( widget ),
        Oxygen::Maps::getState( state ),
        detail );

    Gtk::Detail d( detail );
    if( !d.isVScale() ) {

        Oxygen::Style::instance().drawSeparator( window, clipRect, x1, y, x2-x1, 0, false );

    }

}

//______________________________________________
static void draw_vline( GtkStyle* style,
    GdkWindow* window,
    GtkStateType state,
    GdkRectangle* clipRect,
    GtkWidget* widget,
    const gchar* detail,
    gint y1,
    gint y2,
    gint x )
{
    g_return_if_fail( style && window );

    g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
        "widget=%s, primitive=vline, state=%s, detail=%s",
        G_OBJECT_TYPE_NAME( widget ),
        Oxygen::Maps::getState( state ),
        detail );

    // disable vline in buttons (should correspond to comboboxes)
    Gtk::Detail d( detail );
    if( !( d.isHScale() || Gtk::gtk_parent_button( widget ) ) )
    { Oxygen::Style::instance().drawSeparator( window, clipRect, x, y1, 0, y2-y1, true ); }

}

//___________________________________________________________
static void draw_arrow( GtkStyle* style,
    GdkWindow* window,
    GtkStateType state,
    GtkShadowType shadow,
    GdkRectangle* clipRect,
    GtkWidget* widget,
    const gchar* detail,
    GtkArrowType arrow,
    gboolean fill,
    gint x,
    gint y,
    gint w,
    gint h )
{

    g_return_if_fail( style && window );
    Oxygen::Style::instance().sanitizeSize( window, w, h );

    g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
        "widget=%s, primitive=arrow, state=%s, shadow=%s, detail=%s, arrow=%s, fill=%d",
        G_OBJECT_TYPE_NAME( widget ),
        Oxygen::Maps::getState( state ),
        Oxygen::Maps::getShadow( shadow ),
        detail,
        Oxygen::Maps::getArrow( arrow ),
        fill );

    const Gtk::Detail d( detail );

    Oxygen::Style::ArrowSize arrowSize( Oxygen::Style::ArrowNormal );
    if( d.isMenuItem() && Oxygen::Style::instance().settings().applicationName().isFirefox() )
    { arrowSize = Oxygen::Style::ArrowTiny; }

    Oxygen::StyleOptions options( Oxygen::Contrast );
    options |= Oxygen::styleOptions( widget, state );

    // disable highlight in menus and buttons, for consistancy with oxygen qt style
    if( d.isMenuItem() || (Gtk::gtk_parent_button( widget ) && !Gtk::gtk_parent_treeview( widget ) ) )
    {

        options &= ~( Oxygen::Focus|Oxygen::Hover );

    }

    if( d.isSpinButton() ) {

        /*
        TODO: this should be made more robust. What one really want is an arrow that is
        one pixel away from the centerline, no matter what
        */
        if( arrow == GTK_ARROW_UP ) y += 1;
        if( arrow == GTK_ARROW_DOWN ) y -= 1;

        // disable contrast
        options &= ~Oxygen::Contrast;

    }

    // render
    Oxygen::Style::instance().renderArrow( window, clipRect, arrow, x, y, w, h, arrowSize, options );

}


//___________________________________________________________
static void draw_expander(
    GtkStyle* style,
    GdkWindow* window,
    GtkStateType state,
    GdkRectangle* clipRect,
    GtkWidget* widget,
    const char* detail,
    gint x,
    gint y,
    GtkExpanderStyle expander_style )
{
    g_return_if_fail( style && window );
    g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
        "widget=%s, primitive=expander, state=%s, detail=%s",
        G_OBJECT_TYPE_NAME( widget ),
        Oxygen::Maps::getState( state ),
        detail );


    const GtkArrowType arrow( ( expander_style == GTK_EXPANDER_COLLAPSED || expander_style == GTK_EXPANDER_SEMI_COLLAPSED ) ?
        GTK_ARROW_RIGHT : GTK_ARROW_DOWN );

    // TODO: actually read arrow size from options
    Oxygen::StyleOptions options( Oxygen::styleOptions( widget, state ) );

    const Gtk::Detail d( detail );
    if( !d.isTreeView() ) options |= Oxygen::Contrast;

    const Oxygen::Style::ArrowSize arrowSize( d.isTreeView() ? Oxygen::Style::ArrowSmall : Oxygen::Style::ArrowNormal );
    Oxygen::Style::instance().renderArrow( window, clipRect, arrow, x-16, y-16, 32, 32, arrowSize, options );

}

//___________________________________________________________
static void draw_diamond( GtkStyle* style,
    GdkWindow* window,
    GtkStateType state,
    GtkShadowType shadow,
    GdkRectangle* clipRect,
    GtkWidget* widget,
     const char* detail,
    gint x,
    gint y,
    gint w,
    gint h )
{
    g_return_if_fail( style && window );
    Oxygen::Style::instance().sanitizeSize( window, w, h );
    g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
        "widget=%s, primitive=diamond, state=%s, shadow=%s, detail=%s",
        G_OBJECT_TYPE_NAME( widget ),
        Oxygen::Maps::getState( state ),
        Oxygen::Maps::getShadow( shadow ),
        detail );

    oxygen_style_parent_class->draw_diamond( style, window, state,
        shadow, clipRect, widget, detail,
        x, y, w, h );
}

//___________________________________________________________
static void draw_tab( GtkStyle* style,
    GdkWindow* window,
    GtkStateType state,
    GtkShadowType shadow,
    GdkRectangle* clipRect,
    GtkWidget* widget,
     const char* detail,
    gint x,
    gint y,
    gint w,
    gint h )
{
    g_return_if_fail( style && window );

    Oxygen::Style::instance().sanitizeSize( window, w, h );
    g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
        "widget=%s, primitive=tab, state=%s, shadow=%s, detail=%s",
        G_OBJECT_TYPE_NAME( widget ),
        Oxygen::Maps::getState( state ),
        Oxygen::Maps::getShadow( shadow ),
        detail );

    Gtk::Detail d( detail );
    if( d.isOptionMenuTab() )
    {

        // render
        GtkArrowType arrow = GTK_ARROW_DOWN;
        Oxygen::Style::ArrowSize arrowSize = Oxygen::Style::ArrowNormal;
        Oxygen::StyleOptions options = Oxygen::Contrast;
        Oxygen::Style::instance().renderArrow( window, clipRect, arrow, x, y, w, h, arrowSize, options );
        return;

    } else {

        oxygen_style_parent_class->draw_tab( style, window, state,
            shadow, clipRect, widget, detail,
            x, y, w, h );
    }

}
//___________________________________________________________
static void draw_shadow_gap( GtkStyle* style,
    GdkWindow* window,
    GtkStateType state,
    GtkShadowType shadow,
    GdkRectangle* clipRect,
    GtkWidget* widget,
    const gchar* detail,
    gint x,
    gint y,
    gint w,
    gint h,
    GtkPositionType gap_side,
    gint gap_x,
    gint gap_w )
{
    g_return_if_fail( style && window );

    Oxygen::Style::instance().sanitizeSize( window, w, h );
    g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
        "widget=%s, primitive=shadow_gap, state=%s, shadow=%s, detail=%s",
        G_OBJECT_TYPE_NAME( widget ),
        Oxygen::Maps::getState( state ),
        Oxygen::Maps::getShadow( shadow ),
        detail );

    Gtk::Detail d( detail );
    if( d.isFrame() ) {

        if( shadow == GTK_SHADOW_IN ) {

            Oxygen::Style::instance().renderHoleBackground( window, clipRect, x-1, y-1, w+2, h+1 );
            Oxygen::Style::instance().renderHole( window, clipRect, x-1, y-1, w+2, h+1, Gtk::Gap( gap_x, gap_w, gap_side ), Oxygen::NoFill );

        } else if( shadow == GTK_SHADOW_OUT ) {

            Oxygen::StyleOptions options( Oxygen::NoFill );
            options |= Oxygen::styleOptions( widget, GTK_STATE_NORMAL, shadow );
            options &= ~(Oxygen::Hover|Oxygen::Focus);
            Oxygen::Style::instance().renderSlab( window, clipRect, x-1, y-4, w+2, h+4, Gtk::Gap( gap_x, gap_w, gap_side ), options );

        } else if( shadow == GTK_SHADOW_ETCHED_IN || shadow == GTK_SHADOW_ETCHED_OUT ) {

            Oxygen::Style::instance().renderDockFrame( window, clipRect, x, y-1, w, h+1, Gtk::Gap( gap_x, gap_w, gap_side ), Oxygen::Blend );

        }

        return;

    }

    oxygen_style_parent_class->draw_shadow_gap( style, window, state,
        shadow, clipRect, widget, detail,
        x, y, w, h,
        gap_side, gap_x, gap_w );
}

//___________________________________________________________
static void draw_box_gap( GtkStyle* style,
    GdkWindow* window,
    GtkStateType state,
    GtkShadowType shadow,
    GdkRectangle* clipRect,
    GtkWidget* widget,
    const gchar* detail,
    gint x,
    gint y,
    gint w,
    gint h,
    GtkPositionType gap_side,
    gint gap_x,
    gint gap_w )
{
    g_return_if_fail( style && window );

    Oxygen::Style::instance().sanitizeSize( window, w, h );
    g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
        "widget=%s, primitive=box_gap, state=%s, shadow=%s, detail=%s",
        G_OBJECT_TYPE_NAME( widget ),
        Oxygen::Maps::getState( state ),
        Oxygen::Maps::getShadow( shadow ),
        detail );

    const Gtk::Detail d( detail );
    if( d.isNotebook() )
    {

        Oxygen::Style::instance().renderWindowBackground( window, clipRect, x, y, w, h );

        // this might move to drawShadowGap
        Oxygen::StyleOptions options( Oxygen::NoFill );
        options |= Oxygen::styleOptions( widget, GTK_STATE_NORMAL, shadow );
        options &= ~(Oxygen::Hover|Oxygen::Focus);
        Oxygen::Style::instance().renderSlab( window, clipRect, x, y, w, h, Gtk::Gap( gap_x, gap_w, gap_side ), options );

    } else {

        oxygen_style_parent_class->draw_box_gap( style, window, state,
            shadow, clipRect, widget, detail,
            x, y, w, h,
            gap_side, gap_x, gap_w );
    }

}

//___________________________________________________________
static void draw_slider( GtkStyle* style,
    GdkWindow* window,
    GtkStateType state,
    GtkShadowType shadow,
    GdkRectangle* clipRect,
    GtkWidget* widget,
    const gchar* detail,
    gint x,
    gint y,
    gint w,
    gint h,
    GtkOrientation orientation )
{
    g_return_if_fail( style && window );

    Oxygen::Style::instance().sanitizeSize( window, w, h );
    g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
        "widget=%s, primitive=slider, state=%s, shadow=%s, detail=%s",
        G_OBJECT_TYPE_NAME( widget ),
        Oxygen::Maps::getState( state ),
        Oxygen::Maps::getShadow( shadow ),
        detail );

    if( GTK_IS_VSCROLLBAR( widget ) )
    {

        Oxygen::StyleOptions options( Oxygen::Vertical );
        options |= Oxygen::styleOptions( widget, state, shadow );
        Oxygen::Style::instance().renderScrollBarHandle( window, clipRect, x, y, w-1, h, options );

    } else if( GTK_IS_HSCROLLBAR( widget ) ) {

        Oxygen::StyleOptions options( Oxygen::styleOptions( widget, state, shadow ) );
        Oxygen::Style::instance().renderScrollBarHandle( window, clipRect, x, y, w, h-1, options );

    } else {
        oxygen_style_parent_class->draw_slider( style, window, state,
            shadow, clipRect, widget, detail,
            x, y, w, h,
            orientation );
    }

}

//___________________________________________________________
static void draw_extension( GtkStyle* style,
    GdkWindow* window,
    GtkStateType state,
    GtkShadowType shadow,
    GdkRectangle* clipRect,
    GtkWidget* widget,
    const gchar* detail,
    gint x,
    gint y,
    gint w,
    gint h,
    GtkPositionType gap_side )
{

    g_return_if_fail( style && window );

    Oxygen::Style::instance().sanitizeSize( window, w, h );
    g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
        "widget=%s, primitive=extension, state=%s, shadow=%s, detail=%s",
        G_OBJECT_TYPE_NAME( widget ),
        Oxygen::Maps::getState( state ),
        Oxygen::Maps::getShadow( shadow ),
        detail );

//     oxygen_style_parent_class->draw_extension( style, window, state,
//         shadow, clipRect, widget, detail,
//         x, y, w, h,
//         gap_side );

}

//___________________________________________________________
static void draw_focus(
    GtkStyle* style,
    GdkWindow* window,
    GtkStateType state,
    GdkRectangle* clipRect,
    GtkWidget* widget,
     const char* detail,
    gint x,
    gint y,
    gint w,
    gint h )
{

    g_return_if_fail( style && window );

    Oxygen::Style::instance().sanitizeSize( window, w, h );
    g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
        "widget=%s, primitive=focus, state=%s, detail=%s",
        G_OBJECT_TYPE_NAME( widget ),
        Oxygen::Maps::getState( state ),
        detail );

//     oxygen_style_parent_class->draw_focus( style, window, state,
//         clipRect, widget, detail,
//         x, y, w, h );

}


//___________________________________________________________
static void draw_handle( GtkStyle* style,
    GdkWindow* window,
    GtkStateType state,
    GtkShadowType shadow,
    GdkRectangle* clipRect,
    GtkWidget* widget,
    const gchar* detail,
    gint x,
    gint y,
    gint w,
    gint h,
    GtkOrientation orientation )
{
    g_return_if_fail( style && window );

    Oxygen::Style::instance().sanitizeSize( window, w, h );
    g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
        "widget=%s, primitive=handle, state=%s, shadow=%s, detail=%s",
        G_OBJECT_TYPE_NAME( widget ),
        Oxygen::Maps::getState( state ),
        Oxygen::Maps::getShadow( shadow ),
        detail );

    Gtk::Detail d( detail );
    if( d.isPaned() )
    {

        Oxygen::StyleOptions options( Oxygen::styleOptions( widget, state, shadow ) );
        if( GTK_IS_VPANED( widget ) ) options |= Oxygen::Vertical;
        Oxygen::Style::instance().renderSplitter( window, clipRect, x, y, w, h, options );

    } else if( d.isHandleBox() ) {

        Oxygen::StyleOptions options( Oxygen::styleOptions( widget, state, shadow ) );
        if( orientation == GTK_ORIENTATION_VERTICAL ) options |= Oxygen::Vertical;
        Oxygen::Style::instance().renderWindowBackground( window, clipRect, x, y, w, h );
        Oxygen::Style::instance().renderToolBarHandle( window, clipRect, x, y, w, h, options );

    } else {

        oxygen_style_parent_class->draw_handle( style, window, state,
            shadow, clipRect, widget, detail,
            x, y, w, h,
            orientation );
    }

}

//___________________________________________________________
static void draw_resize_grip(
    GtkStyle* style,
    GdkWindow* window,
    GtkStateType state,
    GdkRectangle* clipRect,
    GtkWidget* widget,
     const char* detail,
    GdkWindowEdge edge,
    gint x,
    gint y,
    gint w,
    gint h )
{
    g_return_if_fail( style && window );
    g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
        "widget=%s, primitive=resize_grip, state=%s, detail=%s",
        G_OBJECT_TYPE_NAME( widget ),
        Oxygen::Maps::getState( state ),
        detail );

    // no resize grip in oxygen no matter what
    return;
}

//___________________________________________________________
static void draw_layout(
    GtkStyle *style, GdkWindow *window, GtkStateType state,
    gboolean use_text,
    GdkRectangle *clipRect,
    GtkWidget *widget, const gchar *detail,
    gint x, gint y,
    PangoLayout *layout)
{

//     g_log( OXYGEN_LOG_DOMAIN, G_LOG_LEVEL_INFO,
//         "widget=%s, primitive=layout, state=%s, detail=%s",
//         G_OBJECT_TYPE_NAME( widget ),
//         Oxygen::Maps::getState( state ),
//         detail );

    oxygen_style_parent_class->draw_layout(
        style, window, state,
        use_text, clipRect, widget, detail, x, y, layout );

}

//___________________________________________________________
extern "C"
void instance_init( OxygenStyle* self )
{}

//___________________________________________________________
static void class_init( OxygenStyleClass* klass )
{
    GtkStyleClass* style_class;

    style_class = GTK_STYLE_CLASS( klass );

    oxygen_style_parent_class = static_cast<GtkStyleClass*>( g_type_class_peek_parent( klass ) );

    style_class->draw_hline = draw_hline;
    style_class->draw_vline = draw_vline;
    style_class->draw_shadow = draw_shadow;
    style_class->draw_arrow = draw_arrow;

    style_class->draw_diamond = draw_diamond;

    style_class->draw_box = draw_box;
    style_class->draw_flat_box = draw_flat_box;
    style_class->draw_check = draw_check;
    style_class->draw_option = draw_option;
    style_class->draw_tab = draw_tab;
    style_class->draw_shadow_gap = draw_shadow_gap;
    style_class->draw_box_gap = draw_box_gap;
    style_class->draw_extension = draw_extension;
    style_class->draw_focus = draw_focus;
    style_class->draw_slider = draw_slider;
    style_class->draw_handle = draw_handle;
    style_class->draw_resize_grip = draw_resize_grip;
    style_class->draw_expander = draw_expander;
    style_class->draw_layout = draw_layout;
    /*
    We just leave those to gtk:
    style_class->draw_polygon = draw_polygon;
    */
}

//___________________________________________________________
void oxygen_style_register_type( GTypeModule* module )
{
    if( !oxygen_style_type )
    {

        static const GTypeInfo info =
        {
            sizeof( OxygenStyleClass ),
            0L,
            0L,
(            GClassInitFunc ) class_init,
            0L,          /* class_finalize */
            0L,          /* class_data */
            sizeof( OxygenStyle ),
            0,           /* n_preallocs */
(            GInstanceInitFunc ) instance_init,
        };

        oxygen_style_type = g_type_module_register_type( module, GTK_TYPE_STYLE, "OxygenStyle", &info, GTypeFlags(0 ) );
    }
}

//___________________________________________________________
GType oxygen_style_get_type( void )
{ return oxygen_style_type; }
