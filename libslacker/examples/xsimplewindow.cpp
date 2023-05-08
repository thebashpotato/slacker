#include <cstdlib>
#include <iostream>
#include <slacker.hpp>

class Application {
public:
  Application() = default;

  [[nodiscard]] auto setup() noexcept -> bool {
    this->m_dpy_conn = std::make_shared<slacker::SlackerDisplay>();
    if (this->m_dpy_conn->init_failure()) {
      return false;
    }

    this->m_screen = DefaultScreen(this->m_dpy_conn->get_raw_display());
    this->m_root = slacker::SlackerWindow(this->m_dpy_conn, this->m_screen);
    return true;
  }

  [[nodiscard]] auto run() -> bool {
    auto window = slacker::SlackerWindow(this->m_dpy_conn);

    // Create the simple window
    if (!window.create_window(this->m_root, slacker::Rect(), this->m_screen)) {
      std::cerr << "Could not create the simple window" << '\n';
      return false;
    } else {
      std::cout << "Window created!!!" << '\n';
    }

    // Map the window to the display server
    if (!window.map()) {
      std::cerr << "Could not map the simple window" << '\n';
      return false;
    }

    XEvent ev;
    while (XNextEvent(this->m_dpy_conn->get_raw_display(), &ev) == 0) {
      switch (ev.type) {
      case ButtonPress:
        return true;
      }
    }
    return true;
  }

private:
  int32_t m_screen{0};
  slacker::SlackerXConnPtr m_dpy_conn{nullptr};
  slacker::SlackerWindow m_root{};
};

void run(slacker::SlackerXConnPtr const &connection) {
  XEvent ev;
  while (XNextEvent(connection->get_raw_display(), &ev) == 0) {
    switch (ev.type) {
    case ButtonPress:
      return;
    }
  }
}

int main() {
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
