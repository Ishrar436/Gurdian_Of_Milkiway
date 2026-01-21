#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>

// ===== Include your CPP-only “classes” in correct order =====
#include "Shapes.cpp"
#include "Player.cpp"
#include "Movement.cpp"
#include "PlayerMove.cpp"
#include "Background.cpp"
#include "EnemySystem.cpp"
#include "Scoreboard.cpp"
#include "Shooting.cpp"
#include "Effects.cpp"
#include "Collision.cpp"
#include "EnemyCombat.cpp"
#include "UI.cpp"
#include "Audio.cpp"
#include "Input.cpp"

// ===== Globals =====
float zoom = 2.0f;
float targetZoom = 2.0f;

int gW = 640, gH = 480;
float g_aspect = 640.0f / 480.0f;

// 0 = HOME, 1 = PLAYING
int gameState = 0;

// Customize selected color (default yellow = index 0)
int gPlayerColorIndex = 0;

int gPrevScore = 0;
int gPrevHP    = 100;

bool gPaused = false;

Player player;
Movement movement;
PlayerMove playerMove;

Background bg;
EnemySystem enemies;

Scoreboard hud;
Shooting shooting;
Effects fx;
Collision collision;
EnemyCombat enemyCombat;

// --------------------------- text helpers ---------------------------
static void drawBitmapString(float x, float y, void* font, const char* s) {
    glRasterPos2f(x, y);
    for (const char* p = s; *p; p++) glutBitmapCharacter(font, *p);
}

static void drawPauseOverlay() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.20f, 0.00f, 0.03f, 0.65f);
    glBegin(GL_QUADS);
        glVertex2f(-0.55f,  0.20f);
        glVertex2f( 0.55f,  0.20f);
        glVertex2f( 0.55f, -0.20f);
        glVertex2f(-0.55f, -0.20f);
    glEnd();

    glColor4f(1.0f, 0.15f, 0.25f, 0.35f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(-0.55f,  0.20f);
        glVertex2f( 0.55f,  0.20f);
        glVertex2f( 0.55f, -0.20f);
        glVertex2f(-0.55f, -0.20f);
    glEnd();

    glColor3f(1.0f, 0.35f, 0.45f);
    drawBitmapString(-0.12f, 0.05f, GLUT_BITMAP_TIMES_ROMAN_24, "PAUSED");

    glColor3f(1.0f, 0.85f, 0.85f);
    drawBitmapString(-0.33f, -0.06f, GLUT_BITMAP_HELVETICA_18, "Press ENTER to continue");

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ---------------------------------
static void resetWorldForPlay() {
    hud.reset();

    enemies.init();
    enemyCombat.bullets.clear();
    fx.booms.clear();

    player.x = 0.0f;
    player.y = 0.0f;
    player.hp = 100;
    player.invuln = 0;

    player.headHitT  = 0.0f;
    player.bodyHitT  = 0.0f;
    player.leftHitT  = 0.0f;
    player.rightHitT = 0.0f;
    player.legsHitT  = 0.0f;

    shooting = Shooting();

    zoom = 2.0f;
    targetZoom = 2.0f;

    gPaused = false;

    // IMPORTANT: update ONCE here
    gPrevScore = hud.score;
    gPrevHP    = player.hp;
}

void startPlaying() {
    gameState = 1;
    resetWorldForPlay();
    Audio::stopAllLoops();
    Audio::playGameBgm();
}

// ---------------------------------
static void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(player.x, player.y, zoom,
              player.x, player.y, 0,
              0, 1, 0);

    glPushMatrix();
    glTranslatef(player.x, player.y, 0.0f);
    glRotatef(movement.rotX, 1, 0, 0);
    glRotatef(movement.rotY, 0, 1, 0);
    glTranslatef(-player.x, -player.y, 0.0f);

    bg.draw(player);

    if (gameState == 1) {
        enemies.draw();
        glDisable(GL_DEPTH_TEST);
        enemyCombat.draw();
        fx.draw();
        glEnable(GL_DEPTH_TEST);
    }

    glPopMatrix();

    glDisable(GL_DEPTH_TEST);
    shooting.drawAimPreview(player);
    shooting.drawBullets();
    glEnable(GL_DEPTH_TEST);

    // apply chosen skin color before drawing player
    player.setColorIndex(gPlayerColorIndex);

    glDisable(GL_DEPTH_TEST);
    player.draw();
    glEnable(GL_DEPTH_TEST);

    if (gameState == 1) hud.draw(gW, gH, player.hp);

    if (gameState == 0) {
        float mxUI, myUI;
        MenuUI::toUiMouse(Input::mouseSX, Input::mouseSY, gW, gH, mxUI, myUI);
        menuUI.drawHome(gW, gH, mxUI, myUI);
    }

    if (gameState == 1 && gPaused) drawPauseOverlay();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;

    gW = w;
    gH = h;
    g_aspect = (float)w / (float)h;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, g_aspect, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    menuUI.layout(gW, gH);
}

// ---------------------------------
void update(int) {
    static bool prevPaused = false;

    // pause transitions
    if (!prevPaused && gPaused) {
        Audio::pauseBgm();
        Audio::stopAllLoops();
    }
    if (prevPaused && !gPaused) {
        Audio::resumeBgm();
    }
    prevPaused = gPaused;

    // freeze gameplay while paused
    if (gameState == 1 && gPaused) {
        glutPostRedisplay();
        glutTimerFunc(16, update, 0);
        return;
    }

    // smooth zoom
    if (targetZoom < 1.2f) targetZoom = 1.2f;
    if (targetZoom > 6.0f) targetZoom = 6.0f;
    zoom += (targetZoom - zoom) * 0.18f;

    // allow movement + aim + shooting update even in HOME
    movement.update(player, targetZoom);
    playerMove.update(player, movement);

    Input::updateAimFromMouse();
    shooting.update(player);

    bg.update(player, movement);

    if (gameState == 1) {
        hud.update();
        enemies.setDifficulty(hud.level);

        enemies.update(player, zoom, g_aspect);
        enemyCombat.update(enemies, player);

        player.updateDamageTimers();
        collision.bulletEnemy(shooting, enemies, fx, hud);

        fx.update();

        // ---- slowdown protection (caps) ----
        if (enemyCombat.bullets.size() > 800) {
            enemyCombat.bullets.erase(enemyCombat.bullets.begin(),
                                      enemyCombat.bullets.begin() + (enemyCombat.bullets.size() - 800));
        }
        if (fx.booms.size() > 300) {
            fx.booms.erase(fx.booms.begin(),
                           fx.booms.begin() + (fx.booms.size() - 300));
        }

        // sound triggers
        bool movingNow = (std::fabs(movement.dx) > 0.00001f) || (std::fabs(movement.dy) > 0.00001f);
        Audio::setMoveLoop(movingNow);

        bool shootingNow = (shooting.fireMouse || shooting.fireKeyR);
        Audio::setShootLoop(shootingNow);

        if (hud.score > gPrevScore) {
            Audio::enemyHit();
            gPrevScore = hud.score;
        }

        if (player.hp < gPrevHP) {
            Audio::playerHit();
            gPrevHP = player.hp;
        }
    } else {
        // HOME
        player.updateDamageTimers();
        fx.update();
        Audio::setMoveLoop(false);
        Audio::setShootLoop(false);
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(gW, gH);
    glutInitWindowPosition(10, 10);
    glutCreateWindow("SpaceShoot");

    glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    Audio::init("Audio");
    Audio::playHomeBgm();

    bg.init();
    enemies.init();
    hud.reset();
    menuUI.layout(gW, gH);

    player.x = 0.0f;
    player.y = 0.0f;
    player.scale = 1.0f;

    Input::init(&player, &movement, &shooting,
                &gW, &gH, &g_aspect,
                &zoom, &targetZoom,
                60.0f);
    Input::installCallbacks();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}
