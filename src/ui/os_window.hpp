//--------------------------------------------------
// Honeybee Democracy
// os_window.hpp
// Date: 2026-05-16
// Author: Breno Cunha Queiroz (brenocq.com)
//--------------------------------------------------
#pragma once

#include <string>

struct GLFWwindow;

namespace ui {

class OSWindow {
  public:
    OSWindow(const std::string& name, size_t width, size_t height);

    bool create();
    void destroy();

    bool should_close();

    void begin_frame();
    void end_frame();

  private:
    std::string _name;
    size_t _width;
    size_t _height;

    GLFWwindow* _window{nullptr};
};

} // namespace ui
