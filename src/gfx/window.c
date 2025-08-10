#ifndef __wasm
#include "window.h"
#include "../state.h"

#include <stdio.h>
#include <stdlib.h>

// global window
struct Window window;

GLFWwindow *handle;

static void _size_callback(GLFWwindow *handle, int width, int height) {
    glViewport(0, 0, width, height);
    window.size = (ivec2s) {{width, height}};
}

static void _cursor_callback(GLFWwindow *handle, double xp, double yp) {
    vec2s p = {{xp, yp}};

    window.mouse.delta = glms_vec2_sub(p, window.mouse.position);
    window.mouse.delta.x = clamp(window.mouse.delta.x, -100.0f, 100.0f);
    window.mouse.delta.y = clamp(window.mouse.delta.y, -100.0f, 100.0f);

    window.mouse.position = p;
}

static void _key_callback(GLFWwindow *handle, int key, int scancode, int action, int mods) {
    if (key < 0) {
        return;
    }

    switch (action) {
        case GLFW_PRESS:
            window.keyboard.keys[key].down = true;
            break;
        case GLFW_RELEASE:
            window.keyboard.keys[key].down = false;
            break;
        default:
            break;
    }
}

static void _mouse_callback(GLFWwindow *handle, int button, int action, int mods) {
    if (button < 0) {
        return;
    }

    switch (action) {
        case GLFW_PRESS:
            window.mouse.buttons[button].down = true;
            break;
        case GLFW_RELEASE:
            window.mouse.buttons[button].down = false;
            break;
        default:
            break;
    }
}

static void _error_callback(int code, const char *description) {
    fprintf(stderr, "GLFW error %d: %s\n", code, description);
}

void window_create(FWindow init, FWindow destroy, FWindow tick,  FWindow update, FWindow render) {
    window.init = init;
    window.destroy = destroy;
    window.tick = tick;
    window.update = update;
    window.render = render;

    window.last_frame = NOW();
    window.last_second = NOW();

    glfwSetErrorCallback(_error_callback);

    if (!glfwInit()){
        fprintf(stderr, "%s",  "error initializing GLFW\n");
        exit(1);
    }

    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);

    window.size = (ivec2s) {{1280, 720}};
    handle = glfwCreateWindow(window.size.x, window.size.y, "Project", NULL, NULL);
    if (handle == NULL) {
        fprintf(stderr, "%s",  "error creating window\n");
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(handle);

    // configure callbacks
    glfwSetFramebufferSizeCallback(handle, _size_callback);
    glfwSetCursorPosCallback(handle, _cursor_callback);
    glfwSetKeyCallback(handle, _key_callback);
    glfwSetMouseButtonCallback(handle, _mouse_callback);

    if (!gladLoadGLES2(glfwGetProcAddress)) {
        fprintf(stderr, "%s",  "error initializing GLAD\n");
        glfwTerminate();
        exit(1);
    }

    glfwSwapInterval(1);
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
    glfwTerminate();
}

static void _tick(void) {
    window.ticks++;
    button_array_tick(GLFW_MOUSE_BUTTON_LAST, window.mouse.buttons);
    button_array_tick(GLFW_KEY_LAST, window.keyboard.keys);
    window.tick();
}

static void _update(void) {
    button_array_update(GLFW_MOUSE_BUTTON_LAST, window.mouse.buttons);
    button_array_update(GLFW_KEY_LAST, window.keyboard.keys);
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

    while (!glfwWindowShouldClose(handle)) {
        const u64 now = NOW();

        window.frame_delta = now - window.last_frame;
        window.last_frame = now;

        if (now - window.last_second > NS_PER_SECOND) {
            window.fps = window.frames;
            window.tps = window.ticks;
            window.frames = 0;
            window.ticks = 0;
            window.last_second = now;

            printf("FPS: %lu | TPS: %lu\n", window.fps, window.tps);
        }

        // tick processing
        const u64 NS_PER_TICK = (NS_PER_SECOND / 60);
        u64 tick_time = window.frame_delta + window.tick_remainder;
        while (tick_time > NS_PER_TICK) {
            _tick();
            tick_time -= NS_PER_TICK;

            // time warp
            if (state.window->keyboard.keys['['].down) {
                state.world.ticks += 30;
            }

            if (state.window->keyboard.keys[']'].pressed_tick) {
                state.world.ticks += (TOTAL_DAY_TICKS) / 3;
            }
        }
        window.tick_remainder = max(tick_time, 0);
    
        _update();

        // wireframe toggle (T)
        // no longer working after desktop port to gles3
        // if (state.window->keyboard.keys[GLFW_KEY_T].pressed) {
        //     state.renderer.flags.wireframe = !state.renderer.flags.wireframe;
        // }

        // mouse toggle (ESC)
        if (state.window->keyboard.keys[GLFW_KEY_ESCAPE].pressed) {
            mouse_set_grabbed(!mouse_get_grabbed());
        }

        _render();
        glfwSwapBuffers(handle);
        glfwPollEvents();
    }

    _destroy();
    exit(0);
}

void mouse_set_grabbed(bool grabbed) {
    glfwSetInputMode(handle, GLFW_CURSOR, grabbed ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

bool mouse_get_grabbed(void) {
    return glfwGetInputMode(handle, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
}
#endif
