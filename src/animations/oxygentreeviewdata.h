#ifndef oxygentreeviewdata_h
#define oxygentreeviewdata_h
/*
* this file is part of the oxygen gtk engine
* Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
* Copyright (c) 2010 Ruslan Kabatsayev <b7.10110111@gmail.com>
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
#include "../oxygengtkutils.h"

#include <gtk/gtk.h>
#include <algorithm>

namespace Oxygen
{
    class TreeViewData
    {

        public:

        //! constructor
        TreeViewData( void ):
            _motionId(-1),
            _leaveId(-1),
            _rowDeletedId(-1),
            _fullWidth( false ),
            _hovered( false ),
            _path(0),
            _column(0),
            _x(-1),
            _y(-1)
        {}

        //! destructor
        virtual ~TreeViewData( void )
        {}

        //! setup connections
        void connect( GtkWidget* );

        //! disconnect
        void disconnect( GtkWidget* );

        //! full width flag
        void setFullWidth( bool value )
        { _fullWidth = value; }

        //! true if hovered
        bool hovered( void ) const
        { return _hovered; }

        //! true if current position is contained int rect
        bool isCellHovered( int x, int y, int w, int h ) const
        { return isCellHovered( x, y, w, h, _fullWidth ); }

        //! true if current position is contained int rect
        bool isCellHovered( int x, int y, int w, int h, bool fullWidth ) const
        {
            if( !( _path && _hovered ) ) return false;
            if( fullWidth ) return ( _y >= y ) && ( _y < y+h );
            else return (_x >= x) && (_x < x+w ) && ( _y >= y ) && ( _y < y+h );
        }

        protected:

        //! set mouse over state
        void setHovered( bool value )
        { _hovered = value; }

        //! update pointer position
        void updatePosition( GtkWidget*, int x, int y );

        //! update pointer position
        void clearPosition( GtkWidget* = 0L );

        //!@name static callbacks
        //@{
        static gboolean motionNotifyEvent( GtkWidget*, GdkEventMotion*, gpointer);
        static gboolean leaveNotifyEvent( GtkWidget*, GdkEventCrossing*, gpointer);
        static void rowActivatedEvent( GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer );
        static void cursorChangedEvent( GtkTreeView*, gpointer );
        static void rowDeletedEvent( GtkTreeModel*, GtkTreePath*, gpointer );
        //@}

        private:

        //!@name callbacks ids
        //@{
        int _motionId;
        int _leaveId;
        int _rowDeletedId;
        //@}

        //! true if hover works on full width
        bool _fullWidth;

        //! true if hovered
        bool _hovered;

        //!@name keep track of the hovered path and
        //@{

        GtkTreePath* _path;
        GtkTreeViewColumn* _column;

        //@}

        //!@name position
        //@{
        int _x;
        int _y;
        //@}

    };

}

#endif
