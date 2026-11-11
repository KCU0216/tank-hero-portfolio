// Fill out your copyright notice in the Description page of Project Settings.

#include "LaserMinion.h"
#include "LaserWeaponComponent.h"
#include "HealthComponent.h"
#include "NiagaraComponent.h"

ALaserMinion::ALaserMinion()
{
	// 눈 구조 (보스 복붙). BaseMesh는 BasePawn 거.
	YawPivot = CreateDefaultSubobject<USceneComponent>(TEXT("YawPivot"));
	YawPivot->SetupAttachment(BaseMesh);

	LaserPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LaserPivot"));
	LaserPivot->SetupAttachment(YawPivot);

	LaserMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserMesh"));
	LaserMesh->SetupAttachment(LaserPivot);

	LaserSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("LaserSpawnPoint"));
	LaserSpawnPoint->SetupAttachment(LaserPivot);

	LaserComp = CreateDefaultSubobject<ULaserWeaponComponent>(TEXT("LaserComp"));

	ChargeComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ChargeComp"));
	ChargeComp->SetupAttachment(LaserSpawnPoint);
	ChargeComp->SetAutoActivate(false);
	ChargeComp->SetUsingAbsoluteScale(true);

	BeamComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BeamComp"));
	BeamComp->SetupAttachment(LaserSpawnPoint);
	BeamComp->SetAutoActivate(false);
}

void ALaserMinion::BeginPlay()
{
	Super::BeginPlay(); // ATower: OnDeath 바인딩, CDO 캐싱
	OnTakeAnyDamage.AddDynamic(this, &ATower::HandleDamage);
	HealthComp->OnDeath.AddDynamic(this, &ATower::OnDeath);
	LaserComp->Init(LaserPivot, LaserSpawnPoint, LaserMesh);
	LaserComp->SetYawPivot(YawPivot);
	LaserComp->SetChargeComp(ChargeComp);
	LaserComp->SetBeamComp(BeamComp);
}

void ALaserMinion::UpdateCombat(const FVector& PlayerLoc, float DeltaTime)
{
	// 페이즈/스턴 가드 없음. bReady 가드가 쿨 관리.
	// Aim 먼저(그 틱 회전 반영) -> Trace(그 방향으로 빔).
	LaserComp->Aim(PlayerLoc, DeltaTime);
	LaserComp->BeginFire();
	LaserComp->Trace(PlayerLoc, DeltaTime);
}