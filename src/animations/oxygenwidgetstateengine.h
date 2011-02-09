#ifndef oxygenwidgetstateengine_h
#define oxygenwidgetstateengine_h
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


#include "../oxygenanimationdata.h"
#include "../oxygenstyleoptions.h"
#include "oxygenbaseengine.h"
#include "oxygendatamap.h"
#include "oxygenwidgetstatedata.h"

#include "../config.h"

#include <gtk/gtk.h>
#include <iostream>

namespace Oxygen
{
    //! forward declaration
    class Animations;

    //! associates widgets with some type of data
    class WidgetStateEngine: public BaseEngine
    {

        public:

        //! constructor
        WidgetStateEngine( Animations* widget ):
            BaseEngine( widget ),
            _duration( 150 )
            {}

        //! destructor
        virtual ~WidgetStateEngine( void )
        {}

        //! register widget
        virtual bool registerWidget( GtkWidget*, AnimationModes );

        //! unregister widget
        virtual void unregisterWidget( GtkWidget* );

        //! enabled state
        virtual void setEnabled( bool );

        //! transition duration
        virtual void setDuration( int );

        //!@name accessors
        //@{

        //! true if widget is included
        virtual bool contains( GtkWidget*, AnimationMode );

        //! true if widget is animated
        virtual bool isAnimated( GtkWidget*, AnimationMode );

        //! animation opacity
        virtual double opacity( GtkWidget*, AnimationMode );

        //! retrieve animation data matching a given widget for provided options
        /*! note: for convenience, this method also calls ::registerWidget and ::updateState */
        virtual AnimationData get( GtkWidget* widget, const StyleOptions& options )
        {

            // check widget
            if( !( enabled() && widget ) ) return AnimationData();

            // register
            registerWidget( widget, AnimationHover|AnimationFocus );

            // stores WidgetStateData locally for speedup
            WidgetStateData& hoverData( _hoverData.value( widget ) );
            WidgetStateData& focusData( _focusData.value( widget ) );

            // update state
            hoverData.updateState( (options&Hover) && !(options&Disabled) );
            focusData.updateState( (options&Focus) && !(options&Disabled) );

            // assume hover takes precedence over focus
            if( hoverData.timeLine().isRunning() ) return AnimationData( hoverData.opacity(), AnimationHover );
            else if( focusData.timeLine().isRunning() ) return AnimationData( focusData.opacity(), AnimationFocus );
            else return AnimationData();

        }

        //@}

        //!@name modifiers
        //@{

        //! update state
        virtual bool updateState( GtkWidget*, AnimationMode, bool );

        //@}

        protected:

        //! register widget in given map
        bool registerWidget( GtkWidget*, DataMap<WidgetStateData>& ) const;

        //! register widget in given map
        void unregisterWidget( GtkWidget*, DataMap<WidgetStateData>& ) const;

        private:

        //! transition duration
        int _duration;

        //!@name maps
        //@{
        DataMap<WidgetStateData> _hoverData;
        DataMap<WidgetStateData> _focusData;
        //@}

    };

}

#endif
