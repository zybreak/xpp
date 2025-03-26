module;
#include <xcb/xcb.h>
export module xpp.cursor;

import std;
import xpp.generic.resource;
import xpp.generic.request;
import xpp.generic.iterator_traits;
import xpp.proto.x;

export namespace xpp {

    class cursor : public xpp::x::cursor {
      protected:
        using base = xpp::x::cursor;

        cursor(xcb_connection_t *c, base::Create create, base::Destroy destroy) : base(c, std::forward<Create>(create), std::forward<Destroy>(destroy)) {}

      public:
        using base::base;
        using base::operator=;

        static cursor create(xcb_connection_t *c,
               xcb_pixmap_t source, xcb_pixmap_t mask,
               uint16_t fore_red, uint16_t fore_green, uint16_t fore_blue,
               uint16_t back_red, uint16_t back_green, uint16_t back_blue,
               uint16_t x, uint16_t y) {
            return cursor(
                c,
                [&](xcb_connection_t *c, xcb_cursor_t const& cursor) {
                xcb_create_cursor(c, cursor,
                                      source, mask,
                                      fore_red, fore_green, fore_blue,
                                      back_red, back_green, back_blue,
                                      x, y);
            },
                [&](xcb_connection_t *c, xcb_cursor_t const& cursor) {
                xcb_free_cursor(c, cursor);
            });
        }

        static cursor
        create_checked(xcb_connection_t *c,
                       xcb_pixmap_t source, xcb_pixmap_t mask,
                       uint16_t fore_red, uint16_t fore_green, uint16_t fore_blue,
                       uint16_t back_red, uint16_t back_green, uint16_t back_blue,
                       uint16_t x, uint16_t y) {
            return cursor(
                c,
                [&](xcb_connection_t *c, xcb_cursor_t const& cursor) {
                xpp::generic::check(c, xcb_create_cursor_checked(c, cursor,
                                              source, mask,
                                              fore_red, fore_green, fore_blue,
                                              back_red, back_green, back_blue,
                                              x, y));
            },
                [&](xcb_connection_t *c, xcb_cursor_t const& cursor) {
                xpp::generic::check(c, xcb_free_cursor_checked(c, cursor));
            });
        }

        static cursor
        create_glyph(xcb_connection_t *c,
                     xcb_font_t source_font, xcb_font_t mask_font,
                     uint16_t source_char, uint16_t mask_char,
                     uint16_t fore_red, uint16_t fore_green, uint16_t fore_blue,
                     uint16_t back_red, uint16_t back_green, uint16_t back_blue) {
            return cursor(
                c,
                [&](xcb_connection_t *c, xcb_cursor_t const& cursor) {
                xpp::generic::check(c, xcb_create_glyph_cursor(c, cursor,
                                            source_font, mask_font,
                                            source_char, mask_char,
                                            fore_red, fore_green, fore_blue,
                                            back_red, back_green, back_blue));
            },
                [](xcb_connection_t *c, xcb_cursor_t const& cursor) {
                xpp::generic::check(c, xcb_free_cursor(c, cursor));
            });
        }

        static cursor
        create_glyph_checked(xcb_connection_t *c,
                             xcb_font_t source_font, xcb_font_t mask_font,
                             uint16_t source_char, uint16_t mask_char,
                             uint16_t fore_red, uint16_t fore_green, uint16_t fore_blue,
                             uint16_t back_red, uint16_t back_green, uint16_t back_blue) {
            return cursor(
                c,
                [&](xcb_connection_t *c, xcb_cursor_t const& cursor) {
                xpp::generic::check(c, xcb_create_glyph_cursor_checked(c, cursor,
                                                    source_font, mask_font,
                                                    source_char, mask_char,
                                                    fore_red, fore_green, fore_blue,
                                                    back_red, back_green, back_blue));
            },
                [](xcb_connection_t *c, xcb_cursor_t const& cursor) {
                xpp::generic::check(c, xcb_free_cursor_checked(c, cursor));
            });
        }
    };

    namespace generic {

        template<>
        struct traits<xpp::cursor> {
            typedef xcb_cursor_t type;
        };

    }  // namespace generic

}  // namespace xpp
