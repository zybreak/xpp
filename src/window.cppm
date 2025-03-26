module;
#include <xcb/xcb.h>
export module xpp.window;

import std;
import xpp.generic.resource;
import xpp.generic.request;
import xpp.generic.iterator_traits;
import xpp.proto.x;

export namespace xpp {

    class window
        : public xpp::x::window {
      protected:
        using base = xpp::x::window;

        window(xcb_connection_t *c, base::Create create, base::Destroy destroy) : base(c, std::forward<Create>(create), std::forward<Destroy>(destroy)) {}

      public:
        window(window const&) = default;
        using base::base;
        using base::operator=;

        static window
        create(xcb_connection_t *c, uint8_t depth, xcb_window_t parent,
               int16_t x, int16_t y, uint16_t width, uint16_t height,
               uint16_t border_width,
               uint16_t _class, xcb_visualid_t visual,
               uint32_t value_mask, xcb_create_window_value_list_t const* value_list) {
            return window(
                c,
                [&](xcb_connection_t *c, xcb_window_t const& window) {
                xcb_create_window(c, depth, window, parent,
                                      x, y, width, height, border_width,
                                      _class, visual,
                                      value_mask, value_list);
            },
                [&](xcb_connection_t *c, xcb_window_t const& window) {
                xcb_destroy_window(c, window);
            });
        }

        static window
        create_checked(xcb_connection_t *c, uint8_t depth, xcb_window_t parent,
                       int16_t x, int16_t y, uint16_t width, uint16_t height,
                       uint16_t border_width,
                       uint16_t _class, xcb_visualid_t visual,
                       uint32_t value_mask, xcb_create_window_value_list_t const* value_list) {
            return window(
                c,
                [&](xcb_connection_t *c, xcb_window_t const& window) {
                xpp::generic::check(c, xcb_create_window_checked(c, depth, window, parent,
                                              x, y, width, height, border_width,
                                              _class, visual,
                                              value_mask, value_list));
            },
                [&](xcb_connection_t *c, xcb_window_t const& window) {
                xpp::generic::check(c, xcb_destroy_window_checked(c, window));
            });
        }
    };

    namespace generic {

        template<>
        struct traits<xpp::window> {
            typedef xcb_window_t type;
        };

    }  // namespace generic

}  // namespace xpp
