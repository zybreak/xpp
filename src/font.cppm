module;
#include <xcb/xcb.h>
export module xpp.font;

import std;
import xpp.generic.resource;
import xpp.generic.iterator_traits;
import xpp.proto.x;

// font
export namespace xpp {

    template <typename Connection, template <typename, typename> class... Interfaces>
    class font
        : public xpp::generic::resource<Connection, xcb_font_t,
                                        xpp::x::font, Interfaces...> {
      protected:
        using base = xpp::generic::resource<Connection, xcb_font_t,
                                            xpp::x::font, Interfaces...>;

        template <typename C, typename Create, typename Destroy>
        font(C&& c, Create&& create, Destroy&& destroy)
            : base(base::make(std::forward<C>(c),
                              std::forward<Create>(create),
                              std::forward<Destroy>(destroy))) {
        }

      public:
        using base::base;
        using base::operator=;

        template <typename C>
        static font<Connection, Interfaces...>
        open(C&& c, std::string const& name) noexcept {
            return font(std::forward<C>(c),
                        [&](Connection const& c, xcb_font_t const& font) {
                xpp::x::open_font(c, font, name);
            },
                        [&](Connection const& c, xcb_font_t const& font) {
                xpp::x::close_font(c, font);
            });
        }

        template <typename C>
        static font<Connection, Interfaces...>
        open_checked(C&& c, std::string const& name) {
            return font(std::forward<C>(c),
                        [&](Connection const& c, xcb_font_t const& font) {
                xpp::x::open_font_checked(c, font, name);
            },
                        [&](Connection const& c, xcb_font_t const& font) {
                xpp::x::close_font_checked(c, font);
            });
        }
    };

    namespace generic {

        template <typename Connection, template <typename, typename> class... Interfaces>
        struct traits<xpp::font<Connection, Interfaces...>> {
            typedef xcb_font_t type;
        };

    }  // namespace generic

}  // namespace xpp
