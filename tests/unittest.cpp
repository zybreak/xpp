#include <xcb/xcb.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

import std;
import xpp;
import xpp.proto.randr;

TEST(XPP, CanConnect) {
    auto conn = xpp::connection{};
    xcb_connection_t *c = *conn;

    ASSERT_NE(c, nullptr);
    
    auto conn_screen = conn.default_screen();

    ASSERT_GE(conn_screen, 0);
    
    auto root = conn.root();
    
    auto atom = conn.intern_atom(true, "MY_ATOM_NAME");
    xpp::font::open_checked(c, "cursor");
}

TEST(XPP, GenerateId) {
    auto xpp = xpp::connection();
    
    auto id = xpp.generate_id();

    ASSERT_GT(id, 0);
}
    
TEST(XPP, CheckConnection) {
    auto xpp = xpp::connection();
    ASSERT_NO_THROW(xpp.check_connection());
}

TEST(XPP, Flush) {
    auto xpp = xpp::connection();
    ASSERT_NO_THROW(xpp.flush());
}

TEST(XPP, Screen) {
    auto xpp = xpp::connection();
    auto conn_screen = xpp.default_screen();
    auto root_screen = xpp.screen_of_display(conn_screen);
    auto window = xpp.root();
    ASSERT_GE(*window, 0);
}

TEST(XPP, KeyMap) {
    auto xpp = xpp::connection();
    auto keymap = xpp.query_keymap();
    
    auto keys = keymap->keys;

    ASSERT_NE(keys, nullptr);
}
    
TEST(XPP, ExtensionPresent) {
    auto conn = xpp::connection{};

    auto randr_ext = conn.extension<xpp::randr::interface>();
    randr_ext.query_version(1, 1);

    ASSERT_TRUE(randr_ext->present);
}