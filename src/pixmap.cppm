module;
#include <xcb/xcb.h>
export module xpp.pixmap;

import std;
import xpp.generic.resource;
import xpp.generic.request;
import xpp.generic.iterator_traits;
import xpp.proto.x;

export namespace xpp {

    class pixmap
        : public xpp::x::pixmap {
      protected:
        using base = xpp::x::pixmap;

        template<typename Create, typename Destroy>
        pixmap(xcb_connection_t *c, Create&& create, Destroy&& destroy) : base(c, std::forward<Create>(create), std::forward<Destroy>(destroy)) {}

      public:
        using base::base;
        using base::operator=;

        static pixmap
        create(xcb_connection_t *c, uint8_t depth, xcb_drawable_t drawable,
               uint16_t width, uint16_t height) {
            return pixmap(
                c,
                [&](xcb_connection_t *c, xcb_pixmap_t const& pixmap) {
                xcb_create_pixmap(c, depth, pixmap, drawable, width, height);
            },
                [&](xcb_connection_t *c, xcb_pixmap_t const& pixmap) {
                xcb_free_pixmap(c, pixmap);
            });
        }

        static pixmap
        create_checked(xcb_connection_t *c, uint8_t depth, xcb_drawable_t drawable,
                       uint16_t width, uint16_t height) {
            return pixmap(
                c,
                [&](xcb_connection_t *c, xcb_pixmap_t const& pixmap) {
                xpp::generic::check(c, xcb_create_pixmap_checked(c, depth, pixmap, drawable, width, height));
            },
                [&](xcb_connection_t *c, xcb_pixmap_t const& pixmap) {
                xpp::generic::check(c, xcb_free_pixmap_checked(c, pixmap));
            });
        }
    };

    namespace generic {

        template<>
        struct traits<xpp::pixmap> {
            typedef xcb_pixmap_t type;
        };

    }  // namespace generic

}  // namespace xpp
