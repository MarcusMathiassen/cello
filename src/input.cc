// Copyright (c) 2020 Marcus Mathiassen

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "common.h"

typedef enum {
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_APOSTROPHE,
    KEY_BACKSLASH,
    KEY_COMMA,
    KEY_EQUAL,
    KEY_GRAVE_ACCENT,
    KEY_LEFT_BRACKET,
    KEY_MINUS,
    KEY_PERIOD,
    KEY_RIGHT_BRACKET,
    KEY_SEMICOLON,
    KEY_SLASH,
    KEY_WORLD_1,
    KEY_BACKSPACE,
    KEY_CAPS_LOCK,
    KEY_DELETE,
    KEY_DOWN,
    KEY_END,
    KEY_ENTER,
    KEY_ESCAPE,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_F13,
    KEY_F14,
    KEY_F15,
    KEY_F16,
    KEY_F17,
    KEY_F18,
    KEY_F19,
    KEY_F20,
    KEY_HOME,
    KEY_INSERT,
    KEY_LEFT,
    KEY_LEFT_ALT,
    KEY_LEFT_CONTROL,
    KEY_LEFT_SHIFT,
    KEY_LEFT_SUPER,
    KEY_MENU,
    KEY_NUM_LOCK,
    KEY_PAGE_DOWN,
    KEY_PAGE_UP,
    KEY_RIGHT,
    KEY_RIGHT_ALT,
    KEY_RIGHT_CONTROL,
    KEY_RIGHT_SHIFT,
    KEY_RIGHT_SUPER,
    KEY_SPACE,
    KEY_TAB,
    KEY_UP,
    KEY_KP_0,
    KEY_KP_1,
    KEY_KP_2,
    KEY_KP_3,
    KEY_KP_4,
    KEY_KP_5,
    KEY_KP_6,
    KEY_KP_7,
    KEY_KP_8,
    KEY_KP_9,
    KEY_KP_ADD,
    KEY_KP_DECIMAL,
    KEY_KP_DIVIDE,
    KEY_KP_ENTER,
    KEY_KP_EQUAL,
    KEY_KP_MULTIPLY,
    KEY_KP_SUBTRACT,
    _KEY_KIND_COUNT_
} Key_Kind;

typedef enum {
    KEY_MOD_SHIFT,
    KEY_MOD_CONTROL,
    KEY_MOD_ALT,
    KEY_MOD_SUPER,
    KEY_MOD_CAPS_LOCK
} Key_Mod;

typedef enum {
    KEY_RELEASED,
    KEY_PRESSED,
    KEY_REPEAT,
} Key_State;

typedef enum {
    MOUSE_BUTTON_LAST,
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_MIDDLE,
} Mouse_Kind;

typedef enum {
    MOUSE_RELEASED,
    MOUSE_PRESSED,
    MOUSE_REPEAT,
} Mouse_State;

typedef enum {
    INPUT_KEY,
    INPUT_MOUSE,
    INPUT_SCROLL,
    INPUT_CURSOR,
} Input_Kind;

typedef struct {
    Key_Kind  kind;
    Key_State state;
    Key_Mod   mod;
} Key_Input;

typedef struct {
    Mouse_Kind kind;
    Mouse_State state;
} Mouse_Input;

typedef struct {
    f64 xoffset;
    f64 yoffset;
} Scroll_Input;

typedef struct {
    f64 xpos;
    f64 ypos;
} Cursor_Input;

typedef struct {
    Input_Kind kind;
    union {
        Key_Input key;
        Mouse_Input mouse;
        Scroll_Input scroll;
        Cursor_Input cursor;
    };
} Input;

#define MAX_INPUTS 32
typedef struct {
    s8 count;
    Input buffer[MAX_INPUTS];
    Key_State keys[_KEY_KIND_COUNT_];
} Input_Info;

internal const char* key_to_str(Key_Kind key)
{
    switch (key)
    {
    case KEY_0:             return "KEY_0";
    case KEY_1:             return "KEY_1";
    case KEY_2:             return "KEY_2";
    case KEY_3:             return "KEY_3";
    case KEY_4:             return "KEY_4";
    case KEY_5:             return "KEY_5";
    case KEY_6:             return "KEY_6";
    case KEY_7:             return "KEY_7";
    case KEY_8:             return "KEY_8";
    case KEY_9:             return "KEY_9";
    case KEY_A:             return "KEY_A";
    case KEY_B:             return "KEY_B";
    case KEY_C:             return "KEY_C";
    case KEY_D:             return "KEY_D";
    case KEY_E:             return "KEY_E";
    case KEY_F:             return "KEY_F";
    case KEY_G:             return "KEY_G";
    case KEY_H:             return "KEY_H";
    case KEY_I:             return "KEY_I";
    case KEY_J:             return "KEY_J";
    case KEY_K:             return "KEY_K";
    case KEY_L:             return "KEY_L";
    case KEY_M:             return "KEY_M";
    case KEY_N:             return "KEY_N";
    case KEY_O:             return "KEY_O";
    case KEY_P:             return "KEY_P";
    case KEY_Q:             return "KEY_Q";
    case KEY_R:             return "KEY_R";
    case KEY_S:             return "KEY_S";
    case KEY_T:             return "KEY_T";
    case KEY_U:             return "KEY_U";
    case KEY_V:             return "KEY_V";
    case KEY_W:             return "KEY_W";
    case KEY_X:             return "KEY_X";
    case KEY_Y:             return "KEY_Y";
    case KEY_Z:             return "KEY_Z";
    case KEY_APOSTROPHE:    return "KEY_APOSTROPHE";
    case KEY_BACKSLASH:     return "KEY_BACKSLASH";
    case KEY_COMMA:         return "KEY_COMMA";
    case KEY_EQUAL:         return "KEY_EQUAL";
    case KEY_GRAVE_ACCENT:  return "KEY_GRAVE_ACCENT";
    case KEY_LEFT_BRACKET:  return "KEY_LEFT_BRACKET";
    case KEY_MINUS:         return "KEY_MINUS";
    case KEY_PERIOD:        return "KEY_PERIOD";
    case KEY_RIGHT_BRACKET: return "KEY_RIGHT_BRACKET";
    case KEY_SEMICOLON:     return "KEY_SEMICOLON";
    case KEY_SLASH:         return "KEY_SLASH";
    case KEY_WORLD_1:       return "KEY_WORLD_1";
    case KEY_BACKSPACE:     return "KEY_BACKSPACE";
    case KEY_CAPS_LOCK:     return "KEY_CAPS_LOCK";
    case KEY_DELETE:        return "KEY_DELETE";
    case KEY_DOWN:          return "KEY_DOWN";
    case KEY_END:           return "KEY_END";
    case KEY_ENTER:         return "KEY_ENTER";
    case KEY_ESCAPE:        return "KEY_ESCAPE";
    case KEY_F1:            return "KEY_F1";
    case KEY_F2:            return "KEY_F2";
    case KEY_F3:            return "KEY_F3";
    case KEY_F4:            return "KEY_F4";
    case KEY_F5:            return "KEY_F5";
    case KEY_F6:            return "KEY_F6";
    case KEY_F7:            return "KEY_F7";
    case KEY_F8:            return "KEY_F8";
    case KEY_F9:            return "KEY_F9";
    case KEY_F10:           return "KEY_F10";
    case KEY_F11:           return "KEY_F11";
    case KEY_F12:           return "KEY_F12";
    case KEY_F13:           return "KEY_F13";
    case KEY_F14:           return "KEY_F14";
    case KEY_F15:           return "KEY_F15";
    case KEY_F16:           return "KEY_F16";
    case KEY_F17:           return "KEY_F17";
    case KEY_F18:           return "KEY_F18";
    case KEY_F19:           return "KEY_F19";
    case KEY_F20:           return "KEY_F20";
    case KEY_HOME:          return "KEY_HOME";
    case KEY_INSERT:        return "KEY_INSERT";
    case KEY_LEFT:          return "KEY_LEFT";
    case KEY_LEFT_ALT:      return "KEY_LEFT_ALT";
    case KEY_LEFT_CONTROL:  return "KEY_LEFT_CONTROL";
    case KEY_LEFT_SHIFT:    return "KEY_LEFT_SHIFT";
    case KEY_LEFT_SUPER:    return "KEY_LEFT_SUPER";
    case KEY_MENU:          return "KEY_MENU";
    case KEY_NUM_LOCK:      return "KEY_NUM_LOCK";
    case KEY_PAGE_DOWN:     return "KEY_PAGE_DOWN";
    case KEY_PAGE_UP:       return "KEY_PAGE_UP";
    case KEY_RIGHT:         return "KEY_RIGHT";
    case KEY_RIGHT_ALT:     return "KEY_RIGHT_ALT";
    case KEY_RIGHT_CONTROL: return "KEY_RIGHT_CONTROL";
    case KEY_RIGHT_SHIFT:   return "KEY_RIGHT_SHIFT";
    case KEY_RIGHT_SUPER:   return "KEY_RIGHT_SUPER";
    case KEY_SPACE:         return "KEY_SPACE";
    case KEY_TAB:           return "KEY_TAB";
    case KEY_UP:            return "KEY_UP";
    case KEY_KP_0:          return "KEY_KP_0";
    case KEY_KP_1:          return "KEY_KP_1";
    case KEY_KP_2:          return "KEY_KP_2";
    case KEY_KP_3:          return "KEY_KP_3";
    case KEY_KP_4:          return "KEY_KP_4";
    case KEY_KP_5:          return "KEY_KP_5";
    case KEY_KP_6:          return "KEY_KP_6";
    case KEY_KP_7:          return "KEY_KP_7";
    case KEY_KP_8:          return "KEY_KP_8";
    case KEY_KP_9:          return "KEY_KP_9";
    case KEY_KP_ADD:        return "KEY_KP_ADD";
    case KEY_KP_DECIMAL:    return "KEY_KP_DECIMAL";
    case KEY_KP_DIVIDE:     return "KEY_KP_DIVIDE";
    case KEY_KP_ENTER:      return "KEY_KP_ENTER";
    case KEY_KP_EQUAL:      return "KEY_KP_EQUAL";
    case KEY_KP_MULTIPLY:   return "KEY_KP_MULTIPLY";
    case KEY_KP_SUBTRACT:   return "KEY_KP_SUBTRACT";
    case _KEY_KIND_COUNT_:  return "_KEY_KIND_COUNT_";
    }
    return NULL;
}

internal const char* key_mod_to_str(Key_Mod mod)
{
    switch (mod)
    {
        case KEY_MOD_SHIFT:     return "KEY_MOD_SHIFT";
        case KEY_MOD_CONTROL:   return "KEY_MOD_CONTROL";
        case KEY_MOD_ALT:       return "KEY_MOD_ALT";
        case KEY_MOD_SUPER:     return "KEY_MOD_SUPER";
        case KEY_MOD_CAPS_LOCK: return "KEY_MOD_CAPS_LOCK";
    }
    return NULL;
}
