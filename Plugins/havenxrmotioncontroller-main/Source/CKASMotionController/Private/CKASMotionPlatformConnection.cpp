// Copyright Epic Games, Inc. All Rights Reserved.

#include "CKASMotionPlatformConnection.h"
#include "Common/UdpSocketBuilder.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

UCKASMotionPlatformConnection::UCKASMotionPlatformConnection()
{
	SocketSubsystem = nullptr;
	Socket = nullptr;
}

void UCKASMotionPlatformConnection::BeginDestroy()
{
	Disconnect();
	Super::BeginDestroy();
}

bool UCKASMotionPlatformConnection::Connect(const FString SetIP, const int SetPort)
{
	// Check if SetIP is empty or invalid
	if (SetIP.IsEmpty() || (!FIPv4Address::Parse(SetIP, RemoteAddress)))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid IP Address: %s"), *SetIP);
		return false;
	}

	RemoteEndpoint = FIPv4Endpoint(RemoteAddress, static_cast<uint16>(SetPort));

	if (SocketSubsystem == nullptr) SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

	// Set LocalEndpoint to Any address with LocalPort
	LocalEndpoint = FIPv4Endpoint(FIPv4Address::Any, LocalPort);

	// If SocketSubsystem is valid and Socket is not yet initialized, create the UDP socket
	if (SocketSubsystem != nullptr && Socket == nullptr)
	{
		Socket = FUdpSocketBuilder(SocketDescription)
			.AsNonBlocking()
			.AsReusable()
			.BoundToEndpoint(LocalEndpoint)
			.WithSendBufferSize(SendSize)
			.WithReceiveBufferSize(BufferSize)
			.WithBroadcast()
			.Build();

		// Check if socket creation failed
		if (Socket == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create UDP socket."));
			return false;
		}
		UE_LOG(LogTemp, Log, TEXT("UDP socket created successfully."));
	}

	// Return true if Socket is not nullptr, indicating successful connection
	return (Socket != nullptr);
}

bool UCKASMotionPlatformConnection::Disconnect()
{
	if (SocketSubsystem != nullptr)
	{
		SocketSubsystem->DestroySocket(Socket);
		Socket = nullptr;
		SocketSubsystem = nullptr;
		return true;
	}
	return false;
}

FORCEINLINE bool UCKASMotionPlatformConnection::SendMessage(const FString& Message, const bool IsConnected)
{
	// Check if Socket is valid and connection is established
	if (!Socket || !IsConnected) return false;

	// Convert FString to const TCHAR* and determine size
	const TCHAR* SerializedChar = Message.GetCharArray().GetData();
	const int32 Size = FCString::Strlen(SerializedChar);

	// Convert TCHAR* to UTF-8 and send over the socket
	int32 BytesSent;
	const bool bSuccess = Socket->SendTo(reinterpret_cast<uint8*>(TCHAR_TO_UTF8(SerializedChar)), Size, BytesSent, *RemoteEndpoint.ToInternetAddr());

	// UE_LOG(LogTemp, Warning, TEXT("Sent message: %s : %s : Address - %s : BytesSent - %d"), *Message, bSuccess ? TEXT("true") : TEXT("false"), *RemoteEndpoint.ToString(), BytesSent);

	// Ensure message was successfully sent and bytes sent is greater than 0
	return bSuccess && BytesSent > 0;
}

void UCKASMotionPlatformConnection::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UCKASMotionPlatformConnection::Deinitialize()
{
	Disconnect();

	Super::Deinitialize();
}
