module;
#include <xcb/xcb.h>
export module xpp.connection;

import std;
import xpp.core;
import xpp.generic.resource;
import xpp.generic.error;
import xpp.generic.factory;
import xpp.proto.x;
import xpp.proto.randr;

#if 0
namespace xpp::detail {

    template<typename... Extensions>
    class interfaces
        : public xpp::x::extension::interface<interfaces<Extensions...>>,
          public Extensions::template interface<interfaces<Extensions...>>... {
      public:
        xpp::connection& connection(void) const {
            return static_cast<xpp::connection &>(*this);
        }
    };  // class interfaces

};  // namespace xpp::detail
#endif
export namespace xpp {

    class connection
        : public xpp::core,
          public xpp::x::interface
          //public xpp::generic::error_dispatcher
          //public detail::interfaces<xpp::randr::extension>,
          // private interfaces: extensions and error_dispatcher
          //private xpp::x::extension
          //private xpp::x::extension::error_dispatcher,
          //private xpp::randr::extension//private Extensions...,
          //private xpp::randr::extension::error_dispatcher //private Extensions::error_dispatcher... {
    {
      private:
        virtual xcb_connection_t* get_connection() const override {
            return *this;
        }

      public:

        connection() : xpp::core::core() {
            
        }
        
        template<typename... Parameters>
        connection(Parameters &&...parameters) : xpp::core::core(std::forward<Parameters>(parameters)...) {
            m_root_window = screen_of_display(this->default_screen())->root;
        }
        
        virtual ~connection() =default;

#if 0
        void operator()(std::shared_ptr<xcb_generic_error_t> const &error) const {
            check<xpp::x::extension, xpp::randr::extension>(error);
        }

        template<typename Extension>
        Extension const &
        extension(void) const {
            return static_cast<Extension const &>(*this);
        }
#endif

        // TODO
        // virtual operator Display * const(void) const
        // {
        // }

        template<typename Window = xcb_window_t>
        Window root() {
            using make = xpp::generic::factory::make<xcb_window_t, Window>;
            return make()(*this, m_root_window);
        }

        template<typename Window = xcb_window_t>
        Window root() const {
            using make = xpp::generic::factory::make<xcb_window_t, Window>;
            return make()(*this, m_root_window);
        }

#if 0
        virtual shared_generic_event_ptr wait_for_event(void) const {
            try {
                return core::wait_for_event();
            } catch (std::shared_ptr<xcb_generic_error_t> const &error) {
                check<xpp::x::extension, xpp::randr::extension>(error);
            }
            // re-throw any exception caused by wait_for_event
            throw;
        }

        virtual shared_generic_event_ptr wait_for_special_event(xcb_special_event_t *se) const {
            try {
                return core::wait_for_special_event(se);
            } catch (std::shared_ptr<xcb_generic_error_t> const &error) {
                check<xpp::x::extension, xpp::randr::extension>(error);
            }
            // re-throw any exception caused by wait_for_special_event
            throw;
        }
#endif
      private:
        xcb_window_t m_root_window;

        template<typename Extension, typename Next, typename... Rest>
        void check(std::shared_ptr<xcb_generic_error_t> const &error) const {
            //check<Extension>(error);
            //check<Next, Rest...>(error);
        }

#if 0
        template<typename Extension>
        void check(std::shared_ptr<xcb_generic_error_t> const &error) const {
            using error_dispatcher = typename Extension::error_dispatcher;
            auto &dispatcher = static_cast<error_dispatcher const &>(*this);
            dispatcher(error);
        }
#endif
    };  // class connection

}  // namespace xpp
