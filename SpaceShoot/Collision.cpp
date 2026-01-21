#include <cmath>
#include <algorithm>

class Collision {
public:
    void bulletEnemy(Shooting &shooting, EnemySystem &enemies, Effects &fx, Scoreboard &hud) {
        const float bulletR = 0.03f;

        for (int bi = (int)shooting.bullets.size() - 1; bi >= 0; --bi) {
            auto &b = shooting.bullets[bi];

            bool bulletRemoved = false;

            for (int ei = (int)enemies.enemies.size() - 1; ei >= 0; --ei) {
                auto &e = enemies.enemies[ei];

                float dx = b.x - e.x;
                float dy = b.y - e.y;
                float rr = bulletR + e.radius;

                if (dx*dx + dy*dy <= rr*rr) {
                    // explosion
                    fx.spawn(e.x, e.y);   // <-- if your Effects uses another name, rename this call

                    // remove enemy
                    enemies.enemies.erase(enemies.enemies.begin() + ei);

                    // remove bullet
                    shooting.bullets.erase(shooting.bullets.begin() + bi);
                    bulletRemoved = true;

                    // score + level logic
                    hud.addKill(1);

                    break;
                }
            }

            if (bulletRemoved) continue;
        }
    }

};
