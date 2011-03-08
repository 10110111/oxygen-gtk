#ifndef oxygenwidgetlookup_h
#define oxygenwidgetlookup_h
/*
* this file is part of the oxygen gtk engine
* Copyright (c) 2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
* Copyright (c) 2010 Ruslan Kabatsayev <b7.10110111@gmail.com>
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

#include "oxygenhook.h"

#include <cairo/cairo.h>
#include <gtk/gtk.h>

#include <vector>

namespace Oxygen
{

    //! handles argb support on a per-application, per-widget basis
    class WidgetLookup
    {

        public:

        //! constructor
        explicit WidgetLookup( void );

        //! destructor
        virtual ~WidgetLookup( void );

        //! initialize hooks
        void initializeHooks( void );

        //! lookup widget matching given context and type
        GtkWidget* lookup( cairo_t*, const GtkWidgetPath* ) const;

        //! lookup widget matching given context and type
        GtkWidget* lookup( cairo_t*, GType ) const;
        protected:

        //! bind widget to context
        void bind( GtkWidget*, cairo_t* );

        //! hook connected to widget's "draw" signal
        static gboolean drawHook( GSignalInvocationHint*, guint, const GValue*, gpointer );

        private:

        //! true if hooks are initialized
        bool _hooksInitialized;

        //! hook connected to widget's "draw" signal
        Hook _drawHook;

        //! store current context
        cairo_t* _context;

        //! store list of associated widgets
        std::vector< GtkWidget* > _widgets;

    };

}

#endif
