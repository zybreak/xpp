#include <xcb/xcb.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

import std;
import xpp;
import xpp.proto.randr;

std::atomic<bool> xvfb_ready = false;  // Flag to indicate Xvfb is ready

void handle_signal(int signum, int sa_flags, void (*handler)(int)) {
    // Set up signal handler
    struct sigaction sa{};
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = sa_flags;

    if (sigaction(signum, &sa, nullptr) == -1) {
        std::cerr << "Failed to set signal handler: " << strerror(errno) << '\n';
        std::terminate();
    }
}

class XvfbServer {
  public:
    XvfbServer() {
        this->start();
    }

    ~XvfbServer() {
        if (child_pid > 0) {
            std::cout << "Stopping Xvfb on display " << display << std::endl;
            if (kill(child_pid, SIGTERM) == 0) {
                std::cout << "Process " << child_pid << " stopped gracefully." << std::endl;
            } else {
                if (kill(child_pid, SIGKILL) == 0) {
                    std::cout << "Process " << child_pid << " killed forcefully." << std::endl;
                } else {
                    perror("Failed to stop process");
                    std::terminate();
                }
            }
        }
    }

    [[nodiscard]]
    std::string getDisplay() const noexcept {
        return display;
    }

  private:
    std::string display;
    pid_t child_pid{-1};

    void start() {
        int pipefd[2];

        handle_signal(SIGUSR1, 0, SIG_IGN);

        if (pipe(pipefd) == -1) {
            perror("pipe failed");
            std::terminate();
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork failed");
            std::terminate();
        }

        if (pid == 0) {
            close(pipefd[0]);  // Close read end

            auto displayFd = std::format("{}", pipefd[1]);
            execlp("Xvfb", "Xvfb", "-displayfd", displayFd.c_str(), "-screen", "0", "1024x768x24", "-nolisten", "tcp", nullptr);

            perror("Failed to start Xvfb");
            std::terminate();
        }

        handle_signal(SIGUSR1, 0, [](int signum) {
            xvfb_ready = true;
            handle_signal(SIGUSR1, 0, SIG_IGN);
        });

        std::cout << "Waiting for Xvfb (ignore any SocketCreateListener failed errors below)" << std::endl;
        child_pid = pid;
        close(pipefd[1]);  // Close write end
        
        std::array<char, 16> buffer{};
        ssize_t n = read(pipefd[0], buffer.data() + 1, buffer.size() - 2);
        close(pipefd[0]);

        if (n <= 0) {
            std::cerr << "Failed to read display number from Xvfb\n";
            std::terminate();
        }

        buffer[0] = ':';

        display = std::string(buffer.data(), n);
        std::cout << std::format("Xvfb started on display {}", display) << std::endl;
    }
};

class Environment : public ::testing::Environment {
  public:

    Environment() : server{std::make_unique<XvfbServer>()} {
        // Set DISPLAY environment variable so tests can connect to Xvfb
        setenv("DISPLAY", server->getDisplay().c_str(), 1);
    }

  protected:
    std::unique_ptr<XvfbServer> server{nullptr};
};

class XPPTest : public ::testing::Test {
};

auto env = new Environment{};

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

TEST_F(XPPTest, CreateWindow) {
    auto conn = xpp::connection{};

    uint32_t wid = conn.generate_id();
    ASSERT_NO_THROW({
        conn.create_window_checked(XCB_COPY_FROM_PARENT, wid, conn.root(), 0, 0, 10, 10, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT, conn.screen_of_display(conn.default_screen())->root_visual, 0, nullptr);
    });

    conn.map_window(wid);

    conn.flush();

    auto query = conn.query_tree(conn.root());
    auto children = query.children();

    EXPECT_EQ(std::distance(children.begin(), children.end()), query->children_len);
    EXPECT_EQ(query->children_len, 1);

    auto first = *children.begin();

    EXPECT_EQ(first, wid);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);

    ::testing::AddGlobalTestEnvironment(env);

    handle_signal(SIGSEGV, SA_NODEFER | SA_RESETHAND, [](int) {
        env->TearDown();
    });

    return RUN_ALL_TESTS();
}
