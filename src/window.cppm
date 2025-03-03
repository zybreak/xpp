module;
#include <xcb/xcb.h>
export module xpp.window;

import std;
import xpp.generic.resource;
import xpp.generic.iterator_traits;
import xpp.proto.x;

export namespace xpp {

    template <typename Connection, template <typename, typename> class... Interfaces>
    class window
        : public xpp::generic::resource<Connection, xcb_window_t,
                                        xpp::x::window, Interfaces...> {
      protected:
        using base = xpp::generic::resource<Connection, xcb_window_t,
                                            xpp::x::window, Interfaces...>;

        template <typename C, typename Create, typename Destroy>
        window(C&& c, Create&& create, Destroy&& destroy)
            : base(base::make(std::forward<C>(c),
                              std::forward<Create>(create),
                              std::forward<Destroy>(destroy))) {
        }

      public:
        window(window const&) = default;
        using base::base;
        using base::operator=;

        template <typename C>
        static window<Connection, Interfaces...>
        create(C&& c, uint8_t depth, xcb_window_t parent,
               int16_t x, int16_t y, uint16_t width, uint16_t height,
               uint16_t border_width,
               uint16_t _class, xcb_visualid_t visual,
               uint32_t value_mask, uint32_t const* value_list) {
            return window(
                std::forward<C>(c),
                [&](Connection const& c, xcb_window_t const& window) {
                xpp::x::create_window(c, depth, window, parent,
                                      x, y, width, height, border_width,
                                      _class, visual,
                                      value_mask, value_list);
            },
                [&](Connection const& c, xcb_window_t const& window) {
                xpp::x::destroy_window(c, window);
            });
        }

        template <typename C>
        static window<Connection, Interfaces...>
        create_checked(C&& c, uint8_t depth, xcb_window_t parent,
                       int16_t x, int16_t y, uint16_t width, uint16_t height,
                       uint16_t border_width,
                       uint16_t _class, xcb_visualid_t visual,
                       uint32_t value_mask, uint32_t const* value_list) {
            return window(
                std::forward<C>(c),
                [&](Connection const& c, xcb_window_t const& window) {
                xpp::x::create_window_checked(c, depth, window, parent,
                                              x, y, width, height, border_width,
                                              _class, visual,
                                              value_mask, value_list);
            },
                [&](Connection const& c, xcb_window_t const& window) {
                xpp::x::destroy_window_checked(c, window);
            });
        }
    };

    namespace generic {

        template <typename Connection, template <typename, typename> class... Interfaces>
        struct traits<xpp::window<Connection, Interfaces...>> {
            typedef xcb_window_t type;
        };

    }  // namespace generic

}  // namespace xpp
