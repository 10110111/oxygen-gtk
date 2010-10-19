#ifndef oxygengtkutils_h
#define oxygengtkutils_h
/*
* this file is part of the oxygen gtk engine
* Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
*
* GdkPixbuf modification code from Walmis
* <http://gnome-look.org/content/show.php?content=77783&forumpage=3>
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

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "oxygenrgba.h"

namespace Oxygen
{

    //! gdkRectangle streamer
    inline std::ostream& operator << (std::ostream& out, const GdkRectangle& rect )
    {
        out << "( " << rect.x << "," << rect.y << "," << rect.width << "," << rect.height << ")";
        return out;
    }

}

namespace Gtk
{

    //@!name gtk utilities
    //@{

    // set all buttons in the container to state NORMAL
    void gtk_container_adjust_buttons_state( GtkContainer*, gpointer=0L );

    // make all the buttons on the tabs normal
    gboolean gtk_notebook_update_close_buttons( GtkNotebook*);

    //! changes the gamma value of an image
    bool gdk_pixbuf_to_gamma( GdkPixbuf* pixbuf, double value );

    //! returns true if widget supports rgba
    bool gtk_widget_has_rgba( GtkWidget* );

    //! trigger area update using GdkRectangle
    inline void gtk_widget_queue_draw( GtkWidget* widget, const GdkRectangle* rect = 0L )
    {
        if( !rect ) ::gtk_widget_queue_draw( widget );
        else ::gtk_widget_queue_draw_area( widget, rect->x, rect->y, rect->width, rect->height );
    }

    //! return parent button if any.
    GtkWidget* gtk_parent_button( GtkWidget* );

    //! return parent menu if any
    GtkWidget* gtk_parent_menu( GtkWidget* );

    //! return parent treeview if any.
    GtkWidget* gtk_parent_treeview( GtkWidget* );

    //! return parent combobox if any.
    GtkWidget* gtk_parent_combobox( GtkWidget* );

    //! return parent combobox if any.
    GtkWidget* gtk_parent_combobox_entry( GtkWidget* );

    //! return parent combobox if any.
    GtkWidget* gtk_parent_scrolled_window( GtkWidget* );

    //! return parent combobox if any.
    GtkWidget* gtk_parent_notebook( GtkWidget* );

    //! true if object match a given type
    bool gtk_object_is_a( const GObject*, const gchar* );

    //! true if a progressbar is horizontal
    /*! adapted from QtCurve code */
    bool gtk_progress_bar_is_horizontal( GtkWidget* );

    //! true for 'flat' buttons (e.g. toolbuttons)
    bool gtk_button_is_flat( GtkWidget* );

    //! returns true if position is in hovered tab
    //* this should move to OxygenTabWidgetData
    bool gtk_notebook_tab_contains( GtkWidget*, int tab, int x, int y );

    //! returns tab matching position or -1 if none
    int gtk_notebook_find_tab( GtkWidget*, int x, int y );

    //! returns index of first visible tab
    int gtk_notebook_find_first_tab( GtkWidget* );

    //! returns true if potentialParent is (maybe indirect) parent of widget
    bool gtk_is_parent( GtkWidget*, GtkWidget* potentialParent );

    //! returns an image on button-container
    GtkWidget* gtk_button_find_image( GtkWidget* );

    //! returns true if widget is a notebook close button
    bool gtk_notebook_is_close_button( GtkWidget* );

    //@}

    //@!name gdk utilities
    //@{

    //! returns OxygenRgba color from GdkColor
    inline ColorUtils::Rgba gdk_get_color( const GdkColor& color )
    {
        return ColorUtils::Rgba(
            double(color.red)/0xffff,
            double(color.green)/0xffff,
            double(color.blue)/0xffff );
    }

    //! map widget origin to top level
    /*!
    x and y correspond to (0,0) maped to toplevel window;
    w and h correspond to toplevel window frame size
    */
    bool gdk_map_to_toplevel( GdkWindow*, GtkWidget*, gint*, gint*, gint*, gint*, bool frame = false );

    //! map widget origin to top level
    /*!
    x and y correspond to (0,0) maped to toplevel window;
    w and h correspond to toplevel window frame size
    */
    inline bool gdk_map_to_toplevel( GdkWindow* window, gint* x, gint* y, gint* w, gint* h, bool frame = false )
    { return gdk_map_to_toplevel( window, 0L, x, y, w, h, frame ); }

    //! get top level windows dimension
    void gdk_toplevel_get_size( GdkWindow*, gint*, gint* );

    //! get top level windows dimension
    void gdk_toplevel_get_frame_size( GdkWindow*, gint*, gint* );

    //! get position relatve to toplevel
    void gdk_window_get_toplevel_origin( GdkWindow*, gint*, gint* );

    //! add alpha channel to pixbuf
    GdkPixbuf* gdk_pixbuf_set_alpha( const GdkPixbuf*, double );

    //! resize pixbuf
    GdkPixbuf* gdk_pixbuf_resize( GdkPixbuf* src, int width, int height );

    //! returns initialized GdkRectangle
    inline GdkRectangle gdk_rectangle( int x = 0, int y = 0, int w = -1, int h = -1 )
    {
        GdkRectangle out = {x, y, w, h};
        return out;
    }

    //! returns true if rectangle is valid
    inline bool gdk_rectangle_is_valid( const GdkRectangle* rect )
    { return rect && rect->width > 0 && rect->height > 0; }

    //! returns true if given rectangle contains point
    inline bool gdk_rectangle_contains( const GdkRectangle* rect, int x, int y )
    {
        return
            rect &&
            ( rect->x <= x && (rect->x + rect->width) > x ) &&
            ( rect->y <= y && (rect->y + rect->height) > y );
    }

    //@}

}

#endif
