# TTGO T-Display S3 + ESP-IDF + LVGL

This is a basic template that provides a starting point for using the excellent and affordable
[TTGO T-Display-S3](https://www.lilygo.cc/products/t-display-s3?variant=42351558590645) module from LILYGO.

#### The project provides a template that implements:
- a working screen
- functions for both buttons on the module

Though it sounds simple, getting these things working was quite an effort for me.
I hope this proves helpful to others so they don't have to go through the same stuggle.

I'd love to hear from you if it helped you out, or if you have any feedback.

#### The project uses:

- [PlatformIO](https://platformio.org)
    - I am using it inside VSCode, though it shouldn't matter.
- the [`ESP-IDF`](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/index.html) framework, not `Arduino`.
- the [LVGL](https://lvgl.io) graphics library.
- the [espressif/button](https://components.espressif.com/components/espressif/button) component.

## Architecture

I also wanted this to be a clean project template where things are properly separated and components can be included
as needed without having to dissect everything. Therefore, functionality is split between files in the `src` directory.

- `main.c` contains minimal setup code and basic sample application logic, including implementation of button press behavior.
- `display.c` contains the user's lvgl code, and `display.h` defines the public interface by which you can use it.
- `display_setup.h` contains the code that configures the lcd panel and sets up lvgl to use it.
- `buttons.c` initializes the button functionality, and `buttons.h` defines the functions that `main.c` should call and implement to define button behavior.
- `pin_config.h` defines board-specific pins and some lvgl preferences.

## Motivation

I couldn't find sample code that used the configurations I was going for, and with some version compatibility
problems and lack of documentation, it was a challenge to get this working.

I wanted to use `ESP-IDF` rather than `Arduino` because of some specific reasons for a project I'm working on.
I had some experience using the module with [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) and the Arduino framework,
but things are different Espressif framework.

There are also a lot of breaking changes to LVGL between `v8.x` and `v9.x`, but I wanted to use the latest version.
I found it hard to find code that works with `v9.1` and a general lack of specific documentation around how to get the driver working.

My understanding is that you can also use the `Arduino` framework alongside `ESP-IDF` as a component.
I tried this in order to try to use `TFT_eSPI` either standalone or as a display driver for `LVGL`
as described [in this guide on using TFT_eSPI with ESP-IDF](https://github.com/Bodmer/TFT_eSPI/blob/master/docs/ESP-IDF/Using%20ESP-IDF.txt).
I tried this and had difficulty getting the Arduino component working.

Improvements and corrections are welcome! I'm an experienced software developer, but I'm very new to c/c++.

Please send a pull request, or open [new GitHub Issue](../../issues/new) and teach me something!

### LVGL:

`display_setup.h` has the display configuration. Most of the code is getting the esp display configured.
A smaller portion is dedicated to using LVGL. I had some trouble understanding how to properly configure the buffer and flush callbacks.

A configuration file is in `include/lv_conf.h`, though I didn't need to change anything except to enable it.

[This issue](https://github.com/Xinyuan-LilyGO/T-Display-S3/issues/103) helped me get the display working, and I found some other
projects using LVGL that helped me figure out how to configure that part.

#### Question:

Why can't I call `lv_timer_handler()` through a task or abstracted function without it crashing?
I'm also aware that trying to call this too frequently causes crashing, and I've seen recommendations to call it
every 2-10ms and using the lower end of the range crashes for me.

### Buttons:

In searching I found some people have had trouble getting both buttons working with esp-idf and it wasn't immediately clear to me how to do it.

I found the `espressif/button` component and wanted to try that out.
It seemed to me that there should be something I could put into `platform.ini` to get the component. But way I figured out how
to get it working is to create `src/idf_component.yml` and include the dependency there, which is used for Espressif dependency management.
This seems not ideal because now we have dependencies defined in both places and it's kind of confusing.
Please let me know if you know a better way.

In order to trigger the build system into pulling down the dependency into the `managed_components` folder, I found that I needed to
use the `Full Clean` feature found in the PlatformIO sidebar under "PROJECT TASKS".

## Learnings:

I also learned about using `monitor_filters = esp32_exception_decoder` in `platformio.ini`, which was very helpful for debugging crashes.
I experienced a lot of crash backtraces trying to get this working and I didn't know how to debug those effectively.

