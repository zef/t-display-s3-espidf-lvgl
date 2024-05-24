#include "display.h"
#include "display_setup.h"

#include "lvgl.h"

lv_obj_t *statusLabel;
lv_obj_t *subLabel;
lv_obj_t *progressBar;
static lv_style_t largeFontStyle;

void set_progress(int32_t value) {
    value = LV_CLAMP(0, value, 100);
    
    lv_bar_set_value(progressBar, value, LV_ANIM_OFF);
    
    char labelText[16];
    snprintf(labelText, sizeof(labelText), "%d/100", value);
    lv_label_set_text(subLabel, labelText);
}

void create_progress_bar() {
    progressBar = lv_bar_create(display);
    lv_obj_set_size(progressBar, 320 - 40, 22);
    lv_obj_center(progressBar);
}

void create_labels() {
    lv_style_init(&largeFontStyle);
    lv_style_set_text_font(&largeFontStyle, &lv_font_montserrat_18);
    
    lv_obj_set_style_text_color(display, lv_color_white(), LV_PART_MAIN);
    // lv_obj_set_style_text_font(lv_screen_active(), &lv_font_montserrat_18, LV_PART_MAIN);

    statusLabel = lv_label_create(display);
    lv_obj_add_style(statusLabel, &largeFontStyle, LV_PART_MAIN);
    lv_obj_align(statusLabel, LV_ALIGN_CENTER, 0, -40);
    lv_label_set_text(statusLabel, "Flashing Firmware...");

    subLabel = lv_label_create(display);
    lv_obj_align(subLabel, LV_ALIGN_CENTER, 0, 30);
}

void start_display() {
    setup_display();
    lv_obj_set_style_bg_color(display, lv_color_hex(0x375686), LV_PART_MAIN);
    create_labels();
    create_progress_bar();
}
