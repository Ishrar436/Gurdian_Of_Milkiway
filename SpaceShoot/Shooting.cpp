#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <algorithm>

class Shooting {
public:
    struct Bullet {
        float x, y;
        float vx, vy;
        float life;
        float angleDeg;
    };

    // input state
    bool fireMouse = false;
    bool fireKeyR  = false;

    // aim
    float aimX = 1.0f, aimY = 0.0f;     // normalized aim dir
    float aimAngleDeg = 0.0f;

    // fire control
    int cooldown = 0;         // frames
    int fireDelay = 8;        // lower = faster
    float bulletSpeed = 0.22f;

    std::vector<Bullet> bullets;

    // called every frame
    void setAimFromWorld(float playerX, float playerY, float worldX, float worldY) {
        float dx = worldX - playerX;
        float dy = worldY - playerY;
        float len = std::sqrt(dx*dx + dy*dy);

        if (len < 1e-6f) { aimX = 1.0f; aimY = 0.0f; }
        else { aimX = dx / len; aimY = dy / len; }

        aimAngleDeg = std::atan2(aimY, aimX) * 180.0f / 3.1415926f;
    }

    bool isFiring() const { return fireMouse || fireKeyR; }

    void update(Player &p) {
        if (cooldown > 0) cooldown--;

        if (isFiring() && cooldown == 0) {
            fireTwoBullets(p);
            cooldown = fireDelay;

            // recoil / shooting animation trigger
            p.shotKick = 1.0f;
            p.shotDirX = aimX;
            p.shotDirY = aimY;
            p.aimAngleDeg = aimAngleDeg;
        }

        // move bullets
        for (auto &b : bullets) {
            b.x += b.vx;
            b.y += b.vy;
            b.life -= 1.0f;
        }

        bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
                                     [](const Bullet& b){ return b.life <= 0.0f; }),
                      bullets.end());
    }

    // draw aim preview + bullets
    void drawAimPreview(const Player &p) const {
        // Aim line from player toward direction
        float startX = p.x;
        float startY = p.y + p.bob;

        float len = 2.5f; // line length
        float endX = startX + aimX * len;
        float endY = startY + aimY * len;

        glLineWidth(2.0f);
        glColor3f(1.0f, 0.0f, 0.0f); // red

        glBegin(GL_LINES);
        glVertex2f(startX, startY);
        glVertex2f(endX, endY);
        glEnd();

        // crosshair at the end
        glBegin(GL_LINES);
        glVertex2f(endX - 0.06f, endY);
        glVertex2f(endX + 0.06f, endY);
        glVertex2f(endX, endY - 0.06f);
        glVertex2f(endX, endY + 0.06f);
        glEnd();

        glLineWidth(1.0f);
    }

    void drawBullets() const {
        for (const auto &b : bullets) {
            glPushMatrix();
            glTranslatef(b.x, b.y, 0.0f);
            glRotatef(b.angleDeg, 0,0,1);

            // bright core
            glColor3f(1.0f, 0.25f, 0.25f);
            Shapes::Rectangle(0.12f, 0.03f);

            // glow layer
            glColor3f(1.0f, 0.65f, 0.25f);
            Shapes::Rectangle(0.07f, 0.06f);

            glPopMatrix();
        }
    }

private:
    void fireTwoBullets(const Player &p) {
        // two guns: offset perpendicular to aim direction
        float px = p.x;
        float py = p.y + p.bob;

        // perpendicular vector
        float nx = -aimY;
        float ny =  aimX;

        float side = 0.08f;      // separation between bullets
        float muzzleForward = 0.22f;

        // left/right muzzle points
        float x1 = px + nx * side + aimX * muzzleForward;
        float y1 = py + ny * side + aimY * muzzleForward;

        float x2 = px - nx * side + aimX * muzzleForward;
        float y2 = py - ny * side + aimY * muzzleForward;

        spawnBullet(x1, y1);
        spawnBullet(x2, y2);
    }

    void spawnBullet(float x, float y) {
        Bullet b;
        b.x = x; b.y = y;
        b.vx = aimX * bulletSpeed;
        b.vy = aimY * bulletSpeed;
        b.life = 140.0f;
        b.angleDeg = aimAngleDeg;
        bullets.push_back(b);
    }
};
