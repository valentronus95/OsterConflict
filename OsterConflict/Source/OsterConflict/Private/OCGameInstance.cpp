#include "OCGameInstance.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "MoviePlayer.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/Connection/NetEnums.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "OCConnection"

namespace
{
    // MoviePlayer's Slate surface can remain active while the game thread is inside synchronous LoadMap/BeginPlay.
    // Only expose lifecycle milestones we actually own. This is intentionally not a fabricated byte percentage.
    TAtomic<int32> GPass45LoadingMilestonePercent{0};
    TAtomic<int32> GPass45LoadingPhase{0};

    FText Pass45LoadingPercentText()
    {
        const int32 Percent = FMath::Clamp(GPass45LoadingMilestonePercent.Load(), 0, 100);
        return FText::FromString(FString::Printf(TEXT("%d%%"), Percent));
    }

    FText Pass45LoadingPhaseText()
    {
        switch (GPass45LoadingPhase.Load())
        {
            case 1: return FText::FromString(TEXT("ПІДГОТОВКА ВІКНА ГРИ"));
            case 2: return FText::FromString(TEXT("ЗАВАНТАЖЕННЯ КАРТИ"));
            case 3: return FText::FromString(TEXT("ІНІЦІАЛІЗАЦІЯ СВІТУ"));
            case 4: return FText::FromString(TEXT("ГОТОВО"));
            default: return FText::FromString(TEXT("ПІДГОТОВКА"));
        }
    }
}

void UOCGameInstance::Init()
{
    Super::Init();
    ConnectionStatusText = LOCTEXT("FrontendReady", "Готово до підключення.");

    // PASS45: map loading belongs to Unreal itself. Pre/PostLoadMap brackets the actual LoadMap call,
    // while MoviePlayer keeps a Slate loading surface alive even when the game thread is blocked.
    FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UOCGameInstance::HandlePreLoadMap);
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UOCGameInstance::HandlePostLoadMap);

    if (GEngine)
    {
        GEngine->OnNetworkFailure().AddUObject(this, &UOCGameInstance::HandleNetworkFailure);
        GEngine->OnTravelFailure().AddUObject(this, &UOCGameInstance::HandleTravelFailure);
    }

    // Fast Preview explicitly asks for an engine-native boot surface before the lightweight frontend is ready.
    // If MoviePlayer has not been initialized yet, the ordinary PreLoadMap delegate remains the fallback owner.
    if (FParse::Param(FCommandLine::Get(), TEXT("OCFastPreview")))
    {
        PrepareRuntimeLoadingScreen(TEXT("FrontendBootstrap"), 5, 1);
    }
}

void UOCGameInstance::Shutdown()
{
    FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

    if (GEngine)
    {
        GEngine->OnNetworkFailure().RemoveAll(this);
        GEngine->OnTravelFailure().RemoveAll(this);
    }
    Super::Shutdown();
}

void UOCGameInstance::PrepareRuntimeLoadingScreen(const FString& Context, int32 MilestonePercent, int32 Phase)
{
    GPass45LoadingMilestonePercent.Store(FMath::Clamp(MilestonePercent, 0, 100));
    GPass45LoadingPhase.Store(Phase);

    if (!IsMoviePlayerEnabled())
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_INGAME_LOADING_MOVIEPLAYER_DISABLED context=%s percent=%d phase=%d"),
            *Context, MilestonePercent, Phase);
        return;
    }

    IGameMoviePlayer* MoviePlayer = GetMoviePlayer();
    if (!MoviePlayer || !MoviePlayer->IsInitialized())
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_INGAME_LOADING_MOVIEPLAYER_NOT_READY context=%s percent=%d phase=%d"),
            *Context, MilestonePercent, Phase);
        return;
    }

    if (MoviePlayer->IsMovieCurrentlyPlaying())
    {
        UE_LOG(LogTemp, Display,
            TEXT("PASS45_INGAME_LOADING_MILESTONE context=%s percent=%d phase=%d reused_surface=1"),
            *Context, MilestonePercent, Phase);
        return;
    }

    FLoadingScreenAttributes LoadingScreen;
    LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
    LoadingScreen.bWaitForManualStop = true;
    LoadingScreen.bMoviesAreSkippable = false;
    LoadingScreen.bAllowEngineTick = false;
    LoadingScreen.MinimumLoadingScreenDisplayTime = 0.20f;
    LoadingScreen.WidgetLoadingScreen =
        SNew(SBorder)
        .Padding(FMargin(64.0f))
        .BorderBackgroundColor(FLinearColor(0.006f, 0.009f, 0.012f, 1.0f))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .FillHeight(1.0f)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(FMargin(0.0f, 0.0f, 0.0f, 26.0f))
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("LoadingBrand", "OSTER CONFLICT"))
                    .ColorAndOpacity(FLinearColor(0.96f, 0.97f, 0.98f, 1.0f))
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(FMargin(0.0f, 0.0f, 0.0f, 16.0f))
                [
                    SNew(STextBlock)
                    .Text_Lambda([]() { return Pass45LoadingPercentText(); })
                    .ColorAndOpacity(FLinearColor(0.96f, 0.97f, 0.98f, 1.0f))
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(FMargin(0.0f, 0.0f, 0.0f, 18.0f))
                [
                    SNew(SThrobber)
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                [
                    SNew(STextBlock)
                    .Text_Lambda([]() { return Pass45LoadingPhaseText(); })
                    .ColorAndOpacity(FLinearColor(0.78f, 0.81f, 0.84f, 1.0f))
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(FMargin(0.0f, 10.0f, 0.0f, 0.0f))
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("LoadingDetail", "Етапний прогрес UE: карта → світ → готовність. Не оцінка байтів/шейдерів."))
                    .ColorAndOpacity(FLinearColor(0.56f, 0.60f, 0.64f, 1.0f))
                ]
            ]
        ];

    MoviePlayer->SetupLoadingScreen(LoadingScreen);
    const bool bStarted = MoviePlayer->PlayMovie();
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_INGAME_LOADING_SURFACE_START context=%s percent=%d phase=%d started=%d manual_stop=1"),
        *Context, MilestonePercent, Phase, bStarted ? 1 : 0);
}

void UOCGameInstance::HandlePreLoadMap(const FString& MapName)
{
    ActiveMapLoadStartedAtSeconds = FPlatformTime::Seconds();
    UE_LOG(LogTemp, Display, TEXT("PASS45_INGAME_LOADING_BEGIN map=%s"), *MapName);
    PrepareRuntimeLoadingScreen(MapName, 20, 2);
}

void UOCGameInstance::HandlePostLoadMap(UWorld* LoadedWorld)
{
    const double CompletedAt = FPlatformTime::Seconds();
    const double Duration = ActiveMapLoadStartedAtSeconds > 0.0
        ? FMath::Max(0.0, CompletedAt - ActiveMapLoadStartedAtSeconds)
        : 0.0;
    const FString LoadedMap = LoadedWorld ? LoadedWorld->GetMapName() : FString(TEXT("<null>"));

    if (IsMoviePlayerEnabled())
    {
        IGameMoviePlayer* MoviePlayer = GetMoviePlayer();
        if (MoviePlayer && MoviePlayer->IsMovieCurrentlyPlaying())
        {
            GPass45LoadingMilestonePercent.Store(70);
            GPass45LoadingPhase.Store(3);
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("PASS45_INGAME_LOADING_MAP_COMPLETE map=%s duration_s=%.3f milestone_percent=70 awaiting_runtime_beginplay=1"),
        *LoadedMap,
        Duration);
    ActiveMapLoadStartedAtSeconds = 0.0;
}

void UOCGameInstance::CompleteRuntimeLoading(const TCHAR* Reason)
{
    GPass45LoadingMilestonePercent.Store(100);
    GPass45LoadingPhase.Store(4);

    if (!IsMoviePlayerEnabled())
    {
        UE_LOG(LogTemp, Display, TEXT("PASS45_INGAME_LOADING_READY reason=%s percent=100 movieplayer=disabled"),
            Reason ? Reason : TEXT("unknown"));
        return;
    }

    IGameMoviePlayer* MoviePlayer = GetMoviePlayer();
    const bool bWasPlaying = MoviePlayer && MoviePlayer->IsMovieCurrentlyPlaying();
    UE_LOG(LogTemp, Display,
        TEXT("PASS45_INGAME_LOADING_READY reason=%s percent=100 movieplayer_playing=%d"),
        Reason ? Reason : TEXT("unknown"), bWasPlaying ? 1 : 0);
    if (bWasPlaying)
    {
        MoviePlayer->StopMovie();
    }
}

void UOCGameInstance::BeginDirectConnect(const FString& Address)
{
    PendingAddress = Address;
    ConnectionFailureCode.Reset();
    FFormatNamedArguments Args;
    Args.Add(TEXT("Address"), FText::FromString(Address));
    ConnectionStatusText = FText::Format(LOCTEXT("ConnectingToServer", "Підключення до {Address}…"), Args);
}

void UOCGameInstance::MarkConnected()
{
    ConnectionFailureCode.Reset();
    PendingAddress.Reset();
    ConnectionStatusText = LOCTEXT("Connected", "Підключено.");
}

void UOCGameInstance::ClearConnectionFailure()
{
    ConnectionFailureCode.Reset();
    ConnectionStatusText = LOCTEXT("FrontendReady", "Готово до підключення.");
}

FString UOCGameInstance::CanonicalCodeFromNetworkFailure(ENetworkFailure::Type FailureType, const FString& ErrorString)
{
    if (ErrorString.Contains(TEXT("VERSION_MISMATCH"), ESearchCase::IgnoreCase) ||
        FailureType == ENetworkFailure::OutdatedClient || FailureType == ENetworkFailure::OutdatedServer ||
        FailureType == ENetworkFailure::NetGuidMismatch || FailureType == ENetworkFailure::NetChecksumMismatch)
    {
        return TEXT("VERSION_MISMATCH");
    }
    if (ErrorString.Contains(TEXT("SERVER_FULL_HUMANS"), ESearchCase::IgnoreCase)) return TEXT("SERVER_FULL_HUMANS");
    if (ErrorString.Contains(TEXT("INVALID_USERNAME"), ESearchCase::IgnoreCase)) return TEXT("INVALID_USERNAME");
    if (ErrorString.Contains(TEXT("SERVER_SHUTDOWN"), ESearchCase::IgnoreCase)) return TEXT("SERVER_SHUTDOWN");
    if (FailureType == ENetworkFailure::ConnectionTimeout) return TEXT("TIMEOUT");
    if (FailureType == ENetworkFailure::ConnectionLost) return TEXT("CONNECTION_LOST");
    if (FailureType == ENetworkFailure::NetDriverCreateFailure || FailureType == ENetworkFailure::NetDriverListenFailure)
        return TEXT("NETWORK_DRIVER_FAILURE");
    return TEXT("NETWORK_FAILURE");
}

void UOCGameInstance::SetFailure(const FString& Code, const FText& Message, const FString& TechnicalDetail)
{
    ConnectionFailureCode = Code;
    ConnectionStatusText = Message;
    UE_LOG(LogTemp, Warning, TEXT("OC_CONNECTION_FAILURE code=%s pending=%s detail=%s"),
        *Code, *PendingAddress, *TechnicalDetail.Left(512));
}

void UOCGameInstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType,
    const FString& ErrorString)
{
    const FString Code = CanonicalCodeFromNetworkFailure(FailureType, ErrorString);
    FText Message = LOCTEXT("GenericNetworkFailure", "Помилка мережевого з’єднання. Можна повторити спробу без перезапуску гри.");
    if (Code == TEXT("VERSION_MISMATCH")) Message = LOCTEXT("VersionMismatch", "Версії клієнта й сервера несумісні.");
    else if (Code == TEXT("SERVER_FULL_HUMANS")) Message = LOCTEXT("ServerFull", "Сервер досяг ліміту реальних гравців.");
    else if (Code == TEXT("INVALID_USERNAME")) Message = LOCTEXT("InvalidUsername", "Сервер відхилив ім’я гравця.");
    else if (Code == TEXT("SERVER_SHUTDOWN")) Message = LOCTEXT("ServerShutdown", "Сервер завершив роботу.");
    else if (Code == TEXT("TIMEOUT")) Message = LOCTEXT("ConnectionTimeout", "Час очікування підключення вичерпано. Перевірте адресу й повторіть спробу.");
    else if (Code == TEXT("CONNECTION_LOST")) Message = LOCTEXT("ConnectionLost", "З’єднання із сервером втрачено. Можна підключитися повторно вручну.");
    SetFailure(Code, Message, ErrorString);
}

void UOCGameInstance::HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString)
{
    SetFailure(TEXT("TRAVEL_FAILURE"),
        LOCTEXT("TravelFailure", "Гра не змогла завантажити потрібний сервер або карту."), ErrorString);
}

#undef LOCTEXT_NAMESPACE
