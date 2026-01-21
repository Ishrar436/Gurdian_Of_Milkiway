
#include <windows.h>
#include <mmsystem.h>
#include <string>
#pragma comment(lib, "winmm.lib")

class Audio {
public:
    // --- change names here if you want ---
    static inline std::string DIR = "Audio/";

    static inline std::string HOME_BGM   = "home.mp3";
    static inline std::string GAME_BGM   = "game.mp3";

    static inline std::string SFX_MOVE   = "move.wav";
    static inline std::string SFX_SHOOT  = "shoot.wav";
    static inline std::string SFX_ENEMY  = "enemyhit.wav";
    static inline std::string SFX_PLAYER = "playerhit.wav";

    static void init(const std::string& folder) {
        DIR = folder;
        if (!DIR.empty() && DIR.back() != '/' && DIR.back() != '\\') DIR += "/";
    }

    // --------- BGM ----------
    static void playHomeBgm() { playBgm("bgm", DIR + HOME_BGM); }
    static void playGameBgm() { playBgm("bgm", DIR + GAME_BGM); }

    static void stopBgm()  { send("stop bgm"); send("close bgm"); bgmOpen = false; bgmPaused = false; }
    static void pauseBgm() { if (bgmOpen) { send("pause bgm"); bgmPaused = true; } }
    static void resumeBgm(){ if (bgmOpen && bgmPaused) { send("resume bgm"); bgmPaused = false; } }

    // --------- LOOP SFX ----------
    static void setMoveLoop(bool on)  { setLoop("move", DIR + SFX_MOVE, on, moveLoopOn); }
    static void setShootLoop(bool on) { setLoop("shoot", DIR + SFX_SHOOT, on, shootLoopOn); }

    static void stopAllLoops() {
        if (moveLoopOn)  setMoveLoop(false);
        if (shootLoopOn) setShootLoop(false);
    }

    // --------- ONE SHOT ----------
    static void enemyHit()  { oneShot("enemy",  DIR + SFX_ENEMY); }
    static void playerHit() { oneShot("player", DIR + SFX_PLAYER); }

private:
    static inline bool bgmOpen = false;
    static inline bool bgmPaused = false;

    static inline bool moveOpen = false;
    static inline bool shootOpen = false;

    static inline bool moveLoopOn = false;
    static inline bool shootLoopOn = false;

    static void send(const std::string& cmd) {
        mciSendStringA(cmd.c_str(), nullptr, 0, nullptr);
    }

    static void openAlias(const std::string& alias, const std::string& path) {
        // mpegvideo handles mp3/wav in most Windows installs
        std::string cmd = "open \"" + path + "\" type mpegvideo alias " + alias;
        send(cmd);
    }

    static void playBgm(const std::string& alias, const std::string& path) {
        // close previous
        send("stop " + alias);
        send("close " + alias);

        openAlias(alias, path);
        bgmOpen = true;
        bgmPaused = false;

        send("play " + alias + " repeat");
    }

    static void setLoop(const std::string& alias, const std::string& path,
                        bool wantOn, bool &stateFlag)
    {
        if (wantOn && !stateFlag) {
            // start
            send("stop " + alias);
            send("close " + alias);
            openAlias(alias, path);
            send("play " + alias + " repeat");
            stateFlag = true;
        }
        else if (!wantOn && stateFlag) {
            // stop
            send("stop " + alias);
            send("close " + alias);
            stateFlag = false;
        }
    }

    static void oneShot(const std::string& alias, const std::string& path) {
        // restart same sound quickly
        send("stop " + alias);
        send("close " + alias);
        openAlias(alias, path);
        send("play " + alias);
    }
};
