#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OCGameInstance.generated.h"

class UNetDriver;
class UWorld;

/**
 * Source-only connection resilience layer for S18C hardening.
 * Captures engine network/travel failures into a canonical user-facing status without changing gameplay authority.
 * PASS45 also owns the engine-native map loading presentation so local preview/gameplay travel never needs an
 * external helper window. The visible percentage is milestone progress owned by real UE lifecycle callbacks,
 * not a guessed byte/package percentage.
 */
UCLASS()
class OSTERCONFLICT_API UOCGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;
    virtual void Shutdown() override;

    UFUNCTION(BlueprintPure, Category="Network") FText GetConnectionStatusText() const { return ConnectionStatusText; }
    UFUNCTION(BlueprintPure, Category="Network") FString GetConnectionFailureCode() const { return ConnectionFailureCode; }
    UFUNCTION(BlueprintPure, Category="Network") bool HasConnectionFailure() const { return !ConnectionFailureCode.IsEmpty(); }

    void BeginDirectConnect(const FString& Address);
    void MarkConnected();
    void ClearConnectionFailure();

    // Called by the runtime-safe GameMode only after its BeginPlay work has returned. This keeps the
    // MoviePlayer surface over synchronous world/model startup instead of exposing a black viewport at PostLoadMap.
    void CompleteRuntimeLoading(const TCHAR* Reason);

private:
    UPROPERTY(Transient) FText ConnectionStatusText;
    UPROPERTY(Transient) FString ConnectionFailureCode;
    FString PendingAddress;
    double ActiveMapLoadStartedAtSeconds = 0.0;

    void HandlePreLoadMap(const FString& MapName);
    void HandlePostLoadMap(UWorld* LoadedWorld);
    void PrepareRuntimeLoadingScreen(const FString& Context, int32 MilestonePercent, int32 Phase);
    void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
    void HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ErrorString);
    void SetFailure(const FString& Code, const FText& Message, const FString& TechnicalDetail);
    static FString CanonicalCodeFromNetworkFailure(ENetworkFailure::Type FailureType, const FString& ErrorString);
};
