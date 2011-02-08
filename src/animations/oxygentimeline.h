#ifndef oxygentimeline_h
#define oxygentimeline_h

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

#include <glib.h>

namespace Oxygen
{

    //! timeline used to handle animations
    class TimeLine
    {

        public:

        //! constructor
        TimeLine( int duration );

        //! destructor
        virtual ~TimeLine( void );

        //! duration
        void setDuration( int duration )
        { _duration = duration; }

        //! connect callback
        void connect( GSourceFunc func, gpointer data )
        {
            _func = func;
            _data = data;
        }

        //! disconnect
        void disconnect( void )
        {
            _func = 0L;
            _data = 0L;
        }

        //! start
        void start( void );

        //! stop
        void stop( void );

        //! duration
        int duration( void ) const
        { return _duration; }

        //! direction
        enum Direction
        {
            Forward,
            Backward
        };

        //! direction
        void setDirection( Direction direction )
        { _direction = direction; }

        //! direction
        Direction direction( void ) const
        { return _direction; }

        //! update value and running state
        /*!
        also emits signal when value has changed since last time.
        returns true if timeline is still running
        */
        bool update( void );

        //! value (between 0 and 1)
        double value( void ) const
        { return _value; }

        //! true if running
        bool isRunning( void ) const
        { return _running; }

        protected:

        //! run callback
        void trigger( void ) const
        { if( _func )  (_func)(_data); }

        private:

        //! duration
        int _duration;

        //! direction
        Direction _direction;

        //! true if timer is running
        bool _running;

        //! value (between 0 and 1)
        double _value;

        //! time (at which _value was last calculated)
        int _time;

        //! timer
        GTimer* _timer;

        //! source function
        GSourceFunc _func;

        //! data
        gpointer _data;

    };


};

#endif
