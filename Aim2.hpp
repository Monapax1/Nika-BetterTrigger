#pragma once
#include "LocalPlayer.hpp"
#include "Player.hpp"

struct Aim {
    HitboxType Hitbox = HitboxType::Neck;
    float FinalDistance = 0;
    float FinalFOV = 0;
    constexpr static float HipfireDistance = 60;

    MyDisplay* display;
    LocalPlayer* lp;
    std::vector<Player*>* players;
    ConfigLoader* cl;
    Player* CurrentTarget = nullptr;
    bool TargetSelected = true;

    Aim(MyDisplay* myDisplay, LocalPlayer* localPlayer, std::vector<Player*>* all_players, ConfigLoader* ConfigLoada)
        : display(myDisplay), lp(localPlayer), players(all_players), cl(ConfigLoada) {}

    bool active() {
        bool aimbotIsOn = cl->FEATURE_AIMBOT_ON;
        bool combatReady = lp->isCombatReady();
        bool activatedByAttack = cl->AIMBOT_ACTIVATED_BY_ATTACK && lp->inAttack;
        bool activatedByADS = cl->AIMBOT_ACTIVATED_BY_ADS && lp->inZoom;
        bool activatedByKey = cl->AIMBOT_ACTIVATED_BY_KEY && !cl->AIMBOT_ACTIVATION_KEY.empty() && display->keyDown(cl->AIMBOT_ACTIVATION_KEY);
        return aimbotIsOn && combatReady && (activatedByAttack || activatedByADS || activatedByKey);
    }

    void update(int counter) {
        if (lp->grippingGrenade || !active()) {
            ReleaseTarget();
            return;
        }

        if (lp->inZoom) {
            FinalFOV = cl->AIMBOT_FOV;
            FinalDistance = cl->AIMBOT_MAX_DISTANCE;
        } else {
            FinalFOV = cl->AIMBOT_FOV + 20;
            FinalDistance = HipfireDistance;
        }

        Player* Target = CurrentTarget;
        if (!IsValidTarget(Target)) {
            if (TargetSelected && !cl->AIMBOT_ALLOW_TARGET_SWITCH) return;
            Target = FindBestTarget();
            if (!IsValidTarget(Target)) {
                CurrentTarget = nullptr;
                return;
            }
            CurrentTarget = Target;
            CurrentTarget->IsLockedOn = true;
            TargetSelected = true;
        }

        double DistanceFromCrosshair = CalculateDistanceFromCrosshair(CurrentTarget);
        if (DistanceFromCrosshair > FinalFOV || DistanceFromCrosshair == -1) {
            ReleaseTarget();
            return;
        }
        StartAiming();
    }

    void StartAiming() {
        QAngle DesiredAngles;
        if (!GetAngle(CurrentTarget, DesiredAngles)) return;

        Vector2D DesiredAnglesIncrement(CalculatePitchIncrement(DesiredAngles), CalculateYawIncrement(DesiredAngles));
        float Extra = cl->AIMBOT_SMOOTH_EXTRA_BY_DISTANCE / CurrentTarget->distanceToLocalPlayer;
        float TotalSmooth = cl->AIMBOT_SMOOTH + Extra;

        Vector2D punchAnglesDiff = lp->punchAnglesDiff.Divide(cl->AIMBOT_SMOOTH).Multiply(cl->AIMBOT_SPEED);
        Vector2D aimbotDelta = DesiredAnglesIncrement.Divide(TotalSmooth).Multiply(cl->AIMBOT_SPEED);

        double totalPitchIncrement = aimbotDelta.x + punchAnglesDiff.x;
        double totalYawIncrement = -aimbotDelta.y + -punchAnglesDiff.y;

        int totalPitchIncrementInt = RoundHalfEven(AL1AF0(totalPitchIncrement));
        int totalYawIncrementInt = RoundHalfEven(AL1AF0(totalYawIncrement));

        if (totalPitchIncrementInt != 0 || totalYawIncrementInt != 0) {
            display->moveMouseRelative(totalPitchIncrementInt, totalYawIncrementInt);
        }
    }

    bool GetAngle(const Player* Target, QAngle& Angle) {
        const QAngle CurrentAngle(lp->viewAngles.x, lp->viewAngles.y).fixAngle();
        return CurrentAngle.isValid() && GetAngleToTarget(Target, Angle);
    }

    bool GetAngleToTarget(const Player* Target, QAngle& Angle) const {
        const Vector3D TargetPosition = Target->GetBonePosition(Hitbox);
        const Vector3D TargetVelocity = Target->AbsoluteVelocity;
        const Vector3D CameraPosition = lp->CameraPosition;

        if (lp->WeaponProjectileSpeed > 1.0f) {
            if (cl->AIMBOT_PREDICT_BULLETDROP && cl->AIMBOT_PREDICT_MOVEMENT) {
                return Resolver::CalculateAimRotationNew(CameraPosition, TargetPosition, TargetVelocity, lp->WeaponProjectileSpeed, lp->WeaponProjectileScale, 255, Angle);
            } else if (cl->AIMBOT_PREDICT_BULLETDROP) {
                return Resolver::CalculateAimRotationNew(CameraPosition, TargetPosition, Vector3D(0, 0, 0), lp->WeaponProjectileSpeed, lp->WeaponProjectileScale, 255, Angle);
            } else if (cl->AIMBOT_PREDICT_MOVEMENT) {
                return Resolver::CalculateAimRotation(CameraPosition, TargetPosition, TargetVelocity, lp->WeaponProjectileSpeed, Angle);
            }
        }

        Angle = Resolver::CalculateAngle(CameraPosition, TargetPosition);
        return true;
    }

    bool IsValidTarget(Player* target) const {
        return target != nullptr &&
            target->isValid() &&
            target->isCombatReady() &&
            target->visible &&
            target->enemy &&
            target->distance2DToLocalPlayer >= Conversion::ToGameUnits(cl->AIMBOT_MIN_DISTANCE) &&
            target->distance2DToLocalPlayer <= Conversion::ToGameUnits(FinalDistance);
    }

    float CalculatePitchIncrement(QAngle AimbotDesiredAngles) const {
        float wayA = AimbotDesiredAngles.x - lp->viewAngles.x;
        float wayB = 180 - abs(wayA);
        if (wayA > 0 && wayB > 0) wayB *= -1;
        return (fabs(wayA) < fabs(wayB)) ? wayA : wayB;
    }

    float CalculateYawIncrement(QAngle AimbotDesiredAngles) const {
        float wayA = AimbotDesiredAngles.y - lp->viewAngles.y;
        float wayB = 360 - abs(wayA);
        if (wayA > 0 && wayB > 0) wayB *= -1;
        return (fabs(wayA) < fabs(wayB)) ? wayA : wayB;
    }

    double CalculateDistanceFromCrosshair(Player* target) const {
        Vector3D CameraPosition = lp->CameraPosition;
        QAngle CurrentAngle(lp->viewAngles.x, lp->viewAngles.y).fixAngle();

        Vector3D TargetPos = target->localOrigin;
        if (CameraPosition.Distance(TargetPos) <= 0.0001f) return -1;

        QAngle TargetAngle = Resolver::CalculateAngle(CameraPosition, TargetPos);
        return TargetAngle.isValid() ? CurrentAngle.distanceTo(TargetAngle) : -1;
    }

    Player* FindBestTarget() const {
        float NearestDistance = 9999;
        Player* BestTarget = nullptr;

        for (Player* p : *players) {
            if (!IsValidTarget(p)) continue;

            double DistanceFromCrosshair = CalculateDistanceFromCrosshair(p);
            if (DistanceFromCrosshair > FinalFOV || DistanceFromCrosshair == -1) continue;

            if (DistanceFromCrosshair < NearestDistance) {
                BestTarget = p;
                NearestDistance = DistanceFromCrosshair;
            }
        }
        return BestTarget;
    }

    void ReleaseTarget() {
        if (CurrentTarget && CurrentTarget->isValid())
            CurrentTarget->IsLockedOn = false;

        TargetSelected = false;
        CurrentTarget = nullptr;
    }

    int RoundHalfEven(float x) const {
        return static_cast<int>(std::round(x >= 0.0 ? x : -x) * (x >= 0.0 ? 1 : -1));
    }

    float AL1AF0(float num) const {
        return (num > 0) ? std::max(num, 1.0f) : std::min(num, -1.0f);
    }
};
