module;
#include <xcb/xcb.h>
export module xpp.gcontext;

import std;
import xpp.generic.resource;
import xpp.generic.request;
import xpp.generic.iterator_traits;
import xpp.proto.x;

export namespace xpp {

    class gcontext
        : public xpp::x::gcontext {
      protected:
        using base = xpp::x::gcontext;

        gcontext(xcb_connection_t *c, base::Create&& create, base::Destroy&& destroy) : base(c, std::forward<Create>(create), std::forward<Destroy>(destroy)) {}

      public:
        using base::base;
        using base::operator=;

        static gcontext
        create(xcb_connection_t *c, xcb_drawable_t drawable,
               uint32_t value_mask, xcb_create_gc_value_list_t const* value_list) {
            return gcontext(
                c,
                [&](xcb_connection_t *c, xcb_gcontext_t const& gcontext) {
                xcb_create_gc(c, gcontext, drawable, value_mask, value_list);
            },
                [&](xcb_connection_t *c, xcb_gcontext_t const& gcontext) {
                xcb_free_gc(c, gcontext);
            });
        }

        static gcontext
        create_checked(xcb_connection_t *c, xcb_drawable_t drawable,
                       uint32_t value_mask, xcb_create_gc_value_list_t const* value_list) {
            return gcontext(
                c,
                [&](xcb_connection_t *c, xcb_gcontext_t const& gcontext) {
                xpp::generic::check(c, xcb_create_gc_checked(c, gcontext, drawable,
                                          value_mask, value_list));
            },
                [&](xcb_connection_t *c, xcb_gcontext_t const& gcontext) {
                xpp::generic::check(c, xcb_free_gc_checked(c, gcontext));
            });
        }

        static gcontext
        copy(xcb_connection_t *c, xcb_gcontext_t src_gc, uint32_t value_mask) {
            return gcontext(
                c,
                [&](xcb_connection_t *c, xcb_gcontext_t const& gcontext) {
                xcb_copy_gc(c, src_gc, gcontext, value_mask);
            },
                [&](xcb_connection_t *c, xcb_gcontext_t const& gcontext) {
                xcb_free_gc(c, gcontext);
            });
        }

        static gcontext
        copy_checked(xcb_connection_t *c, xcb_gcontext_t src_gc, uint32_t value_mask) {
            return gcontext(
                c,
                [&](xcb_connection_t *c, xcb_gcontext_t const& gcontext) {
                xpp::generic::check(c, xcb_copy_gc_checked(c, src_gc, gcontext, value_mask));
            },
                [&](xcb_connection_t *c, xcb_gcontext_t const& gcontext) {
                xpp::generic::check(c, xcb_free_gc_checked(c, gcontext));
            });
        }
    };

    namespace generic {

        template<>
        struct traits<xpp::gcontext> {
            typedef xcb_gcontext_t type;
        };

    }  // namespace generic

}  // namespace xpp
