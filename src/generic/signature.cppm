module;
#include <xcb/xcb.h>
export module xpp.generic.signature;

import std;

export namespace xpp::generic {
    template<typename Signature, Signature& S>
    class signature;
}
