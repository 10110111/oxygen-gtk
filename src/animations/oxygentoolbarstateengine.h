#ifndef oxygentoolbarstateengine_h
#define oxygentoolbarstateengine_h
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

#include "oxygenanimationengine.h"
#include "oxygengenericengine.h"
#include "oxygendatamap.h"
#include "oxygentoolbarstatedata.h"

#include <gtk/gtk.h>

namespace Oxygen
{
    //! forward declaration
    class Animations;

    //! stores data associated to editable comboboxes
    /*!
    ensures that the text entry and the button of editable comboboxes
    gets hovered and focus flags at the same time
    */
    class ToolBarStateEngine: public GenericEngine<ToolBarStateData>, public AnimationEngine
    {

        public:

        //! constructor
        ToolBarStateEngine( Animations* parent ):
            GenericEngine<ToolBarStateData>( parent )
            {}

        //! destructor
        virtual ~ToolBarStateEngine( void )
        {}

        //! register widget [overloaded]
        virtual bool registerWidget( GtkWidget* widget )
        {
            const bool registered( GenericEngine<ToolBarStateData>::registerWidget( widget ) );
            if( registered )
            {
                data().value( widget ).setDuration( duration() );
                data().value( widget ).setEnabled( enabled() );
            }
            return registered;
        }

        //!@name modifiers
        //@{

        //! register widget [overloaded]
        virtual void registerChild( GtkWidget* widget, GtkWidget* child, bool value = true )
        { data().value( widget ).registerChild( child, value ); }

        //! enable animations
        bool setEnabled( bool value )
        {
            if( !BaseEngine::setEnabled( value ) ) return false;

            for( DataMap<ToolBarStateData>::Map::iterator iter = data().map().begin(); iter != data().map().end(); iter++ )
            {
                iter->second.setEnabled( value );
                if( enabled() && !widgetIsBlackListed( iter->first ) ) iter->second.connect( iter->first );
                else iter->second.disconnect( iter->first );
            }
            return true;
        }

        //! transition duration
        virtual bool setDuration( int value )
        {
            if( !AnimationEngine::setDuration( value ) ) return false;
            for( DataMap<ToolBarStateData>::Map::iterator iter = data().map().begin(); iter != data().map().end(); iter++ )
            { iter->second.setDuration( value ); }
            return false;
        }

        //@}

        //!@name accessors
        //@{

        //! returns parent that matches a given child, and is in list
        GtkWidget* findParent( GtkWidget* widget )
        {
            for( GtkWidget *parent = gtk_widget_get_parent( widget ); parent; parent = gtk_widget_get_parent( parent ) )
            { if( data().contains( parent ) ) return parent; }

            return 0L;
        }

        //! true if animated
        bool isAnimated( GtkWidget* widget )
        { return data().value( widget ).isAnimated(); }

        //! true if given animation type is animated
        bool isAnimated( GtkWidget* widget, const AnimationType& type )
        { return data().value( widget ).isAnimated( type ); }

        //! widget matching type
        GtkWidget* widget( GtkWidget* widget, const AnimationType& type )
        { return data().value( widget ).widget( type ); }

        //! animated rect for given widget and type
        const GdkRectangle& rectangle( GtkWidget* widget, const AnimationType& type )
        { return data().value( widget ).rectangle( type ); }

        //! animation data for given widget and type
        AnimationData animationData( GtkWidget* widget, const AnimationType& type )
        { return data().value( widget ).animationData( type ); }

        //@}

    };

}

#endif
