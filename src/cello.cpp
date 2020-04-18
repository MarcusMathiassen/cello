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

#include <vector>

#include "common.h"
#include "input.cpp"
#include "cello.h"

global_variable void (*get_window_size)(s32* w, s32* h);
global_variable void (*get_input_info)(Input_Info* inputs);
global_variable void (*set_cursor_visibility)(b32 is_visible);
global_variable void (*swap_buffers)(Bitmap* buffer);
global_variable u64 (*get_time)();

#include "shader_common.h"
#include "camera.cpp"
#include "kernel.cpp"
#include "dispatch.cpp"

internal void vsync(s32 target_framerate, u64 frame_start_time, u64 swapbuffer_time)
{
    u64 nanoseconds_passed_this_frame = get_time() - frame_start_time;
    u64 target_nanoseconds_per_frame = (u64)(1.0 / (f64)target_framerate * 1e9);

    if (nanoseconds_passed_this_frame < target_nanoseconds_per_frame)
    {
        u64 nanoseconds_remaining = target_nanoseconds_per_frame - nanoseconds_passed_this_frame;

        // nanosleep from testing has an error margin of ~1ms+
        // so we say about 2ms margin.
        nanoseconds_remaining = nanoseconds_remaining*0.8;

        const struct timespec rqtp = (struct timespec)
        {
            .tv_sec = 0,
            .tv_nsec = static_cast<long>(nanoseconds_remaining)
        };
        struct timespec rmtp;
        // u64 t = get_time();
        nanosleep(&rqtp, &rmtp);
        // printf("nanosleep error margin: %lluns\n",(get_time()-t - nanoseconds_remaining));

        // Spin the rest of the time
        u64 target_time = frame_start_time + target_nanoseconds_per_frame;
        while (get_time() < target_time);
    }
}

struct Game_State
{
    b32 is_running;
    Input_Info inputs;
    u64 start_time;
    u64 swap_buffer_time;
    f64 time;
    f64 fps;
    f64 deltaTime;
    s32 threadCount;
    b32 insert_mode;
    Camera camera;
    Bitmap bitmap;
};

internal void allocate_bitmap(Bitmap* bitmap)
{
    if (bitmap->buffer)
    {
        free(bitmap->buffer);
    }
    bitmap->buffer = (u8*)malloc(bitmap->pitch * bitmap->height);
}

extern "C" b32 game_update_and_render(Game_Memory *memory)
{
    Game_State* game_state = (Game_State*)memory->permanent_storage;
        
    // First time through we initialize game state to its
    // default state.
    //

    // If we have reloaded the dylib, these will be reset
    get_window_size        = memory->get_window_size;
    get_input_info         = memory->get_input_info;
    set_cursor_visibility  = memory->set_cursor_visibility;
    swap_buffers           = memory->swap_buffers;
    get_time               = memory->get_time;

    if (!memory->is_initialized)
    {

        game_state->is_running       = true;
        game_state->inputs           = {};
        game_state->start_time       = get_time();
        game_state->swap_buffer_time = 0;
        game_state->time             = 0;
        game_state->fps              = 0;
        game_state->deltaTime        = 0;
        game_state->threadCount      = std::thread::hardware_concurrency();
        game_state->insert_mode      = true;
        game_state->camera           = defaultCamera();

        s32 w,h;
        get_window_size(&w, &h);

        game_state->bitmap = (Bitmap)
        {
            .buffer = NULL,
            .width = w,
            .height = h,
            .bytesPerPixel = 4,
            .pitch = 4 * w
        };

        allocate_bitmap(&game_state->bitmap);

        memory->is_initialized = true;
    }

    //
    // Pull out game state for easy access
    //
    b32 is_running       =  game_state->is_running; 
    Input_Info* inputs   =  &memory->inputs;  //&game_state->inputs; 
    u64 start_time       =  game_state->start_time; 
    u64 swap_buffer_time =  game_state->swap_buffer_time; 
    f64 time             =  game_state->time; 
    f64 fps              =  game_state->fps;  
    f64 deltaTime        =  game_state->deltaTime; 
    s32 threadCount      =  game_state->threadCount; 
    b32 insert_mode      =  game_state->insert_mode; 
    Camera* camera       =  &game_state->camera;
    Bitmap* bitmap       =  &game_state->bitmap;
    //

    // Print some frame stats
    u64 frame_start_time = get_time();
    {
        time = (frame_start_time - start_time) / 1e9;
        fps = 1.0 / deltaTime;
        printf("%fs %d FPS %fms %fms swapBuffer\n", time, (s32)fps, deltaTime * 1e3, (f64)(swap_buffer_time / 1e6));
    }

    // Get inputs
    // get_input_info(inputs);

    if (inputs->keys[KEY_A] == KEY_PRESSED) process_keyboard(camera, MOVE_LEFT, deltaTime);
    if (inputs->keys[KEY_D] == KEY_PRESSED) process_keyboard(camera, MOVE_RIGHT, deltaTime);
    if (inputs->keys[KEY_S] == KEY_PRESSED) process_keyboard(camera, MOVE_BACKWARD, deltaTime);
    if (inputs->keys[KEY_W] == KEY_PRESSED) process_keyboard(camera, MOVE_FORWARD, deltaTime);
    if (inputs->keys[KEY_E] == KEY_PRESSED) process_keyboard(camera, MOVE_UP, deltaTime);
    if (inputs->keys[KEY_Q] == KEY_PRESSED) process_keyboard(camera, MOVE_DOWN, deltaTime);

    foreach(i, inputs->count)
    {
        Input input = inputs->buffer[i];

        switch(input.kind)
        {
            case INPUT_KEY:
            {
                const Key_Kind  key   = input.key.kind;
                const Key_State state = input.key.state;
                const Key_Mod   mod   = input.key.mod;

                if (key == KEY_ESCAPE && state == KEY_PRESSED) is_running = false;
                if (key == KEY_UP && state == KEY_PRESSED)   threadCount *= 2;
                if (key == KEY_DOWN && state == KEY_PRESSED)
                {
                    threadCount /= 2;
                    threadCount = threadCount < 2 ? 2 : threadCount;
                }

                if (key == KEY_I && state == KEY_PRESSED)
                {
                    insert_mode ^= 1;
                    set_cursor_visibility(insert_mode);
                }
            } break;

            case INPUT_MOUSE: break;
            case INPUT_SCROLL: break;
            case INPUT_CURSOR:
            {
                f64 xpos = input.cursor.xpos;
                f64 ypos = input.cursor.ypos;
                if (!insert_mode) process_mouse_movement(camera, xpos, ypos, deltaTime);
            } break;
        }   
    }

    s64 height = bitmap->height;
    s64 width = bitmap->width;

    u32* pixels = (u32*)bitmap->buffer;

    // Calculate camera matrix
    const auto ro = camera->position;
    const auto ta = camera->position + camera->front;

    const auto cr = 0.0;
    const auto cw = normalize(ta - ro);
    const auto cp = v3(sin(cr), cos(cr), 0.0);
    const auto cu = normalize(cross(cw, cp));
    const auto cv = cross(cu, cw);
    const auto matrix = mat3(cu, cv, cw);

    // Setup edits
    Edit_Info edit_info = {};
    edit_info.count = 0;

    edit_info.edits[edit_info.count++] = (Edit) { SET_MATERIAL_ID, (v3) { 2 } };
    edit_info.edits[edit_info.count++] = (Edit) { SET_SIZE, (v3) { 0.2, 0.2, 0.2 } };
    edit_info.edits[edit_info.count++] = (Edit) { OP_REP, (v3) { 0.5, 1000.0, 0.5 } };
    edit_info.edits[edit_info.count++] = (Edit) { SD_BOX, (v3) { 0.0, -1.0, 0.0 } };
    edit_info.edits[edit_info.count++] = (Edit) { OP_RESET };
    edit_info.edits[edit_info.count++] = (Edit) { OP_UNION };

    // edit_info.edits[edit_info.count++] = (Edit) { SET_MATERIAL_ID, (v3) { 2 } };
    // edit_info.edits[edit_info.count++] = (Edit) { SET_SIZE, (v3) { 1.0, 1.0, 1.0 } };
    // edit_info.edits[edit_info.count++] = (Edit) { SD_CAPPED_CYLINDER, (v3) { 0.0, 4.0, 0.0 } };
    // edit_info.edits[edit_info.count++] = (Edit) { OP_UNION };

    edit_info.edits[edit_info.count++] = (Edit) { SET_MATERIAL_ID, (v3) { 10 } };
    edit_info.edits[edit_info.count++] = (Edit) { SET_SIZE, (v3) { 1.0, 1.0, 1.0 } };
    edit_info.edits[edit_info.count++] = (Edit) { SD_SPHERE, (v3) { 0.0, 0.0, 0.0 } };
    edit_info.edits[edit_info.count++] = (Edit) { OP_UNION };

    edit_info.edits[edit_info.count++] = (Edit) { SET_MATERIAL_ID, (v3) { 2 } };
    edit_info.edits[edit_info.count++] = (Edit) { SET_SIZE, (v3) { 1.0, 0.5, 1.0 } };
    edit_info.edits[edit_info.count++] = (Edit) { SD_TORUS, (v3) { 0.0, 0.0, 4.0 } };
    edit_info.edits[edit_info.count++] = (Edit) { OP_UNION };

    edit_info.edits[edit_info.count++] = (Edit) { SET_MATERIAL_ID, (v3) { 3 } };
    edit_info.edits[edit_info.count++] = (Edit) { SET_SIZE, (v3) { 1.0, 1.0, 1.0 } };
    edit_info.edits[edit_info.count++] = (Edit) { SD_BOX, (v3) { 0.0, 0.0, -4.0 } };
    edit_info.edits[edit_info.count++] = (Edit) { OP_UNION };

    // edit_info.edits[edit_info.count++] = (Edit) { SET_MATERIAL_ID, (v3) { 9 } };
    // edit_info.edits[edit_info.count++] = (Edit) { SET_SIZE, (v3) { 0.2, 1.0, 0.0 } };
    // edit_info.edits[edit_info.count++] = (Edit) { SD_CAPPED_CYLINDER, (v3) { 0.0, 0.0, 8.0 } };
    // edit_info.edits[edit_info.count++] = (Edit) { OP_UNION };

    edit_info.edits[edit_info.count++] = (Edit) { SET_MATERIAL_ID, (v3) { 7 } };
    edit_info.edits[edit_info.count++] = (Edit) { SD_CAPPED_CYLINDER, (v3) { 0.0, 0.0, -8.0 } };
    edit_info.edits[edit_info.count++] = (Edit) { OP_UNION };


    // Materials
    Material materials[32];
    s32 materialCount = 0;
    materials[materialCount++] = (Material) { (v3) { 0.3, 0.3, 0.3 }, DIFF, 0.0, 0.3, 0.2 };
    materials[materialCount++] = (Material) { (v3) { 1.0, 0.3, 0.4 }, SPEC, 0.0, 0.3, 1.0 };
    materials[materialCount++] = (Material) { (v3) { 1.0, 0.8, 0.7 }, DIFF, 0.0, 0.3, 0.9 };
    materials[materialCount++] = (Material) { (v3) { 1.0, 0.3, 0.5 }, SPEC, 1.0, 0.5, 0.5 };
    materials[materialCount++] = (Material) { (v3) { 1.0, 1.0, 1.0 }, REFR, 0.0, 0.5, 0.5 };
    materials[materialCount++] = (Material) { (v3) { 1.0, 0.1, 0.1 }, DIFF, 10.0, 0.3, 0.2 };
    materials[materialCount++] = (Material) { (v3) { 0.3, 0.4, 0.3 }, SPEC, 0.0, 0.3, 0.2 };
    materials[materialCount++] = (Material) { (v3) { 0.1, 0.9, 0.3 }, DIFF, 0.0, 0.3, 0.2 };
    materials[materialCount++] = (Material) { (v3) { 0.1, 0.9, 0.1 }, REFR, 0.0, 0.3, 0.2 };
    materials[materialCount++] = (Material) { (v3) { 1.0, 1.0, 1.0 }, SPEC, 0.0, 0.3, 0.2 };
    materials[materialCount++] = (Material) { (v3) { 0.25, 0.25, 0.8 }, DIFF, 0.0, 0.3, 1 };
    materials[materialCount++] = (Material) { (v3) { 1.0, 0.1, 0.1 }, DIFF, 1.0, 0.5, 1.0 };
    materials[materialCount++] = (Material) { (v3) { 0.8, 0.1, 0.3 }, DIFF, 0.0, 0.3, 0.2 };
    materials[materialCount++] = (Material) { (v3) { 0.58, 0.38, 0.21 }, DIFF, 0.0, 0.1, 0.01 };

    // Setup lights 
    Light_Info light_info;
    light_info.count = 0;
    light_info.lights[light_info.count++] = (Light) { (v3) { 1000, 1000, 0 }, (v3) { 0.7, 0.5, 0.3 }, 1000.0 };
    light_info.lights[light_info.count - 1].pos = (v3) { static_cast<float>(sin(time) * 100.0), 100, static_cast<float>(cos(time) * 100) };
    light_info.lights[light_info.count++] = (Light) { (v3) { 0, 100, 0 }, (v3) { 0.7, 0.76, 0.95 }, 1000.0 };

    Uniform uniform = {
        .camera_position = ro,
        .camera_target = ta,
        .camera_zoom = 1.0,
        .camera_matrix = matrix,
        .viewport_size = ushort2(width, height)
    };

    s32 workload_count = threadCount;

    auto tasks = std::vector<std::future<void>>();
    tasks.reserve(workload_count);

    s32 col_count = workload_count/2;
    s32 row_count = workload_count/2;

    const s32 col = width / col_count;
    const s32 row = height / row_count;

    //
    // Uber kernel CPU
    //
    foreach(y, row_count)
    foreach(x, col_count)
    {
        tasks.emplace_back(
            dispatch.async(
                uber,
                uniform,
                light_info,
                materials,
                edit_info,
                pixels,
                ushort2(x * col, y * row),
                ushort2((x + 1) * col, (y + 1) * row)
            )
        );
    }
    for (auto& task : tasks) task.get();

    // vsync(60, frame_start_time, swap_buffer_time);
    swap_buffers(bitmap);

    deltaTime = (get_time() - frame_start_time) / 1e9;

    //
    // Update game state
    //
    game_state->is_running       = is_running;
    game_state->inputs           = *inputs;
    game_state->start_time       = start_time;
    game_state->swap_buffer_time = swap_buffer_time;
    game_state->time             = time;
    game_state->fps              = fps;
    game_state->deltaTime        = deltaTime;
    game_state->threadCount      = threadCount;
    game_state->insert_mode      = insert_mode;
    game_state->camera           = *camera;
    game_state->bitmap           = *bitmap;

    return is_running;
}

