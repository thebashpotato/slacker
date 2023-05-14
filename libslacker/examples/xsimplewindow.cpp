#include <cstdlib>
#include <iostream>
#include <slacker.hpp>

class Application {
public:
    Application() = default;

    [[nodiscard]] auto setup() noexcept -> bool {
        display_ = slacker::X11Display::open();
        if (!display_->isOpen()) {
            return false;
        }

        screen_ = DefaultScreen(display_->raw_display());
        this->root_ = slacker::X11Window(this->display_->shared_display(), this->screen_);
        return true;
    }

    [[nodiscard]] auto run() -> bool {
        auto window = slacker::X11Window(this->display_->shared_display());

        // Create the simple window
        if (!window.create_window(this->root_, slacker::Rect(), this->screen_)) {
            std::cerr << "Could not create the simple window" << '\n';
            return false;
        } else {
            std::cout << "Window created!!!" << '\n';
        }

        // Map the window to the shared_display server
        if (!window.map()) {
            std::cerr << "Could not map the simple window" << '\n';
            return false;
        }

        XEvent event;
        while (XNextEvent(this->display_->raw_display(), &event) == 0) {
            switch (event.type) {
                case ButtonPress:
                    return true;
            }
        }
        return true;
    }

private:
    int32_t screen_{0};
    std::unique_ptr<slacker::X11Display> display_{};
    slacker::X11Window root_{};
};


auto main() -> int {
    std::cout << "Lib Slacker Version: " << slacker::get_version() << '\n';
    auto app = Application();

    if (!app.setup()) {
        return EXIT_FAILURE;
    }

    if (!app.run()) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
