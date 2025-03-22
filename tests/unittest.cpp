#include <xcb/xcb.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

import std;
import xpp;
import xpp.proto.randr;

TEST(XPP, CanConnect) {
    auto conn = new xpp::connection();
    xcb_connection_t *c = *conn;

    ASSERT_NE(c, nullptr);
    
    auto conn_screen = conn->default_screen();

    ASSERT_GE(conn_screen, 0);
    
    auto root = conn->root<xpp::x::window>();
    
    auto atom = conn->intern_atom(true, 12, "MY_ATOM_NAME");
    xpp::font::open_checked(c, "cursor");
}

#if 0
TEST(XPP, ExtensionPresent) {
    using x_connection = xpp::connection<xpp::randr::extension>;
    auto conn = new x_connection();

    auto randr_ext = conn->extension<xpp::randr::extension>();

    ASSERT_TRUE(randr_ext->present);
}
#endif