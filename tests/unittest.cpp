#include <xcb/xcb.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

import std;
import xpp;
import xpp.proto.randr;

class XvfbServer {
  public:
    XvfbServer() {
        this->start();
    }

    ~XvfbServer() {
        if (!display.empty()) {
            std::cout << std::format("Stopping Xvfb on display {}\n", display);
            std::system("pkill Xvfb");
        }
    }

    [[nodiscard]]
    std::string getDisplay() const noexcept {
        return display;
    }

  private:
    std::string display{":99"};

    void start() {
        std::array<char, 16> buffer{};
        if (std::system(std::format("Xvfb {} -screen 0 1024x768x24 &", display).c_str()) != 0) {
            std::cerr << "Failed to start Xvfb\n";
            std::exit(EXIT_FAILURE);
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
#if 0
        auto pipe = popen("Xvfb -displayfd 1 -screen 0 1024x768x24 2>/dev/null", "r");
        if (!pipe) {
            throw std::runtime_error("Failed to start Xvfb");
        }

        if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            display = std::format(":{}", std::string(buffer.data()).substr(0, std::string(buffer.data()).find('\n')));
            setenv("DISPLAY", display.c_str(), /*overwrite=*/1);
            std::cout << std::format("Xvfb started on display {}\n", display);
        } else {
            throw std::runtime_error("Failed to read display number from Xvfb");
        }

        pclose(pipe);
#endif
    }
};

class Environment : public testing::Environment {
  public:
    ~Environment() override = default;

    // Override this to define how to set up the environment.
    void SetUp() override {

        // Set DISPLAY environment variable so tests can connect to Xvfb
        setenv("DISPLAY", server.getDisplay().c_str(), 1);
    }

    // Override this to define how to tear down the environment.
    void TearDown() override {
    }

    XvfbServer server{};
};

class XPPTest : public testing::Test {
  protected:
    static void SetUpTestSuite() {
        std::cout << "before all" << std::endl;
    }

    static void TearDownTestSuite() {
        std::cout << "after all" << std::endl;
    }

    void SetUp() override {
        std::cout << "before" << std::endl;
    }

    void TearDown() override {
        std::cout << "after" << std::endl;
    }
};

TEST_F(XPPTest, CanConnect) {
    auto conn = xpp::connection{};
    xcb_connection_t *c = *conn;

    ASSERT_EQ(conn.screen_of_display(0)->width_in_pixels, 1024);

    ASSERT_NE(c, nullptr);

    auto conn_screen = conn.default_screen();

    ASSERT_GE(conn_screen, 0);

    auto root = conn.root();

    auto atom = conn.intern_atom(true, "MY_ATOM_NAME");
    xpp::font::open_checked(c, "cursor");
}

TEST_F(XPPTest, GenerateId) {
    auto xpp = xpp::connection();

    auto id = xpp.generate_id();

    ASSERT_GT(id, 0);
}

TEST_F(XPPTest, CheckConnection) {
    auto xpp = xpp::connection();
    ASSERT_NO_THROW(xpp.check_connection());
}

TEST_F(XPPTest, Flush) {
    auto xpp = xpp::connection();
    ASSERT_NO_THROW(xpp.flush());
}

TEST_F(XPPTest, Screen) {
    auto xpp = xpp::connection();
    auto conn_screen = xpp.default_screen();
    auto root_screen = xpp.screen_of_display(conn_screen);
    auto window = xpp.root();
    ASSERT_GE(*window, 0);
}

TEST_F(XPPTest, KeyMap) {
    auto xpp = xpp::connection();
    auto keymap = xpp.query_keymap();

    auto keys = keymap->keys;

    ASSERT_NE(keys, nullptr);
}

TEST_F(XPPTest, ExtensionPresent) {
    auto conn = xpp::connection{};

    auto randr_ext = conn.extension<xpp::randr::interface>();
    randr_ext.query_version(1, 1);

    ASSERT_TRUE(randr_ext->present);
}

int main(int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);

    // Register the Xephyr environment
    Environment *env = new Environment{};
    testing::AddGlobalTestEnvironment(env);

    return RUN_ALL_TESTS();
}
