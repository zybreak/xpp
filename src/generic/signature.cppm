module;
#include <xcb/xcb.h>
export module xpp.generic.signature;

import std;

export namespace xpp {
    namespace generic {
        template <typename Signature, Signature& S>
        class signature;
    }
}  // namespace xpp
