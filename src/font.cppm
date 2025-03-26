module;
#include <xcb/xcb.h>
export module xpp.font;

import std;
import xpp.generic.resource;
import xpp.generic.request;
import xpp.generic.iterator_traits;
import xpp.proto.x;

// font
export namespace xpp {

    class font : public xpp::x::font {
      protected:
        using base = xpp::x::font;

        font(xcb_connection_t *c, base::Create create, base::Destroy destroy) : base(c, create, destroy) {}

      public:
        using base::base;
        using base::operator=;

        static font open(xcb_connection_t *c, std::string const& name) noexcept {
            return font(c,
                        [&](xcb_connection_t *c, xcb_font_t const& font) {
                xcb_open_font(c, font, name.length(), name.data());
            },
                        [&](xcb_connection_t *c, xcb_font_t const& font) {
                xcb_close_font(c, font);
            });
        }

        static font open_checked(xcb_connection_t *c, std::string const& name) {
            return font(c,
                        [&](xcb_connection_t *c, xcb_font_t const& font) {
                xpp::generic::check(c, xcb_open_font_checked(c, font, name.length(), name.data()));
            },
                        [&](xcb_connection_t *c, xcb_font_t const& font) {
                xpp::generic::check(c, xcb_close_font_checked(c, font));
            });
        }
    };

    namespace generic {

        template<>
        struct traits<xpp::font> {
            typedef xcb_font_t type;
        };

    }  // namespace generic

}  // namespace xpp
