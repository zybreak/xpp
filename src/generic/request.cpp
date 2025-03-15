module;
#include <xcb/xcb.h>
module xpp.generic.request;

import xpp.connection;
import std;

namespace xpp::generic {
    void check(xcb_connection_t *c, xcb_void_cookie_t const &cookie) {
        xcb_generic_error_t *error = xcb_request_check(c, cookie);
        if (error) {
            dispatch(c, std::shared_ptr<xcb_generic_error_t>(error, std::free));
        }
    }
}