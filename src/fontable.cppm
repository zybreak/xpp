module;
#include <xcb/xcb.h>
export module xpp.fontable;

import std;
import xpp.generic.resource;
import xpp.generic.iterator_traits;
import xpp.proto.x;

export namespace xpp {

    class fontable : public xpp::x::fontable {
      protected:
        using base = xpp::x::fontable;

      public:
        using base::base;
        using base::operator=;
    };

    namespace generic {

        template<>
        struct traits<xpp::fontable> {
            typedef xcb_fontable_t type;
        };

    }  // namespace generic

}  // namespace xpp
