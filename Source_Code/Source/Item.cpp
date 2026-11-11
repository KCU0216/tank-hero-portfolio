// Fill out your copyright notice in the Description page of Project Settings.
#include "Item.h"
#include "Kismet/GameplayStatics.h"
#include "Tank.h"


// Sets default values
AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();


	PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!PlayerPawn)
	{
		PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
		
		if (!PlayerPawn) return;
	}
	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	FVector Direction = PlayerLocation - GetActorLocation();
	float Dist = Direction.Size();
	if (Dist <= PickupRange)
	{
		ApplyEffect(PlayerPawn);
		Destroy();
	}
}

void AItem::ApplyEffect(APawn* Player)
{
	ATank* Tank = Cast<ATank>(Player);
	
}
