#include <GL/glut.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>

// NOTE: Shapes is expected to be available because main.cpp includes Shapes.cpp first.

class EnemySystem {
public:
    enum Type { MONSTER_A = 0, MONSTER_B = 1, MONSTER_C = 2 };

    struct Enemy {
        Type type;
        float x, y;
        float vx, vy;
        float speed;
        float radius;
        float r, g, b;
        float wobblePhase;
        int shootCD = 0;
int touchCD = 0;

    };

    std::vector<Enemy> enemies;

    int maxEnemies = 300;

    // difficulty parameters (updated by setDifficulty)
    int diffLevel = 1;
    int spawnBurst = 2;
    int spawnInterval = 300;   // frames (300 ~ 5 sec @60fps)
    int spawnJitter = 40;
    float speedMul = 1.0f;

    // internal spawn timer
    int spawnCountdown = 180;

    void init(int seed = 0) {
        if (seed == 0) seed = (int)std::time(nullptr);
        std::srand(seed);

        enemies.clear();
        spawnCountdown = 120;
        lastSpawnAngle = 9999.0f;
    }

    void setDifficulty(int lvl) {
        diffLevel = lvl;

        float L = (float)std::min(lvl, 25);
        float t = (L - 1.0f) / 24.0f; // 0..1

        // speed multiplier: 1.0 .. 1.45
        speedMul = 1.0f + 0.45f * t;

        // spawn interval: 5 sec .. ~2.3 sec
        spawnInterval = (int)(300 - 160 * t); // 300..140

        // burst: 2..8 slowly
        spawnBurst = 2 + (int)(6 * t + 0.5f);

        // after level 25: keep speed fixed, increase spawn slowly
        if (lvl > 25) {
            int extra = (lvl - 25) / 5; // every 5 levels
            spawnBurst += extra;
            spawnInterval = std::max(90, spawnInterval - extra * 4);
        }
    }

    void update(const Player& player, float zoom, float aspect) {
        // ---- spawning ----
        if ((int)enemies.size() < maxEnemies) {
            spawnCountdown--;
            if (spawnCountdown <= 0) {
                for (int i = 0; i < spawnBurst && (int)enemies.size() < maxEnemies; i++) {
                    spawnOne(player, zoom, aspect);
                }
                int jitter = randRangeInt(-spawnJitter, spawnJitter);
                spawnCountdown = std::max(40, spawnInterval + jitter);
            }
        }

        // ---- chase ----
        for (auto &e : enemies) {
            e.wobblePhase += 0.05f;

            float dx = player.x - e.x;
            float dy = player.y - e.y;
            float d = std::sqrt(dx*dx + dy*dy) + 1e-6f;

            float ux = dx / d;
            float uy = dy / d;

            float targetVx = ux * e.speed;
            float targetVy = uy * e.speed;

            e.vx = lerp(e.vx, targetVx, 0.07f);
            e.vy = lerp(e.vy, targetVy, 0.07f);

            e.x += e.vx;
            e.y += e.vy;
        }

        applySeparation();
    }

    void draw() const {
        for (const auto& e : enemies) {
            glPushMatrix();
            glTranslatef(e.x, e.y, 0.0f);

            float wob = 0.03f * std::sin(e.wobblePhase);
            glTranslatef(0.0f, wob, 0.0f);

            glColor3f(e.r, e.g, e.b);

            switch (e.type) {
                case MONSTER_A: drawMonsterA(e); break;
                case MONSTER_B: drawMonsterB(e); break;
                case MONSTER_C: drawMonsterC(e); break;
            }

            glPopMatrix();
        }
    }

private:
    float lastSpawnAngle = 9999.0f;

    static void drawMonsterA(const Enemy& e) {
        Shapes::Circle(0.10f, 40);

        glColor3f(0.95f, 0.95f, 0.95f);
        glPushMatrix(); glTranslatef(0.02f, 0.02f, 0); Shapes::Circle(0.03f, 30); glPopMatrix();

        glColor3f(0, 0, 0);
        glPushMatrix(); glTranslatef(0.03f, 0.02f, 0); Shapes::Circle(0.012f, 20); glPopMatrix();

        glColor3f(e.r * 0.8f, e.g * 0.8f, e.b * 0.8f);
        for (int i = 0; i < 4; i++) {
            glPushMatrix();
            glRotatef(i * 90.0f, 0, 0, 1);
            glTranslatef(0.0f, 0.10f, 0.0f);
            Shapes::Triangle(0.03f);
            glPopMatrix();
        }
    }

    static void drawMonsterB(const Enemy& e) {
        Shapes::Triangle(0.12f);

        glColor3f(1, 1, 1);
        glPushMatrix(); glTranslatef(0.0f, -0.03f, 0.0f); Shapes::Rectangle(0.10f, 0.03f); glPopMatrix();

        glColor3f(e.r * 0.7f, e.g * 0.7f, e.b * 0.7f);
        glPushMatrix(); glTranslatef(-0.10f, -0.02f, 0); glRotatef(20, 0, 0, 1); Shapes::Triangle(0.05f); glPopMatrix();
        glPushMatrix(); glTranslatef( 0.10f, -0.02f, 0); glRotatef(-20,0, 0, 1); Shapes::Triangle(0.05f); glPopMatrix();
    }

    static void drawMonsterC(const Enemy& e) {
        Shapes::HalfCircle(0.12f, 40);

        glColor3f(e.r * 0.8f, e.g * 0.8f, e.b * 0.8f);
        for (int i = 0; i < 4; i++) {
            float x = -0.06f + i * 0.04f;
            glPushMatrix(); glTranslatef(x, -0.10f, 0.0f); Shapes::Rectangle(0.015f, 0.08f); glPopMatrix();
            glPushMatrix(); glTranslatef(x, -0.15f, 0.0f); Shapes::Circle(0.012f, 16); glPopMatrix();
        }

        glColor3f(0, 0, 0);
        glPushMatrix(); glTranslatef(-0.03f, 0.03f, 0); Shapes::Circle(0.01f, 14); glPopMatrix();
        glPushMatrix(); glTranslatef( 0.03f, 0.03f, 0); Shapes::Circle(0.01f, 14); glPopMatrix();
    }

    void spawnOne(const Player& player, float zoom, float aspect) {
        float halfH = 0.577f * zoom;
        float halfW = halfH * aspect;
        float margin = 0.40f;

        Enemy e;
        e.type = (Type)(std::rand() % 3);
        e.vx = e.vy = 0.0f;
        e.wobblePhase = randRange(0.0f, 6.28f);

        if (e.type == MONSTER_A) { e.radius = 0.12f; e.speed = randRange(0.010f, 0.016f); setColor(e, 0); }
        if (e.type == MONSTER_B) { e.radius = 0.13f; e.speed = randRange(0.012f, 0.018f); setColor(e, 1); }
        if (e.type == MONSTER_C) { e.radius = 0.14f; e.speed = randRange(0.009f, 0.014f); setColor(e, 2); }

        // apply difficulty speed multiplier (caps at lvl 25)
        e.speed *= speedMul;

        for (int tries = 0; tries < 30; tries++) {
            int side = std::rand() % 4;
            float x = player.x, y = player.y;

            if (side == 0) { x = player.x - (halfW + margin); y = player.y + randRange(-halfH, halfH); }
            else if (side == 1) { x = player.x + (halfW + margin); y = player.y + randRange(-halfH, halfH); }
            else if (side == 2) { x = player.x + randRange(-halfW, halfW); y = player.y + (halfH + margin); }
            else { x = player.x + randRange(-halfW, halfW); y = player.y - (halfH + margin); }

            float ang = std::atan2(y - player.y, x - player.x);
            if (lastSpawnAngle != 9999.0f) {
                float diff = angleDiff(ang, lastSpawnAngle);
                if (diff < 0.7f) continue;
            }

            e.x = x; e.y = y;

            if (!overlapsAny(e)) {
                enemies.push_back(e);
                lastSpawnAngle = ang;
                break;
            }
        }
    }

    bool overlapsAny(const Enemy& e) const {
        for (const auto& o : enemies) {
            float dx = e.x - o.x;
            float dy = e.y - o.y;
            float d2 = dx*dx + dy*dy;
            float rr = (e.radius + o.radius + 0.05f);
            if (d2 < rr*rr) return true;
        }
        return false;
    }

    void applySeparation() {
        for (int i = 0; i < (int)enemies.size(); i++) {
            for (int j = i + 1; j < (int)enemies.size(); j++) {
                auto &a = enemies[i];
                auto &b = enemies[j];

                float dx = a.x - b.x;
                float dy = a.y - b.y;
                float d = std::sqrt(dx*dx + dy*dy) + 1e-6f;
                float minD = a.radius + b.radius + 0.02f;

                if (d < minD) {
                    float ux = dx / d;
                    float uy = dy / d;
                    float push = (minD - d) * 0.20f;

                    a.x += ux * push;
                    a.y += uy * push;
                    b.x -= ux * push;
                    b.y -= uy * push;
                }
            }
        }
    }

    static void setColor(Enemy& e, int kind) {
        if (kind == 0) { e.r = randRange(0.3f, 0.9f); e.g = randRange(0.2f, 0.6f); e.b = randRange(0.6f, 1.0f); }
        if (kind == 1) { e.r = randRange(0.6f, 1.0f); e.g = randRange(0.2f, 0.8f); e.b = randRange(0.2f, 0.6f); }
        if (kind == 2) { e.r = randRange(0.2f, 0.6f); e.g = randRange(0.7f, 1.0f); e.b = randRange(0.2f, 0.8f); }
    }

    static float randRange(float a, float b) {
        return a + (b - a) * (float(std::rand()) / float(RAND_MAX));
    }
    static int randRangeInt(int a, int b) {
        return a + (std::rand() % (b - a + 1));
    }
    static float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }
    static float angleDiff(float a, float b) {
        float d = std::fabs(a - b);
        while (d > 3.1415926f) d = std::fabs(d - 2.0f * 3.1415926f);
        return d;
    }
};
