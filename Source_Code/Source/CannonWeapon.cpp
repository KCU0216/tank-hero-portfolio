// Fill out your copyright notice in the Description page of Project Settings.



#include "CannonWeapon.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DangerIndicatorComponent.h"
#include "Projectile.h"
#include "DangerUtils.h"


// Sets default values for this component's properties
UCannonWeapon::UCannonWeapon()
{
	PrimaryComponentTick.bCanEverTick = true;

	TurretPivot = CreateDefaultSubobject<USceneComponent>(TEXT("TurretPivot"));
	CannonTurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CannonTurretMesh"));
	BarrelPivot = CreateDefaultSubobject<USceneComponent>(TEXT("BarrelPivot"));
	AimRef = CreateDefaultSubobject<USceneComponent>(TEXT("AimRef"));

	FBarrel B;
	B.Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh"));
	B.HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
	B.SpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPoint"));
	Barrels.Add(B);
	// 생성만, attach는 액터(미니언)가 해줌
}

//void UCannonWeapon::OnRegister()
//{
//    Super::OnRegister();
//
//    if (TurretPivot)
//        TurretPivot->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
//    if (CannonTurretMesh)
//        CannonTurretMesh->AttachToComponent(TurretPivot, FAttachmentTransformRules::KeepRelativeTransform);
//    if (BarrelPivot)
//        BarrelPivot->AttachToComponent(TurretPivot, FAttachmentTransformRules::KeepRelativeTransform);
//    if (AimRef)
//        AimRef->AttachToComponent(BarrelPivot, FAttachmentTransformRules::KeepRelativeTransform);
//}


// Called when the game starts
void UCannonWeapon::BeginPlay()
{
	Super::BeginPlay();

	// 캐논 BeginPlay에 추가
	UE_LOG(LogTemp, Warning, TEXT("TurretPivot AttachParent: %s, Owner: %s"),
		TurretPivot->GetAttachParent() ? *TurretPivot->GetAttachParent()->GetName() : TEXT("NULL"),
		TurretPivot->GetOwner() ? *TurretPivot->GetOwner()->GetName() : TEXT("NULL"));
	if (!ProjectileClass) return;
	
		AProjectile* CDO = ProjectileClass->GetDefaultObject<AProjectile>();
		if (CDO && CDO->GetMoveComp()&&GetWorld())
		{
			CachedSpeed = CDO->GetMoveComp()->InitialSpeed;
			CachedGravity = GetWorld()->GetGravityZ() * CDO->GetMoveComp()->ProjectileGravityScale;
		}

		
	
	// ...
	
}


// Called every frame
void UCannonWeapon::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UCannonWeapon::Aim(const FVector& Target, float DeltaTime)
{
	const FVector AimOrigin = AimRef->GetComponentLocation();
	const FVector GoalDist = Target - AimOrigin;
	if (!UDangerUtils::CalcLaunchPitch(GoalDist.Size2D(), GoalDist.Z, CachedSpeed, CachedGravity, GoalRotator.Pitch))
		return false;

	GoalRotator.Yaw = GoalDist.Rotation().Yaw;
	GoalRotator.Roll = 0.f;

	const FRotator NewTurretRot = FMath::RInterpTo(
	TurretPivot->GetComponentRotation(), FRotator(0,GoalRotator.Yaw, 0), DeltaTime, RotationSpeed);
	TurretPivot->SetWorldRotation(FRotator(0, NewTurretRot.Yaw, 0));

	const FRotator NewBarrelRot = FMath::RInterpTo(
	BarrelPivot->GetComponentRotation(), FRotator(GoalRotator.Pitch, 0, 0), DeltaTime, RotationSpeed);
	BarrelPivot->SetWorldRotation(FRotator(NewBarrelRot.Pitch, NewTurretRot.Yaw, 0));

	return true;
}

bool UCannonWeapon::IsReady() const
{
	const FVector Forward = AimRef->GetForwardVector();
	const FVector Goal = GoalRotator.Vector();
	const float Dot = FVector::DotProduct(Forward, Goal);
	return (Dot >= AimThreshold);
}

void UCannonWeapon::Fire()
{
	UWorld* World = GetWorld();
	if (!World || !ProjectileClass) return;

	for (FBarrel& B : Barrels)
	{
		if (!B.bAlive || !B.SpawnPoint) continue;

		const FTransform T = B.SpawnPoint->GetComponentTransform();
		AProjectile* Proj = World->SpawnActor<AProjectile>(
			ProjectileClass, T.GetLocation(), T.Rotator());
		if (Proj) Proj->SetOwner(GetOwner());
	}
}

bool UCannonWeapon::HandleHit(UPrimitiveComponent* Hit, float Damage)
{
	for (FBarrel& B : Barrels)
	{
		if (B.HitBox == Hit && B.bAlive)
		{
			B.Health -= Damage;
			if (B.Health <= 0.f)
			{
				B.bAlive = false;
				if (B.Mesh) B.Mesh->SetVisibility(false);   // 연출(선택)
				if (B.HitBox) B.HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
			return true;   // 내 총열 맞음 — 처리 끝
		}
	}
	return false;   // 내 거 아님 — 보스가 다른 무기/몸체 체크하게
}

void UCannonWeapon::TryFire(const FVector& Target, float DeltaTime)
{
	if (!bCanAim) return;              // 재장전 중이면 무시
	if (!Aim(Target, DeltaTime)) return;  // 조준(매프레임 수렴)
	if (!IsReady()) return;            // 아직 정렬 안 됨

	// 조준 완료 → 사이클 시작
	bCanAim = false;                   // 잠금

	if (DangerIndicator)               // 위험표시 (살아있는 총열마다)
	{
		for (FBarrel& B : Barrels)
		{
			if (!B.bAlive || !B.SpawnPoint) continue;
			FVector Loc = B.SpawnPoint->GetComponentLocation();
			FVector Vel = B.SpawnPoint->GetForwardVector() * CachedSpeed;
			TArray<FVector> Pts = UDangerUtils::PredictTrajectory(this, Loc, Vel, CachedGravity);
			DangerIndicator->ShowTrajectory(Pts, AimDelay);
		}
	}

	GetWorld()->GetTimerManager().SetTimer(AimTimer, this, &UCannonWeapon::OnFired, AimDelay, false);
}

void UCannonWeapon::OnFired()
{
	Fire();
	GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &UCannonWeapon::FinishReload, ReloadTime, false);
}

void UCannonWeapon::FinishReload()
{
	bCanAim = true;
}