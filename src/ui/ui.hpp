//--------------------------------------------------
// Honeybee Democracy
// ui.hpp
// Date: 2026-05-16
// Author: Breno Cunha Queiroz (brenocq.com)
//--------------------------------------------------
#pragma once

namespace ui {

class UI {
  public:
    void startup();
    void shutdown();

    void update();

  private:
    void render_menu_bar();
    void setup_dockspace();

    bool _first_render = true;
    bool _show_imgui_demo = false;
    bool _show_implot_demo = false;
};

} // namespace ui
