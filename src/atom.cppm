module;
#include <xcb/xcb.h>
export module xpp.atom;

import std;
import xpp.generic.resource;
import xpp.generic.iterator_traits;
import xpp.proto.x;

export namespace xpp {
    
    template<typename Connection, template<typename, typename> class ... Interfaces>
    class atom
        : public xpp::generic::resource<Connection, xcb_atom_t,
                                        xpp::x::atom, Interfaces ...>
    {
      protected:
        using base = xpp::generic::resource<Connection, xcb_atom_t, Interfaces ...>;
        
      public:
        using base::base;
        using base::operator=;
    };

    namespace generic {

        template<typename Connection, template<typename, typename> class ... Interfaces>
        struct traits<xpp::atom<Connection, Interfaces ...>>
        {
            typedef xcb_atom_t type;
        };

    } // namespace generic

} // namespace xpp
