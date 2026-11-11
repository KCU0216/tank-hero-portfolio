// Fill out your copyright notice in the Description page of Project Settings.

#include "CubeTankGameMode.h"
#include "BasePawn.h"
#include "Tank.h"
#include "Kismet/GameplayStatics.h"
#include "Tower.h"
#include "TankPlayerController.h"
#include "Item.h"

ACubeTankGameMode::ACubeTankGameMode()
{

}

void ACubeTankGameMode::ActorDied(AActor* DeadActor)
{
	if (DeadActor == Tank)
	{
		GameOver(false);
		TankPC->EnableTankInput(false);
	}
	else
	{
			if (Cast<ATower>(DeadActor))
				++TargetTowers;
	}
}

void ACubeTankGameMode::BeginPlay()
{
	Super::BeginPlay();
	Tank = Cast<ATank>(UGameplayStatics::GetPlayerPawn(this, 0));
	TankPC = Cast<ATankPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if (!Tank || !TankPC) return;
	TankPC->EnableTankInput(true); //can't move at firsttime
	 // will define in the blueprint

	FTimerHandle PlayerEnableTimerHandle;
	FTimerDelegate PlayerEnableTimerDelegate =
		FTimerDelegate::CreateUObject(this, &ACubeTankGameMode::HandleGameStart);

	GetWorldTimerManager().SetTimer(PlayerEnableTimerHandle,
		PlayerEnableTimerDelegate,
		StartDelay,
		false);
}

void ACubeTankGameMode::HandleGameStart()
{
	if (!TankPC) return;
	TankPC->EnableTankInput(true);

}

int ACubeTankGameMode::GetTargetTowerCount()
{
	return TargetTowers;
}
