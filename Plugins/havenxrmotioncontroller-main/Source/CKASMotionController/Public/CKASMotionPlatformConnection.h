// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Networking.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CKASMotionPlatformConnection.generated.h"

UCLASS()
class CKASMOTIONCONTROLLER_API UCKASMotionPlatformConnection : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UCKASMotionPlatformConnection();
	void BeginDestroy();

	UFUNCTION(BlueprintCallable, Category = "CKAS Motion Platform")
	bool Connect(UPARAM(DisplayName = "Remote IP") FString SetIP = "", UPARAM(DisplayName = "Remote Port") int SetPort = 1288);

	UFUNCTION(BlueprintCallable, Category = "CKAS Motion Platform")
	bool Disconnect();

	UFUNCTION(BlueprintCallable, Category = "CKAS Motion Platform")
	bool SendMessage(UPARAM(DisplayName = "Message") const FString& Message, bool IsConnected);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	FString SocketDescription = "UDP Listen Socket";
	FIPv4Endpoint LocalEndpoint;
	FIPv4Endpoint RemoteEndpoint;
	FIPv4Address RemoteAddress;
	uint16 LocalPort = 54000;
	uint16 RemotePort = 1288;
	FString RemoteIP = "127.0.0.1";
	int32 SendSize = 2 * 1024 * 1024;
	int32 BufferSize = 2 * 1024 * 1024;
	TArray<uint8> ReceivedData;
	ISocketSubsystem* SocketSubsystem;
	FSocket* Socket;
};
