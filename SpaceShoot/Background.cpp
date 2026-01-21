#include <GL/glut.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

class Background {
public:
    struct Star {
        float x, y;
        float r, g, b;
        float size;
        float parallax; // 0.2..1.0
    };

    struct Planet {
        float x, y;
        float radius;
        float r, g, b;
        float parallax;     // slow
        int moons;          // 0..2
        float moonDist;
        float moonRadius;
        float moonSpeed;
        float moonPhase;
    };

    struct Meteor {
        bool active = false;
        float x, y;
        float vx, vy;
        float size;
        float life; // countdown
    };

    std::vector<Star> stars;
    std::vector<Planet> planets;
    Meteor meteor;

    float t = 0.0f;
    int meteorCooldown = 180; // frames until next meteor (randomized)

    // ---------- init ----------
    void init(int seed = 0) {
        if (seed == 0) seed = (int)std::time(nullptr);
        std::srand(seed);

        stars.clear();
        planets.clear();
        meteor.active = false;
        meteorCooldown = 120 + (std::rand() % 240);

        // create a starfield around origin
        for (int i = 0; i < 160; i++) stars.push_back(makeStar(0.0f, 0.0f));
        for (int i = 0; i < 6; i++)   planets.push_back(makePlanetFar(0.0f, 0.0f));
    }

    // ---------- update ----------
    void update(const Player &player, const Movement &move) {
        t += 0.016f;

        // Wrap stars/planets around player so it feels infinite
        wrapStars(player);
        wrapPlanets(player);

        // Occasionally spawn new planets (slow)
        if ((std::rand() % 240) == 0 && (int)planets.size() < 10) {
            Planet p = makePlanetFar(player.x, player.y);
            planets.push_back(p);
        }

        // Meteor logic
        if (!meteor.active) {
            meteorCooldown--;
            if (meteorCooldown <= 0) {
                spawnMeteor(player);
                meteorCooldown = 200 + (std::rand() % 260);
            }
        } else {
            meteor.x += meteor.vx;
            meteor.y += meteor.vy;
            meteor.life -= 1.0f;

            if (meteor.life <= 0.0f) meteor.active = false;
        }

        // Update moons
        for (auto &p : planets) p.moonPhase += p.moonSpeed;
    }

    // ---------- draw ----------
    void draw(const Player &player) const {
        // Draw stars (small circles)
        for (const auto &s : stars) {
            glColor3f(s.r, s.g, s.b);
            glPushMatrix();
            glTranslatef(s.x, s.y, 0.0f);
            Shapes::Circle(s.size, 10);
            glPopMatrix();
        }

        // Draw planets (big circles + moons)
        for (const auto &p : planets) {
            glColor3f(p.r, p.g, p.b);
            glPushMatrix();
            glTranslatef(p.x, p.y, 0.0f);
            Shapes::Circle(p.radius, 60);
            glPopMatrix();

            // moons
            for (int m = 0; m < p.moons; m++) {
                float ph = p.moonPhase + m * 3.14159f * 0.7f;
                float mx = p.x + std::cos(ph) * p.moonDist;
                float my = p.y + std::sin(ph) * p.moonDist;

                glColor3f(0.95f, 0.95f, 0.95f); // moon color
                glPushMatrix();
                glTranslatef(mx, my, 0.0f);
                Shapes::Circle(p.moonRadius, 30);
                glPopMatrix();
            }
        }

        // Draw a “sun” (fixed far away, warm color)
        // Draw a “guiding star” near the player (shiny white star)
float twinkle = 0.78f + 0.22f * std::sin(t * 2.2f);
glPushMatrix();
glTranslatef(player.x + 2.2f, player.y + 1.4f, 0.0f);
Shapes::Star5Shiny(0.22f, 0.10f, twinkle);
glPopMatrix();


        // Draw meteor (passes sometimes)
        if (meteor.active) {
            drawMeteor();
        }
    }

private:
    // --------- helpers ----------
    static float rf(float a, float b) {
        return a + (b - a) * (float(std::rand()) / float(RAND_MAX));
    }

    Star makeStar(float cx, float cy) {
        Star s;
        float range = rf(3.0f, 7.0f);
        s.x = cx + rf(-range, range);
        s.y = cy + rf(-range, range);

        // subtle star colors
        float tint = rf(0.8f, 1.0f);
        s.r = tint;
        s.g = tint;
        s.b = rf(0.85f, 1.0f);

        s.size = rf(0.006f, 0.02f);
        s.parallax = rf(0.25f, 1.0f);
        return s;
    }

    Planet makePlanetFar(float px, float py) {
        Planet p;

        // spawn far around player, but not on top
        float range = rf(2.5f, 6.5f);
        float angle = rf(0.0f, 2.0f * 3.1415926f);

        p.x = px + std::cos(angle) * range;
        p.y = py + std::sin(angle) * range;

        p.radius = rf(0.12f, 0.45f);
        p.r = rf(0.2f, 1.0f);
        p.g = rf(0.2f, 1.0f);
        p.b = rf(0.2f, 1.0f);

        p.parallax = rf(0.08f, 0.20f);

        p.moons = std::rand() % 3; // 0..2
        p.moonDist = p.radius + rf(0.10f, 0.25f);
        p.moonRadius = rf(0.03f, 0.07f);
        p.moonSpeed = rf(0.02f, 0.05f);
        p.moonPhase = rf(0.0f, 6.28f);

        return p;
    }

    void spawnMeteor(const Player &player) {
        meteor.active = true;

        // start from top-left-ish off screen and cross
        float startRange = 2.8f;
        meteor.x = player.x + rf(-startRange, startRange);
        meteor.y = player.y + rf(2.0f, 3.4f);

        // velocity diagonally downward
        meteor.vx = rf(0.03f, 0.06f);
        meteor.vy = rf(-0.08f, -0.05f);

        meteor.size = rf(0.03f, 0.07f);
        meteor.life = 140.0f; // frames
    }

    void drawMeteor() const {
        // head
        glColor3f(1.0f, 0.9f, 0.6f);
        glPushMatrix();
        glTranslatef(meteor.x, meteor.y, 0.0f);
        Shapes::Circle(meteor.size, 20);
        glPopMatrix();

        // tail (3 fading circles)
        for (int i = 1; i <= 3; i++) {
            float f = 1.0f - i * 0.25f;
            glColor3f(1.0f * f, 0.7f * f, 0.3f * f);

            glPushMatrix();
            glTranslatef(meteor.x - meteor.vx * i * 10.0f, meteor.y - meteor.vy * i * 10.0f, 0.0f);
            Shapes::Circle(meteor.size * (0.9f - i * 0.15f), 16);
            glPopMatrix();
        }
    }

    void wrapStars(const Player &player) {
        float wrapR = 8.0f;
        for (auto &s : stars) {
            float dx = s.x - player.x;
            float dy = s.y - player.y;

            if (dx > wrapR) s.x -= 2 * wrapR;
            if (dx < -wrapR) s.x += 2 * wrapR;
            if (dy > wrapR) s.y -= 2 * wrapR;
            if (dy < -wrapR) s.y += 2 * wrapR;


        }
    }

   void wrapPlanets(const Player &player) {
    float wrapR = 10.0f;

    for (auto &p : planets) {
        float dx = p.x - player.x;
        float dy = p.y - player.y;

        // Only wrap when it goes far out of the area (infinite world effect)
        if (dx > wrapR)  p.x -= 2 * wrapR;
        if (dx < -wrapR) p.x += 2 * wrapR;
        if (dy > wrapR)  p.y -= 2 * wrapR;
        if (dy < -wrapR) p.y += 2 * wrapR;

        // ✅ DO NOT push planets away when player gets close
        // (Removed the "keep planets away from player zone" block)
    }
}

};

