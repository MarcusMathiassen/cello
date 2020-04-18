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

#include <mach/mach_time.h>
#include <sys/mman.h> // mmap
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#import <QuartzCore/QuartzCore.h>
#import <AppKit/AppKit.h>

#include "common.h"
#include "cello.h"

#include <dlfcn.h> // dlopen, dlsym, dlerror

global_variable NSWindow* window;
global_variable b32 cursor_is_locked = 0;


internal u64 get_file_last_time_changed(char* path)
{
    struct stat s;
    stat(path, &s);
    return s.st_mtimespec.tv_nsec;
}

global_variable b32 (*game_update_and_render)(Game_Memory* memory) = NULL;
global_variable void* game_code_dylib = NULL;

internal void unload_game_code()
{
    if(game_code_dylib)
    if (dlclose(game_code_dylib)) printf("dlcose error\n");
    printf("UNLOADED game code\n");
}

u64 time_last_changed = 0;
internal void maybe_load_game_code()
{
    // Early return if the game code has not changed
    u64 new_time_last_changed = get_file_last_time_changed("./cello.dylib");
    if (time_last_changed == new_time_last_changed) return;
    time_last_changed = new_time_last_changed;

    // We unload the game code first
    unload_game_code();

    game_code_dylib = dlopen("./cello.dylib", RTLD_LAZY|RTLD_GLOBAL);
    if (!game_code_dylib)
    {
        printf("dlopen error: %s\n", dlerror());
        game_update_and_render = NULL;
        return;
    }

    dlerror(); // clear error

    auto dummy = (b32 (*)(Game_Memory*))dlsym(game_code_dylib, "game_update_and_render");

    // Check for dlsym errors
    char* errstr;
    if((errstr = dlerror()))
    {
        printf("dlsym error: %s\n", errstr);
        if (dlclose(game_code_dylib))
        {
            printf("dlcose error\n");
        }
        game_update_and_render = NULL;
        return;
    }

    game_update_and_render = dummy;

    printf("LOADED game code\n");
}


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

- (NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)frameSize
{
    NSLog(@"Window is resizing: (%f, %f)", frameSize.width, frameSize.height);
    return frameSize;
}
@end

PLATFORM_API void get_input_info(Input_Info* inputs)
{
    inputs->count = 0;
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
                
                if (cursor_is_locked)
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

PLATFORM_API void set_cursor_visibility(b32 is_visible)
{
    if (is_visible)
    {
        cursor_is_locked = false;
        CGDisplayShowCursor(0);
        CGAssociateMouseAndMouseCursorPosition(1);
    }
    else
    {
        cursor_is_locked = true;
        CGDisplayHideCursor(0);
        CGAssociateMouseAndMouseCursorPosition(0);
    }
}

PLATFORM_API void get_window_size(s32* w, s32* h)
{
    *w = window.contentView.bounds.size.width;
    *h = window.contentView.bounds.size.height;
}

PLATFORM_API void swap_buffers(Bitmap* bitmap)
{
    @autoreleasepool
    {
        NSBitmapImageRep* rep = [[[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes:   &bitmap->buffer
            pixelsWide:                 bitmap->width
            pixelsHigh:                 bitmap->height
            bitsPerSample:              8
            samplesPerPixel:            4
            hasAlpha:                   YES
            isPlanar:                   NO
            colorSpaceName:             NSDeviceRGBColorSpace
            bytesPerRow:                bitmap->pitch
            bitsPerPixel:               bitmap->bytesPerPixel * 8
        ] autorelease];

        NSSize imageSize = NSMakeSize(bitmap->width, bitmap->height);
        NSImage* image = [[[NSImage alloc] initWithSize: imageSize] autorelease];

        [image addRepresentation: rep];
        [[[window contentView] layer] setContents: image];
    }
}

PLATFORM_API u64 get_time()
{
    mach_timebase_info_data_t info;
    if (mach_timebase_info(&info) != KERN_SUCCESS) abort();
    return (mach_absolute_time() * info.numer / info.denom);
}


global_variable b32 is_running = 1;

internal CVReturn displayCallback(CVDisplayLinkRef displayLink,
    const CVTimeStamp *inNow, const CVTimeStamp *inOutputTime,
    CVOptionFlags flagsIn, CVOptionFlags *flagsOut,
    void *displayLinkContext)
{
    Game_Memory *game_memory = (Game_Memory*)displayLinkContext;

    // maybe_load_game_code();

    // cello.cpp has global function ptr to these
    // whenever we reload the dylib they get invalidated.
    // so we need to reset them.
    // game_memory->get_window_size       = get_window_size;
    // game_memory->get_input_info        = get_input_info;
    // game_memory->set_cursor_visibility = set_cursor_visibility;
    // game_memory->swap_buffers          = swap_buffers;
    // game_memory->get_time              = get_time;

    // get_input_info(&game_memory->inputs);

    if (game_update_and_render)
        is_running = game_update_and_render(game_memory);

    if(!is_running)
    {
        CVDisplayLinkRelease(displayLink);
    }

    return kCVReturnSuccess;
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

    // Setup platform functions
    game_memory.get_window_size       = get_window_size;
    game_memory.get_input_info        = get_input_info;
    game_memory.set_cursor_visibility = set_cursor_visibility;
    game_memory.swap_buffers          = swap_buffers;
    game_memory.get_time              = get_time;

    @autoreleasepool {
        NSRect screenRect = [[NSScreen mainScreen] frame];
        NSRect windowRect = NSMakeRect(
            (screenRect.size.width - DEFAULT_WINDOW_WIDTH) * 0.5,
            (screenRect.size.height - DEFAULT_WINDOW_HEIGHT) * 0.5,
            DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);

        window = [[NSWindow alloc]initWithContentRect:windowRect
                      styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable
                      backing:NSBackingStoreBuffered defer:NO
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

        // CVDisplayLinkRef    displayLink;
        // CGDirectDisplayID   displayID = CGMainDisplayID();

        // CVReturn            error = kCVReturnSuccess;
        // error = CVDisplayLinkCreateWithCGDisplay(displayID, &displayLink);
        // if (error)
        // {
        //     NSLog(@"DisplayLink created with error:%d", error);
        //     displayLink = NULL;
        // }
        // CVDisplayLinkSetOutputCallback(displayLink, displayCallback, &game_memory);
        // CVDisplayLinkStart(displayLink);

        // [NSApp run];

        while (is_running)
        {
            maybe_load_game_code();

            // cello.cpp has global function ptr to these
            // whenever we reload the dylib they get invalidated.
            // so we need to reset them.
            game_memory.get_window_size       = get_window_size;
            game_memory.get_input_info        = get_input_info;
            game_memory.set_cursor_visibility = set_cursor_visibility;
            game_memory.swap_buffers          = swap_buffers;
            game_memory.get_time              = get_time;

            get_input_info(&game_memory.inputs);
            if (game_update_and_render)
                is_running = game_update_and_render(&game_memory);
        }
    }

    return 0;
}
