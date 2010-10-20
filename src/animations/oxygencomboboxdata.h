#ifndef oxygencomboboxdata_h
#define oxygencomboboxdata_h
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

#include <gtk/gtk.h>

namespace Oxygen
{

    // handles focus and hover and pressed down state for comboboxes
    class ComboBoxData
    {

        public:

        //! constructor
        ComboBoxData( void )
        {}

        //! destructor
        virtual ~ComboBoxData( void )
        {}

        //! setup connections
        /*! does nothing. Only kept here for consistency with other data */
        void connect( GtkWidget* );

        //! disconnect
        void disconnect( GtkWidget* );

        //!@name modifiers
        //@{

        //! assign button
        void setButton( GtkWidget* value );

        //! assign entry
        void setEntry( GtkWidget* value );

        //! button focus
        void setButtonFocus( bool value )
        {
            if( _button._focus == value ) return;
            _button._focus = value;

            // trigger entry update
            if( _entry._widget )
            { gtk_widget_queue_draw( _entry._widget ); }

            return;
        }

        //@}

        //! pressed
        bool pressed( void ) const
        { return _entry._pressed || _button._pressed; }

        //! true if either button or entry has focus
        bool hasFocus( void ) const
        { return _button._focus || _entry._focus; }

        protected:

        //! set hover flag for given widget
        void setPressed( GtkWidget*, bool );

        //! disconnect child
        void unregisterChild( GtkWidget* );

        //!@name callbacks
        //@{

        static void childStyleChangeNotifyEvent( GtkWidget*, GtkStyle*, gpointer );
        static gboolean childDestroyNotifyEvent( GtkWidget*, gpointer );
        static void childToggledEvent( GtkWidget*, gpointer);

        //@}

        private:

        class Data
        {

            public:

            //! constructor
            Data( void ):
                _widget( 0L ),
                _pressed( false ),
                _focus( false ),
                _destroyId(-1),
                _styleChangeId(-1),
                _toggledId(-1)
            {}

            //! disconnect
            void disconnect( void );

            GtkWidget* _widget;
            bool _pressed;
            bool _focus;

            //!@name callback ids
            //@{
            int _destroyId;
            int _styleChangeId;
            int _toggledId;
            //@}

        };

        //! button data
        Data _button;

        //! entry data
        Data _entry;

    };

}

#endif
