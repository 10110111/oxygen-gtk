#ifndef oxygengtkutils_h
#define oxygengtkutils_h
/*
* this file is part of the oxygen gtk engine
* Copyright( c ) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
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

#include <gdk/gdktypes.h>
#include <gtk/gtkstyle.h>

namespace Gtk
{

    //@!name gtk utilities
    //@{

    //! returns true if widget supports rgba
    bool gtk_widget_has_rgba( GtkWidget* );

    //! return parent button if any.
    GtkWidget* gtk_parent_button( GtkWidget* );

    //! return parent menu if any
    GtkWidget* gtk_parent_menu( GtkWidget* );

    //! return parent treeview if any.
    GtkWidget* gtk_parent_treeview( GtkWidget* );

    //! true if a progressbar is horizontal
    /*! adapted from QtCurve code */
    bool gtk_progress_bar_is_horizontal( GtkWidget* );

    //@}

    //@!name gdk utilities
    //@{

    //! get top level windows dimension
    void gdk_toplevel_get_size( GdkWindow*, gint*, gint* );

    //! get position relatve to toplevel
    void gdk_window_get_toplevel_origin( GdkWindow*, gint*, gint* );

    //! return parent combobox if any.
    GtkWidget* gtk_parent_combobox( GtkWidget* );

    bool gtk_object_is_a( const GObject*, const gchar* );

    //@}

}

#endif
