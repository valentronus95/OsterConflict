#include "OCGameInstance.h"

#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/Connection/NetEnums.h"

#define LOCTEXT_NAMESPACE "OCConnection"

void UOCGameInstance::Init()
{
    Super::Init();
    ConnectionStatusText = LOCTEXT("FrontendReady", "Готово до підключення.");
    if (GEngine)
    {
        GEngine->OnNetworkFailure().AddUObject(this, &UOCGameInstance::HandleNetworkFailure);
        GEngine->OnTravelFailure().AddUObject(this, &UOCGameInstance::HandleTravelFailure);
    }
}

void UOCGameInstance::Shutdown()
{
    if (GEngine)
    {
        GEngine->OnNetworkFailure().RemoveAll(this);
        GEngine->OnTravelFailure().RemoveAll(this);
    }
    Super::Shutdown();
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
