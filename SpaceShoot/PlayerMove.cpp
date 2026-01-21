
#include <cmath>

class PlayerMove {
public:
    float phase = 0.0f;
    // Idle float (smooth bob)


    // smooth helper
    static float smoothTo(float cur, float target, float alpha) {
        return cur + (target - cur) * alpha;
    }

    void update(Player &p, const Movement &m) {
    // speed for animation
    float v = std::sqrt(m.dx*m.dx + m.dy*m.dy);

    // targets
    float tArmSwing = 0.0f, tLegSwing = 0.0f;
    float tArmLift  = 0.0f, tLegLift  = 0.0f;
    float tSpread   = 0.0f, tLean     = 0.0f, tCrouch = 0.0f;

    // choose animation state
    if (m.inAir) {
        // JUMP: spread + arms up
        tSpread   = 0.09f;
        tArmLift  = 0.05f;
        tLegLift  = 0.03f;
        tArmSwing = 0.04f;
        tLegSwing = 0.04f;
    }
    else if (m.dy > 0.0001f) {
        // CLIMB UP
        phase += 0.22f;
        tArmLift  = 0.06f + 0.02f * std::sin(phase);
        tLegLift  = 0.03f + 0.02f * std::sin(phase + 3.14159f);
        tArmSwing = 0.05f * std::sin(phase);
        tLegSwing = 0.03f * std::sin(phase + 3.14159f);
    }
    else if (m.dy < -0.0001f) {
        // GO DOWN
        phase += 0.18f;
        tCrouch   = 0.06f + 0.01f * std::sin(phase);
        tArmSwing = 0.02f * std::sin(phase);
        tLegSwing = 0.02f * std::sin(phase + 3.14159f);
    }
    else if (std::fabs(m.dx) > 0.0001f) {
        // WALK LEFT/RIGHT
        phase += 0.20f + v * 8.0f;
        tArmSwing = 0.045f * std::sin(phase);
        tLegSwing = 0.035f * std::sin(phase + 3.14159f);
        tLean     = (m.dx > 0 ? 0.03f : -0.03f);
    }
    else {
        // IDLE: tiny breathing + FLOATING
        phase += 0.04f;
        tArmSwing = 0.008f * std::sin(phase);
        tLegSwing = 0.006f * std::sin(phase + 3.14159f);

        // ✅ Idle float bob target
        float targetBob = 0.02f * std::sin(phase);
        p.bob = smoothTo(p.bob, targetBob, 0.10f);
    }

    // ✅ If NOT idle, smoothly bring bob back to 0
    if (m.inAir || std::fabs(m.dx) > 0.0001f || std::fabs(m.dy) > 0.0001f) {
        p.bob = smoothTo(p.bob, 0.0f, 0.18f);
    }

    // smooth transitions (alpha controls smoothness)
    float aFast = 0.22f;
    float aSlow = 0.12f;
    // smoothly decay shooting kick back to 0
p.shotKick = smoothTo(p.shotKick, 0.0f, 0.25f);



    p.armSwing = smoothTo(p.armSwing, tArmSwing, aFast);
    p.legSwing = smoothTo(p.legSwing, tLegSwing, aFast);

    p.armLift  = smoothTo(p.armLift,  tArmLift,  aSlow);
    p.legLift  = smoothTo(p.legLift,  tLegLift,  aSlow);

    p.spread   = smoothTo(p.spread,   tSpread,   aSlow);
    p.lean     = smoothTo(p.lean,     tLean,     aSlow);
    p.crouch   = smoothTo(p.crouch,   tCrouch,   aSlow);
}

};
