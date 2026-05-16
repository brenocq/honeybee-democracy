//--------------------------------------------------
// Honeybee Democracy
// main.cpp
// Date: 2026-05-16
// Author: Breno Cunha Queiroz (brenocq.com)
//--------------------------------------------------
#include <ui/os_window.hpp>
#include <ui/ui.hpp>

int main() {
    ui::OSWindow window("Honeybee Democracy", 1200, 800);
    ui::UI ui;

    if (!window.create())
        return -1;
    ui.startup();

    while (!window.should_close()) {
        window.begin_frame();
        ui.update();
        window.end_frame();
    }

    ui.shutdown();
    window.destroy();

    return 0;
}
