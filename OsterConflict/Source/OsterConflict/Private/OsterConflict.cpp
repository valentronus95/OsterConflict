#include "OsterConflict.h"
#include "OCBuildVersion.h"
#include "HAL/IConsoleManager.h"
#include "Modules/ModuleManager.h"
#include "MoviePlayer.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

static void OCPrintBuildInfo()
{
    UE_LOG(LogTemp, Display, TEXT("OC_BUILD Milestone=%s Version=%s Protocol=%d ReleaseMap=%s"),
        OCBuildVersion::Milestone, OCBuildVersion::ProjectVersion, OCBuildVersion::NetworkProtocol,
        OCBuildVersion::ReleaseMap);
}

static FAutoConsoleCommand GOCBuildInfoCommand(
    TEXT("oc.BuildInfo"),
    TEXT("Print Oster Conflict build/version fingerprint."),
    FConsoleCommandDelegate::CreateStatic(&OCPrintBuildInfo));

static void OCSetupStartupLoadingScreen()
{
    if (!IsMoviePlayerEnabled())
    {
        return;
    }

    // GameDefaultMap is the full OsterConflict_Runtime world. The platform splash disappears before
    // that map and its frontend widgets are ready, which otherwise exposes an engine-black frame.
    // MoviePlayer lives above world/UMG lifetime, so it can bridge that startup gap safely.
    FLoadingScreenAttributes LoadingScreen;
    LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
    LoadingScreen.bWaitForManualStop = false;
    LoadingScreen.bMoviesAreSkippable = false;
    LoadingScreen.MinimumLoadingScreenDisplayTime = 0.0f;
    LoadingScreen.WidgetLoadingScreen =
        SNew(SBorder)
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Center)
        .BorderBackgroundColor(FLinearColor(0.012f, 0.016f, 0.020f, 1.0f))
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("OSTER CONFLICT\nЗАВАНТАЖЕННЯ")))
            .Justification(ETextJustify::Center)
        ];

    GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
}

class FOsterConflictModule final : public FDefaultGameModuleImpl
{
public:
    virtual void StartupModule() override
    {
        FDefaultGameModuleImpl::StartupModule();
        OCPrintBuildInfo();
        OCSetupStartupLoadingScreen();
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE(FOsterConflictModule, OsterConflict, "OsterConflict");