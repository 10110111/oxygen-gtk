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

#include "oxygenbuttondemowidget.h"

#include <string>

namespace Oxygen
{

    //____________________________________________________
    ButtonDemoWidget::ButtonDemoWidget( void )
    {

        // main widget
        GtkWidget* mainWidget( gtk_vbox_new( false, 0 ) );
        gtk_box_set_spacing( GTK_BOX( mainWidget ), 5 );
        setWidget( mainWidget );

        // setup
        setName( "Buttons" );
        setComments( "Shows the appearance of buttons" );
        setIconName( "go-jump-locationbar" );
        realize();

        // pushbuttons
        {
            GtkWidget* frame( gtk_frame_new( "Pushbutton" ) );
            gtk_box_pack_start( GTK_BOX( mainWidget ), frame, false, true, 0 );
            gtk_widget_show( frame );

            // inner table
            GtkWidget* table = gtk_table_new( 2, 4, false );
            gtk_table_set_row_spacings( GTK_TABLE( table ), 5 );
            gtk_container_add( GTK_CONTAINER( frame ), table );
            gtk_widget_show( table );

            // spacing
            GtkWidget* spacing( gtk_label_new( "" ) );
            gtk_table_attach( GTK_TABLE( table ), spacing, 3, 4, 0, 1, GTK_EXPAND, GTK_FILL, 2, 0  );
            gtk_widget_show( spacing );

            // generic label
            GtkWidget* label( 0L );

            {
                // text only
                gtk_table_attach( GTK_TABLE( table ), label = gtk_label_new( "Text only: " ), 0, 1, 0, 1, GTK_FILL, GTK_FILL, 2, 0  );
                gtk_misc_set_alignment( GTK_MISC( label ), 1, 0.5 );
                gtk_widget_show( label );

                // button
                GtkWidget* button( gtk_button_new_with_label( "Normal" ) );
                gtk_table_attach( GTK_TABLE( table ), button, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 2, 0  );
                gtk_widget_show( button );

                // combobox model
                GtkListStore* model( gtk_list_store_new( 1, G_TYPE_STRING ) );
                const char* columns[] =
                {
                    "Small",
                    "Normal",
                    "Large"
                };

                for( unsigned int i=0; i<3; i++ )
                {
                    GtkTreeIter iter;
                    gtk_list_store_append( model, &iter );
                    gtk_list_store_set( model, &iter, 0, columns[i], -1 );
                }

                GtkWidget* comboBox( gtk_combo_box_new() );
                gtk_combo_box_set_model( GTK_COMBO_BOX( comboBox ), GTK_TREE_MODEL( model ) );
                g_object_unref( model );

                // text renderer
                GtkCellRenderer* cell( gtk_cell_renderer_text_new() );
                gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( comboBox ), cell, FALSE );
                gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT( comboBox ), cell, "text", 0, NULL );

                gtk_combo_box_set_active( GTK_COMBO_BOX( comboBox ), 0 );

                gtk_table_attach( GTK_TABLE( table ), comboBox, 2, 3, 0, 1, GTK_FILL, GTK_FILL, 2, 0 );
                gtk_widget_show( comboBox );

            }

            {
                // text and icons
                gtk_table_attach( GTK_TABLE( table ), label = gtk_label_new( "Text and icon: " ), 0, 1, 1, 2, GTK_FILL, GTK_FILL, 2, 0  );
                gtk_misc_set_alignment( GTK_MISC( label ), 1, 0.5 );
                gtk_widget_show( label );

                // button
                GtkWidget* button( gtk_button_new_with_label( "Normal" ) );
                gtk_table_attach( GTK_TABLE( table ), button, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 2, 0  );

                GtkIconTheme* theme( gtk_icon_theme_get_default() );
                GdkPixbuf* icon = gtk_icon_theme_load_icon( theme, "oxygen", 16, (GtkIconLookupFlags) 0, 0L );
                GtkWidget* image( gtk_image_new_from_pixbuf( icon ) );
                g_object_unref( icon );

                gtk_button_set_image( GTK_BUTTON( button ), image );
                gtk_widget_show( button );

                // combobox model
                GtkListStore* model( gtk_list_store_new( 2, GDK_TYPE_PIXBUF, G_TYPE_STRING ) );
                const char* columns[] =
                {
                    "New",
                    "Open",
                    "Save"
                };

                const char* icons[] =
                {
                    "document-new",
                    "document-open",
                    "document-save"
                };

                // store into model
                for( unsigned int i=0; i<3; i++ )
                {
                    GtkTreeIter iter;
                    gtk_list_store_append( model, &iter );

                    GdkPixbuf* icon = gtk_icon_theme_load_icon( theme, icons[i], 16, (GtkIconLookupFlags) 0, 0L );
                    gtk_list_store_set( model, &iter, 0, icon, 1, columns[i], -1 );
                    g_object_unref( icon );
                }

                GtkWidget* comboBox( gtk_combo_box_new() );
                gtk_combo_box_set_model( GTK_COMBO_BOX( comboBox ), GTK_TREE_MODEL( model ) );
                g_object_unref( model );

                {
                    // pixbuf renderer
                    GtkCellRenderer* cell = gtk_cell_renderer_pixbuf_new();
                    gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( comboBox ), cell, FALSE );
                    gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT( comboBox ), cell,
                        "pixbuf", 0,
                        NULL );
                }

                {
                    // text renderer
                    GtkCellRenderer* cell( gtk_cell_renderer_text_new() );
                    gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( comboBox ), cell, FALSE );
                    gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT( comboBox ), cell, "text", 1, NULL );
                }

                gtk_combo_box_set_active( GTK_COMBO_BOX( comboBox ), 0 );

                gtk_table_attach( GTK_TABLE( table ), comboBox, 2, 3, 1, 2, GTK_FILL, GTK_FILL, 2, 0 );
                gtk_widget_show( comboBox );

            }


        }

        GtkWidget* hbox( gtk_hbox_new( false, 0 ) );
        gtk_box_set_spacing( GTK_BOX( hbox ), 5 );
        gtk_box_pack_start( GTK_BOX( mainWidget ), hbox, false, true, 0 );
        gtk_widget_show( hbox );

        {
            // checkboxes
            GtkWidget* frame( gtk_frame_new( "Checkboxes" ) );
            gtk_box_pack_start( GTK_BOX( hbox ), frame, true, true, 0 );
            gtk_widget_show( frame );

            GtkWidget* vbox( gtk_vbox_new( false, 0 ) );
            gtk_container_add( GTK_CONTAINER( frame ), vbox );
            gtk_widget_show( vbox );

            GtkWidget* checkbutton;
            gtk_box_pack_start( GTK_BOX( vbox ), checkbutton = gtk_check_button_new_with_label( "Off" ), false, true, 0 );
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( checkbutton ), false );
            gtk_widget_show( checkbutton );

            gtk_box_pack_start( GTK_BOX( vbox ), checkbutton = gtk_check_button_new_with_label( "Partial" ), false, true, 0 );
            gtk_toggle_button_set_inconsistent( GTK_TOGGLE_BUTTON( checkbutton ), true );
            gtk_widget_show( checkbutton );

            gtk_box_pack_start( GTK_BOX( vbox ), checkbutton = gtk_check_button_new_with_label( "On" ), false, true, 0 );
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( checkbutton ), true );
            gtk_widget_show( checkbutton );

        }

        {
            // radio buttons
            GtkWidget* frame( gtk_frame_new( "Radiobuttons" ) );
            gtk_box_pack_start( GTK_BOX( hbox ), frame, true, true, 0 );
            gtk_widget_show( frame );

            GtkWidget* vbox( gtk_vbox_new( false, 0 ) );
            gtk_container_add( GTK_CONTAINER( frame ), vbox );
            gtk_widget_show( vbox );

            GtkWidget* radiobutton;
            gtk_box_pack_start( GTK_BOX( vbox ), radiobutton = gtk_radio_button_new_with_label( 0L, "First Choice" ), false, true, 0 );
            gtk_widget_show( radiobutton );

            gtk_box_pack_start( GTK_BOX( vbox ), radiobutton = gtk_radio_button_new_with_label_from_widget( GTK_RADIO_BUTTON( radiobutton ), "Second Choice" ), false, true, 0 );
            gtk_widget_show( radiobutton );

            gtk_box_pack_start( GTK_BOX( vbox ), radiobutton = gtk_radio_button_new_with_label_from_widget( GTK_RADIO_BUTTON( radiobutton ), "Third Choice" ), false, true, 0 );
            gtk_widget_show( radiobutton );
        }

    }

    //____________________________________________________
    ButtonDemoWidget::~ButtonDemoWidget( void )
    {}

}
