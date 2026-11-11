// Fill out your copyright notice in the Description page of Project Settings.


#include "MortarMinion.h"
#include "MortarWeaponComponent.h"
#include "HealthComponent.h"

AMortarMinion::AMortarMinion()
{
	
	YawPivot = CreateDefaultSubobject<USceneComponent>(TEXT("YawPivot"));
	YawPivot->SetupAttachment(BaseMesh);

	MortarPivot = CreateDefaultSubobject<USceneComponent>(TEXT("MortarPivot"));
	MortarPivot->SetupAttachment(YawPivot);

	MortarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MortarMesh"));
	MortarMesh->SetupAttachment(MortarPivot);

	MortarSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("MortarSpawnPoint"));
	MortarSpawnPoint->SetupAttachment(MortarPivot);

	MortarComp=CreateDefaultSubobject<UMortarWeaponComponent>(TEXT("MortarComp"));
	
}

void AMortarMinion::BeginPlay()
{
	Super::BeginPlay(); // ATower: OnDeath 바인딩, CDO 캐싱
	OnTakeAnyDamage.AddDynamic(this, &ATower::HandleDamage);
	HealthComp->OnDeath.AddDynamic(this, &ATower::OnDeath);
	MortarComp->Init(MortarPivot, MortarPivot, MortarPivot, MortarPivot, MortarSpawnPoint, MortarSpawnPoint);
	MortarComp->SetRotateMode(true);
	MortarComp->SetYawPivot(YawPivot);
}

void AMortarMinion::OnDeath()
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	MortarComp->DestroyAllShots();
	Super::OnDeath();
}

void AMortarMinion::UpdateCombat(const FVector& PlayerLoc, float DeltaTime)
{

	MortarComp->BeginFire(PlayerLoc);
	MortarComp->UpdatePivots(DeltaTime);


}
