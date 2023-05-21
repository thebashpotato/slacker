#include <cstdlib>
#include <iostream>
#include <slacker.hpp>

class Application {
private:
    slacker::x::X11Display display_{};
    slacker::x::X11Window root_{};

public:
    Application() : root_(display_.sharedDisplay()) {}


    [[nodiscard]] auto run() -> bool {
        if (!display_.isOpen()) {
            return false;
        }

        auto window = slacker::x::X11Window(this->display_.sharedDisplay());

        // Create the simple window
        if (!window.createWindow(display_.root(), display_.screenId(), slacker::pure::Rect())) {
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
        while (XNextEvent(display_.rawDisplay(), &event) == 0) {
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
