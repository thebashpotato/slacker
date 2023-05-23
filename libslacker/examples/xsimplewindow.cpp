#include "slacker/x/display.hpp"
#include "slacker/x/window.hpp"
#include <cstdlib>
#include <iostream>
#include <slacker.hpp>

class Application {
private:
    slacker::x::SharedX11DisplayPtr display_{};
    slacker::x::X11Window window_{};

public:
    Application() : display_(nullptr), window_{nullptr} {
        auto result = slacker::x::X11Display::open();
        if (result.isOk()) {
            display_ = std::move(result.ok().value());
            window_ = slacker::x::X11Window(display_);
        }
    }


    [[nodiscard]] auto run() -> bool {
        // Create the simple window
        if (!display_->isOpen()) {
            std::cerr << "Could not open the X display" << '\n';
            return false;
        }

        if (!window_.createWindow(slacker::pure::Rect())) {
            std::cerr << "Could not create the simple window" << '\n';
            return false;
        } else {
            std::cout << "Window created!!!" << '\n';
        }

        // Map the window to the shared_display server
        if (!window_.map()) {
            std::cerr << "Could not map the simple window" << '\n';
            return false;
        }

        XEvent event;
        while (XNextEvent(display_->raw(), &event) == 0) {
            switch (event.type) {
                case ButtonPress:
                    return true;
            }
        }
        return true;
    }
};


auto main() -> int {
    std::cout << "Lib Slacker Version: " << slacker::utils::getVersion() << '\n';
    auto app = Application();

    if (!app.run()) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
