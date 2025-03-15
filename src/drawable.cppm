module;
#include <xcb/xcb.h>
export module xpp.drawable;

import std;
import xpp.generic.resource;
import xpp.generic.iterator_traits;
import xpp.proto.x;

export namespace xpp {

    class drawable : xpp::x::drawable {
      protected:
        using base = xpp::x::drawable;

      public:
        using base::base;
        using base::operator=;
    };

    namespace generic {

        template<>
        struct traits<xpp::drawable> {
            typedef xcb_drawable_t type;
        };

    }  // namespace generic

}  // namespace xpp
