#include <GL/glut.h>
#include <cstring>
#include <cmath>

class Movement {
public:
    // movement
    float speed = 0.02f;
    float zoomSpeed = 0.03f;
    float zoomMin = 1.0f, zoomMax = 6.0f;

    // jump
    float gravity = -0.0040f;
    float jumpVel = 0.08f;
    bool inAir = false;
    float vy = 0.0f;
    float groundY = 0.0f;

    // rotation
    float rotStep = 2.5f;
    float rotXMin = -35.0f, rotXMax = 35.0f;
    float rotYMin = -45.0f, rotYMax = 45.0f;

    float rotX = 0.0f;
    float rotY = 0.0f;

    // current frame velocity (for animation)
    float dx = 0.0f;
    float dy = 0.0f;

    bool keyDown[256];
    bool specialDown[256];   // for arrow keys etc.

    Movement() {
        std::memset(keyDown, 0, sizeof(keyDown));
        std::memset(specialDown, 0, sizeof(specialDown));
    }

    void clearKeys() {
        std::memset(keyDown, 0, sizeof(keyDown));
        std::memset(specialDown, 0, sizeof(specialDown));
    }

    void onKeyDown(unsigned char key, Player &player) {
        keyDown[(unsigned char)key] = true;

        // jump triggers once
        if (key == ' ' && !inAir) {
            inAir = true;
            vy = jumpVel;
            groundY = player.y;
        }
    }

    void onKeyUp(unsigned char key) {
        keyDown[(unsigned char)key] = false;
    }

    void onSpecialDown(int key) {
        if (key >= 0 && key < 256) specialDown[key] = true;
    }

    void onSpecialUp(int key) {
        if (key >= 0 && key < 256) specialDown[key] = false;
    }

    void update(Player &player, float &targetZoom) {
        dx = 0.0f;
        dy = 0.0f;

        // clamp rotations
        if (rotX < rotXMin) rotX = rotXMin;
        if (rotX > rotXMax) rotX = rotXMax;

        // movement (WASD + Arrow Keys)
        if (keyDown['a'] || specialDown[GLUT_KEY_LEFT])  dx -= speed;
        if (keyDown['d'] || specialDown[GLUT_KEY_RIGHT]) dx += speed;
        if (keyDown['w'] || specialDown[GLUT_KEY_UP])    dy += speed;
        if (keyDown['s'] || specialDown[GLUT_KEY_DOWN])  dy -= speed;

        player.x += dx;

        // if jumping, Y is controlled by physics
        if (!inAir) player.y += dy;

        // zoom (U/I)
        if (keyDown['u']) targetZoom -= zoomSpeed;
        if (keyDown['i']) targetZoom += zoomSpeed;

        // rotate X with Z/C
        if (keyDown['z']) rotX += rotStep;
        if (keyDown['c']) rotX -= rotStep;

        // rotate Y with Q/E
        if (keyDown['q']) rotY += rotStep;
        if (keyDown['e']) rotY -= rotStep;

        // jump physics
        if (inAir) {
            player.y += vy;
            vy += gravity;

            if (player.y <= groundY) {
                player.y = groundY;
                inAir = false;
                vy = 0.0f;
            }
        }
    }
};
