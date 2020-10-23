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

#include <math.h>

#define PI 3.141592653589793

#define YAW         -123.8488
#define PITCH       -11.60
#define SPEED        30.0
#define SENSITIVTY   3.0
#define ZOOM         1.0
#define MIN_ZOOM    0.4 
#define MAX_ZOOM    10.0

static f32 radians(f32 degrees) { return (degrees / 180) * PI; }

typedef enum {
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    MOVE_FORWARD,
    MOVE_BACKWARD
} MovementDirection;

typedef struct {
    v3 position;
    v3 front;
    v3 right;
    v3 up;
    v3 world_up;
    f32 yaw;
    f32 pitch;
    f32 movement_speed;
    f32 mouse_sensitivity;
    f32 zoom;
    s32 first_mouse;
    f32 last_x;
    f32 last_y;
} Camera;

static void update_camera_vectors(Camera* cam)
{
    // Calculate the new front vector
    v3 n_front = (v3) { 0, 0, 0 };
    n_front.x = cos(radians(cam->yaw)) * cos(radians(cam->pitch));
    n_front.y = sin(radians(cam->pitch));
    n_front.z = sin(radians(cam->yaw)) * cos(radians(cam->pitch));
    
    cam->front = normalize(n_front);
    // Also re-calculate the right and Up vector
    cam->right = normalize(cross(cam->front, cam->world_up)); // Normalize the vectors, because their length
    // gets closer to 0 the more you look up or down
    // which results in slower movement.
    cam->up = normalize(cross(cam->right, cam->front));
}


static void process_keyboard(Camera* cam, MovementDirection direction, f32 deltaTime)
{
    f32 velocity = cam->movement_speed * deltaTime;
    switch (direction)
    {
        case MOVE_FORWARD:  cam->position += cam->front*velocity;  break;
        case MOVE_BACKWARD: cam->position -= cam->front*velocity;  break;
        case MOVE_LEFT:     cam->position -= cam->right*velocity;  break;
        case MOVE_RIGHT:    cam->position += cam->right*velocity;  break;
        case MOVE_DOWN:     cam->position -= cam->up*velocity;     break;
        case MOVE_UP:       cam->position += cam->up*velocity;     break;
    }
}

static void process_mouse_movement(Camera* cam, f32 xpos, f32 ypos, f32 deltaTime)
{
    cam->yaw += xpos * cam->mouse_sensitivity * deltaTime;
    cam->pitch -= ypos * cam->mouse_sensitivity * deltaTime;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (cam->pitch > 89.0) {
        cam->pitch = 89.0;
    }
    if (cam->pitch < -89.0) {
        cam->pitch = -89.0;
    }

    // Update front, right and Up Vectors using the updated Eular angles
    update_camera_vectors(cam);
}

static Camera defaultCamera()
{
    Camera cam = (Camera) {
        (v3){13.433692, 6.3185554, 20.717651},
        (v3){-0.5511119, -0.20278727, -0.80941516},
        (v3){0.8265894, 0.0, -0.5628054},
        (v3){-0.11412979, 0.97922283, -0.16762182},
        (v3) { 0, 1, 0 },
        YAW,
        PITCH,
        SPEED,
        SENSITIVTY,
        ZOOM,
        1,
        512 / 2.0,
        512 / 2.0 };
    update_camera_vectors(&cam);
    return cam;
}
