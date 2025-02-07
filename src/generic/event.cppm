module;
#include <xcb/xcb.h>
export module xpp.generic.event;

import std;

export namespace xpp { namespace generic {
        
        template<typename Event>
        class event {
          public:
            event(const std::shared_ptr<xcb_generic_event_t> & event)
                : m_event(event)
            {}
            
            virtual
                ~event(void) {}

            virtual
            operator const Event &(void) const
            {
                return reinterpret_cast<const Event &>(*m_event);
            }

            virtual
                const Event &
                operator*(void) const
            {
                return reinterpret_cast<const Event &>(*m_event);
            }

            virtual
                Event *
                operator->(void) const
            {
                return reinterpret_cast<Event * const>(m_event.get());
            }

          protected:
            std::shared_ptr<xcb_generic_event_t> m_event;
        }; // class event

    } } // namespace xpp::generic
