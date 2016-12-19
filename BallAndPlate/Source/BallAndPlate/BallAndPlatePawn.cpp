// Fill out your copyright notice in the Description page of Project Settings.

#include "BallAndPlate.h"
#include "BallAndPlatePawn.h"
#include <thread>
#include <string>

float a = 0.0f;

float differenceX_Z;
float differenceY_Z;
FRotator rotation;
// Sets default values
ABallAndPlatePawn::ABallAndPlatePawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Create a dummy root component we can attach things to.
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	OurVisibleComponent1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OurVisibleComponent1"));
	OurVisibleComponent2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OurVisibleComponent2"));

	OurVisibleComponent1->SetupAttachment(RootComponent);// AttachTo(RootComponent);
	OurVisibleComponent2->SetupAttachment(RootComponent); // AttachTo(RootComponent);

}

// Called when the game starts or when spawned
void ABallAndPlatePawn::BeginPlay()
{
	Super::BeginPlay();
	differenceX_Z = kolXReference->GetActorLocation().Z - kolX->GetActorLocation().Z;
	differenceY_Z = kolYReference->GetActorLocation().Z - kolY->GetActorLocation().Z;
	rotation = FRotator(RootComponent->GetRelativeTransform().GetRotation());
	setUpLights();
	ConnectToServer();

}

// Called every frame
void ABallAndPlatePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	readValueFromSocket();
	if (!CurrentVelocity.IsZero())
	{
		FVector refXLocation = kolXReference->GetActorLocation();;
		FVector armXLocation = kolX->GetActorLocation();
		armXLocation.Z = refXLocation.Z - differenceX_Z;
		kolX->SetActorLocation(armXLocation);

		FVector refYLocation = kolYReference->GetActorLocation();;
		FVector armYLocation = kolY->GetActorLocation();
		armYLocation.Z = refYLocation.Z - differenceY_Z;
		kolY->SetActorLocation(armYLocation);

		RootComponent->SetRelativeRotation(rotation);
	}
}

// Called to bind functionality to input
void ABallAndPlatePawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{

	Super::SetupPlayerInputComponent(InputComponent);

	// Respond every frame to the values of our two movement axes, "MoveX" and "MoveY".
	InputComponent->BindAxis("MoveX", this, &ABallAndPlatePawn::Move_XAxis);
	InputComponent->BindAxis("MoveY", this, &ABallAndPlatePawn::Move_YAxis);

}

void ABallAndPlatePawn::Move_XAxis(float AxisValue)
{
	// Move at 100 units per second forward or backward
	CurrentVelocity.X = FMath::Clamp(AxisValue, -1.0f, 10.0f) * 100.0f;
	rot.X = FMath::Clamp(AxisValue, -1.0f, 10.0f) * 5.0f;
	rotation.Pitch += (a = AxisValue + 0.01f);
}

void ABallAndPlatePawn::Move_YAxis(float AxisValue)
{
	// Move at 100 units per second right or left
	CurrentVelocity.Y = FMath::Clamp(AxisValue, -1.0f, 1.0f) * 100.0f;
	rot.Y = FMath::Clamp(AxisValue, -1.0f, 1.0f)* 5.0f;
	rotation.Roll += (a = AxisValue + 0.01f);
}

void ABallAndPlatePawn::setUpLights()
{
	int i = 0, j = 0;

	lights.Add(UprightLights1);
	lights.Add(UprightLights2);
	lights.Add(UprightLights3);
	lights.Add(UprightLights4);
	lights.Add(UprightLights5);
	lights.Add(UprightLights6);
	lights.Add(UprightLights7);
	lights.Add(UprightLights8);
	lights.Add(UprightLights9);
	lights.Add(UprightLights10);
	lights.Add(UprightLights11);
	lights.Add(UprightLights12);
	lights.Add(UprightLights13);


	for (i = 0; i < lights.Num(); ++i) {
		for (j = 0; j < lights[i].Num(); ++j) {
			if (lights[i][j] != nullptr) {
				lights[i][j]->SetEnabled(false);
			}

		}
	}
}

void ABallAndPlatePawn::ConnectToServer()
{
	FString address = TEXT("127.0.0.1");
	int32 port = 9999;
	FIPv4Address ipAddress;
	FIPv4Address::Parse(address, ipAddress);
	Connect("SocketListener", ipAddress, port);
	if (!didConnect)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Can't connect to socket.Program closing!")));
		//FPlatformProcess::Sleep(2);
		//FGenericPlatformMisc::RequestExit(false);
	}
}

void ABallAndPlatePawn::Connect(
	const FString& YourChosenSocketName,
	const FIPv4Address& ip,
	int32 port
	) {

	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.Value);
	addr->SetPort(port);

	ConnectionSocket = FTcpSocketBuilder(YourChosenSocketName).AsBlocking().AsReusable().Build();
	didConnect = ConnectionSocket->Connect(*addr);
	if (didConnect)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Successful")));
	else
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Failed")));

}

void ABallAndPlatePawn::readValueFromSocket() {
	FString serialized = TEXT("GET_COORDINATE");
	TCHAR *serializedChar = serialized.GetCharArray().GetData();
	int32 size = FCString::Strlen(serializedChar);
	int32 sent = 0;
	
	bool succesfull = ConnectionSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
	if (succesfull)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Successfulllllllllll")));
		
		TArray<uint8> ReceivedData;
		uint32 Size;
		ReceivedData.Init(0, FMath::Min(Size, 65507u));
		int32 Read = 0;
		
		succesfull  = ConnectionSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);
		if (succesfull && ReceivedData.Num() != 0 ) {
			const FString ReceivedUE4String = StringFromBinaryArray(ReceivedData);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("As String Data ~> %s"), *ReceivedUE4String));
		}
		else {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("No receive ")));

		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Failedddd")));
	}

	// servera istek gonder
	// mesaj gelmesini bekle
	// mesaji al
	// gelen bilgiyi parse et
	// degiskeneler assign et
}

void ABallAndPlatePawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (ConnectionSocket)
	{
		ConnectionSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket);
	}
}

///////////////////// RAMA'S FUNCTIONS ///////////////////
/*
//Rama's Create TCP Connection Listener
FSocket* ABallAndPlatePawn::CreateTCPConnectionListener(const FString& YourChosenSocketName, const FString& TheIP, const int32 ThePort, const int32 ReceiveBufferSize)
{
uint8 IP4Nums[4];
if (!FormatIP4ToNumber(TheIP, IP4Nums))
{
GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Invalid IP!Expecting 4 parts separated by .")));
return false;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//Create Socket
FIPv4Endpoint Endpoint(FIPv4Address(IP4Nums[0], IP4Nums[1], IP4Nums[2], IP4Nums[3]), ThePort);
FSocket* ListenSocket = FTcpSocketBuilder(*YourChosenSocketName)
.AsReusable()
.BoundToEndpoint(Endpoint)
.Listening(8);

//Set Buffer Size
int32 NewSize = 0;
ListenSocket->SetReceiveBufferSize(ReceiveBufferSize, NewSize);

//Done!
return ListenSocket;
}

//Rama's TCP Connection Listener
void ABallAndPlatePawn::TCPConnectionListener()
{
//~~~~~~~~~~~~~
if (!ListenerSocket) return;
//~~~~~~~~~~~~~

//Remote address
TSharedRef<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
bool Pending;

// handle incoming connections
if (ListenerSocket->HasPendingConnection(Pending) && Pending)
{
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Already have a Connection? destroy previous
if (ConnectionSocket)
{
ConnectionSocket->Close();
ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket);
GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Removed old socket")));
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//New Connection receive!
ConnectionSocket = ListenerSocket->Accept(*RemoteAddress, TEXT("Received Socket Connection"));

if (ConnectionSocket != NULL)
{
//Global cache of current Remote Address
RemoteAddressForConnection = FIPv4Endpoint(RemoteAddress);
GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("New Connection came.")));

//UE_LOG "Accepted Connection! WOOOHOOOO!!!";

//can thread this too
//FTimerDelegate MyDel;

//			MyDel.BindRaw(this, &ABallAndPlatePawn::TCPSocketListener);
GetWorldTimerManager().SetTimer(timeHandler, this,
&ABallAndPlatePawn::TCPSocketListener, 0.01f, true);
//SetTimer(MyDel, 0.01, true);
}
}
}
//Rama's TCP Socket Listener
void ABallAndPlatePawn::TCPSocketListener()
{
	//~~~~~~~~~~~~~
	if (!ConnectionSocket) return;
	//~~~~~~~~~~~~~


	//Binary Array!
	TArray<uint8> ReceivedData;

	uint32 Size;
	while (ConnectionSocket->HasPendingData(Size))
	{

		ReceivedData.Init(0, FMath::Min(Size, 65507u));

		int32 Read = 0;
		ConnectionSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Data Read! %d"), ReceivedData.Num()));
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	if (ReceivedData.Num() <= 0)
	{
		return;
	}

	//VShow("Total Data read!", ReceivedData.Num());
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Data Bytes Read ~> %d"), ReceivedData.Num()));


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//						Rama's String From Binary Array
	const FString ReceivedUE4String = StringFromBinaryArray(ReceivedData);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


	//VShow("As String!!!!! ~>", ReceivedUE4String);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("As String Data ~> %s"), *ReceivedUE4String));
}
*/
FString ABallAndPlatePawn::StringFromBinaryArray(const TArray<uint8>& BinaryArray)
{
	//Create a string from a byte array!
	const std::string cstr(reinterpret_cast<const char*>(BinaryArray.GetData()), BinaryArray.Num());

	//FString can take in the c_str() of a std::string
	return FString(cstr.c_str());
}
