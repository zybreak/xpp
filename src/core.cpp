module;
#include <xcb/xcb.h>
module xpp.connection;

import std;
import xpp.core;

namespace xpp {

    connection::shared_generic_event_ptr connection::dispatch(std::string const &producer, xcb_generic_event_t *event) const {
        if (event) {
            if (event->response_type == 0) {
                throw std::shared_ptr<xcb_generic_error_t>(
                    reinterpret_cast<xcb_generic_error_t *>(event));
            }

            return shared_generic_event_ptr(event, std::free);
        }

        check_connection();
        throw std::runtime_error(producer + " failed");
    }

    xcb_connection_t *connection::operator*() const {
        return m_c.get();
    }

    connection::operator xcb_connection_t *() const {
        return m_c.get();
    }

    int connection::default_screen() const {
        return m_screen;
    }

    int connection::flush() const {
        return xcb_flush(m_c.get());
    }

    uint32_t connection::get_maximum_request_length() const {
        return xcb_get_maximum_request_length(m_c.get());
    }

    void connection::prefetch_maximum_request_length() const {
        xcb_prefetch_maximum_request_length(m_c.get());
    }

    connection::shared_generic_event_ptr connection::wait_for_event() const {
        return dispatch("wait_for_event", xcb_wait_for_event(m_c.get()));
    }

    connection::shared_generic_event_ptr connection::poll_for_event() const {
        return shared_generic_event_ptr(xcb_poll_for_event(m_c.get()), std::free);
    }

    connection::shared_generic_event_ptr connection::poll_for_queued_event() const {
        return shared_generic_event_ptr(xcb_poll_for_queued_event(m_c.get()), std::free);
    }

    connection::shared_generic_event_ptr connection::poll_for_special_event(xcb_special_event_t *se) const {
        return shared_generic_event_ptr(xcb_poll_for_special_event(m_c.get(), se), std::free);
    }

    connection::shared_generic_event_ptr connection::wait_for_special_event(xcb_special_event_t *se) const {
        return dispatch("wait_for_special_event",
                        xcb_wait_for_special_event(m_c.get(), se));
    }

    xcb_special_event_t *connection::register_for_special_xge(xcb_extension_t *ext,
                                                        uint32_t eid,
                                                        uint32_t *stamp) const {
        return xcb_register_for_special_xge(m_c.get(), ext, eid, stamp);
    }

    void connection::unregister_for_special_event(xcb_special_event_t *se) const {
        xcb_unregister_for_special_event(m_c.get(), se);
    }

    std::shared_ptr<xcb_generic_error_t> connection::request_check(xcb_void_cookie_t cookie) const {
        return std::shared_ptr<xcb_generic_error_t>(
            xcb_request_check(m_c.get(), cookie));
    }

    void connection::discard_reply(unsigned int sequence) const {
        xcb_discard_reply(m_c.get(), sequence);
    }

    // The result must not be freed.
    // This storage is managed by the cache itself.
    xcb_query_extension_reply_t const * connection::get_extension_data(xcb_extension_t *ext) const {
        return xcb_get_extension_data(m_c.get(), ext);
    }

    void connection::prefetch_extension_data(xcb_extension_t *ext) const {
        xcb_prefetch_extension_data(m_c.get(), ext);
    }

    xcb_setup_t const *connection::get_setup() const {
        return xcb_get_setup(m_c.get());
    }

    int connection::get_file_descriptor() const {
        return xcb_get_file_descriptor(m_c.get());
    }

    // TODO: Return a proper enum
    int connection::connection_has_error() const {
        return xcb_connection_has_error(m_c.get());
    }

    void connection::disconnect() {
        //xcb_disconnect(m_c.get());
        m_c.reset(); // TODO: This resolves a double-free when connection class has aquired its own xcb_connection_t, but has no effect when passed an existing xcb_connection_t
    }

    // hostname, display, screen
    std::tuple<std::string, int, int> connection::parse_display(std::string const &name) const {
        int screen = 0;
        int display = 0;
        char *host = nullptr;
        std::string hostname;

        xcb_parse_display(name.c_str(), &host, &display, &screen);
        if (host != nullptr) {
            hostname = std::string(host);
        }

        return std::make_tuple(hostname, display, screen);
    }

    uint32_t connection::generate_id() const {
        return xcb_generate_id(m_c.get());
    }

    xcb_screen_t *connection::screen_of_display(int screen) {
        for (xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(m_c.get())); iter.rem; --screen, xcb_screen_next(&iter)) {
            if (screen == 0) {
                return iter.data;
            }
        }

        return nullptr;
    }

    void connection::check_connection() const {
        switch (xcb_connection_has_error(m_c.get())) {
            case XCB_CONN_ERROR:
                throw(connection_error(
                    XCB_CONN_ERROR, "XCB_CONN_ERROR"));

            case XCB_CONN_CLOSED_EXT_NOTSUPPORTED:
                throw(connection_error(XCB_CONN_CLOSED_EXT_NOTSUPPORTED,
                                       "XCB_CONN_CLOSED_EXT_NOTSUPPORTED"));

            case XCB_CONN_CLOSED_MEM_INSUFFICIENT:
                throw(connection_error(XCB_CONN_CLOSED_MEM_INSUFFICIENT,
                                       "XCB_CONN_CLOSED_MEM_INSUFFICIENT"));

            case XCB_CONN_CLOSED_REQ_LEN_EXCEED:
                throw(connection_error(XCB_CONN_CLOSED_REQ_LEN_EXCEED,
                                       "XCB_CONN_CLOSED_REQ_LEN_EXCEED"));

            case XCB_CONN_CLOSED_PARSE_ERR:
                throw(connection_error(XCB_CONN_CLOSED_PARSE_ERR,
                                       "XCB_CONN_CLOSED_PARSE_ERR"));

            case XCB_CONN_CLOSED_INVALID_SCREEN:
                throw(connection_error(XCB_CONN_CLOSED_INVALID_SCREEN,
                                       "XCB_CONN_CLOSED_INVALID_SCREEN"));
        };
    }
}  // namespace xpp
