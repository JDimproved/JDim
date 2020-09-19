// ライセンス: GPL2

#ifndef _TIMEOUT_H
#define _TIMEOUT_H

#include <gtkmm.h>

#include <memory>


namespace JDLIB
{
    class Timeout
    {
        sigc::slot< bool > m_slot_timeout;
        sigc::connection m_connection;

    public:
        ~Timeout();
        
        static std::unique_ptr<Timeout> connect( const sigc::slot< bool > slot_timeout, unsigned int interval );

    private:
        explicit Timeout( const sigc::slot< bool > slot_timeout );
    };
}

#endif // _TIMEOUT_H
