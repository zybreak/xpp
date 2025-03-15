module;
#include <xcb/xcb.h>
export module xpp.atom;

import std;
import xpp.generic.resource;
import xpp.generic.iterator_traits;
import xpp.proto.x;

export namespace xpp {

    class atom : public xpp::x::atom {
      protected:
        using base = xpp::x::atom;

      public:
        using base::base;
        using base::operator=;
    };

    namespace generic {

        template<>
        struct traits<xpp::atom> {
            typedef xcb_atom_t type;
        };

    }  // namespace generic

}  // namespace xpp
