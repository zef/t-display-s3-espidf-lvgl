#include <functional>

void setup_display();
uint32_t fire_lvgl_timer(uint32_t task_delay_ms);

bool lvgl_lock(int timeout_ms = -1);
void lvgl_unlock();

// provides block-based approach for lvgl lock/unlock
void update_ui(const std::function<void()>& block);
