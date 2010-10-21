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
#include <algorithm>
#include <map>

namespace Oxygen
{

    // handles focus and hover and pressed down state for comboboxes
    class ComboBoxData
    {

        public:

        //! constructor
        ComboBoxData( void ):
            _target( 0L ),
            _cellLayoutInitialized( false )
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

        //! button focus
        void setButtonFocus( bool value )
        {
            if( _button._focus == value ) return;
            _button._focus = value;

            // trigger update
            if( _target ) gtk_widget_queue_draw( _target );

            return;
        }

        //! register child
        void registerChild( GtkWidget*, bool recursive = true );

        //! initialize cell layout
        void initializeCellLayout( void );

        //@}

        //!@name accessors
        //@{

        //! pressed
        bool pressed( void ) const
        { return _button._pressed; }

        //! true if either button or entry has focus
        bool hasFocus( void ) const
        { return _button._focus; }

        //! true if either button or entry has hover
        bool hovered( void ) const
        { return std::find_if( _hoverData.begin(), _hoverData.end(), HoveredFTor() ) != _hoverData.end(); }

        //@}

        protected:

        //! initialize cell view
        void initializeCellView( GtkWidget* value );

        //! set hover flag for given widget
        void setPressed( GtkWidget*, bool );

        //! set hover flag for given widget
        void setHovered( GtkWidget*, bool );

        //! disconnect child
        void unregisterChild( GtkWidget* );

        //!@name callbacks
        //@{

        static void childStyleChangeNotifyEvent( GtkWidget*, GtkStyle*, gpointer );
        static gboolean childDestroyNotifyEvent( GtkWidget*, gpointer );
        static void childToggledEvent( GtkWidget*, gpointer);

        static gboolean enterNotifyEvent( GtkWidget*, GdkEventCrossing*, gpointer);
        static gboolean leaveNotifyEvent( GtkWidget*, GdkEventCrossing*, gpointer);

        //@}

        private:

        //! target widget
        GtkWidget* _target;

        // handle child registration
        class ChildData
        {

            public:

            //! constructor
            explicit ChildData( void ):
                _widget(0L),
                _destroyId(-1),
                _styleChangeId(-1)
            {}

            //! destructor
            virtual ~ChildData( void )
            {}

            //! disconnect
            virtual void disconnect( void );

            //! widget
            GtkWidget* _widget;

            //!@name callback ids
            //@{
            int _destroyId;
            int _styleChangeId;
            //@}

        };

        // handle focus and toggle state
        class ButtonData: public ChildData
        {

            public:

            //! constructor
            explicit ButtonData( void ):
                _pressed( false ),
                _focus( false ),
                _toggledId(-1)
            {}

            //! destructor
            virtual ~ButtonData( void )
            {}

            //! disconnect
            virtual void disconnect( void );

            //! true if widget is down
            bool _pressed;

            //! true if widget has focus
            bool _focus;

            //! toggled callback Id
            int _toggledId;

        };

        class HoverData: public ChildData
        {
            public:

            //! constructor
            explicit HoverData( void ):
                _hovered( false ),
                _enterId(-1),
                _leaveId(-1)
            {}

            //! destructor
            virtual ~HoverData( void )
            {}

            //! disconnect
            virtual void disconnect( void );

            //! true if widget is hovered
            bool _hovered;

            //!@name callback ids
            //@{
            int _enterId;
            int _leaveId;
            //@}

        };

        //! need to detect hovered child
        class HoveredFTor
        {
            public:

            bool operator () ( const std::pair<GtkWidget*, HoverData>& dataPair )
            { return dataPair.second._hovered; }

        };

        typedef std::map<GtkWidget*, HoverData> HoverDataMap;
        HoverDataMap _hoverData;

        //! true if cell layout has been initialized
        bool _cellLayoutInitialized;

        //! cell data
        ChildData _cell;

        //! button data
        ButtonData _button;

    };

}

#endif
