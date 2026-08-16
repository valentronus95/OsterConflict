#include "OsterConflict.h"
#include "OCBuildVersion.h"
#include "HAL/IConsoleManager.h"
#include "Modules/ModuleManager.h"

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

class FOsterConflictModule final : public FDefaultGameModuleImpl
{
public:
    virtual void StartupModule() override
    {
        FDefaultGameModuleImpl::StartupModule();
        OCPrintBuildInfo();
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE(FOsterConflictModule, OsterConflict, "OsterConflict");
