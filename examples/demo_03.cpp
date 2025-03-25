#include <xcb/xcb.h>
#include <xcb/randr.h>

import std;
import xpp;
import xpp.proto.randr;
import xpp.proto.damage;
import xpp.proto.render;
import xpp.proto.x;

int main() {
    auto xpp = xpp::connection();
    
    auto id = xpp.generate_id();
    
    xpp.check_connection();
    
    xpp.flush();

    auto conn_screen = xpp.default_screen();
    auto root_screen = xpp.screen_of_display(conn_screen);
    auto window = xpp.root();

    auto keymap = xpp.query_keymap();
    
    auto keys = keymap->keys;
    
    xpp.flush();

    auto i = xpp.get_input_focus();
    auto focused = i.focus<xpp::x::window>();
    std::cout << focused << std::endl;

    //xpp::x::change_property(xpp, 0, focused, 0, 0, 0, "hej");

    for (auto child : focused.query_tree().parent<xpp::x::window>().query_tree().children()) {
        std::cout << child << std::endl;
    }

    auto q_v = xpp.extension<xpp::randr::interface>().query_version(1, 1);

    std::cout << q_v->major_version << q_v->minor_version << std::endl;

    xpp.disconnect();
    
}