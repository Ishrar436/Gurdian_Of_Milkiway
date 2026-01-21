#include <GL/glut.h>
#include <cmath>

class Shapes {
public:
    // Rectangle centered at (0,0)
    static void Rectangle(float w, float h) {
        glBegin(GL_QUADS);
        glVertex2f(-w/2,  h/2);
        glVertex2f( w/2,  h/2);
        glVertex2f( w/2, -h/2);
        glVertex2f(-w/2, -h/2);
        glEnd();
    }

    static void Box(float w, float h) { Rectangle(w, h); }

    static void Triangle(float size) {
        glBegin(GL_TRIANGLES);
        glVertex2f(0.0f,  size);
        glVertex2f( size, -size);
        glVertex2f(-size, -size);
        glEnd();
    }

    static void Circle(float r, int segments = 60) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0.0f, 0.0f);
        for(int i = 0; i <= segments; i++) {
            float t = 2.0f * 3.1415926f * i / segments;
            glVertex2f(r * std::cos(t), r * std::sin(t));
        }
        glEnd();
    }

    static void HalfCircle(float r, int segments = 40) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0.0f, 0.0f);
        for(int i = 0; i <= segments; i++) {
            float t = 3.1415926f * i / segments;
            glVertex2f(r * std::cos(t), r * std::sin(t));
        }
        glEnd();
    }

    static void Line(float x1, float y1, float x2, float y2) {
        glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glEnd();
    }

    // Shiny 5-point star (filled). twinkle: 0..1
    static void Star5Shiny(float outerR, float innerR, float twinkle = 1.0f) {
        float outerBright = std::min(1.0f, 0.92f * twinkle + 0.08f);
        float innerBright = std::min(1.0f, 0.75f * twinkle + 0.12f);

        struct P { float x, y; };
        P outer[5], inner[5];

        const float PI = 3.1415926f;
        float a0 = -PI * 0.5f;

        for (int i = 0; i < 5; i++) {
            float ao = a0 + i * (2.0f * PI / 5.0f);
            float ai = ao + (PI / 5.0f);
            outer[i] = { outerR * std::cos(ao), outerR * std::sin(ao) };
            inner[i] = { innerR * std::cos(ai), innerR * std::sin(ai) };
        }

        // halo
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.75f * twinkle, 0.85f * twinkle, 1.0f, 0.18f);
        Circle(outerR * 1.25f, 40);

        // spikes
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < 5; i++) {
            int j = (i + 1) % 5;

            glColor3f(0.80f * innerBright, 0.88f * innerBright, 1.0f * innerBright);
            glVertex2f(inner[i].x, inner[i].y);

            glColor3f(1.0f * outerBright, 1.0f * outerBright, 1.0f);
            glVertex2f(outer[i].x, outer[i].y);

            glColor3f(0.78f * innerBright, 0.86f * innerBright, 1.0f * innerBright);
            glVertex2f(inner[j].x, inner[j].y);
        }
        glEnd();

        // center pentagon
        glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.95f * innerBright, 0.97f * innerBright, 1.0f);
        glVertex2f(0.0f, 0.0f);
        for (int i = 0; i <= 5; i++) {
            int k = i % 5;
            glVertex2f(inner[k].x, inner[k].y);
        }
        glEnd();

        // outline
        glColor4f(1.0f, 1.0f, 1.0f, 0.35f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 5; i++) {
            glVertex2f(outer[i].x, outer[i].y);
            glVertex2f(inner[(i + 1) % 5].x, inner[(i + 1) % 5].y);
        }
        glEnd();

        // sparkle cross (top-right)
        glColor4f(1.0f, 1.0f, 1.0f, 0.55f * twinkle);
        float sx = outerR * 0.55f;
        float sy = outerR * 0.35f;
        glLineWidth(2.0f);
        Line(sx - outerR * 0.12f, sy, sx + outerR * 0.12f, sy);
        Line(sx, sy - outerR * 0.12f, sx, sy + outerR * 0.12f);
        glLineWidth(1.0f);

        glDisable(GL_BLEND);
    }
};
