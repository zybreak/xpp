module;
#include <xcb/xcb.h>
export module xpp.colormap;

import std;
import xpp.generic.resource;
import xpp.generic.iterator_traits;
import xpp.proto.x;

export namespace xpp {

    class colormap : public xpp::x::colormap {
      protected:
        using base = xpp::x::colormap;

        colormap(xcb_connection_t *c, base::Create create, base::Destroy destroy) : base(c, std::forward<Create>(create), std::forward<Destroy>(destroy)) {
        }

      public:
        using base::base;
        using base::operator=;

        static colormap create(xcb_connection_t *c, uint8_t alloc, xcb_window_t window, xcb_visualid_t visual) {
            return colormap(
                c,
                [&](xcb_connection_t *c, xcb_colormap_t const& colormap) {
                xpp::x::create_colormap(c, alloc, colormap, window, visual);
            },
                [&](xcb_connection_t *c, xcb_colormap_t const& colormap) {
                xpp::x::free_colormap(c, colormap);
            });
        }

        static colormap create_checked(xcb_connection_t *c, uint8_t alloc,
                       xcb_window_t window, xcb_visualid_t visual) {
            return colormap(
                c,
                [&](xcb_connection_t *c, xcb_colormap_t const& colormap) {
                xpp::x::create_colormap_checked(c, alloc, colormap, window, visual);
            },
                [&](xcb_connection_t *c, xcb_colormap_t const& colormap) {
                xpp::x::free_colormap_checked(c, colormap);
            });
        }
    };

    namespace generic {

        template<>
        struct traits<xpp::colormap> {
            typedef xcb_colormap_t type;
        };

    }  // namespace generic

}  // namespace xpp
