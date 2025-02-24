module;
#include <xcb/xcb.h>
export module xpp.core;

import std;

export namespace xpp {

    class connection_error
        : public std::runtime_error {
      public:
        connection_error(uint8_t code, std::string const &description)
            : std::runtime_error(description + "(" + std::to_string(code) + ")"), m_code(code), m_description(description) {
        }

        uint8_t
        code() {
            return m_code;
        }

        std::string
        description() {
            return m_description;
        }

      protected:
        uint8_t m_code;
        std::string m_description;
    };

    class core {
      protected:
        using shared_generic_event_ptr = std::shared_ptr<xcb_generic_event_t>;

        int m_screen = 0;
        // reference counting for xcb_connection_t
        std::shared_ptr<xcb_connection_t> m_c;

        shared_generic_event_ptr dispatch(std::string const &producer, xcb_generic_event_t *event) const;

      public:
        explicit core(xcb_connection_t *c)
            : m_c(std::shared_ptr<xcb_connection_t>(c, [](...) {})) {
        }

        template <typename... ConnectionParameter>
        explicit core(xcb_connection_t *(*Connect)(ConnectionParameter...),
                      ConnectionParameter... connection_parameter)
            : m_c(std::shared_ptr<xcb_connection_t>(
                  Connect(connection_parameter...),
                  [&](void *) { disconnect(); })) {
        }

        // xcb_connect (const char *displayname, int *screenp)
        explicit core(std::string const &displayname = "")
            : core(xcb_connect, displayname.c_str(), &m_screen) {
        }

        // xcb_connect_to_fd (int fd, xcb_auth_info_t *auth_info)
        explicit core(int fd, xcb_auth_info_t *auth_info)
            : core(xcb_connect_to_fd, fd, auth_info) {
        }

        // xcb_connect_to_display_with_auth_info (
        //     const char *display, xcb_auth_info_t *auth, int *screen)
        explicit core(std::string const &display, xcb_auth_info_t *auth)
            : core(xcb_connect_to_display_with_auth_info,
                   display.c_str(), auth, &m_screen) {
        }

        virtual ~core() {
        }

        virtual xcb_connection_t * operator*() const;
        virtual operator xcb_connection_t *() const;
        virtual int default_screen() const;
        virtual int flush() const;
        virtual uint32_t get_maximum_request_length() const;
        virtual void prefetch_maximum_request_length() const;
        virtual shared_generic_event_ptr wait_for_event() const;
        virtual shared_generic_event_ptr poll_for_event() const;
        virtual shared_generic_event_ptr poll_for_queued_event() const;
        virtual shared_generic_event_ptr poll_for_special_event(xcb_special_event_t *se) const;
        virtual shared_generic_event_ptr wait_for_special_event(xcb_special_event_t *se) const;
        virtual xcb_special_event_t * register_for_special_xge(xcb_extension_t *ext, uint32_t eid, uint32_t *stamp) const;
        virtual void unregister_for_special_event(xcb_special_event_t *se) const;
        virtual std::shared_ptr<xcb_generic_error_t> request_check(xcb_void_cookie_t cookie) const;
        virtual void discard_reply(unsigned int sequence) const;
        // The result must not be freed.
        // This storage is managed by the cache itself.
        virtual xcb_query_extension_reply_t const * get_extension_data(xcb_extension_t *ext) const;
        virtual void prefetch_extension_data(xcb_extension_t *ext) const;
        virtual xcb_setup_t const * get_setup() const;
        virtual int get_file_descriptor() const;
        virtual int connection_has_error() const;
        virtual void disconnect() const;
        // hostname, display, screen
        virtual std::tuple<std::string, int, int> parse_display(std::string const &name) const;
        virtual uint32_t generate_id() const;
        xcb_screen_t * screen_of_display(int screen);
        void check_connection() const;
    };  // class core

}  // namespace xpp
