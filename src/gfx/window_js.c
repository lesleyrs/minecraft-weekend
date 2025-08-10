#ifdef __wasm
#include "window.h"
#include "../state.h"

#include <stdio.h>
#include <stdlib.h>

#include <js/glue.h>
#include <js/gl3.h>

// global window
struct Window window;

void __unordtf2() {}

static bool g_locked = false;

static void _pointerlock_callback(bool locked) {
    g_locked = locked;
}
static bool _cursor_callback(void *userdata, int x, int y) {
    if (!g_locked) return 0;

    window.mouse.delta = (vec2s){x, y};
    return 0;
}

static bool _key_callback(void *userdata, bool pressed, int key, int code, int modifiers) {
    if (key < 0) return 0;
    // glfw mod values
    window.keyboard.keys[340].down = modifiers & KMOD_SHIFT;
    window.keyboard.keys[341].down = modifiers & KMOD_CTRL;
    window.keyboard.keys[342].down = modifiers & KMOD_ALT;
    window.keyboard.keys[343].down = modifiers & KMOD_META;

    window.keyboard.keys[key].down = pressed;
    return 0;
}

static bool _mouse_callback(void *userdata, bool pressed, int button) {
    if (button < 0) return 0;

    JS_requestPointerLock();
    window.mouse.buttons[button].down = pressed;
    return 0;
}

void window_create(FWindow init, FWindow destroy, FWindow tick,  FWindow update, FWindow render) {
    window.init = init;
    window.destroy = destroy;
    window.tick = tick;
    window.update = update;
    window.render = render;

    window.last_frame = NOW();
    window.last_second = NOW();

    window.size = (ivec2s) {{1280, 720}};
    JS_createCanvas(window.size.x, window.size.y, "webgl2");
    JS_setTitle("Project");
    JS_addMouseEventListener(NULL, _mouse_callback, _cursor_callback, NULL);
    JS_addKeyEventListener(NULL, _key_callback);
    JS_addPointerLockChangeEventListener(_pointerlock_callback);
}

static void button_array_tick(size_t n, struct Button *buttons) {
    for (size_t i = 0; i < n; i++) {
        buttons[i].pressed_tick = buttons[i].down && !buttons[i].last_tick;
        buttons[i].last_tick = buttons[i].down;
    }
}

static void button_array_update(size_t n, struct Button *buttons) {
    for (size_t i = 0; i < n; i++) {
        buttons[i].pressed = buttons[i].down && !buttons[i].last;
        buttons[i].last = buttons[i].down;
    }
}

static void _init(void) {
    window.init();
}

static void _destroy(void) {
    window.destroy();
}

static void _tick(void) {
    window.ticks++;
    button_array_tick(INT8_MAX, window.mouse.buttons);
    button_array_tick(INT16_MAX, window.keyboard.keys);
    window.tick();
}

static void _update(void) {
    button_array_update(INT8_MAX, window.mouse.buttons);
    button_array_update(INT16_MAX, window.keyboard.keys);
    window.update();

    // reset update delta
    window.mouse.delta = GLMS_VEC2_ZERO;
}

static void _render(void) {
    window.frames++;
    window.render();
}

void window_loop(void) {
    _init();

    while(1) {
        const u64 now = NOW();

        window.frame_delta = now - window.last_frame;
        window.last_frame = now;

        if (now - window.last_second > NS_PER_SECOND) {
            window.fps = window.frames;
            window.tps = window.ticks;
            window.frames = 0;
            window.ticks = 0;
            window.last_second = now;

            printf("FPS: %llu | TPS: %llu\n", window.fps, window.tps);
        }

        // tick processing
        const u64 NS_PER_TICK = (NS_PER_SECOND / 60);
        u64 tick_time = window.frame_delta + window.tick_remainder;
        // while (tick_time > NS_PER_TICK) {
            _tick();

            tick_time -= NS_PER_TICK;

            // time warp
            if (state.window->keyboard.keys['['].down) {
                state.world.ticks += 30;
            }

            if (state.window->keyboard.keys[']'].pressed_tick) {
                state.world.ticks += (TOTAL_DAY_TICKS) / 3;
            }
        // }
        window.tick_remainder = max(tick_time, 0);
    
        _update();
        _render();
        JS_requestAnimationFrame();
    }

    _destroy();
    exit(0);
}

void mouse_set_grabbed(bool grabbed) {
    // pointerlock goes active on click after canvas creation
}

bool mouse_get_grabbed(void) {
    return g_locked;
}
#endif
