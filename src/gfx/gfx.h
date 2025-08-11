#ifndef GFX_H
#define GFX_H

#ifdef __wasm
#include <js/gl3.h>
#else
#include <gles2.h>
#endif

#ifdef __wasm
#include <js/glue.h>
#define GLFW_KEY_LEFT_SHIFT 340
#define GLFW_MOUSE_BUTTON_LEFT MBTN_LEFT
#define GLFW_MOUSE_BUTTON_RIGHT MBTN_RIGHT
#define GLFW_KEY_F 'f'
#define GLFW_KEY_W 'w'
#define GLFW_KEY_A 'a'
#define GLFW_KEY_S 's'
#define GLFW_KEY_D 'd'
#define GLFW_KEY_I 'i'
#define GLFW_KEY_J 'j'
#define GLFW_KEY_K 'k'
#define GLFW_KEY_L 'l'
#define GLFW_KEY_SPACE ' '
#else
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#endif
