#include "OCRuntimeRecoveryPass15Subsystem.h"

#include "OCGameInstance.h"
#include "OCGameUIRootWidget.h"
#include "OCPlayerController.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/UObjectIterator.h"

namespace
{
    bool GPass15ConnectionFailureReturnedToFrontend = false;

    FString SanitizeTravelValue(FString Value)
    {
        Value.TrimStartAndEndInline();
        Value.ReplaceInline(TEXT("?"), TEXT(""));
        Value.ReplaceInline(TEXT("&"), TEXT(""));
        Value.ReplaceInline(TEXT("="), TEXT(""));
        Value.ReplaceInline(TEXT(" "), TEXT("_"));
        return Value.Left(24);
    }

    FString NormalizeDifficulty(FString Value)
    {
        Value.TrimStartAndEndInline();
        if (Value.Equals(TEXT("Easy"), ESearchCase::IgnoreCase)) return TEXT("Easy");
        if (Value.Equals(TEXT("Hard"), ESearchCase::IgnoreCase)) return TEXT("Hard");
        if (Value.Equals(TEXT("Veteran"), ESearchCase::IgnoreCase)) return TEXT("Veteran");
        return TEXT("Normal");
    }

    UOCGameUIRootWidget* FindRoot(UWorld* World, AOCPlayerController* PC)
    {
        if (!World || !PC) return nullptr;
        for (TObjectIterator<UOCGameUIRootWidget> It; It; ++It)
        {
            if (IsValid(*It) && It->GetWorld() == World && It->GetOwningPlayer() == PC)
            {
                return *It;
            }
        }
        return nullptr;
    }

    UButton* FindPrimaryButton(UOCGameUIRootWidget* Root)
    {
        if (!Root) return nullptr;
        UVerticalBox* Box = Cast<UVerticalBox>(Root->GetWidgetFromName(TEXT("R13_PlayerFrontend")));
        if (!Box) return nullptr;

        for (int32 Index = 0; Index < Box->GetChildrenCount(); ++Index)
        {
            USizeBox* Size = Cast<USizeBox>(Box->GetChildAt(Index));
            UButton* Button = Size ? Cast<UButton>(Size->GetContent()) : nullptr;
            UTextBlock* Label = Button ? Cast<UTextBlock>(Button->GetContent()) : nullptr;
            if (!Button || !Label) continue;

            const FString Text = Label->GetText().ToString();
            if (Text == TEXT("СТАРТ") || Text == TEXT("СТВОРИТИ СЕРВЕР") ||
                Text == TEXT("ПІДКЛЮЧИТИСЯ") || Text == TEXT("ПРОДОВЖИТИ ГРУ"))
            {
                return Button;
            }
        }
        return nullptr;
    }

    UEditableTextBox* GetField(UVerticalBox* Fields, int32 FieldIndex)
    {
        if (!Fields) return nullptr;
        int32 Seen = 0;
        for (int32 Index = 0; Index < Fields->GetChildrenCount(); ++Index)
        {
            if (UEditableTextBox* Field = Cast<UEditableTextBox>(Fields->GetChildAt(Index)))
            {
                if (Seen == FieldIndex) return Field;
                ++Seen;
            }
        }
        return nullptr;
    }
}

bool UOCRuntimeRecoveryPass15Subsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCRuntimeRecoveryPass15Subsystem::Tick(float DeltaTime)
{
    if (DeltaTime < 0.0f) return;
    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_DedicatedServer) return;

    AOCPlayerController* PC = Cast<AOCPlayerController>(World->GetFirstPlayerController());
    if (!PC || !PC->IsLocalController()) return;

    if (UOCGameInstance* GI = Cast<UOCGameInstance>(World->GetGameInstance()))
    {
        if (GI->HasConnectionFailure())
        {
            RecoverConnectionFailure(PC);
            return;
        }
        GPass15ConnectionFailureReturnedToFrontend = false;
    }

    // The R13 frontend subsystem also updates presentation every frame. Applying the repair from
    // TimerManager keeps this pass after normal UI state updates and prevents the old 0.36-alpha
    // server panel from winning depending on subsystem tick order.
    World->GetTimerManager().SetTimerForNextTick(this, &UOCRuntimeRecoveryPass15Subsystem::ApplyFrontendRepairs);
}

void UOCRuntimeRecoveryPass15Subsystem::ApplyFrontendRepairs()
{
    UWorld* World = GetWorld();
    AOCPlayerController* PC = World ? Cast<AOCPlayerController>(World->GetFirstPlayerController()) : nullptr;
    if (!World || !PC || !PC->IsLocalController()) return;

    UOCGameUIRootWidget* Root = FindRoot(World, PC);
    if (!Root) return;

    BindPrimaryFallback(Root);

    UVerticalBox* Fields = Cast<UVerticalBox>(Root->GetWidgetFromName(TEXT("R13_FrontendFields")));
    const bool bFieldsPageVisible = Fields && Fields->GetVisibility() != ESlateVisibility::Collapsed;
    if (!bFieldsPageVisible) return;

    if (UBorder* Panel = Cast<UBorder>(Root->GetWidgetFromName(TEXT("R13_MenuPanel"))))
    {
        Panel->SetBrushColor(FLinearColor(0.025f, 0.030f, 0.035f, 0.96f));
    }

    for (int32 Index = 0; Index < Fields->GetChildrenCount(); ++Index)
    {
        if (UEditableTextBox* Field = Cast<UEditableTextBox>(Fields->GetChildAt(Index)))
        {
            Field->SetBackgroundColor(FLinearColor(0.055f, 0.065f, 0.075f, 1.0f));
            Field->SetForegroundColor(FLinearColor(0.94f, 0.93f, 0.89f, 1.0f));
        }
    }

    if (!bStylePassLogged)
    {
        bStylePassLogged = true;
        UE_LOG(LogTemp, Display, TEXT("PASS15_FRONTEND_FIELDS_OPAQUE_READY"));
    }
}

void UOCRuntimeRecoveryPass15Subsystem::BindPrimaryFallback(UOCGameUIRootWidget* Root)
{
    UButton* Primary = FindPrimaryButton(Root);
    if (!Primary || BoundPrimaryButton.Get() == Primary) return;

    if (BoundPrimaryButton.IsValid())
    {
        BoundPrimaryButton->OnClicked.RemoveDynamic(this, &UOCRuntimeRecoveryPass15Subsystem::OnPrimaryClickedFallback);
    }
    BoundPrimaryButton = Primary;
    Primary->OnClicked.AddUniqueDynamic(this, &UOCRuntimeRecoveryPass15Subsystem::OnPrimaryClickedFallback);
}

void UOCRuntimeRecoveryPass15Subsystem::OnPrimaryClickedFallback()
{
    UButton* Primary = BoundPrimaryButton.Get();
    UTextBlock* Label = Primary ? Cast<UTextBlock>(Primary->GetContent()) : nullptr;
    UWorld* World = GetWorld();
    if (!Label || !World) return;

    if (Label->GetText().ToString() == TEXT("СТВОРИТИ СЕРВЕР"))
    {
        // If the original ConsoleCommand(open...) works, this world is torn down and its timer
        // disappears. If it silently does nothing, this timer performs the reliable OpenLevel path.
        World->GetTimerManager().ClearTimer(HostFallbackTimer);
        World->GetTimerManager().SetTimer(
            HostFallbackTimer, this, &UOCRuntimeRecoveryPass15Subsystem::RunHostTravelFallback, 0.20f, false);
    }
}

void UOCRuntimeRecoveryPass15Subsystem::RunHostTravelFallback()
{
    UWorld* World = GetWorld();
    AOCPlayerController* PC = World ? Cast<AOCPlayerController>(World->GetFirstPlayerController()) : nullptr;
    if (!World || !PC || !PC->IsLocalController() || PC->GetNetMode() != NM_Standalone || !PC->IsFrontendMenuVisible()) return;

    UOCGameUIRootWidget* Root = FindRoot(World, PC);
    UButton* Primary = FindPrimaryButton(Root);
    UTextBlock* Label = Primary ? Cast<UTextBlock>(Primary->GetContent()) : nullptr;
    if (!Root || !Label || Label->GetText().ToString() != TEXT("СТВОРИТИ СЕРВЕР")) return;

    UVerticalBox* Fields = Cast<UVerticalBox>(Root->GetWidgetFromName(TEXT("R13_FrontendFields")));
    if (!Fields) return;

    FString Username = GetField(Fields, 0) ? GetField(Fields, 0)->GetText().ToString() : TEXT("Player");
    Username = SanitizeTravelValue(Username);
    if (Username.IsEmpty()) Username = TEXT("Player");

    const int32 MaxPlayers = FMath::Clamp(
        FCString::Atoi(*(GetField(Fields, 2) ? GetField(Fields, 2)->GetText().ToString() : FString(TEXT("16")))), 2, 64);
    const int32 Bots = FMath::Clamp(
        FCString::Atoi(*(GetField(Fields, 3) ? GetField(Fields, 3)->GetText().ToString() : FString(TEXT("0")))), 0, MaxPlayers);
    const FString Difficulty = NormalizeDifficulty(
        GetField(Fields, 4) ? GetField(Fields, 4)->GetText().ToString() : FString(TEXT("Normal")));

    const FString Options = FString::Printf(
        TEXT("listen?Mode=Conquest?Name=%s?Bots=%d?Population=%d?BotFill=0?MaxPlayers=%d?BotDifficulty=%s?PerfProfile=LowCPU?R13Gameplay=1"),
        *Username, Bots, Bots, MaxPlayers, *Difficulty);

    UE_LOG(LogTemp, Warning,
        TEXT("PASS15_HOST_OPENLEVEL_FALLBACK max_players=%d bots=%d difficulty=%s"),
        MaxPlayers, Bots, *Difficulty);
    UGameplayStatics::OpenLevel(World, FName(TEXT("/Game/Maps/OsterConflict_Runtime")), true, Options);
}

void UOCRuntimeRecoveryPass15Subsystem::RecoverConnectionFailure(AOCPlayerController* PC)
{
    UWorld* World = GetWorld();
    UOCGameInstance* GI = World ? Cast<UOCGameInstance>(World->GetGameInstance()) : nullptr;
    if (!World || !PC || !GI || GPass15ConnectionFailureReturnedToFrontend) return;

    GPass15ConnectionFailureReturnedToFrontend = true;
    UE_LOG(LogTemp, Warning,
        TEXT("PASS15_CONNECTION_FAILURE_RETURN_FRONTEND code=%s message=%s"),
        *GI->GetConnectionFailureCode(), *GI->GetConnectionStatusText().ToString());

    // Reload the same map as a clean standalone frontend. UGameInstance survives the travel and
    // preserves the failure text, while the world/frontend subsystem state is reset. This prevents
    // failed direct-connect from exposing the suppressed gameplay shell or a fake pause menu.
    UGameplayStatics::OpenLevel(World, FName(TEXT("/Game/Maps/OsterConflict_Runtime")), true, TEXT(""));
}

TStatId UOCRuntimeRecoveryPass15Subsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UOCRuntimeRecoveryPass15Subsystem, STATGROUP_Tickables);
}
