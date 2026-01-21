#include <GL/glut.h>
#include <cmath>
#include <cstdlib>

// Forward declarations (these classes exist in your included cpp files)
class Player;
class Movement;
class Shooting;
class MenuUI;

// ===== IMPORTANT =====
// gameState is int now: 0 = HOME, 1 = PLAYING
extern int  gameState;
extern bool gPaused;

// from UI.cpp
extern MenuUI menuUI;

// from main.cpp
extern void startPlaying();
extern int  gPlayerColorIndex;

class Input {
public:
    static int mouseSX, mouseSY;

    static Player*   p;
    static Movement* mv;
    static Shooting* sh;

    static int*   pW;
    static int*   pH;
    static float* pAspect;
    static float* pZoom;
    static float* pTargetZoom;
    static float  fovyDeg;

    static void init(Player* _p, Movement* _mv, Shooting* _sh,
                     int* _w, int* _h, float* _aspect,
                     float* _zoom, float* _tzoom,
                     float _fovyDeg)
    {
        p = _p; mv = _mv; sh = _sh;
        pW = _w; pH = _h; pAspect = _aspect;
        pZoom = _zoom; pTargetZoom = _tzoom;
        fovyDeg = _fovyDeg;
    }

    static void installCallbacks() {
        glutKeyboardFunc(onKeyDown);
        glutKeyboardUpFunc(onKeyUp);

        glutSpecialFunc(onSpecialDown);
        glutSpecialUpFunc(onSpecialUp);

        glutMouseFunc(onMouseButton);
        glutMotionFunc(onMouseMove);
        glutPassiveMotionFunc(onMouseMove);
    }

    static void updateAimFromMouse() {
        // If your Shooting code already reads Input::mouseSX/mouseSY, leave this empty.
        // The important part is: mouseSX/mouseSY MUST update (fixed below).
    }

private:
    // Treat these as "constants" for int gameState
    static int HOME()    { return 0; }
    static int PLAYING() { return 1; }

    static bool uiPanelOpenBlockingGameInput() {
        // only block gameplay input when you're on HOME and a panel is open
        // menuUI.panel and MenuUI::NONE exist in your UI.cpp
        return (gameState == HOME() && menuUI.panel != MenuUI::NONE);
    }

    static void onKeyDown(unsigned char key, int, int) {
        // ESC
        if (key == 27) {
            if (gameState == PLAYING()) {
                gPaused = !gPaused;
            } else {
                // HOME: close panel if open
                if (menuUI.panel != MenuUI::NONE) menuUI.closePanel();
            }
            return;
        }

        // ENTER resumes when paused
        if (key == 13) {
            if (gameState == PLAYING() && gPaused) gPaused = false;
            return;
        }

        // if UI panel open, block movement/shoot keys
        if (uiPanelOpenBlockingGameInput()) return;

        // movement keys should work in HOME + PLAYING
        if (mv && p) mv->onKeyDown(key, *p);

        // shooting key (R)
        if (sh) {
            if (key == 'r' || key == 'R') sh->fireKeyR = true;
        }
    }

    static void onKeyUp(unsigned char key, int, int) {
        if (uiPanelOpenBlockingGameInput()) return;

        if (mv) mv->onKeyUp(key);

        if (sh) {
            if (key == 'r' || key == 'R') sh->fireKeyR = false;
        }
    }

    static void onSpecialDown(int key, int, int) {
        if (uiPanelOpenBlockingGameInput()) return;
        if (!mv) return;

        // arrow keys behave like WASD
        if (key == GLUT_KEY_LEFT)  mv->keyDown[(unsigned char)'a'] = true;
        if (key == GLUT_KEY_RIGHT) mv->keyDown[(unsigned char)'d'] = true;
        if (key == GLUT_KEY_UP)    mv->keyDown[(unsigned char)'w'] = true;
        if (key == GLUT_KEY_DOWN)  mv->keyDown[(unsigned char)'s'] = true;
    }

    static void onSpecialUp(int key, int, int) {
        if (uiPanelOpenBlockingGameInput()) return;
        if (!mv) return;

        if (key == GLUT_KEY_LEFT)  mv->keyDown[(unsigned char)'a'] = false;
        if (key == GLUT_KEY_RIGHT) mv->keyDown[(unsigned char)'d'] = false;
        if (key == GLUT_KEY_UP)    mv->keyDown[(unsigned char)'w'] = false;
        if (key == GLUT_KEY_DOWN)  mv->keyDown[(unsigned char)'s'] = false;
    }

    static void onMouseButton(int button, int state, int x, int y) {
        mouseSX = x;
        mouseSY = y;

        // ----- MENU CLICK (HOME) -----
        if (gameState == HOME() && state == GLUT_DOWN) {
            float mx, my;
            MenuUI::toUiMouse(mouseSX, mouseSY, *pW, *pH, mx, my);

            MenuUI::Action act = menuUI.click(mx, my);

            if (act == MenuUI::ACT_START_PLAY) { startPlaying(); glutPostRedisplay(); return; }
            if (act == MenuUI::ACT_OPEN_SETTINGS) { menuUI.openSettings(); glutPostRedisplay(); return; }
            if (act == MenuUI::ACT_OPEN_CUSTOMIZE) { menuUI.openCustomize(gPlayerColorIndex); glutPostRedisplay(); return; }
            if (act == MenuUI::ACT_OPEN_EXIT) { menuUI.openExit(); glutPostRedisplay(); return; }

            if (act == MenuUI::ACT_COLOR_PREV) { menuUI.colorPrev(); glutPostRedisplay(); return; }
            if (act == MenuUI::ACT_COLOR_NEXT) { menuUI.colorNext(); glutPostRedisplay(); return; }

            if (act == MenuUI::ACT_PANEL_OK) {
                gPlayerColorIndex = menuUI.previewColorIndex; // APPLY SKIN
                menuUI.closePanel();
                glutPostRedisplay();
                return;
            }

            if (act == MenuUI::ACT_PANEL_BACK) { menuUI.closePanel(); glutPostRedisplay(); return; }

            if (act == MenuUI::ACT_EXIT_YES) std::exit(0);
            if (act == MenuUI::ACT_EXIT_NO) { menuUI.closePanel(); glutPostRedisplay(); return; }

            return;
        }

        // ----- SHOOTING (PLAYING) -----
        if (gameState == PLAYING() && sh) {
            if (button == GLUT_LEFT_BUTTON) {
                sh->fireMouse = (state == GLUT_DOWN);
            }
        }
    }

    static void onMouseMove(int x, int y) {
        mouseSX = x;
        mouseSY = y;
        // forcing redraw makes hover + aim feel instant
        glutPostRedisplay();
    }
};

// ===== static storage =====
int Input::mouseSX = 0;
int Input::mouseSY = 0;

Player*   Input::p  = nullptr;
Movement* Input::mv = nullptr;
Shooting* Input::sh = nullptr;

int*   Input::pW = nullptr;
int*   Input::pH = nullptr;
float* Input::pAspect = nullptr;
float* Input::pZoom = nullptr;
float* Input::pTargetZoom = nullptr;
float  Input::fovyDeg = 60.0f;
