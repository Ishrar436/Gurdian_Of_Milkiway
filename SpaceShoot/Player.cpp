#include <GL/glut.h>
#include <cmath>
#include <algorithm>

// NOTE: Shapes is expected to already be available because main.cpp includes Shapes.cpp first.

class Player {

public:
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;
    float scale = 1.0f;

    // animation params
    float armSwing = 0.0f;
    float legSwing = 0.0f;
    float armLift  = 0.0f;
    float legLift  = 0.0f;
    float spread   = 0.0f;
    float lean     = 0.0f;
    float crouch   = 0.0f;
    float bob      = 0.0f;

    // aiming + recoil
    float aimAngleDeg = 0.0f;
    float shotKick = 0.0f;     // 0..1
    float shotDirX = 1.0f;
    float shotDirY = 0.0f;

    // health
    int hp = 100;
    float invuln = 0;

    // hit glow timers (0..1)
    float headHitT  = 0.0f;
    float bodyHitT  = 0.0f;
    float leftHitT  = 0.0f;
    float rightHitT = 0.0f;
    float legsHitT  = 0.0f;

    // skin color
    int colorIndex = 0;
    float baseR = 1.0f, baseG = 1.0f, baseB = 0.0f; // default yellow

    void setColorIndex(int idx) {
        colorIndex = (idx % 10 + 10) % 10;
        palette(colorIndex, baseR, baseG, baseB);
    }

    // call each frame
    void updateDamageTimers() {
        if (invuln > 0) invuln--;

        headHitT = std::max(0.0f, headHitT - 0.08f);
        bodyHitT = std::max(0.0f, bodyHitT - 0.08f);
        leftHitT = std::max(0.0f, leftHitT - 0.08f);
        rightHitT= std::max(0.0f, rightHitT- 0.08f);
        legsHitT = std::max(0.0f, legsHitT - 0.08f);
    }

    // used by EnemyCombat when enemy bullet hits a part
    void applyDamageHead(int dmg)  { hp = std::max(0, hp - dmg); headHitT  = 1.0f; }
    void applyDamageBody(int dmg)  { hp = std::max(0, hp - dmg); bodyHitT  = 1.0f; }
    void applyDamageLeft(int dmg)  { hp = std::max(0, hp - dmg); leftHitT  = 1.0f; }
    void applyDamageRight(int dmg) { hp = std::max(0, hp - dmg); rightHitT = 1.0f; }
    void applyDamageLegs(int dmg)  { hp = std::max(0, hp - dmg); legsHitT  = 1.0f; }

    void draw() const {
        glPushMatrix();

        glTranslatef(x, y + bob, 0.0f);
        glRotatef(angle, 0.0f, 0.0f, 1.0f);
        glScalef(scale, scale, 1.0f);

        float bodyX = lean;
        float bodyY = crouch * 0.2f;

        // recoil offsets
        float recoilX = shotDirX * (0.08f * shotKick);
        float recoilY = shotDirY * (0.08f * shotKick);

        float aimHandX = shotDirX * 0.05f;
        float aimHandY = shotDirY * 0.05f;

        bool flash = (shotKick > 0.65f);

        // ---------- BODY ----------
        setBaseWithHit(baseR, baseG, baseB, bodyHitT);
        glPushMatrix();
        glTranslatef(bodyX, bodyY, 0.0f);
        Shapes::Rectangle(0.22f, 0.32f);
        glPopMatrix();

        // ---------- HEAD ----------
        setBaseWithHit(baseR, baseG, baseB, headHitT);
        glPushMatrix();
        glTranslatef(bodyX, 0.22f + bodyY, 0.0f);
        Shapes::Circle(0.10f, 60);
        glPopMatrix();

        // visor (cyan)
        glColor3f(0.4f, 0.9f, 1.0f);
        glPushMatrix();
        glTranslatef(bodyX + 0.03f, 0.22f + bodyY, 0.0f);
        Shapes::Circle(0.055f, 50);
        glPopMatrix();

        // ---------- ARMS ----------
        float shY = 0.08f + armLift - crouch * 0.3f;
        float shX = 0.16f + spread * 0.6f;

        // LEFT ARM
        setBaseWithHit(baseR, baseG, baseB, leftHitT);
        glPushMatrix();
        glTranslatef(bodyX - shX, shY, 0.0f);
        Shapes::Rectangle(0.08f, 0.12f);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(bodyX - shX, shY - 0.10f + armSwing, 0.0f);
        Shapes::Rectangle(0.02f, 0.14f);
        glPopMatrix();

        float leftHandX = bodyX - shX;
        float leftHandY = shY - 0.18f + armSwing;

        glPushMatrix();
        glTranslatef(leftHandX, leftHandY, 0.0f);
        Shapes::Circle(0.03f, 40);
        glPopMatrix();

        drawGun(leftHandX + aimHandX - recoilX,
                leftHandY + aimHandY - recoilY,
                aimAngleDeg, flash);

        // RIGHT ARM
        setBaseWithHit(baseR, baseG, baseB, rightHitT);
        glPushMatrix();
        glTranslatef(bodyX + shX, shY, 0.0f);
        Shapes::Rectangle(0.08f, 0.12f);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(bodyX + shX, shY - 0.10f - armSwing, 0.0f);
        Shapes::Rectangle(0.02f, 0.14f);
        glPopMatrix();

        float rightHandX = bodyX + shX;
        float rightHandY = shY - 0.18f - armSwing;

        glPushMatrix();
        glTranslatef(rightHandX, rightHandY, 0.0f);
        Shapes::Circle(0.03f, 40);
        glPopMatrix();

        drawGun(rightHandX + aimHandX - recoilX,
                rightHandY + aimHandY - recoilY,
                aimAngleDeg, flash);

        // ---------- LEGS ----------
        setBaseWithHit(baseR, baseG, baseB, legsHitT);
        float hipY = -0.26f + legLift - crouch * 0.6f;
        float hipX = 0.06f + spread * 0.8f;

        glPushMatrix();
        glTranslatef(bodyX - hipX, hipY + legSwing, 0.0f);
        Shapes::Rectangle(0.08f, 0.18f);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(bodyX + hipX, hipY - legSwing, 0.0f);
        Shapes::Rectangle(0.08f, 0.18f);
        glPopMatrix();

        glPopMatrix();
    }

private:
    static void palette(int idx, float &r, float &g, float &b) {
        // 10 cool colors
        switch (idx) {
            default:
            case 0: r=1.00f; g=1.00f; b=0.00f; break; // yellow
            case 1: r=0.20f; g=0.85f; b=1.00f; break; // cyan
            case 2: r=0.25f; g=1.00f; b=0.55f; break; // mint
            case 3: r=1.00f; g=0.35f; b=0.90f; break; // pink/purple
            case 4: r=1.00f; g=0.55f; b=0.10f; break; // orange
            case 5: r=0.65f; g=0.45f; b=1.00f; break; // violet
            case 6: r=0.95f; g=0.95f; b=1.00f; break; // soft white
            case 7: r=0.20f; g=0.95f; b=0.20f; break; // neon green
            case 8: r=0.25f; g=0.55f; b=1.00f; break; // blue
            case 9: r=1.00f; g=0.20f; b=0.25f; break; // red-ish
        }
    }

    static void setBaseWithHit(float br, float bg, float bb, float hitT) {
        // blend base toward red when hit
        float r = br + (1.0f - br) * (0.85f * hitT);
        float g = bg * (1.0f - 0.75f * hitT);
        float b = bb * (1.0f - 0.75f * hitT);
        glColor3f(r, g, b);
    }

    void drawGun(float gx, float gy, float angleDeg, bool flash) const {
        glPushMatrix();
        glTranslatef(gx, gy, 0.0f);
        glRotatef(angleDeg, 0, 0, 1);

        // barrel (ash)
        glColor3f(0.55f, 0.55f, 0.55f);
        Shapes::Rectangle(0.14f, 0.03f);

        // top rail
        glColor3f(0.35f, 0.35f, 0.35f);
        glPushMatrix();
        glTranslatef(0.03f, 0.02f, 0.0f);
        Shapes::Rectangle(0.10f, 0.01f);
        glPopMatrix();

        // handle (brown)
        glColor3f(0.45f, 0.25f, 0.12f);
        glPushMatrix();
        glTranslatef(-0.04f, -0.06f, 0.0f);
        Shapes::Rectangle(0.03f, 0.07f);
        glPopMatrix();

        if (flash) {
            glColor3f(1.0f, 0.35f, 0.0f);
            glPushMatrix();
            glTranslatef(0.09f, 0.0f, 0.0f);
            Shapes::Circle(0.02f, 14);
            glPopMatrix();
        }

        glPopMatrix();
    }


};
void drawPlayerPreviewAt(float cx, float cy, float scale, int colorIndex) {
    Player temp;
    temp.x = cx;
    temp.y = cy;
    temp.scale = scale;

    temp.bob = 0.0f;
    temp.angle = 0.0f;
    temp.aimAngleDeg = 0.0f;
    temp.shotKick = 0.0f;

    temp.setColorIndex(colorIndex);   // must exist in Player
    temp.draw();
}

