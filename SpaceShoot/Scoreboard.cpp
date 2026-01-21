#include <GL/glut.h>
#include <cstdio>
#include <cstring>
#include <algorithm>

class Scoreboard {
public:
    int elapsedSec = 0;
    int score = 0;
    int level = 1;

    int killsInLevel = 0;
    int killsNeed = 10;
    int frameCounter = 0;

    void reset() {
        elapsedSec = 0;
        score = 0;
        level = 1;
        killsInLevel = 0;
        killsNeed = killsRequiredForLevel(level);
        frameCounter = 0;
    }

    void update() {
        frameCounter++;
        if (frameCounter >= 60) {
            elapsedSec++;
            frameCounter -= 60;
        }
    }

    void addKill(int k = 1) {
        for (int i = 0; i < k; i++) {
            score++;
            killsInLevel++;

            while (killsInLevel >= killsNeed) {
                killsInLevel -= killsNeed;
                level++;
                killsNeed = killsRequiredForLevel(level);
            }
        }
    }

    void draw(int screenW, int screenH, int health) const {
        glDisable(GL_DEPTH_TEST);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, screenW, 0, screenH);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        int minutes = elapsedSec / 60;
        int seconds = elapsedSec % 60;

        char timeBuf[64];
        std::snprintf(timeBuf, sizeof(timeBuf), "%d:%02d", minutes, seconds);

        char scoreBuf[64];
        std::snprintf(scoreBuf, sizeof(scoreBuf), "Score : %d", score);

        char levelBuf[64];
        std::snprintf(levelBuf, sizeof(levelBuf), "Level : %d", level);

        // Right-aligned HUD block (tight to top-right, no overlaps)
        const int rightEdge = screenW - 12;
        const int topY = screenH - 18;
        const int lineH = 22;

        int wTime  = textWidth(timeBuf);
        int wScore = textWidth(scoreBuf);
        int wLevel = textWidth(levelBuf);

        int xTime  = rightEdge - wTime;
        int xScore = rightEdge - wScore;
        int xLevel = rightEdge - wLevel;

        glColor3f(1, 0, 0);
        drawText(xTime,  topY,             timeBuf);
        drawText(xScore, topY - lineH,     scoreBuf);
        drawText(xLevel, topY - 2*lineH,   levelBuf);

        // Health bar under the text, aligned to the same right edge
        int barY = topY - 3*lineH - 8;
        drawHealthBarRightAligned(rightEdge, barY, health);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

        glEnable(GL_DEPTH_TEST);
    }

private:
    static int killsRequiredForLevel(int lvl) {
        int group = (lvl - 1) / 10;
        int mult = 1;
        for (int i = 0; i < group; i++) mult *= 2;
        return 10 * mult;
    }

    static void drawText(int x, int y, const char* s) {
        glRasterPos2i(x, y);
        for (int i = 0; s[i] != '\0'; i++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, s[i]);
        }
    }

    static int textWidth(const char* s) {
        int w = 0;
        for (int i = 0; s[i] != '\0'; i++) {
            w += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, s[i]);
        }
        return w;
    }

    static void drawRect2D(float x, float y, float w, float h, bool filled) {
        if (filled) glBegin(GL_QUADS);
        else        glBegin(GL_LINE_LOOP);

        glVertex2f(x,     y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x,     y + h);

        glEnd();
    }

    void drawHealthBarRightAligned(int rightEdge, int y0, int health) const {
        float hp = (float)health;
        hp = std::max(0.0f, std::min(100.0f, hp));

        float barW = 140;
        float barH = 14;
        float x0 = (float)rightEdge - barW;

        float fillW = barW * (hp / 100.0f);

        glColor3f(1, 0, 0);
        drawRect2D(x0, y0, barW, barH, false);

        if (hp >= 60) glColor3f(0, 1, 0);
        else if (hp >= 30) glColor3f(1, 1, 0);
        else glColor3f(1, 0, 0);

        drawRect2D(x0, y0, fillW, barH, true);
    }
};
