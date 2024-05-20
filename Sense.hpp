#pragma once
#include <vector>

struct Sense {
    ConfigLoader* cl;
    Level* map;
    LocalPlayer* lp;
    std::vector<Player*>& players;

    Sense(ConfigLoader* configLoader, Level* level, LocalPlayer* localPlayer, std::vector<Player*>& all_players)
        : cl(configLoader), map(level), lp(localPlayer), players(all_players) {}

    void update(int counter) {
        if (!map->playable || !cl->FEATURE_SENSE_ON)
            return;

        for (Player* p : players) {
            if (!p->isValid() || p->friendly)
                continue;

            double distance = math::calculateDistanceInMeters(
                lp->localOrigin.x, lp->localOrigin.y, lp->localOrigin.z,
                p->localOrigin.x, p->localOrigin.y, p->localOrigin.z);

            if (distance < cl->SENSE_MAXRANGE) {
                p->setGlowEnable(1);
                p->setGlowThroughWall(1);
                if (!p->visible && !p->knocked) {
                    int healthShield = p->currentHealth + p->currentShields;
                    p->setCustomGlow(healthShield, true, false);
                } else {
                    p->setCustomGlow(0, false, false);
                }
            } else if (p->getGlowEnable() == 1 && p->getGlowThroughWall() == 1) {
                p->setGlowEnable(0);
                p->setGlowThroughWall(0);
            }
        }
    }

    void itemGlow(int counter) {
        if (!map->playable || !cl->FEATURE_ITEM_GLOW_ON)
            return;

        const GlowMode newGlowMode = { 137, 0, 0, 127 };

        for (int highlightId = 15; highlightId < 65; ++highlightId) {
            const GlowMode oldGlowMode = mem::Read<GlowMode>(lp->highlightSettingsPtr + (HIGHLIGHT_TYPE_SIZE * highlightId) + 0, "Player oldGlowMode");
            if (newGlowMode != oldGlowMode)
                mem::Write<GlowMode>(lp->highlightSettingsPtr + (HIGHLIGHT_TYPE_SIZE * highlightId) + 0, newGlowMode);
        }
    }
};
