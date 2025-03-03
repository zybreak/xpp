module;
#include <xcb/xcb.h>
export module xpp.pixmap;

import std;
import xpp.generic.resource;
import xpp.generic.iterator_traits;
import xpp.proto.x;

export namespace xpp {

    template <typename Connection, template <typename, typename> class... Interfaces>
    class pixmap
        : public xpp::generic::resource<Connection, xcb_pixmap_t,
                                        xpp::x::pixmap, Interfaces...> {
      protected:
        using base = xpp::generic::resource<Connection, xcb_pixmap_t,
                                            xpp::x::pixmap, Interfaces...>;

        template <typename C, typename Create, typename Destroy>
        pixmap(C&& c, Create&& create, Destroy&& destroy)
            : base(base::make(std::forward<C>(c),
                              std::forward<Create>(create),
                              std::forward<Destroy>(destroy))) {
        }

      public:
        using base::base;
        using base::operator=;

        template <typename C>
        static pixmap<Connection, Interfaces...>
        create(C&& c, uint8_t depth, xcb_drawable_t drawable,
               uint16_t width, uint16_t height) {
            return pixmap(
                std::forward<C>(c),
                [&](Connection const& c, xcb_pixmap_t const& pixmap) {
                xpp::x::create_pixmap(c, depth, pixmap, drawable, width, height);
            },
                [&](Connection const& c, xcb_pixmap_t const& pixmap) {
                xpp::x::free_pixmap(c, pixmap);
            });
        }

        template <typename C>
        static pixmap<Connection, Interfaces...>
        create_checked(C&& c, uint8_t depth, xcb_drawable_t drawable,
                       uint16_t width, uint16_t height) {
            return pixmap(
                std::forward<C>(c),
                [&](Connection const& c, xcb_pixmap_t const& pixmap) {
                xpp::x::create_pixmap_checked(c, depth, pixmap, drawable, width, height);
            },
                [&](Connection const& c, xcb_pixmap_t const& pixmap) {
                xpp::x::free_pixmap_checked(c, pixmap);
            });
        }
    };

    namespace generic {

        template <typename Connection, template <typename, typename> class... Interfaces>
        struct traits<xpp::pixmap<Connection, Interfaces...>> {
            typedef xcb_pixmap_t type;
        };

    }  // namespace generic

}  // namespace xpp
