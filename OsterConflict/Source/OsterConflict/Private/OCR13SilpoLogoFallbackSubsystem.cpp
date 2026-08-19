#include "OCR13SilpoLogoFallbackSubsystem.h"

#include "OCGameMode.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Font.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

namespace
{
    constexpr float SilpoLogoFallbackDelaySeconds = 6.85f;

    AActor* FindSilpoModel(UWorld& World)
    {
        for (TActorIterator<AActor> It(&World); It; ++It)
        {
            AActor* Actor = *It;
            if (Actor && Actor->ActorHasTag(TEXT("R13_SilpoPhotoModel"))) return Actor;
        }
        return nullptr;
    }

    UTextRenderComponent* FindText(AActor* Actor, const FName Name)
    {
        if (!Actor) return nullptr;
        TInlineComponentArray<UTextRenderComponent*> Components;
        Actor->GetComponents(Components);
        for (UTextRenderComponent* Component : Components)
        {
            if (Component && Component->GetFName() == Name) return Component;
        }
        return nullptr;
    }

    bool FontSupports(const UFont* Font, const FString& Text)
    {
        if (!Font) return false;
        for (const TCHAR Character : Text)
        {
            if (FChar::IsWhitespace(Character)) continue;
            if (Font->RemapChar(Character) == TCHAR(0)) return false;
        }
        return true;
    }

    UMaterialInstanceDynamic* MakeColorMaterial(AActor* Owner, UMaterialInterface* Base,
        const FName Name, const FLinearColor& Color)
    {
        if (!Owner || !Base) return nullptr;
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(Base, Owner, Name);
        if (Material) Material->SetVectorParameterValue(TEXT("Color"), Color);
        return Material;
    }

    UInstancedStaticMeshComponent* MakeISM(AActor* Owner, USceneComponent* Root, UStaticMesh* Mesh,
        UMaterialInterface* Material, const FName Name)
    {
        if (!Owner || !Root || !Mesh) return nullptr;
        UInstancedStaticMeshComponent* Component = NewObject<UInstancedStaticMeshComponent>(Owner, Name);
        if (!Component) return nullptr;
        Component->SetupAttachment(Root);
        Component->SetStaticMesh(Mesh);
        if (Material) Component->SetMaterial(0, Material);
        Component->SetMobility(EComponentMobility::Static);
        Component->SetCollisionProfileName(TEXT("NoCollision"));
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetCastShadow(true);
        Component->SetCullDistances(0, 90000);
        Owner->AddInstanceComponent(Component);
        Component->RegisterComponent();
        return Component;
    }

    void AddRect(UInstancedStaticMeshComponent* Component, const float X, const float Y, const float Z,
        const float Width, const float Depth, const float Height)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(FRotator::ZeroRotator,
            FVector(X, Y, Z), FVector(Width, Depth, Height) / 100.0f), false);
    }

    void AddDiagonal(UInstancedStaticMeshComponent* Component, const float X, const float Y, const float Z,
        const float Length, const float Thickness, const float Depth, const float PitchDegrees)
    {
        if (!Component) return;
        Component->AddInstance(FTransform(FRotator(PitchDegrees, 0.0f, 0.0f),
            FVector(X, Y, Z), FVector(Length, Depth, Thickness) / 100.0f), false);
    }

    void AddGlyphC(UInstancedStaticMeshComponent* C, const float X, const float Y, const float Z,
        const float W, const float H, const float S, const float D)
    {
        AddRect(C, X, Y, Z + H * 0.5f - S * 0.5f, W, D, S);
        AddRect(C, X, Y, Z - H * 0.5f + S * 0.5f, W, D, S);
        AddRect(C, X - W * 0.5f + S * 0.5f, Y, Z, S, D, H);
    }

    void AddGlyphI(UInstancedStaticMeshComponent* C, const float X, const float Y, const float Z,
        const float W, const float H, const float S, const float D)
    {
        AddRect(C, X, Y, Z, S, D, H);
        AddRect(C, X, Y, Z + H * 0.5f - S * 0.5f, W * 0.64f, D, S);
        AddRect(C, X, Y, Z - H * 0.5f + S * 0.5f, W * 0.64f, D, S);
    }

    void AddGlyphL(UInstancedStaticMeshComponent* C, const float X, const float Y, const float Z,
        const float W, const float H, const float S, const float D)
    {
        const float DiagonalLength = H * 0.93f;
        AddDiagonal(C, X - W * 0.22f, Y, Z - H * 0.03f, DiagonalLength, S, D, -78.0f);
        AddDiagonal(C, X + W * 0.22f, Y, Z - H * 0.03f, DiagonalLength, S, D, 78.0f);
    }

    void AddGlyphSoft(UInstancedStaticMeshComponent* C, const float X, const float Y, const float Z,
        const float W, const float H, const float S, const float D)
    {
        AddRect(C, X - W * 0.34f, Y, Z, S, D, H);
        AddRect(C, X - W * 0.05f, Y, Z - H * 0.5f + S * 0.5f, W * 0.58f, D, S);
        AddRect(C, X - W * 0.05f, Y, Z - H * 0.10f, W * 0.58f, D, S);
        AddRect(C, X + W * 0.24f, Y, Z - H * 0.30f, S, D, H * 0.40f);
    }

    void AddGlyphP(UInstancedStaticMeshComponent* C, const float X, const float Y, const float Z,
        const float W, const float H, const float S, const float D)
    {
        AddRect(C, X - W * 0.5f + S * 0.5f, Y, Z, S, D, H);
        AddRect(C, X + W * 0.5f - S * 0.5f, Y, Z, S, D, H);
        AddRect(C, X, Y, Z + H * 0.5f - S * 0.5f, W, D, S);
    }

    void AddGlyphO(UInstancedStaticMeshComponent* C, const float X, const float Y, const float Z,
        const float W, const float H, const float S, const float D)
    {
        AddRect(C, X, Y, Z + H * 0.5f - S * 0.5f, W, D, S);
        AddRect(C, X, Y, Z - H * 0.5f + S * 0.5f, W, D, S);
        AddRect(C, X - W * 0.5f + S * 0.5f, Y, Z, S, D, H);
        AddRect(C, X + W * 0.5f - S * 0.5f, Y, Z, S, D, H);
    }

    void BuildWord(UInstancedStaticMeshComponent* Component, const float Y,
        const float Scale, const float ExtraStroke)
    {
        const float W = 106.0f * Scale;
        const float H = 145.0f * Scale;
        const float S = (17.0f + ExtraStroke) * Scale;
        const float D = 9.0f * Scale;
        const float Step = 129.0f * Scale;
        const float StartX = -170.0f;
        const float Z = 605.0f;

        AddGlyphC(Component, StartX + Step * 0.0f, Y, Z, W, H, S, D);
        AddGlyphI(Component, StartX + Step * 1.0f, Y, Z, W, H, S, D);
        AddGlyphL(Component, StartX + Step * 2.0f, Y, Z, W, H, S, D);
        AddGlyphSoft(Component, StartX + Step * 3.0f, Y, Z, W, H, S, D);
        AddGlyphP(Component, StartX + Step * 4.0f, Y, Z, W, H, S, D);
        AddGlyphO(Component, StartX + Step * 5.0f, Y, Z, W, H, S, D);
    }
}

bool UOCR13SilpoLogoFallbackSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (!Super::ShouldCreateSubsystem(Outer)) return false;
    const UWorld* World = Cast<UWorld>(Outer);
    return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UOCR13SilpoLogoFallbackSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    if (!InWorld.GetMapName().Contains(TEXT("OsterConflict_Runtime"))) return;

    if (const AOCGameMode* GameMode = InWorld.GetAuthGameMode<AOCGameMode>())
    {
        if (GameMode->IsFrontendOnlySession()) return;
    }

    TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
    FTimerHandle Timer;
    InWorld.GetTimerManager().SetTimer(Timer,
        FTimerDelegate::CreateWeakLambda(this, [this, WeakWorld]()
        {
            if (UWorld* World = WeakWorld.Get()) ValidateLogo(*World);
        }), SilpoLogoFallbackDelaySeconds, false);
}

void UOCR13SilpoLogoFallbackSubsystem::ValidateLogo(UWorld& World)
{
    AActor* Model = FindSilpoModel(World);
    if (!Model || Model->ActorHasTag(TEXT("R13_SilpoLogoGlyphChecked"))) return;

    UTextRenderComponent* LogoText = FindText(Model, TEXT("R13SilpoDetail_LogoText"));
    UTextRenderComponent* LogoOutline = FindText(Model, TEXT("R13SilpoDetail_LogoWhiteOutline"));
    const FString RequiredWord = TEXT("Сільпо");

    const bool bTextFontSupportsCyrillic = LogoText && FontSupports(LogoText->Font, RequiredWord);
    if (bTextFontSupportsCyrillic)
    {
        Model->Tags.Add(TEXT("R13_SilpoLogoGlyphChecked"));
        UE_LOG(LogTemp, Display, TEXT("R13 Silpo logo glyph guard: active TextRender font supports Сільпо; text logo retained."));
        return;
    }

    if (LogoText)
    {
        LogoText->SetVisibility(false, true);
        LogoText->SetHiddenInGame(true, true);
    }
    if (LogoOutline)
    {
        LogoOutline->SetVisibility(false, true);
        LogoOutline->SetHiddenInGame(true, true);
    }

    USceneComponent* Root = Model->GetRootComponent();
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* Basic = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Root || !Cube || !Basic) return;

    UMaterialInstanceDynamic* WhiteMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoGlyph_White"),
        FLinearColor(0.94f, 0.94f, 0.91f, 1.0f));
    UMaterialInstanceDynamic* BlueMat = MakeColorMaterial(Model, Basic, TEXT("R13SilpoGlyph_Blue"),
        FLinearColor(0.025f, 0.095f, 0.30f, 1.0f));
    UInstancedStaticMeshComponent* WhiteWord = MakeISM(Model, Root, Cube, WhiteMat,
        TEXT("R13SilpoGlyph_WhiteOutline"));
    UInstancedStaticMeshComponent* BlueWord = MakeISM(Model, Root, Cube, BlueMat,
        TEXT("R13SilpoGlyph_BlueFace"));

    BuildWord(WhiteWord, -978.0f, 1.04f, 6.0f);
    BuildWord(BlueWord, -988.0f, 1.0f, 0.0f);

    Model->Tags.Add(TEXT("R13_SilpoLogoGlyphChecked"));
    Model->Tags.Add(TEXT("R13_SilpoLogoGeometryFallback"));
    UE_LOG(LogTemp, Warning,
        TEXT("R13 Silpo logo glyph guard: active TextRender font lacks required Cyrillic; geometric СІЛЬПО fallback enabled."));
}
