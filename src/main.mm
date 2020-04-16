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

#include <sys/mman.h> // mmap
#include <vector>
#import <AppKit/AppKit.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "common.h"
#include "shader_common.h"

#include "input.cpp"
#include "dispatch.cpp"
#include "camera.cpp"
#include "kernel.cpp"
#include "utility.cpp"

global_variable b32 gIsRunning = true;

global_variable s32 windowWidth = 512;
global_variable s32 windowHeight = 512;

global_variable f64 gTime;
global_variable f64 gFps;
global_variable f64 gDeltaTime;
global_variable u64 g_swap_buffer_time;
global_variable s32 threadCount = 4;

global_variable b32 insert_mode = true;
global_variable Camera gCamera;

typedef struct
{
    s32 width;
    s32 height;
    s32 bytesPerPixel;
    s32 pitch;
} Bitmap_Info;

typedef struct
{
    u8* buffer;
    Bitmap_Info info;
} Bitmap;

internal Bitmap g_bitmap;


internal Key_Kind translate_keys(u16 key)
{
    switch(key)
    {
        case 0x1D: return KEY_0;
        case 0x12: return KEY_1;
        case 0x13: return KEY_2;
        case 0x14: return KEY_3;
        case 0x15: return KEY_4;
        case 0x17: return KEY_5;
        case 0x16: return KEY_6;
        case 0x1A: return KEY_7;
        case 0x1C: return KEY_8;
        case 0x19: return KEY_9;
        case 0x00: return KEY_A;
        case 0x0B: return KEY_B;
        case 0x08: return KEY_C;
        case 0x02: return KEY_D;
        case 0x0E: return KEY_E;
        case 0x03: return KEY_F;
        case 0x05: return KEY_G;
        case 0x04: return KEY_H;
        case 0x22: return KEY_I;
        case 0x26: return KEY_J;
        case 0x28: return KEY_K;
        case 0x25: return KEY_L;
        case 0x2E: return KEY_M;
        case 0x2D: return KEY_N;
        case 0x1F: return KEY_O;
        case 0x23: return KEY_P;
        case 0x0C: return KEY_Q;
        case 0x0F: return KEY_R;
        case 0x01: return KEY_S;
        case 0x11: return KEY_T;
        case 0x20: return KEY_U;
        case 0x09: return KEY_V;
        case 0x0D: return KEY_W;
        case 0x07: return KEY_X;
        case 0x10: return KEY_Y;
        case 0x06: return KEY_Z;
        case 0x27: return KEY_APOSTROPHE;
        case 0x2A: return KEY_BACKSLASH;
        case 0x2B: return KEY_COMMA;
        case 0x18: return KEY_EQUAL;
        case 0x32: return KEY_GRAVE_ACCENT;
        case 0x21: return KEY_LEFT_BRACKET;
        case 0x1B: return KEY_MINUS;
        case 0x2F: return KEY_PERIOD;
        case 0x1E: return KEY_RIGHT_BRACKET;
        case 0x29: return KEY_SEMICOLON;
        case 0x2C: return KEY_SLASH;
        case 0x0A: return KEY_WORLD_1;
        case 0x33: return KEY_BACKSPACE;
        case 0x39: return KEY_CAPS_LOCK;
        case 0x75: return KEY_DELETE;
        case 0x7D: return KEY_DOWN;
        case 0x77: return KEY_END;
        case 0x24: return KEY_ENTER;
        case 0x35: return KEY_ESCAPE;
        case 0x7A: return KEY_F1;
        case 0x78: return KEY_F2;
        case 0x63: return KEY_F3;
        case 0x76: return KEY_F4;
        case 0x60: return KEY_F5;
        case 0x61: return KEY_F6;
        case 0x62: return KEY_F7;
        case 0x64: return KEY_F8;
        case 0x65: return KEY_F9;
        case 0x6D: return KEY_F10;
        case 0x67: return KEY_F11;
        case 0x6F: return KEY_F12;
        case 0x69: return KEY_F13;
        case 0x6B: return KEY_F14;
        case 0x71: return KEY_F15;
        case 0x6A: return KEY_F16;
        case 0x40: return KEY_F17;
        case 0x4F: return KEY_F18;
        case 0x50: return KEY_F19;
        case 0x5A: return KEY_F20;
        case 0x73: return KEY_HOME;
        case 0x72: return KEY_INSERT;
        case 0x7B: return KEY_LEFT;
        case 0x3A: return KEY_LEFT_ALT;
        case 0x3B: return KEY_LEFT_CONTROL;
        case 0x38: return KEY_LEFT_SHIFT;
        case 0x37: return KEY_LEFT_SUPER;
        case 0x6E: return KEY_MENU;
        case 0x47: return KEY_NUM_LOCK;
        case 0x79: return KEY_PAGE_DOWN;
        case 0x74: return KEY_PAGE_UP;
        case 0x7C: return KEY_RIGHT;
        case 0x3D: return KEY_RIGHT_ALT;
        case 0x3E: return KEY_RIGHT_CONTROL;
        case 0x3C: return KEY_RIGHT_SHIFT;
        case 0x36: return KEY_RIGHT_SUPER;
        case 0x31: return KEY_SPACE;
        case 0x30: return KEY_TAB;
        case 0x7E: return KEY_UP;
        case 0x52: return KEY_KP_0;
        case 0x53: return KEY_KP_1;
        case 0x54: return KEY_KP_2;
        case 0x55: return KEY_KP_3;
        case 0x56: return KEY_KP_4;
        case 0x57: return KEY_KP_5;
        case 0x58: return KEY_KP_6;
        case 0x59: return KEY_KP_7;
        case 0x5B: return KEY_KP_8;
        case 0x5C: return KEY_KP_9;
        case 0x45: return KEY_KP_ADD;
        case 0x41: return KEY_KP_DECIMAL;
        case 0x4B: return KEY_KP_DIVIDE;
        case 0x4C: return KEY_KP_ENTER;
        case 0x51: return KEY_KP_EQUAL;
        case 0x43: return KEY_KP_MULTIPLY;
        case 0x4E: return KEY_KP_SUBTRACT;
    }

    UNREACHABLE;
    return KEY_0;
};

internal Key_Mod translate_mods(NSUInteger flags)
{
    u32 mod = 0;

    if (flags & NSEventModifierFlagShift)    mod |= KEY_MOD_SHIFT;
    if (flags & NSEventModifierFlagControl)  mod |= KEY_MOD_CONTROL;
    if (flags & NSEventModifierFlagOption)   mod |= KEY_MOD_ALT;
    if (flags & NSEventModifierFlagCommand)  mod |= KEY_MOD_SUPER;
    if (flags & NSEventModifierFlagCapsLock) mod |= KEY_MOD_CAPS_LOCK;

    return (Key_Mod)mod;
}

internal void refreshBitmap(Bitmap* bitmap)
{
    if (bitmap->buffer)
        free(bitmap->buffer);
    bitmap->buffer = (u8*)malloc(bitmap->info.pitch * bitmap->info.height);
}

global_variable Input_Info* g_input_info;

@interface MyView : NSView
@end

@implementation MyView

- (BOOL) canBecomeKeyView
{
    return YES;
}

- (BOOL) acceptsFirstResponder
{
    return YES;
}

- (BOOL) wantsUpdateLayer
{
    return YES;
}
@end

@interface WindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation WindowDelegate

- (void)windowWillClose:(id)sender
{
    gIsRunning = false;
}
- (NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)frameSize
{
    NSLog(@"Window is resizing: (%f, %f)", frameSize.width, frameSize.height);

    g_bitmap.info = (Bitmap_Info) {
        .width = static_cast<s32>(frameSize.width),
        .height = static_cast<s32>(frameSize.height),
        .bytesPerPixel = 4,
        .pitch = static_cast<s32>(4 * frameSize.width)
    };

    refreshBitmap(&g_bitmap);

    return frameSize;
}
@end

internal void vsync(s32 target_framerate, u64 frame_start_time, u64 swapbuffer_time)
{
    u64 nanoseconds_passed_this_frame = get_time() - frame_start_time;
    u64 target_nanoseconds_per_frame = (u64)(1.0 / (f64)target_framerate * 1e9) - swapbuffer_time;

    if (nanoseconds_passed_this_frame < target_nanoseconds_per_frame) {

        u64 nanoseconds_remaining = target_nanoseconds_per_frame - nanoseconds_passed_this_frame;

        // nanosleep will sometimes oversleep, put in a buffer
        nanoseconds_remaining = (u64)(nanoseconds_remaining * 0.7);

        const struct timespec rqtp = {
            .tv_sec = 0,
            .tv_nsec = static_cast<long>(nanoseconds_remaining)
        };
        struct timespec rmtp;
        nanosleep(&rqtp, &rmtp);

        // Spin the rest of the time
        u64 target_time = frame_start_time + target_nanoseconds_per_frame;
        while (get_time() < target_time)
            ;
    }
}

internal u64 swapBuffer(NSWindow* window)
{
    u64 t = get_time();
    @autoreleasepool {
        NSBitmapImageRep* rep = [[[NSBitmapImageRep alloc] initWithBitmapDataPlanes:&g_bitmap.buffer
                                                                         pixelsWide:g_bitmap.info.width
                                                                         pixelsHigh:g_bitmap.info.height
                                                                      bitsPerSample:8
                                                                    samplesPerPixel:4
                                                                           hasAlpha:YES
                                                                           isPlanar:NO
                                                                     colorSpaceName:NSDeviceRGBColorSpace
                                                                        bytesPerRow:g_bitmap.info.pitch
                                                                       bitsPerPixel:g_bitmap.info.bytesPerPixel * 8] autorelease];

        NSSize imageSize = NSMakeSize(g_bitmap.info.width, g_bitmap.info.height);
        NSImage* image = [[[NSImage alloc] initWithSize:imageSize] autorelease];
        [image addRepresentation:rep];
        window.contentView.layer.contents = image;
    }
    return get_time() - t;
}


global_variable s32 deltaX = 0, deltaY = 0;

internal void fill_inputs(Input_Info* inputs)
{
    NSEvent* event;
    do {

        event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];

        switch([event type])
        {
            case NSEventTypeLeftMouseDown:
            {
                inputs->buffer[inputs->count++] = (Input)
                {
                    .kind = INPUT_MOUSE,
                    .mouse = (Mouse_Input) {
                        .kind = MOUSE_BUTTON_LEFT,
                        .state = MOUSE_PRESSED,
                    },
                };
            } break;
            case NSEventTypeLeftMouseUp:
            {
                inputs->buffer[inputs->count++] = (Input)
                {
                    .kind = INPUT_MOUSE,
                    .mouse = (Mouse_Input) {
                        .kind = MOUSE_BUTTON_LEFT,
                        .state = MOUSE_RELEASED,
                    },
                };
            } break;
            case NSEventTypeRightMouseDown:
            {
                inputs->buffer[inputs->count++] = (Input)
                {
                    .kind = INPUT_MOUSE,
                    .mouse = (Mouse_Input) {
                        .kind = MOUSE_BUTTON_RIGHT,
                        .state = MOUSE_PRESSED,
                    },
                };
            } break;
            case NSEventTypeRightMouseUp:
            {
                inputs->buffer[inputs->count++] = (Input)
                {
                    .kind = INPUT_MOUSE,
                    .mouse = (Mouse_Input) {
                        .kind = MOUSE_BUTTON_RIGHT,
                        .state = MOUSE_RELEASED,
                    },
                };
            } break;
            
            case NSEventTypeMouseMoved:
            {
                NSPoint pos;
                
                if (!insert_mode)
                {
                    pos.x = [event deltaX];
                    pos.y = [event deltaY];
                }
                else
                {
                    pos = [event locationInWindow];
                }

                inputs->buffer[inputs->count++] = (Input)
                {
                    .kind = INPUT_CURSOR,
                    .cursor = (Cursor_Input) {
                        .xpos = pos.x,
                        .ypos = pos.y,
                    },
                };
            } break;

            // case NSEventTypeLeftMouseDragged:   break;
            // case NSEventTypeRightMouseDragged:  break;
            // case NSEventTypeMouseEntered:       break;
            // case NSEventTypeMouseExited:        break;
            
            case NSEventTypeKeyDown:
            {
                const Key_Kind key = translate_keys([event keyCode]);
                const Key_State state = KEY_PRESSED;
                const Key_Mod mod = translate_mods([event modifierFlags]);

                inputs->buffer[inputs->count++] = (Input)
                {
                    .kind = INPUT_KEY,
                    .key = (Key_Input) {
                        .kind = key,
                        .state = state,
                        .mod = mod,
                    },
                };
                inputs->keys[key] = state;
            } break;

            case NSEventTypeKeyUp:
            {
                const Key_Kind key = translate_keys([event keyCode]);
                const Key_State state = KEY_RELEASED;
                const Key_Mod mod = translate_mods([event modifierFlags]);

                inputs->buffer[inputs->count++] = (Input)
                {
                    .kind = INPUT_KEY,
                    .key = (Key_Input) {
                        .kind = key,
                        .state = state,
                        .mod = mod,
                    },
                };
                inputs->keys[key] = state;
            } break;

            // case NSEventTypeFlagsChanged:       break;
            // case NSEventTypeAppKitDefined:      break;
            // case NSEventTypeSystemDefined:      break;
            // case NSEventTypeApplicationDefined: break;
            // case NSEventTypePeriodic:           break;
            // case NSEventTypeCursorUpdate:       break;
            // case NSEventTypeScrollWheel:        break;
            // case NSEventTypeTabletPoint:        break;
            // case NSEventTypeTabletProximity:    break;
            // case NSEventTypeOtherMouseDown:     break;
            // case NSEventTypeOtherMouseUp:       break;
            // case NSEventTypeOtherMouseDragged:  break;
            // case NSEventTypeGesture:            break;
            // case NSEventTypeMagnify:            break;
            // case NSEventTypeSwipe:              break;
            // case NSEventTypeRotate:             break;
            // case NSEventTypeBeginGesture:       break;
            // case NSEventTypeEndGesture:         break;
            // case NSEventTypeSmartMagnify:       break;
            // case NSEventTypePressure:           break;
            // case NSEventTypeDirectTouch:        break;
            // case NSEventTypeQuickLook:          break;
            // case NSEventTypeChangeMode:         break;

            // If WE do not handle it, the system does.
            default: [NSApp sendEvent:event];
        }

    } while (event);
}

struct Game_Memory
{
    b32 is_initialized;

    void* permanent_storage;
    u64 permanent_storage_size;

    void* transient_storage;
    u64 transient_storage_size;

};

struct Game_State
{
    s32 blue_offset;
    s32 green_offset;
    s32 tone_hz;
};

internal void
game_update_and_render(Game_Memory *memory, Input_Info* inputs)
{
    Game_State* game_state = (Game_State*)memory->permanent_storage;
    if (!memory->is_initialized)
    {
        game_state->tone_hz = 256;
    }

    if (inputs->keys[KEY_A] == KEY_PRESSED) process_keyboard(&gCamera, MOVE_LEFT, gDeltaTime);
    if (inputs->keys[KEY_D] == KEY_PRESSED) process_keyboard(&gCamera, MOVE_RIGHT, gDeltaTime);
    if (inputs->keys[KEY_S] == KEY_PRESSED) process_keyboard(&gCamera, MOVE_BACKWARD, gDeltaTime);
    if (inputs->keys[KEY_W] == KEY_PRESSED) process_keyboard(&gCamera, MOVE_FORWARD, gDeltaTime);
    if (inputs->keys[KEY_E] == KEY_PRESSED) process_keyboard(&gCamera, MOVE_UP, gDeltaTime);
    if (inputs->keys[KEY_Q] == KEY_PRESSED) process_keyboard(&gCamera, MOVE_DOWN, gDeltaTime);

    foreach(i, inputs->count)
    {
        Input input = inputs->buffer[i];

        switch(input.kind)
        {
            case INPUT_KEY:
            {
                Key_Kind  key   = input.key.kind;
                Key_State state = input.key.state;
                Key_Mod   mod   = input.key.mod;


                if (key == KEY_ESCAPE && state == KEY_PRESSED) gIsRunning = false;
                if (key == KEY_UP && state == KEY_PRESSED)   threadCount *= 2;
                if (key == KEY_DOWN && state == KEY_PRESSED)
                {
                    threadCount /= 2;
                    threadCount = threadCount < 2 ? 2 : threadCount;
                }

                if (key == KEY_I && state == KEY_PRESSED)
                {
                    insert_mode ^= 1;
                    if (insert_mode) {
                        CGDisplayShowCursor(0);
                        CGAssociateMouseAndMouseCursorPosition(1);
                    } else {
                        CGDisplayHideCursor(0);
                        CGAssociateMouseAndMouseCursorPosition(0);
                    }
                }
            } break;

            case INPUT_MOUSE: break;
            case INPUT_SCROLL: break;
            case INPUT_CURSOR:
            {
                f64 xpos = input.cursor.xpos;
                f64 ypos = input.cursor.ypos;
                if (!insert_mode) process_mouse_movement(&gCamera, xpos, ypos, gDeltaTime);
            } break;
        }   
    }

    s64 height = g_bitmap.info.height;
    s64 width = g_bitmap.info.width;
    s64 pitch = g_bitmap.info.pitch;

    s64 xoffset = gTime * 255;
    s64 yoffset = gTime * 255;

    u32* pixels = (u32*)g_bitmap.buffer;

    ushort2 gs = ushort2(width, height);

    // Calculate camera matrix
    const auto ro = gCamera.position;
    const auto ta = gCamera.position + gCamera.front;

    const auto cr = 0.0;
    const auto cw = normalize(ta - ro);
    const auto cp = v3(sin(cr), cos(cr), 0.0);
    const auto cu = normalize(cross(cw, cp));
    const auto cv = cross(cu, cw);
    const auto matrix = mat3(cu, cv, cw);

    Light_Info light_info;
    light_info.count = 0;
    light_info.lights[light_info.count++] = (Light) { (v3) { 1000, 1000, 0 }, (v3) { 0.7, 0.5, 0.3 }, 1000.0 };
    light_info.lights[light_info.count++] = (Light) { (v3) { 0, 100, 0 }, (v3) { 0.7, 0.76, 0.95 }, 1000.0 };

    light_info.lights[light_info.count - 1].pos = (v3) { static_cast<float>(sin(gTime) * 100.0), 100, static_cast<float>(cos(gTime) * 100) };

    Uniform uniform = {
        .camera_position = ro,
        .camera_target = ta,
        .camera_zoom = 1.5,
        .camera_matrix = matrix,
        .viewport_size = ushort2(width, height)
    };

    s32 workload_count = threadCount; //std::thread::hardware_concurrency();

    auto tasks = std::vector<std::future<void>>();
    tasks.reserve(workload_count);

    s32 col_count = 1;
    s32 row_count = workload_count;

    const s32 col = width / col_count;
    const s32 row = height / row_count;

    //
    // Uber kernel
    //
    foreach(y, row_count)
    foreach(x, col_count)
    {
        tasks.emplace_back(
            dispatch.async(
                uber,
                uniform,
                light_info,
                pixels,
                ushort2(x * col, y * row),
                ushort2((x + 1) * col, (y + 1) * row)
            )
        );
    }
    for (auto& task : tasks) task.get();
}

s32 main(s32 argc, char** argv)
{
    //
    // Allocate all the memory for the applications lifetime
    //

    Game_Memory game_memory = {};

    // These are never allocated again. They are set ONCE at the startup.
    // So this is all the memory you will get. Keep an eye on it.
    game_memory.permanent_storage_size = MEGABYTES(64);
    game_memory.transient_storage_size = GIGABYTES(1);

#if DEV
    // In DEV mode we expect to get the same memory address every time.
    // This is needed for our compile-while-running development style.
    void* base_address = (void*)TERABYTES(8);
    u32 allocation_flags = MAP_PRIVATE | MAP_ANON | MAP_FIXED;
#else
    // When not in DEV mode we can take any address given to us, because we will no longer
    // need any consistant memory addresses for our pointers.
    void* base_address = NULL;
    u32 allocation_flags = MAP_PRIVATE | MAP_ANON;
#endif

    u32 access_flags = PROT_READ | PROT_WRITE;

    // Allocate for both permanent and transient at the same time
    game_memory.permanent_storage = mmap(
        base_address,
        game_memory.permanent_storage_size + game_memory.transient_storage_size,
        access_flags,
        allocation_flags,
        -1,
        0);

    // And just bump the pointer to set the transient pointer
    game_memory.transient_storage = (u8*)game_memory.permanent_storage + game_memory.permanent_storage_size;


    // Now this allocation might fail so handle it gracefully.
    // TODO(marcus): replace 'printf' with custom error routine.
    if (game_memory.permanent_storage == MAP_FAILED)
    {
        printf("mmap error: %d %s\n", errno, strerror(errno));
        return 1;
    }

    gCamera = defaultCamera();
    threadCount = std::thread::hardware_concurrency();

    @autoreleasepool {
        NSRect screenRect = [[NSScreen mainScreen] frame];
        NSRect windowRect = NSMakeRect(
            (screenRect.size.width - windowWidth) * 0.5,
            (screenRect.size.height - windowHeight) * 0.5,
            windowWidth, windowHeight);

        NSWindow* window = [[NSWindow alloc]

            initWithContentRect:windowRect
                      styleMask:
                          NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable

                        backing:NSBackingStoreBuffered
                          defer:NO

        ];

        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp activateIgnoringOtherApps:YES];

        [window makeKeyAndOrderFront:nil];

        MyView* mv = [[MyView alloc] init];
        WindowDelegate* windowDelegate = [[WindowDelegate alloc] init];

        [window setContentView:mv];
        [window makeFirstResponder:mv];
        [window setTitle:@"Cello"];
        [window setDelegate:windowDelegate];
        [window setAcceptsMouseMovedEvents:YES];
        [window setRestorable:NO];

        // Bitmap buffer
        g_bitmap = (Bitmap) {
            .buffer = NULL,
            .info = (Bitmap_Info) {
                .width = static_cast<s32>(window.contentView.bounds.size.width),
                .height = static_cast<s32>(window.contentView.bounds.size.height),
                .bytesPerPixel = 4,
                .pitch = static_cast<s32>(4 * window.contentView.bounds.size.width) }
        };

        NSTrackingArea* trackingArea = [[[NSTrackingArea alloc] initWithRect:screenRect options:NSTrackingMouseMoved | NSTrackingEnabledDuringMouseDrag | NSTrackingActiveInKeyWindow owner:window.contentView userInfo:nil] autorelease];

        [window.contentView addTrackingArea:trackingArea];

        refreshBitmap(&g_bitmap);

        u64 start_time = get_time();

        Input_Info input_info = {};
        while (gIsRunning)
        {
            u64 frame_start_time = get_time();
            {
                gTime = (frame_start_time - start_time) / 1e9;
                gFps = 1.0 / gDeltaTime;
                printf("%fs %d FPS %fms %fms swapBuffer\n", gTime, (s32)gFps, gDeltaTime * 1e3, (f64)(g_swap_buffer_time / 1e6));
            }

            input_info.count = 0;
            fill_inputs(&input_info);

            game_update_and_render(&game_memory, &input_info);

            vsync(60, frame_start_time, g_swap_buffer_time);
            g_swap_buffer_time = swapBuffer(window);


            gDeltaTime = (get_time() - frame_start_time) / 1e9;
        }
    }

    return 0;
}
