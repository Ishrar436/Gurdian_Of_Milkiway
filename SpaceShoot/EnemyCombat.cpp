#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdlib>

class EnemyCombat {
public:
    struct EBullet {
        float x, y;
        float vx, vy;
        float life;     // frames remaining
    };

    std::vector<EBullet> bullets;

    // tune values
    float bulletSpeed = 0.020f;     // ✅ slower than player bullet
    float bulletR     = 0.030f;     // visible size
    float playerR     = 0.20f;      // approximate player radius
    float shootRangeMax   = 5.5f;   // enemy can shoot if within this
    float closeNoShoot    = 0.55f;  // if closer than this, do NOT shoot (melee)
    int   shootCooldownMin = 40;    // frames
    int   shootCooldownMax = 90;    // frames

    int touchDamage = 2;            // 2 hp each tick
    int bulletDamage = 2;
    int hp = 100;
int invuln = 0;   // <-- add this (i-frames)
    // Clear all enemy bullets (used when starting a new run)
    void reset() { bullets.clear(); }


    int touchTickFrames = 18;       // ✅ continuous touch damage every ~0.3s at 60fps

    void update(EnemySystem &enemies, Player &player) {
        // ---- 1) enemy firing + touch damage ----
        for (auto &e : enemies.enemies) {
            // cooldown tick handled in EnemySystem.update(), but safe here too
            if (e.shootCD > 0) e.shootCD--;
            if (e.touchCD > 0) e.touchCD--;

            float dx = player.x - e.x;
            float dy = player.y - e.y;
            float d  = std::sqrt(dx*dx + dy*dy) + 1e-6f;

            // (A) TOUCH DAMAGE (zombie style continuous)
            float touchDist = e.radius + playerR;
            if (d < touchDist) {
                // continuous glow
                player.bodyHitT = 1.0f;

                // continuous damage tick
                if (e.touchCD <= 0) {
                    applyDamage(player, bulletDamage);  // same 2 damage
                    e.touchCD = touchTickFrames;
                }
            }

            // (B) SHOOTING (only if not too close)
            if (d < shootRangeMax && d > closeNoShoot) {
                if (e.shootCD <= 0) {
                    fireFromEnemy(e, player);
                    e.shootCD = randRangeInt(shootCooldownMin, shootCooldownMax);
                }
            }
        }

        // ---- 2) update bullets movement ----
        for (auto &b : bullets) {
            b.x += b.vx;
            b.y += b.vy;
            b.life -= 1.0f;
        }

        // ---- 3) bullet vs player collision ----
        for (auto &b : bullets) {
            float dx = b.x - player.x;
            float dy = b.y - player.y;
            float rr = bulletR + playerR;

            if (dx*dx + dy*dy < rr*rr) {
                // hit glow
                player.bodyHitT = 1.0f;

                applyDamage(player, bulletDamage);
                b.life = 0.0f; // kill only this bullet
            }
        }

        // ---- 4) cleanup dead bullets ----
        bullets.erase(
            std::remove_if(bullets.begin(), bullets.end(),
                           [](const EBullet& b){ return b.life <= 0.0f; }),
            bullets.end()
        );
    }

    void draw() const {
        // danger red bullets
        glColor3f(1.0f, 0.0f, 0.0f);

        for (const auto &b : bullets) {
            glPushMatrix();
            glTranslatef(b.x, b.y, 0.0f);

            // simple glowing bullet: circle + small tail line
            Shapes::Circle(bulletR, 18);

            glBegin(GL_LINES);
            glVertex2f(0.0f, 0.0f);
            glVertex2f(-b.vx * 10.0f, -b.vy * 10.0f);
            glEnd();

            glPopMatrix();
        }
    }

private:
    void fireFromEnemy(const EnemySystem::Enemy &e, const Player &player) {
        float dx = player.x - e.x;
        float dy = player.y - e.y;
        float d  = std::sqrt(dx*dx + dy*dy) + 1e-6f;
        float ux = dx / d;
        float uy = dy / d;

        EBullet b;
        b.x = e.x + ux * (e.radius + 0.02f);
        b.y = e.y + uy * (e.radius + 0.02f);
        b.vx = ux * bulletSpeed;
        b.vy = uy * bulletSpeed;
        b.life = 320.0f; // ~5 seconds

        bullets.push_back(b);
    }

    void applyDamage(Player &player, int dmg) {
        // ✅ prevent draining too fast: invuln frames
        if (player.invuln > 0) return;

        player.hp -= dmg;
        if (player.hp < 0) player.hp = 0;

        player.invuln = 10; // ~0.16s i-frames

        // glow a bit (you can make headHitT etc. based on direction later)
        player.bodyHitT = 1.0f;
    }

    static int randRangeInt(int a, int b) {
        if (b <= a) return a;
        return a + (std::rand() % (b - a + 1));
    }
};
