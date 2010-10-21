#ifndef oxygenlineeditdata_h
#define oxygenlineeditdata_h
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

#include <oxygenhoverdata.h>

#include <gtk/gtk.h>

namespace Oxygen
{
    class LineEditData: public HoverData
    {

        public:

        //! constructor
        LineEditData( void )
        {}

        //! destructor
        virtual ~LineEditData( void )
        {}

        protected:

        //! set mouse over state
        virtual bool setHovered( GtkWidget* widget, bool value )
        {
            if( !HoverData::setHovered( widget, value ) ) return false;
            gtk_widget_queue_draw( widget );
            return true;
        }

    };

}

#endif
