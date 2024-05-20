#pragma once

struct NoRecoil {
    ConfigLoader* cl;
    MyDisplay* display;
    Level* map;
    LocalPlayer* lp;
    int noRecPitch;
    int noRecYaw;

    NoRecoil(ConfigLoader* cl, MyDisplay* myDisplay, Level* level, LocalPlayer* localPlayer) 
        : cl(cl), display(myDisplay), map(level), lp(localPlayer), noRecPitch(0), noRecYaw(0) {}

    void controlWeapon() {
        if (!cl || !display || !lp || !map) {
            // Überprüfen, ob die Zeiger gültig sind
            return;
        }

        if (!cl->FEATURE_NORECOIL_ON || !lp->isCombatReady() || !lp->inAttack) {
            // Überprüfen, ob die Funktion aktiviert ist und der Spieler kampfbereit ist
            return;
        }

        Vector2D punchAnglesDiff = lp->punchAnglesDiff;
        if (punchAnglesDiff.IsZeroVector()) {
            // Überprüfen, ob es Rückstoß gibt
            return;
        }

        // Reduzierung des Rückstoßes basierend auf den Konfigurationswerten
        noRecPitch = cl->NORECOIL_PITCH_REDUCTION; 
        noRecYaw = cl->NORECOIL_YAW_REDUCTION; 

        // Berechnung der zu bewegenden Maus
        int pitch = (punchAnglesDiff.x > 0)
            ? roundHalfEven(punchAnglesDiff.x * noRecPitch)
            : 0;
        int yaw = roundHalfEven(-punchAnglesDiff.y * noRecYaw);

        // Bewegung der Maus relativ zum aktuellen Standpunkt
        display->moveMouseRelative(pitch, yaw);
    }

    int roundHalfEven(float x) {
        return (x >= 0.0)
            ? static_cast<int>(std::round(x))
            : static_cast<int>(std::round(-x)) * -1;
    }
};
