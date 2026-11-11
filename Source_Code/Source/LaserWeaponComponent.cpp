// Fill out your copyright notice in the Description page of Project Settings.


#include "LaserWeaponComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"

// Sets default values for this component's properties
ULaserWeaponComponent::ULaserWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void ULaserWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	DefaultChargeTime = LaserChargeTime;
	// ...
	
}


// Called every frame
void ULaserWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void ULaserWeaponComponent::Init(USceneComponent* Pivot, USceneComponent* Point, UStaticMeshComponent* Mesh)
{
	LaserPivot = Pivot;
	LaserSpawnPoint = Point;
	LaserMesh = Mesh;
}

void ULaserWeaponComponent::BeginFire()
{
	if (bReady == false) return;
	LaserChargeTime = DefaultChargeTime;
	bReady = false;
	bCharging = true;
	if (ChargeComp)
	{
		ChargeComp->Activate(true);
		ChargeComp->SetVariableBool(TEXT("bCharged"), false);
	}

	CurrentLaserType = bRandomType
		? (FMath::RandBool() ? ELaserType::LaserSweep : ELaserType::LaserTracking)
		: ELaserType::LaserTracking;   // 고정이면 Tracking

	if (CurrentLaserType == ELaserType::LaserSweep)
		SetupSweep();

	GetWorld()->GetTimerManager().SetTimer(
		LaserChargeTimer, this, &ULaserWeaponComponent::CheckAutoFire, LaserChargeTime, false);


}
void ULaserWeaponComponent::SetupSweep()
{
	SweepSpeed = SweepInitialSpeed;
	SweepHitActors.Empty();
	float Rand = FMath::FRandRange(20.f, SweepOffset);
	SweepOffsetPitch = FMath::RandBool() ? Rand : -Rand;
	Rand = FMath::FRandRange(20.f, SweepOffset);
	SweepOffsetYaw = FMath::RandBool() ? Rand : -Rand;
	SweepDir = FVector2D(-SweepOffsetPitch, -SweepOffsetYaw).GetSafeNormal();
}
void ULaserWeaponComponent::Aim(const FVector& Target, float DeltaTime)
{
	if (!bCharging && !bLaserFiring) return;

	// ---- Sweep 발사 중: 누적 회전 ----
	if (bLaserFiring && CurrentLaserType == ELaserType::LaserSweep)
	{
		if (YawPivot)
		{
			float NewYaw = YawPivot->GetComponentRotation().Yaw + SweepDir.Y * SweepSpeed * DeltaTime;
			YawPivot->SetWorldRotation(FRotator(0, NewYaw, 0));

			float NewPitch = LaserPivot->GetRelativeRotation().Pitch + SweepDir.X * SweepSpeed * DeltaTime;
			LaserPivot->SetRelativeRotation(FRotator(NewPitch, 0, 0));
		}
		else
		{
			float NewPitch = LaserPivot->GetComponentRotation().Pitch + SweepDir.X * SweepSpeed * DeltaTime;
			float NewYaw = LaserPivot->GetComponentRotation().Yaw + SweepDir.Y * SweepSpeed * DeltaTime;
			LaserPivot->SetWorldRotation(FRotator(NewPitch, NewYaw, 0));
		}
		SweepSpeed += SweepAccel * DeltaTime;
		return;
	}

	// ---- 충전 / Tracking 발사: 목표 조준 ----
	FRotator GoalRotator = (Target - LaserPivot->GetComponentLocation()).Rotation();

	if (CurrentLaserType == ELaserType::LaserSweep)
	{
		GoalRotator.Pitch += SweepOffsetPitch;
		GoalRotator.Yaw += SweepOffsetYaw;
	}

	if (YawPivot)
	{
		// yaw → YawPivot (월드)
		FRotator NewYawRot = FMath::RInterpTo(
			YawPivot->GetComponentRotation(), FRotator(0, GoalRotator.Yaw, 0), DeltaTime, EyeRotationSpeed);
		YawPivot->SetWorldRotation(FRotator(0, NewYawRot.Yaw, 0));

		// pitch → LaserPivot (상대)
		FRotator NewPitchRot = FMath::RInterpTo(
			LaserPivot->GetRelativeRotation(), FRotator(GoalRotator.Pitch, 0, 0), DeltaTime, EyeRotationSpeed);
		LaserPivot->SetRelativeRotation(FRotator(NewPitchRot.Pitch, 0, 0));
	}
	else
	{
		FRotator NewRotator = FMath::RInterpTo(
			LaserPivot->GetComponentRotation(), GoalRotator, DeltaTime, EyeRotationSpeed);
		LaserPivot->SetWorldRotation(NewRotator);
	}
}

void ULaserWeaponComponent::SetChargeTime(float Time)
{
	DefaultChargeTime = Time;
	LaserChargeTime = Time;
}

void ULaserWeaponComponent::CheckAutoFire()
{
	if (bAutoFire)
	{
		FireLaser();
	}
	else
	{
		if (ChargeComp) ChargeComp->SetVariableBool(TEXT("bCharged"), true);
		bChargedReady = true;
	}

}
void ULaserWeaponComponent::Release()
{
	if (bChargedReady) { FireLaser(); return; }   // 완충 → 발사
	if (bCharging) { CancelCharge(); return; }     // 미완충 → 취소
}

void ULaserWeaponComponent::FireLaser()
{
	bCharging = false;
	bChargedReady = false;
	bLaserFiring = true;
	if (ChargeComp) ChargeComp->Deactivate();
	SweepHitActors.Empty();

	GetWorld()->GetTimerManager().SetTimer(
		LaserFireTimer, this, &ULaserWeaponComponent::EndFire, LaserFireTime, false);

	if (BeamComp)BeamComp->Activate(true);
}
void ULaserWeaponComponent::Trace(const FVector& Target, float DeltaTime)
{
	if (!bLaserFiring) return;

	FVector Start = LaserSpawnPoint->GetComponentLocation();
	FVector Dir = bAimAtTarget
		? (Target - Start).GetSafeNormal()        // 플레이어: 조준점으로
		: LaserSpawnPoint->GetForwardVector();    // 보스/미니언: 총구 forward
	FVector End = Start + LaserDist * Dir;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	TArray<FHitResult> OutHits;
	GetWorld()->LineTraceMultiByChannel(OutHits, Start, End, ECC_GameTraceChannel4, Params);

	TSet<AActor*> DamagedActors; // 한 틱 내 액터당 1회. 히트박스 여러 개 걸려도 데미지는 첫 히트 기준 1번
	for (FHitResult& Hit : OutHits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor) continue;
		if (Hit.GetComponent() && Hit.GetComponent()->ComponentHasTag(TEXT("Shield")))
			continue;   // 방패 = 무적, 빔은 여기서 막히지만 딩은 안 줌
		if (DamagedActors.Contains(HitActor)) continue;
		

		float Dmg;
		if (CurrentLaserType == ELaserType::LaserTracking)
		{
			Dmg = LaserDamage * DeltaTime;          // 지속딜
		}
		else // Sweep: 패스당 1회 풀데미지
		{
			if (SweepHitActors.Contains(HitActor)) continue;
			SweepHitActors.Add(HitActor);
			Dmg = LaserDamage;
		}
		DamagedActors.Add(HitActor);
		UGameplayStatics::ApplyPointDamage(HitActor, Dmg, -Hit.ImpactNormal, Hit,
			GetOwner()->GetInstigatorController(), GetOwner(), UDamageType::StaticClass());
	}

	FVector BeamEnd = End;
	if (OutHits.Num() > 0 && OutHits.Last().bBlockingHit)
		BeamEnd = OutHits.Last().ImpactPoint;
	if (BeamComp)
	{
		BeamComp->SetVectorParameter(TEXT("Beam End"), BeamEnd);
	}
}

void ULaserWeaponComponent::EndFire()
{
	if (!bLaserFiring) return; // 멱등

	if (BeamComp) BeamComp->Deactivate();
	if (CurrentLaserType == ELaserType::LaserSweep && ++SweepCount < 2)
	{
		// 충전으로 복귀: offset 다시 뽑고 재조준 예고

		LaserChargeTime *= 0.5f;
		bLaserFiring = false;
		bCharging = true;
		if (ChargeComp)
		{
			ChargeComp->Activate(true);
			ChargeComp->SetVariableBool(TEXT("bCharged"), false);
		}
		SetupSweep();

		GetWorld()->GetTimerManager().SetTimer(
			LaserChargeTimer, this, &ULaserWeaponComponent::FireLaser, LaserChargeTime, false);
		return;
	}
	bLaserFiring = false;
	SweepCount = 0;
	GetWorld()->GetTimerManager().SetTimer(
		LaserReloadTimer, this, &ULaserWeaponComponent::Reload, LaserReloadTime, false);
}

void ULaserWeaponComponent::Reload()
{
	bReady = true;
}

void ULaserWeaponComponent::StopSequence()
{
	FTimerManager& TM = GetWorld()->GetTimerManager();
	TM.ClearTimer(LaserChargeTimer);
	TM.ClearTimer(LaserFireTimer);
	TM.ClearTimer(LaserReloadTimer);
	if (BeamComp) BeamComp->Deactivate();
	if (ChargeComp) ChargeComp->Deactivate();

	bChargedReady = false;
	bCharging = false;
	bLaserFiring = false;
	SweepCount = 0;
	SweepHitActors.Empty();
	bReady = true;
}

void ULaserWeaponComponent::ApplyEnrage()
{
	SweepAccel *= 1.5f;
	LaserReloadTime *= 0.5f;
}


void ULaserWeaponComponent::CancelCharge()
{
	if (!bCharging) return;   // 충전 중 아니면 무시 (발사 중이면 EndFire 따로 부름)
	if (ChargeComp) ChargeComp->Deactivate();
	GetWorld()->GetTimerManager().ClearTimer(LaserChargeTimer);
	bCharging = false;
	bReady = true;            // 다시 충전 가능
}

bool ULaserWeaponComponent::IsReady() const
{
	return bReady;
}

float ULaserWeaponComponent::GetReloadProgress() const
{
	if (bReady) return 1.f;
	if (bCharging || bChargedReady) return 1.f;   // 아직 발사 전 = 소모 안 됨

	float Elapsed = GetWorld()->GetTimerManager().GetTimerElapsed(LaserReloadTimer);
	if (Elapsed < 0.f) return 0.f;   // 발사 중
	return Elapsed / LaserReloadTime;
}