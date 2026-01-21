#include <GL/glut.h>
#include <vector>
#include <cmath>
#include <algorithm>

class Effects {
public:
    struct Boom {
        float x, y;
        float t;      // 0..1
        float life;   // decreases
    };

    std::vector<Boom> booms;

    // ✅ Collision.cpp calls this
    void spawn(float x, float y) {
        Boom b;
        b.x = x; b.y = y;
        b.t = 0.0f;
        b.life = 1.0f;
        booms.push_back(b);
    }

    void update() {
        for (auto &b : booms) {
            b.t += 0.08f;
            b.life -= 0.06f;
        }
        booms.erase(
            std::remove_if(booms.begin(), booms.end(),
                           [](const Boom& b){ return b.life <= 0.0f; }),
            booms.end()
        );
    }

    void draw() const {
        for (const auto &b : booms) {
            float r = 0.04f + 0.14f * b.t;
            float alpha = std::max(0.0f, b.life);

            // simple ring explosion
            glColor3f(1.0f, 0.6f * alpha, 0.1f * alpha);
            glPushMatrix();
            glTranslatef(b.x, b.y, 0.0f);
            drawRing(r, r + 0.02f);
            glPopMatrix();
        }
    }

private:
    static void drawRing(float r1, float r2) {
        const int N = 40;
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= N; i++) {
            float a = (float)i / (float)N * 2.0f * 3.1415926f;
            float cx = std::cos(a);
            float cy = std::sin(a);
            glVertex2f(cx * r1, cy * r1);
            glVertex2f(cx * r2, cy * r2);
        }
        glEnd();
    }
};
