#include "OCHUD.h"

#include "OCCharacter.h"
#include "OCCapturePoint.h"
#include "OCGameState.h"
#include "OCHealthComponent.h"
#include "OCPlayerController.h"
#include "OCPlayerState.h"
#include "OCPlayerUserSettings.h"
#include "OCCharacterVisualTypes.h"
#include "OCWeaponBase.h"
#include "OCVehicleBase.h"
#include "OCArmedVehicleBase.h"
#include "OCOrdnanceTypes.h"
#include "OCLobbyTypes.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"

void AOCHUD::DrawHUD()
{
    Super::DrawHUD();

    if (!Canvas)
    {
        return;
    }

    const UOCPlayerUserSettings* UserPrefs = UOCPlayerUserSettings::Get();
    if (UserPrefs && UserPrefs->bShowFPS && GetWorld())
    {
        const float Dt = FMath::Max(GetWorld()->GetDeltaSeconds(), 0.0001f);
        const FString FPSText = FString::Printf(TEXT("FPS %d"), FMath::RoundToInt(1.0f / Dt));
        DrawText(FPSText, FLinearColor(0.75f,0.75f,0.75f,1.0f), Canvas->ClipX - 120.0f, 46.0f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 0.85f, false);
    }

    const AOCPlayerController* PC = Cast<AOCPlayerController>(GetOwningPlayerController());
    if (PC && PC->IsDeploymentPanelVisible())
    {
        if (!PC->HasRichUI())
        {
            DrawDeploymentPanel(PC);
            DrawChat(PC);
        }
        return;
    }
    if (PC && PC->IsScoreboardVisible())
    {
        if (!PC->HasRichUI()) DrawScoreboard();
        return;
    }
    if (PC && PC->IsAdminPanelVisible())
    {
        if (!PC->HasRichUI()) DrawSandboxAdminPanel(PC);
        return;
    }

    DrawMatchStatus();
    if (PC)
    {
        if (!PC->HasRichUI()) DrawChat(PC);
        DrawSquadOrder(PC);
    }
    const AOCGameState* MatchState = GetWorld() ? GetWorld()->GetGameState<AOCGameState>() : nullptr;
    if (MatchState && MatchState->GetOCMatchPhase() == EOCMatchPhase::Ended)
    {
        return;
    }

    if (PC && Cast<AOCVehicleBase>(PC->GetPawn()))
    {
        DrawVehicleHUD();
        return;
    }

    const AOCCharacter* Character = PC ? Cast<AOCCharacter>(PC->GetPawn()) : nullptr;
    if (!Character)
    {
        return;
    }

    if (Character->IsVehicleGunner())
    {
        DrawGunnerHUD(Character);
        return;
    }

    const float CenterX = Canvas->ClipX * 0.5f;
    const float CenterY = Canvas->ClipY * 0.5f;

    if (Character->IsDowned())
    {
        const float Remaining = Character->GetDownedTimeRemaining();
        const int32 RemainingSeconds = FMath::Max(0, FMath::CeilToInt(Remaining));
        const FString TimeText = FString::Printf(TEXT("BLEED OUT  %02d:%02d"), RemainingSeconds / 60, RemainingSeconds % 60);

        DrawRect(FLinearColor(0.18f, 0.0f, 0.0f, 0.12f), 0.0f, 0.0f, Canvas->ClipX, Canvas->ClipY);
        DrawText(TEXT("DOWNED"), FLinearColor(0.95f, 0.25f, 0.22f, 1.0f), CenterX - 52.0f, CenterY - 82.0f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 1.35f, false);
        DrawText(TimeText, FLinearColor::White, CenterX - 78.0f, CenterY - 48.0f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 1.0f, false);
        DrawText(TEXT("WAIT FOR MEDIC  |  CRAWL TO COVER"), FLinearColor(0.82f, 0.84f, 0.86f, 1.0f),
            CenterX - 148.0f, CenterY + 4.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.9f, false);
        DrawText(TEXT("HOLD SPACE TO GIVE UP"), FLinearColor(0.72f, 0.72f, 0.72f, 1.0f),
            CenterX - 103.0f, CenterY + 34.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.86f, false);

        const float GiveUpProgress = Character->GetGiveUpProgress();
        if (GiveUpProgress > KINDA_SMALL_NUMBER)
        {
            const float BarWidth = 240.0f;
            const float BarHeight = 8.0f;
            const float BarX = CenterX - BarWidth * 0.5f;
            const float BarY = CenterY + 62.0f;
            DrawRect(FLinearColor(0.08f, 0.08f, 0.08f, 0.85f), BarX, BarY, BarWidth, BarHeight);
            DrawRect(FLinearColor(0.78f, 0.18f, 0.16f, 0.95f), BarX, BarY, BarWidth * GiveUpProgress, BarHeight);
        }

        if (UOCPlayerUserSettings::Get()->bShowPing)
        {
            if (const AOCPlayerState* LocalState = PC->GetPlayerState<AOCPlayerState>())
            {
                const FString PingText = FString::Printf(TEXT("PING %d ms"), LocalState->GetPingMs());
                DrawText(PingText, FLinearColor(0.75f, 0.75f, 0.75f, 1.0f), Canvas->ClipX - 120.0f, 24.0f,
                    GEngine ? GEngine->GetSmallFont() : nullptr, 0.85f, false);
            }
        }
        return;
    }

    const float Gap = Character->GetCrosshairGap();
    const float Length = Character->IsAiming() ? 6.0f : 8.0f;
    const FLinearColor CrosshairColor = FLinearColor::White;

    if (!UserPrefs || UserPrefs->bShowCrosshair)
    {
        DrawLine(CenterX - Gap - Length, CenterY, CenterX - Gap, CenterY, CrosshairColor, 1.4f);
        DrawLine(CenterX + Gap, CenterY, CenterX + Gap + Length, CenterY, CrosshairColor, 1.4f);
        DrawLine(CenterX, CenterY - Gap - Length, CenterX, CenterY - Gap, CrosshairColor, 1.4f);
        DrawLine(CenterX, CenterY + Gap, CenterX, CenterY + Gap + Length, CrosshairColor, 1.4f);
    }

    const float HitAlpha = Character->GetHitMarkerAlpha();
    if ((!UserPrefs || UserPrefs->bShowHitMarker) && HitAlpha > KINDA_SMALL_NUMBER)
    {
        const float HitGap = 7.0f;
        const float HitLength = 7.0f;
        const FLinearColor HitColor = Character->WasLastHitFatal()
            ? FLinearColor(1.0f, 0.18f, 0.18f, HitAlpha)
            : FLinearColor(1.0f, 1.0f, 1.0f, HitAlpha);

        DrawLine(CenterX - HitGap - HitLength, CenterY - HitGap - HitLength,
            CenterX - HitGap, CenterY - HitGap, HitColor, 2.0f);
        DrawLine(CenterX + HitGap, CenterY - HitGap,
            CenterX + HitGap + HitLength, CenterY - HitGap - HitLength, HitColor, 2.0f);
        DrawLine(CenterX - HitGap - HitLength, CenterY + HitGap + HitLength,
            CenterX - HitGap, CenterY + HitGap, HitColor, 2.0f);
        DrawLine(CenterX + HitGap, CenterY + HitGap,
            CenterX + HitGap + HitLength, CenterY + HitGap + HitLength, HitColor, 2.0f);
    }

    const float DamageDirectionAlpha = Character->GetDamageIndicatorAlpha();
    if (DamageDirectionAlpha > KINDA_SMALL_NUMBER)
    {
        const float AngleRadians = FMath::DegreesToRadians(Character->GetLastDamageDirectionDegrees());
        const float Radius = 86.0f;
        const FVector2D Direction(FMath::Sin(AngleRadians), -FMath::Cos(AngleRadians));
        const FVector2D Tangent(-Direction.Y, Direction.X);
        const FVector2D Tip(CenterX + Direction.X * Radius, CenterY + Direction.Y * Radius);
        const FVector2D Base = Tip - Direction * 13.0f;
        const FVector2D Left = Base + Tangent * 7.0f;
        const FVector2D Right = Base - Tangent * 7.0f;
        const FLinearColor DamageIndicatorColor(0.9f, 0.02f, 0.02f, DamageDirectionAlpha * 0.9f);

        DrawLine(Left.X, Left.Y, Tip.X, Tip.Y, DamageIndicatorColor, 3.0f);
        DrawLine(Tip.X, Tip.Y, Right.X, Right.Y, DamageIndicatorColor, 3.0f);
    }

    if (const AOCWeaponBase* Weapon = Character->GetCurrentWeapon())
    {
        DrawText(Weapon->GetWeaponDisplayName(), FLinearColor::White, Canvas->ClipX - 220.0f, Canvas->ClipY - 126.0f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 1.0f, false);

        const FString AmmoText = FString::Printf(TEXT("%d / %d"), Weapon->GetAmmoInMagazine(), Weapon->GetReserveAmmo());
        DrawText(AmmoText, FLinearColor::White, Canvas->ClipX - 155.0f, Canvas->ClipY - 76.0f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 1.25f, false);

        const FString FireModeText = Weapon->GetCurrentFireMode() == EOCFireMode::Automatic ? TEXT("AUTO") : TEXT("SEMI");
        DrawText(FireModeText, FLinearColor(0.82f, 0.82f, 0.82f, 1.0f), Canvas->ClipX - 155.0f, Canvas->ClipY - 101.0f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 0.95f, false);

        if (Weapon->IsReloading())
        {
            DrawText(TEXT("RELOAD"), FLinearColor(0.95f, 0.78f, 0.25f, 1.0f), CenterX - 31.0f, CenterY + 42.0f,
                GEngine ? GEngine->GetSmallFont() : nullptr, 1.0f, false);
        }

        const FString Attachments = Weapon->GetAttachmentSummary();
        if (!Attachments.IsEmpty())
        {
            DrawText(Attachments, FLinearColor(0.62f, 0.66f, 0.70f, 1.0f), Canvas->ClipX - 220.0f, Canvas->ClipY - 148.0f,
                GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f, false);
        }
    }

    const AOCWeaponBase* Primary = Character->GetPrimaryWeapon();
    const AOCWeaponBase* Secondary = Character->GetSecondaryWeapon();
    const FString PrimaryText = FString::Printf(TEXT("1  %s"), Primary ? *Primary->GetWeaponDisplayName() : TEXT("EMPTY"));
    const FString SecondaryText = FString::Printf(TEXT("2  %s"), Secondary ? *Secondary->GetWeaponDisplayName() : TEXT("EMPTY"));
    const bool bPrimaryActive = Character->GetActiveWeaponSlot() == EOCInventorySlot::Primary;
    const bool bSecondaryActive = Character->GetActiveWeaponSlot() == EOCInventorySlot::Secondary;
    DrawText(PrimaryText, bPrimaryActive ? FLinearColor::White : FLinearColor(0.55f, 0.58f, 0.62f, 1.0f),
        28.0f, Canvas->ClipY - 92.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.9f, false);
    DrawText(SecondaryText, bSecondaryActive ? FLinearColor::White : FLinearColor(0.55f, 0.58f, 0.62f, 1.0f),
        28.0f, Canvas->ClipY - 68.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.9f, false);
    const FString GrenadeText=FString::Printf(TEXT("4 %s x%d   F THROW"),*OCGrenadeTypeToString(Character->GetSelectedGrenadeType()),Character->GetSelectedGrenadeCount());
    DrawText(GrenadeText,FLinearColor(0.70f,0.74f,0.78f,1.0f),28.0f,Canvas->ClipY-42.0f,GEngine?GEngine->GetSmallFont():nullptr,0.78f,false);
    if(const AOCPlayerState* LocalRole=PC->GetPlayerState<AOCPlayerState>();LocalRole&&LocalRole->IsEngineer())
    {
        const FString TrapText=FString::Printf(TEXT("N %s   M DEPLOY x%d"),*OCTrapPresetToString(Character->GetSelectedTrapPreset()),Character->GetTrapCount());
        DrawText(TrapText,FLinearColor(0.70f,0.74f,0.78f,1.0f),28.0f,Canvas->ClipY-18.0f,GEngine?GEngine->GetSmallFont():nullptr,0.70f,false);
    }

    const FString InteractionPrompt = Character->GetInteractionPrompt();
    if (!InteractionPrompt.IsEmpty())
    {
        DrawText(InteractionPrompt, FLinearColor(0.93f, 0.93f, 0.93f, 1.0f), CenterX - 95.0f, CenterY + 72.0f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 0.95f, false);
    }

    const float ReviveProgress = Character->GetReviveProgress();
    if (ReviveProgress > KINDA_SMALL_NUMBER)
    {
        const float BarWidth = 260.0f;
        const float BarHeight = 9.0f;
        const float BarX = CenterX - BarWidth * 0.5f;
        const float BarY = CenterY + 104.0f;
        const FString ReviveText = FString::Printf(TEXT("REVIVING  %s"), *Character->GetReviveTargetName());
        DrawText(ReviveText, FLinearColor::White, CenterX - 90.0f, CenterY + 82.0f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 0.9f, false);
        DrawRect(FLinearColor(0.08f, 0.08f, 0.08f, 0.88f), BarX, BarY, BarWidth, BarHeight);
        DrawRect(FLinearColor(0.65f, 0.88f, 0.72f, 0.95f), BarX, BarY, BarWidth * ReviveProgress, BarHeight);
    }

    if (UOCPlayerUserSettings::Get()->bShowPing)
    {
        if (const AOCPlayerState* LocalState = PC->GetPlayerState<AOCPlayerState>())
        {
            const FString PingText = FString::Printf(TEXT("PING %d ms"), LocalState->GetPingMs());
            DrawText(PingText, FLinearColor(0.75f, 0.75f, 0.75f, 1.0f), Canvas->ClipX - 120.0f, 24.0f,
                GEngine ? GEngine->GetSmallFont() : nullptr, 0.85f, false);
        }
    }

    if (const float FlashAlpha = Character->GetFlashEffectAlpha(); FlashAlpha > KINDA_SMALL_NUMBER)
    {
        DrawRect(FLinearColor(1.0f,1.0f,1.0f,FMath::Clamp(FlashAlpha,0.0f,0.94f)),0.0f,0.0f,Canvas->ClipX,Canvas->ClipY);
    }

    if (const UOCHealthComponent* Health = Character->GetHealthComponent())
    {
        const float DamageFactor = 1.0f - Health->GetHealthNormalized();
        if (DamageFactor > KINDA_SMALL_NUMBER)
        {
            const float Alpha = FMath::Clamp(DamageFactor * 0.35f, 0.0f, 0.35f);
            const FLinearColor DamageColor(0.65f, 0.0f, 0.0f, Alpha);
            const float Border = 24.0f;

            DrawRect(DamageColor, 0.0f, 0.0f, Canvas->ClipX, Border);
            DrawRect(DamageColor, 0.0f, Canvas->ClipY - Border, Canvas->ClipX, Border);
            DrawRect(DamageColor, 0.0f, 0.0f, Border, Canvas->ClipY);
            DrawRect(DamageColor, Canvas->ClipX - Border, 0.0f, Border, Canvas->ClipY);
        }
    }
}

void AOCHUD::DrawVehicleHUD()
{
    if (!Canvas)
    {
        return;
    }

    const AOCPlayerController* PC = Cast<AOCPlayerController>(GetOwningPlayerController());
    const AOCVehicleBase* Vehicle = PC ? Cast<AOCVehicleBase>(PC->GetPawn()) : nullptr;
    if (!PC || !Vehicle)
    {
        return;
    }

    const float CenterX = Canvas->ClipX * 0.5f;
    const float Speed = Vehicle->GetSpeedKmh();
    const int32 SpeedRounded = FMath::RoundToInt(Speed);
    const int32 HealthPercent = FMath::RoundToInt(Vehicle->GetVehicleHealthNormalized() * 100.0f);
    const FString SpeedText = FString::Printf(TEXT("%03d km/h"), SpeedRounded);
    const FString HealthText = FString::Printf(TEXT("VEHICLE %d%%"), HealthPercent);
    const FString CameraText = Vehicle->IsFirstPersonCameraActive() ? TEXT("CAM: INTERIOR") : TEXT("CAM: THIRD PERSON");

    DrawRect(FLinearColor(0.02f, 0.025f, 0.03f, 0.78f), CenterX - 122.0f, Canvas->ClipY - 126.0f, 244.0f, 92.0f);
    DrawText(SpeedText, FLinearColor::White, CenterX - 74.0f, Canvas->ClipY - 116.0f,
        GEngine ? GEngine->GetSmallFont() : nullptr, 1.25f, false);
    DrawText(HealthText, FLinearColor(0.82f, 0.84f, 0.86f, 1.0f), CenterX - 67.0f, Canvas->ClipY - 86.0f,
        GEngine ? GEngine->GetSmallFont() : nullptr, 0.9f, false);
    DrawText(CameraText, FLinearColor(0.70f, 0.72f, 0.74f, 1.0f), CenterX - 76.0f, Canvas->ClipY - 62.0f,
        GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f, false);

    DrawText(TEXT("W/S DRIVE   A/D STEER   SPACE HANDBRAKE"), FLinearColor(0.78f, 0.80f, 0.82f, 1.0f),
        28.0f, Canvas->ClipY - 66.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f, false);
    DrawText(TEXT("RMB FREE LOOK   C CAMERA   E EXIT WHEN SLOW"), FLinearColor(0.78f, 0.80f, 0.82f, 1.0f),
        28.0f, Canvas->ClipY - 42.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f, false);

    if (Vehicle->IsVehicleDestroyed())
    {
        DrawText(TEXT("VEHICLE DESTROYED"), FLinearColor(0.95f, 0.20f, 0.17f, 1.0f), CenterX - 92.0f, Canvas->ClipY * 0.62f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 1.2f, false);
    }

    if (UOCPlayerUserSettings::Get()->bShowPing)
    {
        if (const AOCPlayerState* LocalState = PC->GetPlayerState<AOCPlayerState>())
        {
            const FString PingText = FString::Printf(TEXT("PING %d ms"), LocalState->GetPingMs());
            DrawText(PingText, FLinearColor(0.75f, 0.75f, 0.75f, 1.0f), Canvas->ClipX - 120.0f, 24.0f,
                GEngine ? GEngine->GetSmallFont() : nullptr, 0.85f, false);
        }
    }
}


void AOCHUD::DrawGunnerHUD(const AOCCharacter* Character)
{
    if (!Canvas || !Character)
    {
        return;
    }

    const AOCArmedVehicleBase* Vehicle = Cast<AOCArmedVehicleBase>(Character->GetCurrentVehicle());
    const AOCPlayerController* PC = Cast<AOCPlayerController>(GetOwningPlayerController());
    if (!Vehicle || !PC)
    {
        return;
    }

    const float CX = Canvas->ClipX * 0.5f;
    const float CY = Canvas->ClipY * 0.5f;
    const int32 HealthPercent = FMath::RoundToInt(Vehicle->GetVehicleHealthNormalized() * 100.0f);
    const FString AmmoText = Vehicle->IsTurretReloading()
        ? TEXT("RELOADING")
        : FString::Printf(TEXT("%03d / %03d"), Vehicle->GetTurretAmmoInMagazine(), Vehicle->GetTurretReserveAmmo());
    const FString StatusText = FString::Printf(TEXT("%s   HULL %d%%"), *Vehicle->GetTurretDisplayName(), HealthPercent);

    DrawRect(FLinearColor(0.02f, 0.025f, 0.03f, 0.80f), CX - 170.0f, Canvas->ClipY - 116.0f, 340.0f, 78.0f);
    DrawText(StatusText, FLinearColor::White, CX - 145.0f, Canvas->ClipY - 106.0f,
        GEngine ? GEngine->GetSmallFont() : nullptr, 0.92f, false);
    DrawText(AmmoText, FLinearColor(0.86f, 0.87f, 0.89f, 1.0f), CX - 54.0f, Canvas->ClipY - 78.0f,
        GEngine ? GEngine->GetSmallFont() : nullptr, 1.05f, false);

    const float Gap = 8.0f;
    const float Len = 13.0f;
    const float Thick = 2.0f;
    DrawRect(FLinearColor::White, CX - Gap - Len, CY - Thick * 0.5f, Len, Thick);
    DrawRect(FLinearColor::White, CX + Gap, CY - Thick * 0.5f, Len, Thick);
    DrawRect(FLinearColor::White, CX - Thick * 0.5f, CY - Gap - Len, Thick, Len);
    DrawRect(FLinearColor::White, CX - Thick * 0.5f, CY + Gap, Thick, Len);

    DrawText(TEXT("MOUSE AIM   LMB FIRE   R RELOAD   E EXIT WHEN SLOW"),
        FLinearColor(0.78f, 0.80f, 0.82f, 1.0f), 28.0f, Canvas->ClipY - 42.0f,
        GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f, false);

    if (UOCPlayerUserSettings::Get()->bShowPing)
    {
        if (const AOCPlayerState* LocalState = PC->GetPlayerState<AOCPlayerState>())
        {
            const FString PingText = FString::Printf(TEXT("PING %d ms"), LocalState->GetPingMs());
            DrawText(PingText, FLinearColor(0.75f, 0.75f, 0.75f, 1.0f), Canvas->ClipX - 120.0f, 24.0f,
                GEngine ? GEngine->GetSmallFont() : nullptr, 0.85f, false);
        }
    }
}

void AOCHUD::DrawMatchStatus()
{
    if (!Canvas || !GetWorld())
    {
        return;
    }

    const AOCGameState* MatchState = GetWorld()->GetGameState<AOCGameState>();
    const AOCPlayerController* PC = Cast<AOCPlayerController>(GetOwningPlayerController());
    if (!MatchState || !PC)
    {
        return;
    }

    const float CenterX = Canvas->ClipX * 0.5f;
    if (MatchState->IsSandboxMode())
    {
        DrawRect(FLinearColor(0.02f, 0.025f, 0.03f, 0.80f), CenterX - 205.0f, 18.0f, 410.0f, 58.0f);
        DrawText(TEXT("SANDBOX / TEST RANGE"), FLinearColor::White, CenterX - 118.0f, 27.0f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 1.05f, false);
        DrawText(TEXT("F10 ADMIN PANEL"), FLinearColor(0.70f,0.82f,0.95f,1.0f), CenterX - 72.0f, 51.0f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f, false);
        return;
    }

    const FString TicketText = FString::Printf(TEXT("TEAM 1   %03d     |     %03d   TEAM 2"),
        MatchState->GetTickets(EOCTeam::TeamOne), MatchState->GetTickets(EOCTeam::TeamTwo));
    DrawRect(FLinearColor(0.02f, 0.025f, 0.03f, 0.78f), CenterX - 205.0f, 18.0f, 410.0f, 58.0f);
    DrawText(TicketText, FLinearColor::White, CenterX - 174.0f, 28.0f,
        GEngine ? GEngine->GetSmallFont() : nullptr, 1.05f, false);

    FString ObjectiveText;
    for (TActorIterator<AOCCapturePoint> It(GetWorld()); It; ++It)
    {
        const AOCCapturePoint* Point = *It;
        if (!Point)
        {
            continue;
        }

        FString Status = TEXT("N");
        if (Point->IsContested()) Status = TEXT("X");
        else if (Point->GetOwnerTeam() == EOCTeam::TeamOne) Status = TEXT("T1");
        else if (Point->GetOwnerTeam() == EOCTeam::TeamTwo) Status = TEXT("T2");
        else if (FMath::Abs(Point->GetCaptureProgress()) > KINDA_SMALL_NUMBER)
        {
            Status = FString::Printf(TEXT("%d%%"), FMath::RoundToInt(FMath::Abs(Point->GetCaptureProgress()) * 100.0f));
        }

        if (!ObjectiveText.IsEmpty()) ObjectiveText += TEXT("     ");
        ObjectiveText += FString::Printf(TEXT("%s [%s]"), *Point->GetPointId().ToString(), *Status);
    }
    DrawText(ObjectiveText, FLinearColor(0.78f, 0.80f, 0.82f, 1.0f), CenterX - 145.0f, 54.0f,
        GEngine ? GEngine->GetSmallFont() : nullptr, 0.85f, false);

    if (const AOCPlayerState* LocalState = PC->GetPlayerState<AOCPlayerState>())
    {
        const FString TeamRole = FString::Printf(TEXT("%s  |  %s  |  %s%s"), *OCTeamToString(LocalState->GetTeamId()),
            *OCRoleToString(LocalState->GetPlayerRole()), *OCSquadName(LocalState->GetSquadId()), LocalState->IsSquadLeader()?TEXT(" LEAD"):TEXT(""));
        DrawText(TeamRole, FLinearColor(0.72f, 0.74f, 0.76f, 1.0f), 28.0f, 24.0f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 0.85f, false);
    }

    if (MatchState->GetOCMatchPhase() == EOCMatchPhase::Ended)
    {
        DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.58f), 0.0f, 0.0f, Canvas->ClipX, Canvas->ClipY);
        const FString ResultText = MatchState->GetWinningTeam() == EOCTeam::None
            ? TEXT("ROUND ENDED  |  DRAW")
            : FString::Printf(TEXT("ROUND ENDED  |  %s WINS"), *OCTeamToString(MatchState->GetWinningTeam()));
        DrawText(ResultText, FLinearColor::White, CenterX - 150.0f, Canvas->ClipY * 0.45f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 1.45f, false);
    }
}

void AOCHUD::DrawScoreboard()
{
    const AOCPlayerController* PC = Cast<AOCPlayerController>(GetOwningPlayerController());
    const AOCGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AOCGameState>() : nullptr;
    if (!Canvas || !PC || !GameState)
    {
        return;
    }

    TArray<AOCPlayerState*> TeamOnePlayers;
    TArray<AOCPlayerState*> TeamTwoPlayers;
    for (APlayerState* PlayerState : GameState->PlayerArray)
    {
        AOCPlayerState* OCState = Cast<AOCPlayerState>(PlayerState);
        if (!OCState)
        {
            continue;
        }
        if (OCState->GetTeamId() == EOCTeam::TeamTwo) TeamTwoPlayers.Add(OCState);
        else TeamOnePlayers.Add(OCState);
    }

    auto SortPlayers = [](TArray<AOCPlayerState*>& Players)
    {
        Players.Sort([](const AOCPlayerState& A, const AOCPlayerState& B)
        {
            if (A.GetKills() != B.GetKills()) return A.GetKills() > B.GetKills();
            if (!FMath::IsNearlyEqual(A.GetScore(), B.GetScore())) return A.GetScore() > B.GetScore();
            return A.GetDeaths() < B.GetDeaths();
        });
    };
    SortPlayers(TeamOnePlayers);
    SortPlayers(TeamTwoPlayers);

    const int32 MaxRows = FMath::Max(TeamOnePlayers.Num(), TeamTwoPlayers.Num());
    const float PanelWidth = FMath::Min(1120.0f, Canvas->ClipX - 60.0f);
    const float RowHeight = 28.0f;
    const float HeaderHeight = 118.0f;
    const float PanelHeight = HeaderHeight + RowHeight * FMath::Max(1, MaxRows) + 28.0f;
    const float X = (Canvas->ClipX - PanelWidth) * 0.5f;
    const float Y = FMath::Max(40.0f, Canvas->ClipY * 0.09f);
    const float ColumnWidth = (PanelWidth - 48.0f) * 0.5f;

    DrawRect(FLinearColor(0.02f, 0.025f, 0.03f, 0.93f), X, Y, PanelWidth, PanelHeight);
    DrawText(TEXT("OSTER CONFLICT  |  CONQUEST"), FLinearColor::White, X + 22.0f, Y + 16.0f,
        GEngine ? GEngine->GetSmallFont() : nullptr, 1.2f, false);

    const FString TicketHeader = FString::Printf(TEXT("TICKETS   %03d  -  %03d"),
        GameState->GetTickets(EOCTeam::TeamOne), GameState->GetTickets(EOCTeam::TeamTwo));
    DrawText(TicketHeader, FLinearColor(0.72f, 0.74f, 0.76f, 1.0f), X + PanelWidth - 205.0f, Y + 18.0f,
        GEngine ? GEngine->GetSmallFont() : nullptr, 0.92f, false);

    const float LeftX = X + 20.0f;
    const float RightX = X + 28.0f + ColumnWidth;
    const float TeamY = Y + 52.0f;
    DrawText(FString::Printf(TEXT("TEAM 1  |  %d PLAYERS"), TeamOnePlayers.Num()), FLinearColor::White, LeftX, TeamY,
        GEngine ? GEngine->GetSmallFont() : nullptr, 1.0f, false);
    DrawText(FString::Printf(TEXT("TEAM 2  |  %d PLAYERS"), TeamTwoPlayers.Num()), FLinearColor::White, RightX, TeamY,
        GEngine ? GEngine->GetSmallFont() : nullptr, 1.0f, false);

    auto DrawHeaders = [&](float BaseX)
    {
        DrawText(TEXT("PLAYER"), FLinearColor(0.62f, 0.65f, 0.68f, 1.0f), BaseX, Y + 82.0f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f, false);
        DrawText(TEXT("K"), FLinearColor(0.62f, 0.65f, 0.68f, 1.0f), BaseX + ColumnWidth - 205.0f, Y + 82.0f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f, false);
        DrawText(TEXT("D"), FLinearColor(0.62f, 0.65f, 0.68f, 1.0f), BaseX + ColumnWidth - 165.0f, Y + 82.0f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f, false);
        DrawText(TEXT("R"), FLinearColor(0.62f, 0.65f, 0.68f, 1.0f), BaseX + ColumnWidth - 125.0f, Y + 82.0f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f, false);
        DrawText(TEXT("S"), FLinearColor(0.62f, 0.65f, 0.68f, 1.0f), BaseX + ColumnWidth - 85.0f, Y + 82.0f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f, false);
        DrawText(TEXT("PING"), FLinearColor(0.62f, 0.65f, 0.68f, 1.0f), BaseX + ColumnWidth - 45.0f, Y + 82.0f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f, false);
    };
    DrawHeaders(LeftX);
    DrawHeaders(RightX);

    const APlayerState* LocalState = PC->GetPlayerState<APlayerState>();
    auto DrawTeamRows = [&](const TArray<AOCPlayerState*>& Players, float BaseX)
    {
        for (int32 Index = 0; Index < Players.Num(); ++Index)
        {
            const AOCPlayerState* State = Players[Index];
            if (!State) continue;
            const float RowY = Y + HeaderHeight + static_cast<float>(Index) * RowHeight;
            if (State == LocalState)
            {
                DrawRect(FLinearColor(0.18f, 0.22f, 0.26f, 0.78f), BaseX - 8.0f, RowY - 3.0f, ColumnWidth, RowHeight);
            }
            const FLinearColor RowColor = State == LocalState ? FLinearColor::White : FLinearColor(0.82f, 0.84f, 0.86f, 1.0f);
            const FString BotTag = State->IsBotPlayer() ? TEXT("BOT ") : TEXT("");
            const FString LeaderTag = State->IsSquadLeader() ? TEXT("*") : TEXT("");
            const FString NameRole = FString::Printf(TEXT("%s%s [%s/%s%s]"), *BotTag, *State->GetPlayerName(),
                *OCRoleToString(State->GetPlayerRole()), *OCSquadName(State->GetSquadId()), *LeaderTag);
            DrawText(NameRole, RowColor, BaseX, RowY, GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f, false);
            DrawText(FString::FromInt(State->GetKills()), RowColor, BaseX + ColumnWidth - 205.0f, RowY,
                GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f, false);
            DrawText(FString::FromInt(State->GetDeaths()), RowColor, BaseX + ColumnWidth - 165.0f, RowY,
                GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f, false);
            DrawText(FString::FromInt(State->GetRevives()), RowColor, BaseX + ColumnWidth - 125.0f, RowY,
                GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f, false);
            DrawText(FString::FromInt(FMath::RoundToInt(State->GetScore())), RowColor, BaseX + ColumnWidth - 85.0f, RowY,
                GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f, false);
            DrawText(FString::FromInt(State->GetPingMs()), RowColor, BaseX + ColumnWidth - 45.0f, RowY,
                GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f, false);
        }
    };
    DrawTeamRows(TeamOnePlayers, LeftX);
    DrawTeamRows(TeamTwoPlayers, RightX);

    if (GameState->GetOCMatchPhase() == EOCMatchPhase::Ended)
    {
        const FString Result = GameState->GetWinningTeam() == EOCTeam::None
            ? TEXT("DRAW") : FString::Printf(TEXT("%s WINS"), *OCTeamToString(GameState->GetWinningTeam()));
        DrawText(Result, FLinearColor::White, X + PanelWidth * 0.5f - 55.0f, Y + PanelHeight - 27.0f,
            GEngine ? GEngine->GetSmallFont() : nullptr, 1.0f, false);
    }
}


void AOCHUD::DrawSandboxAdminPanel(const AOCPlayerController* PC)
{
    if (!Canvas || !PC || !PC->IsSandboxAdmin()) return;
    const float W=520.0f,H=430.0f,X=Canvas->ClipX*0.5f-W*0.5f,Y=Canvas->ClipY*0.5f-H*0.5f;
    DrawRect(FLinearColor(0.015f,0.02f,0.025f,0.94f),X,Y,W,H);
    DrawText(TEXT("SANDBOX / TEST RANGE ADMIN"),FLinearColor::White,X+24,Y+20,GEngine?GEngine->GetSmallFont():nullptr,1.15f,false);
    DrawText(TEXT("UP/DOWN select   ENTER execute   F10 close"),FLinearColor(0.65f,0.69f,0.73f,1),X+24,Y+50,GEngine?GEngine->GetSmallFont():nullptr,0.78f,false);
    for(int32 I=0;I<14;++I){const bool S=I==PC->GetSelectedAdminActionIndex();if(S)DrawRect(FLinearColor(0.16f,0.24f,0.30f,0.88f),X+18,Y+82+I*26,W-36,23);DrawText(FString::Printf(TEXT("%s %02d  %s"),S?TEXT(">") : TEXT(" "),I+1,*PC->GetAdminActionLabel(I)),S?FLinearColor::White:FLinearColor(0.76f,0.79f,0.82f,1),X+28,Y+84+I*26,GEngine?GEngine->GetSmallFont():nullptr,0.82f,false);}
}


void AOCHUD::DrawDeploymentPanel(const AOCPlayerController* PC)
{
    if (!Canvas || !PC) return;
    const AOCPlayerState* PS = PC->GetPlayerState<AOCPlayerState>();
    const AOCGameState* GS = GetWorld() ? GetWorld()->GetGameState<AOCGameState>() : nullptr;
    const float W = 720.0f, H = 430.0f, X = Canvas->ClipX*0.5f-W*0.5f, Y = Canvas->ClipY*0.5f-H*0.5f;
    DrawRect(FLinearColor(0.012f,0.016f,0.022f,0.96f), X,Y,W,H);
    DrawText(TEXT("OSTER CONFLICT  |  PRE-GAME / DEPLOYMENT"), FLinearColor::White, X+28,Y+24,
        GEngine?GEngine->GetSmallFont():nullptr,1.2f,false);
    DrawText(TEXT("Source milestone panel - final mouse/keyboard text fields are scheduled for S17 UI"),
        FLinearColor(0.62f,0.66f,0.70f,1),X+28,Y+54,GEngine?GEngine->GetSmallFont():nullptr,0.78f,false);
    if (PS)
    {
        DrawText(FString::Printf(TEXT("USERNAME      %s"),*PS->GetPlayerName()),FLinearColor::White,X+36,Y+105,GEngine?GEngine->GetSmallFont():nullptr,1.0f,false);
        DrawText(FString::Printf(TEXT("TEAM          %s"),*OCTeamToString(PS->GetTeamId())),FLinearColor::White,X+36,Y+140,GEngine?GEngine->GetSmallFont():nullptr,1.0f,false);
        DrawText(FString::Printf(TEXT("SQUAD         %s%s"),*OCSquadName(PS->GetSquadId()),PS->IsSquadLeader()?TEXT("  [LEADER]"):TEXT("")),FLinearColor::White,X+36,Y+175,GEngine?GEngine->GetSmallFont():nullptr,1.0f,false);
        DrawText(FString::Printf(TEXT("ROLE          %s"),*OCRoleToString(PS->GetPlayerRole())),FLinearColor::White,X+36,Y+210,GEngine?GEngine->GetSmallFont():nullptr,1.0f,false);
        DrawText(FString::Printf(TEXT("FACTION       %s"),*OCFactionToString(PS->GetFactionArchetype())),FLinearColor::White,X+36,Y+245,GEngine?GEngine->GetSmallFont():nullptr,0.92f,false);
        DrawText(FString::Printf(TEXT("READY         %s"),PS->IsLobbyReady()?TEXT("YES"):TEXT("NO")),PS->IsLobbyReady()?FLinearColor(0.55f,0.9f,0.62f,1):FLinearColor(0.9f,0.72f,0.35f,1),X+36,Y+280,GEngine?GEngine->GetSmallFont():nullptr,1.0f,false);
    }
    if (GS)
    {
        DrawText(FString::Printf(TEXT("SERVER        %d HUMAN + %d BOT / %d SLOTS   TARGET POPULATION %d"),
            GS->GetHumanPlayerCount(),GS->GetBotPlayerCount(),GS->GetMaxPlayerSlots(),GS->GetTargetPopulation()),
            FLinearColor(0.72f,0.76f,0.80f,1),X+36,Y+315,GEngine?GEngine->GetSmallFont():nullptr,0.9f,false);
    }
    DrawText(TEXT("F2 ROLE     F3 SQUAD     F4 READY/DEPLOY     F8 CLOSE/OPEN"),FLinearColor(0.72f,0.82f,0.95f,1),X+36,Y+350,GEngine?GEngine->GetSmallFont():nullptr,0.9f,false);
    DrawText(TEXT("Username backend: SetNickname <name>   |   final editable username box: S17"),FLinearColor(0.62f,0.66f,0.70f,1),X+36,Y+384,GEngine?GEngine->GetSmallFont():nullptr,0.78f,false);
}

void AOCHUD::DrawChat(const AOCPlayerController* PC)
{
    if (!Canvas || !PC) return;
    const TArray<FOCChatMessage>& Lines = PC->GetRecentChatMessages();
    const int32 Start = FMath::Max(0, Lines.Num()-6);
    float Y = Canvas->ClipY - 250.0f;
    for (int32 I=Start; I<Lines.Num(); ++I)
    {
        const FOCChatMessage& M = Lines[I];
        const FString Text = FString::Printf(TEXT("[%s] %s: %s"),*OCChatChannelToString(M.Channel),*M.SenderName,*M.Message);
        const FLinearColor Color = M.Channel==EOCChatChannel::Squad ? FLinearColor(0.62f,0.90f,0.67f,1) :
            M.Channel==EOCChatChannel::Team ? FLinearColor(0.55f,0.74f,0.96f,1) : FLinearColor(0.90f,0.90f,0.90f,1);
        DrawText(Text,Color,28.0f,Y,GEngine?GEngine->GetSmallFont():nullptr,0.82f,false);
        Y += 21.0f;
    }
}

void AOCHUD::DrawSquadOrder(const AOCPlayerController* PC)
{
    if (!Canvas || !PC) return;
    const FOCSquadOrder& Order = PC->GetCurrentSquadOrder();
    if (!Order.IsActive()) return;
    DrawRect(FLinearColor(0.02f,0.03f,0.025f,0.78f),Canvas->ClipX*0.5f-145.0f,84.0f,290.0f,38.0f);
    DrawText(FString::Printf(TEXT("SQUAD ORDER: %s"),*OCSquadOrderToString(Order)),FLinearColor(0.62f,0.90f,0.67f,1),
        Canvas->ClipX*0.5f-118.0f,95.0f,GEngine?GEngine->GetSmallFont():nullptr,0.86f,false);
}
