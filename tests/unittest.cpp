#include <gtest/gtest.h>
#include <gmock/gmock.h>

import std;
import xpp;
import xpp.proto.randr;

TEST(XPP, CanConnect) {
    using x_connection = xpp::connection<xpp::randr::extension>;
    auto conn = new x_connection();

    auto conn_screen = conn->default_screen();

    ASSERT_GE(conn_screen, 0);
}

TEST(XPP, ExtensionPresent) {
    using x_connection = xpp::connection<xpp::randr::extension>;
    auto conn = new x_connection();

    auto randr_ext = conn->extension<xpp::randr::extension>();

    ASSERT_TRUE(randr_ext->present);
}