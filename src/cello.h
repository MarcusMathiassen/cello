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

#ifndef _CELLO_H_
#define _CELLO_H_

#include "common.h"
#include "input.cc"

#define DEFAULT_WINDOW_WIDTH 200
#define DEFAULT_WINDOW_HEIGHT 200

typedef enum {
    COMPILE_SUCCESS,
    COMPILE_FAILURE,
} Compile_State;

struct Bitmap
{
    u8* buffer;
    s32 width;
    s32 height;
    s32 bytesPerPixel;
    s32 pitch;
};

struct Game_Memory
{
    b32 is_initialized;
    void* permanent_storage;
    u64 permanent_storage_size;
    void* transient_storage;
    u64 transient_storage_size;

    Input_Info inputs;

    void (*get_window_size)(s32* w, s32* h);
    void (*get_input_info)(Input_Info* inputs);
    void (*set_cursor_visibility)(b32 is_visible);
    void (*swap_buffers)(Bitmap* buffer);
    u64 (*get_time)();
    Compile_State (*get_compile_state)();
};


// void get_window_size(s32* w, s32* h);
// void get_input_info(Input_Info* inputs);
// void set_cursor_visibility(b32 is_visible);
// void swap_buffers(Bitmap* buffer);
// u64 get_time();
// b32 game_update_and_render(Game_Memory *memory);

#endif
