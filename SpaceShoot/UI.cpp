#include <GL/glut.h>
#include <cmath>
#include <algorithm>

// This function is implemented in Player.cpp (we added it earlier)
extern void drawPlayerPreviewAt(float cx, float cy, float scale, int colorIndex);

class MenuUI {
public:
    enum Panel { NONE, SETTINGS, CUSTOMIZE, EXIT_CONFIRM };

    Panel panel = NONE;

    // In CUSTOMIZE, this changes with arrows.
    int previewColorIndex = 0;   // 0..9

    struct Rect { float cx, cy, w, h; };

    // home buttons
    Rect playBtn{};
    Rect exitBtn{};
    Rect settingsBtn{};
    Rect customizeBtn{};

    // panel buttons
    Rect panelOk{};
    Rect panelBack{};
    Rect panelYes{};
    Rect panelNo{};
    Rect arrowLeft{};
    Rect arrowRight{};

    // actions returned to Input
    enum Action {
        ACT_NONE,
        ACT_START_PLAY,
        ACT_OPEN_SETTINGS,
        ACT_OPEN_CUSTOMIZE,
        ACT_OPEN_EXIT,
        ACT_PANEL_BACK,
        ACT_PANEL_OK,
        ACT_EXIT_YES,
        ACT_EXIT_NO,
        ACT_COLOR_PREV,
        ACT_COLOR_NEXT
    };

    void layout(int, int) {
        // UI coordinate system: -1..1

        // bottom center play (square)
        playBtn = { 0.0f, -0.62f, 0.24f, 0.24f };

        // top-right vertical stack: Exit, Settings, Customize
        float x = 0.82f;
        exitBtn      = { x,  0.82f, 0.14f, 0.14f };
        settingsBtn  = { x,  0.62f, 0.14f, 0.14f };
        customizeBtn = { x,  0.42f, 0.14f, 0.14f };

        // common panel buttons
        panelBack = { -0.20f, -0.58f, 0.26f, 0.14f };
        panelOk   = {  0.20f, -0.58f, 0.26f, 0.14f };

        panelYes  = { -0.18f, -0.18f, 0.26f, 0.14f };
        panelNo   = {  0.18f, -0.18f, 0.26f, 0.14f };

        // customize arrows around center
        arrowLeft  = { -0.46f, 0.02f, 0.14f, 0.14f };
        arrowRight = {  0.46f, 0.02f, 0.14f, 0.14f };
    }

    static void toUiMouse(int sx, int sy, int w, int h, float &mx, float &my) {
        mx = (2.0f * sx / (float)w) - 1.0f;
        my = 1.0f - (2.0f * sy / (float)h);
    }

    Action click(float mx, float my) {
        if (panel == NONE) {
            if (hit(playBtn, mx, my)) return ACT_START_PLAY;
            if (hit(exitBtn, mx, my)) return ACT_OPEN_EXIT;
            if (hit(settingsBtn, mx, my)) return ACT_OPEN_SETTINGS;
            if (hit(customizeBtn, mx, my)) return ACT_OPEN_CUSTOMIZE;
            return ACT_NONE;
        }

        if (panel == SETTINGS) {
            if (hit(panelBack, mx, my)) return ACT_PANEL_BACK;
            if (hit(panelOk, mx, my))   return ACT_PANEL_BACK; // OK same as back for now
            return ACT_NONE;
        }

        if (panel == CUSTOMIZE) {
            if (hit(arrowLeft, mx, my))  return ACT_COLOR_PREV;
            if (hit(arrowRight, mx, my)) return ACT_COLOR_NEXT;
            if (hit(panelBack, mx, my))  return ACT_PANEL_BACK;
            if (hit(panelOk, mx, my))    return ACT_PANEL_OK;
            return ACT_NONE;
        }

        if (panel == EXIT_CONFIRM) {
            if (hit(panelYes, mx, my)) return ACT_EXIT_YES;
            if (hit(panelNo, mx, my))  return ACT_EXIT_NO;
            return ACT_NONE;
        }

        return ACT_NONE;
    }

    void openSettings() { panel = SETTINGS; }
    void openExit()     { panel = EXIT_CONFIRM; }
    void closePanel()   { panel = NONE; }

    void openCustomize(int currentColorIndex) {
        panel = CUSTOMIZE;
        previewColorIndex = currentColorIndex;
    }

    void colorPrev() { previewColorIndex = (previewColorIndex + 9) % 10; }
    void colorNext() { previewColorIndex = (previewColorIndex + 1) % 10; }

    int getPreviewColorIndex() const { return previewColorIndex; }

    // draw entire HOME overlay (buttons + any open panel)
    void drawHome(int w, int h, float mx, float my) {
        (void)w; (void)h;

        beginOrtho();

        // main buttons (only visible in HOME)
        drawPlayButton(mx, my);
        drawIconButtons(mx, my);

        // panels
        if (panel == SETTINGS)      drawSettingsPanel(mx, my);
        if (panel == CUSTOMIZE)     drawCustomizePanel(mx, my);
        if (panel == EXIT_CONFIRM)  drawExitConfirmPanel(mx, my);

        endOrtho();
    }

private:
    static bool hit(const Rect &r, float mx, float my) {
        return (mx >= r.cx - r.w*0.5f && mx <= r.cx + r.w*0.5f &&
                my >= r.cy - r.h*0.5f && my <= r.cy + r.h*0.5f);
    }

    static float clamp01(float t){ return std::max(0.0f, std::min(1.0f, t)); }

    static float hoverT(const Rect &r, float mx, float my) {
        float dx = (mx - r.cx) / (r.w*0.5f);
        float dy = (my - r.cy) / (r.h*0.5f);
        float d = std::sqrt(dx*dx + dy*dy);
        return clamp01(1.2f - d);
    }

    static void beginOrtho() {
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
    }

    static void endOrtho() {
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    static void drawRectFill(const Rect &r) {
        glBegin(GL_QUADS);
        glVertex2f(r.cx - r.w*0.5f, r.cy + r.h*0.5f);
        glVertex2f(r.cx + r.w*0.5f, r.cy + r.h*0.5f);
        glVertex2f(r.cx + r.w*0.5f, r.cy - r.h*0.5f);
        glVertex2f(r.cx - r.w*0.5f, r.cy - r.h*0.5f);
        glEnd();
    }

    static void drawRectBorder(const Rect &r) {
        glBegin(GL_LINE_LOOP);
        glVertex2f(r.cx - r.w*0.5f, r.cy + r.h*0.5f);
        glVertex2f(r.cx + r.w*0.5f, r.cy + r.h*0.5f);
        glVertex2f(r.cx + r.w*0.5f, r.cy - r.h*0.5f);
        glVertex2f(r.cx - r.w*0.5f, r.cy - r.h*0.5f);
        glEnd();
    }

    static void drawText(float x, float y, void* font, const char* s) {
        glRasterPos2f(x, y);
        for (const char* p = s; *p; ++p) glutBitmapCharacter(font, *p);
    }

    static void drawTriPlay(float cx, float cy, float s) {
        glBegin(GL_TRIANGLES);
        glVertex2f(cx - 0.03f, cy + s*0.20f);
        glVertex2f(cx - 0.03f, cy - s*0.20f);
        glVertex2f(cx + s*0.22f, cy);
        glEnd();
    }

    void drawPlayButton(float mx, float my) {
        float ht = hoverT(playBtn, mx, my);

        // blue shade base
        glColor4f(0.10f, 0.40f, 0.95f, 0.85f);
        drawRectFill(playBtn);

        // glow border
        glLineWidth(2.0f);
        glColor4f(0.60f, 0.85f, 1.0f, 0.20f + 0.45f*ht);
        drawRectBorder(playBtn);
        glLineWidth(1.0f);

        // white triangle in middle
        glColor4f(1,1,1,0.95f);
        drawTriPlay(playBtn.cx, playBtn.cy, playBtn.w);
    }

    void drawIconButtons(float mx, float my) {
        drawExitIcon(exitBtn, mx, my);
        drawSettingsIcon(settingsBtn, mx, my);
        drawCustomizeIcon(customizeBtn, mx, my);
    }

    void drawBlueIconBase(const Rect &r, float mx, float my) {
        float ht = hoverT(r, mx, my);
        glColor4f(0.08f, 0.30f, 0.85f, 0.82f);
        drawRectFill(r);

        glLineWidth(2.0f);
        glColor4f(0.75f, 0.92f, 1.0f, 0.20f + 0.50f*ht);
        drawRectBorder(r);
        glLineWidth(1.0f);
    }

    void drawExitIcon(const Rect &r, float mx, float my) {
        float ht = hoverT(r, mx, my);

        glColor4f(0.85f, 0.10f, 0.18f, 0.88f);
        drawRectFill(r);

        glLineWidth(2.0f);
        glColor4f(1.0f, 0.35f, 0.45f, 0.25f + 0.50f*ht);
        drawRectBorder(r);
        glLineWidth(1.0f);

        glColor4f(1,1,1,0.95f);
        glBegin(GL_LINES);
        glVertex2f(r.cx - 0.035f, r.cy - 0.035f);
        glVertex2f(r.cx + 0.035f, r.cy + 0.035f);
        glVertex2f(r.cx - 0.035f, r.cy + 0.035f);
        glVertex2f(r.cx + 0.035f, r.cy - 0.035f);
        glEnd();
    }

    void drawSettingsIcon(const Rect &r, float mx, float my) {
        drawBlueIconBase(r, mx, my);

        glColor4f(1,1,1,0.95f);
        glPushMatrix();
        glTranslatef(r.cx, r.cy, 0);

        float rr = 0.035f;
        glBegin(GL_LINE_LOOP);
        for (int i=0;i<24;i++){
            float a = 2.0f*3.1415926f*i/24.0f;
            glVertex2f(std::cos(a)*rr, std::sin(a)*rr);
        }
        glEnd();

        glBegin(GL_LINES);
        for (int i=0;i<6;i++){
            float a = 2.0f*3.1415926f*i/6.0f;
            glVertex2f(std::cos(a)*rr, std::sin(a)*rr);
            glVertex2f(std::cos(a)*(rr*1.25f), std::sin(a)*(rr*1.25f));
        }
        glEnd();

        glPopMatrix();
    }

    void drawCustomizeIcon(const Rect &r, float mx, float my) {
        drawBlueIconBase(r, mx, my);

        glColor4f(1,1,1,0.95f);
        glPushMatrix();
        glTranslatef(r.cx, r.cy, 0);

        // shirt body
        glBegin(GL_QUADS);
        glVertex2f(-0.030f,  0.020f);
        glVertex2f( 0.030f,  0.020f);
        glVertex2f( 0.030f, -0.040f);
        glVertex2f(-0.030f, -0.040f);
        glEnd();

        // sleeves
        glBegin(GL_QUADS);
        glVertex2f(-0.055f,  0.020f);
        glVertex2f(-0.030f,  0.020f);
        glVertex2f(-0.030f, -0.010f);
        glVertex2f(-0.055f, -0.010f);

        glVertex2f( 0.030f,  0.020f);
        glVertex2f( 0.055f,  0.020f);
        glVertex2f( 0.055f, -0.010f);
        glVertex2f( 0.030f, -0.010f);
        glEnd();

        glPopMatrix();
    }

    void drawPanelFrame(float w, float h) {
        Rect p{0.0f, 0.0f, w, h};
        glColor4f(0.04f, 0.12f, 0.30f, 0.80f);
        drawRectFill(p);

        glLineWidth(2.0f);
        glColor4f(0.55f, 0.85f, 1.0f, 0.35f);
        drawRectBorder(p);
        glLineWidth(1.0f);
    }

    void drawOkBackButtons(float mx, float my) {
        float htB = hoverT(panelBack, mx, my);
        float htO = hoverT(panelOk, mx, my);

        // back
        glColor4f(0.08f, 0.30f, 0.85f, 0.82f);
        drawRectFill(panelBack);
        glColor4f(0.75f,0.92f,1.0f, 0.20f + 0.55f*htB);
        glLineWidth(2); drawRectBorder(panelBack); glLineWidth(1);

        // ok
        glColor4f(0.08f, 0.30f, 0.85f, 0.82f);
        drawRectFill(panelOk);
        glColor4f(0.75f,0.92f,1.0f, 0.20f + 0.55f*htO);
        glLineWidth(2); drawRectBorder(panelOk); glLineWidth(1);

        glColor4f(1,1,1,0.95f);
        drawText(panelBack.cx - 0.05f, panelBack.cy - 0.01f, GLUT_BITMAP_HELVETICA_18, "BACK");
        drawText(panelOk.cx   - 0.03f, panelOk.cy   - 0.01f, GLUT_BITMAP_HELVETICA_18, "OK");
    }

    void drawSettingsPanel(float mx, float my) {
        drawPanelFrame(1.25f, 1.35f);

        glColor4f(0.75f, 0.92f, 1.0f, 0.90f);
        drawText(-0.12f, 0.55f, GLUT_BITMAP_TIMES_ROMAN_24, "SETTINGS");

        glColor4f(1,1,1,0.75f);
        drawText(-0.42f, 0.25f, GLUT_BITMAP_HELVETICA_18, "Sound:");
        drawText(-0.42f, 0.10f, GLUT_BITMAP_HELVETICA_18, "Controls:");
        drawText(-0.42f,-0.05f, GLUT_BITMAP_HELVETICA_18, "Graphics:");

        drawOkBackButtons(mx, my);
    }

    void drawArrowButton(const Rect &r, float mx, float my, bool left) {
        float ht = hoverT(r, mx, my);

        glColor4f(0.08f, 0.30f, 0.85f, 0.80f);
        drawRectFill(r);

        glColor4f(0.75f,0.92f,1.0f, 0.20f + 0.55f*ht);
        glLineWidth(2); drawRectBorder(r); glLineWidth(1);

        glColor4f(1,1,1,0.95f);
        if (left) {
            glBegin(GL_TRIANGLES);
            glVertex2f(r.cx - 0.030f, r.cy);
            glVertex2f(r.cx + 0.020f, r.cy + 0.030f);
            glVertex2f(r.cx + 0.020f, r.cy - 0.030f);
            glEnd();
        } else {
            glBegin(GL_TRIANGLES);
            glVertex2f(r.cx + 0.030f, r.cy);
            glVertex2f(r.cx - 0.020f, r.cy + 0.030f);
            glVertex2f(r.cx - 0.020f, r.cy - 0.030f);
            glEnd();
        }
    }

    void drawCustomizePanel(float mx, float my) {
        drawPanelFrame(1.25f, 1.35f);

        glColor4f(0.75f, 0.92f, 1.0f, 0.90f);
        drawText(-0.16f, 0.55f, GLUT_BITMAP_TIMES_ROMAN_24, "CUSTOMIZE");

        drawArrowButton(arrowLeft, mx, my, true);
        drawArrowButton(arrowRight, mx, my, false);

        // player preview in center
        drawPlayerPreviewAt(0.0f, -0.05f, 1.8f, previewColorIndex);

        glColor4f(1,1,1,0.70f);
        drawText(-0.34f, -0.40f, GLUT_BITMAP_HELVETICA_18, "Choose color with arrows");

        drawOkBackButtons(mx, my);
    }

    void drawExitConfirmPanel(float mx, float my) {
        drawPanelFrame(1.10f, 0.75f);

        glColor4f(0.95f, 0.95f, 1.0f, 0.92f);
        drawText(-0.46f, 0.12f, GLUT_BITMAP_HELVETICA_18, "ARE YOU SURE YOU WANT TO EXIT?");

        float htY = hoverT(panelYes, mx, my);
        float htN = hoverT(panelNo, mx, my);

        // yes
        glColor4f(0.85f, 0.10f, 0.18f, 0.90f);
        drawRectFill(panelYes);
        glColor4f(1.0f,0.35f,0.45f, 0.20f + 0.55f*htY);
        glLineWidth(2); drawRectBorder(panelYes); glLineWidth(1);
        glColor4f(1,1,1,0.95f);
        drawText(panelYes.cx - 0.03f, panelYes.cy - 0.01f, GLUT_BITMAP_HELVETICA_18, "YES");

        // no
        glColor4f(0.08f, 0.30f, 0.85f, 0.90f);
        drawRectFill(panelNo);
        glColor4f(0.75f,0.92f,1.0f, 0.20f + 0.55f*htN);
        glLineWidth(2); drawRectBorder(panelNo); glLineWidth(1);
        glColor4f(1,1,1,0.95f);
        drawText(panelNo.cx - 0.02f, panelNo.cy - 0.01f, GLUT_BITMAP_HELVETICA_18, "NO");
    }
};

// global instance
MenuUI menuUI;
