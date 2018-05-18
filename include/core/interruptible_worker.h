/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2018 CERN
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef __INTERRUPTIBLE_WORKER_H
#define __INTERRUPTIBLE_WORKER_H

#include <thread>
#include <atomic>
#include <functional>

class INTERRUPTIBLE_WORKER
{
    public:
        INTERRUPTIBLE_WORKER( std::function<void( INTERRUPTIBLE_WORKER* )> aFunc ) :
            m_kill(false),
            m_func(aFunc)
        {}

        ~INTERRUPTIBLE_WORKER()
        {
            if( m_thread.joinable() )
                Interrupt();
        }

        virtual void DoWork()
        {
            if (m_func)
                m_func( this );
        }

        bool CheckInterrupt() const { return m_kill; }

        void Run( bool aSynchronous = false )
        {
            auto theThread = [&] () -> void
            {
                DoWork();
            };

            m_kill = false;
            m_thread = std::thread( theThread );

            if( aSynchronous && m_thread.joinable() )
                m_thread.join();
        }

        void Interrupt()
        {
            m_kill = true;
            if( m_thread.joinable() )
                m_thread.join();
        }

        bool Running()
        {
            return m_thread.joinable();
        }

        void Join()
        {
            if( m_thread.joinable() )
                m_thread.join();
        }

    private:

        std::function<void(INTERRUPTIBLE_WORKER*)> m_func;
        std::atomic<bool> m_kill;
        std::thread m_thread;
};

#endif // __INTERRUPTIBLE_WORKER_H
