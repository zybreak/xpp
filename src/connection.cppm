module;
#include <xcb/xcb.h>
export module xpp.connection;

import std;
import xpp.core;
import xpp.generic.resource;
import xpp.generic.error;
import xpp.generic.factory;
import xpp.generic.extension;
import xpp.proto.x;
import xpp.proto.randr;

export namespace xpp {

    class connection : public xpp::x::interface {

      public:
        using shared_generic_event_ptr = std::shared_ptr<xcb_generic_event_t>;
        
        explicit connection(xcb_connection_t *c) : m_c(std::shared_ptr<xcb_connection_t>(c, [](...) {})) {}
        
        template<typename... ConnectionParameter>
        explicit connection(xcb_connection_t *(*Connect)(ConnectionParameter...),
                      ConnectionParameter... connection_parameter)
            : m_c(std::shared_ptr<xcb_connection_t>(
                  Connect(connection_parameter...),
                  [&](xcb_connection_t *c) { xcb_disconnect(c); })) {
        }

        // xcb_connect (const char *displayname, int *screenp)
        explicit connection(std::string const &displayname = "")
            : connection(xcb_connect, displayname.c_str(), &m_screen) {}

        // xcb_connect_to_fd (int fd, xcb_auth_info_t *auth_info)
        explicit connection(int fd, xcb_auth_info_t *auth_info)
            : connection(xcb_connect_to_fd, fd, auth_info) {}

        // xcb_connect_to_display_with_auth_info (
        //     const char *display, xcb_auth_info_t *auth, int *screen)
        explicit connection(std::string const &display, xcb_auth_info_t *auth)
            : connection(xcb_connect_to_display_with_auth_info,
                   display.c_str(), auth, &m_screen) {}
        
        template<typename... Parameters>
        connection(Parameters &&...parameters) : connection(std::forward<Parameters>(parameters)...) {
            m_root_window = screen_of_display(this->default_screen())->root;
        }

        void operator()(std::shared_ptr<xcb_generic_error_t> const &error) const {
            check(error);
        }
        
        xpp::x::window root() {
            using make = xpp::generic::factory::make<xcb_window_t, xpp::x::window>;
            return make()(*this, m_root_window);
        }

        xpp::x::window root() const {
            using make = xpp::generic::factory::make<xcb_window_t, xpp::x::window>;
            return make()(*this, m_root_window);
        }

        xpp::randr::interface randr() const {
            return xpp::randr::interface{*this};
        }
        
        xcb_connection_t *operator*() const;
        operator xcb_connection_t *() const;
        int default_screen() const;
        int flush() const;
        uint32_t get_maximum_request_length() const;
        void prefetch_maximum_request_length() const;
        shared_generic_event_ptr wait_for_event() const;
        shared_generic_event_ptr poll_for_event() const;
        shared_generic_event_ptr poll_for_queued_event() const;
        shared_generic_event_ptr poll_for_special_event(xcb_special_event_t *se) const;
        shared_generic_event_ptr wait_for_special_event(xcb_special_event_t *se) const;
        xcb_special_event_t *register_for_special_xge(xcb_extension_t *ext, uint32_t eid, uint32_t *stamp) const;
        void unregister_for_special_event(xcb_special_event_t *se) const;
        std::shared_ptr<xcb_generic_error_t> request_check(xcb_void_cookie_t cookie) const;
        void discard_reply(unsigned int sequence) const;
        // The result must not be freed.
        // This storage is managed by the cache itself.
        xcb_query_extension_reply_t const *get_extension_data(xcb_extension_t *ext) const;
        void prefetch_extension_data(xcb_extension_t *ext) const;
        xcb_setup_t const *get_setup() const;
        int get_file_descriptor() const;
        int connection_has_error() const;
        void disconnect();
        // hostname, display, screen
        std::tuple<std::string, int, int> parse_display(std::string const &name) const;
        uint32_t generate_id() const;
        xcb_screen_t *screen_of_display(int screen);
        void check_connection() const;

      private:

        virtual xcb_connection_t* get_connection() const override {
            return *this;
        }
        
        xcb_window_t m_root_window;
        
        int m_screen = 0;
        // reference counting for xcb_connection_t
        std::shared_ptr<xcb_connection_t> m_c;

        shared_generic_event_ptr dispatch(std::string const &producer, xcb_generic_event_t *event) const;


        void check(std::shared_ptr<xcb_generic_error_t> const &error) const {
            // TODO: iterate over extensions and do
            //using error_dispatcher = typename Extension::error_dispatcher;
            //auto & dispatcher = static_cast<const error_dispatcher &>(*this);
            //dispatcher(error);
        }

    };  // class connection

}  // namespace xpp
