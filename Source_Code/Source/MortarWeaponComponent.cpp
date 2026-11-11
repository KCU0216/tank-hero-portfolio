#include "MortarWeaponComponent.h"
#include "MortarProjectile.h"
#include "DangerUtils.h"
#include "Components/DecalComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UMortarWeaponComponent::UMortarWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // 보스가 매 틱 지휘
}

void UMortarWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	// 기본 위치 캐싱 (내릴 때 복귀 지점)
	if (MortarPivot_L) MortarDefaultPos_L = MortarPivot_L->GetRelativeLocation();
	if (MortarPivot_R) MortarDefaultPos_R = MortarPivot_R->GetRelativeLocation();

	// CDO 캐싱 - 단일 소스
	if (MortarProjectileClass)
	{
		AMortarProjectile* CDO = MortarProjectileClass->GetDefaultObject<AMortarProjectile>();
		if (CDO && CDO->GetMoveComp())
		{
			CachedSpeed = CDO->GetMoveComp()->MaxSpeed;
			CachedGravity = GetWorld()->GetGravityZ() * CDO->GetMoveComp()->ProjectileGravityScale;
		}
	}
}

void UMortarWeaponComponent::Init(USceneComponent* InPivotL, USceneComponent* InPivotR,
	USceneComponent* InGoalL, USceneComponent* InGoalR,
	USceneComponent* InSpawnL, USceneComponent* InSpawnR)
{
	MortarPivot_L = InPivotL;
	MortarPivot_R = InPivotR;
	MortarPivotGoal_L = InGoalL;
	MortarPivotGoal_R = InGoalR;
	MortarSpawnPoint_L = InSpawnL;   // ← 추가
	MortarSpawnPoint_R = InSpawnR;   // ← 추가

	MortarDefaultPos_L = MortarPivot_L->GetRelativeLocation();
	MortarDefaultPos_R = MortarPivot_R->GetRelativeLocation();

}

// =====================================================================
//  사이클: BeginFire(위치확정+데칼) -> 올리기 -> OnRaised(발사) -> Reload(내리기)
// =====================================================================
void UMortarWeaponComponent::BeginFire(const FVector& Target)
{
	if (!bReady) return;
	if (!PlayerPawn || !MortarPivot_L) return;

	bReady = false;

	// 오프셋 + 위치 확정 (기존과 동일)
	CachedOffsets.Empty();
	for (int32 i = 0; i < MortarShotCount; i++)
	{
		const float RandX = FMath::RandRange(-MortarScatterRadius, MortarScatterRadius);
		const float MaxY = FMath::Sqrt(MortarScatterRadius * MortarScatterRadius - RandX * RandX);
		const float RandY = FMath::RandRange(-MaxY, MaxY);
		CachedOffsets.Add(FVector(RandX, RandY, 0));
	}

	CachedGoalLocs.Empty();
	for (int32 i = 0; i < MortarShotCount; i++)
		CachedGoalLocs.Add(Target + CachedOffsets[i]);

	if (bRotateMode)
	{
		const FVector Origin = MortarSpawnPoint_L->GetComponentLocation();
		const FVector Dist = CachedGoalLocs[0] - Origin;
		double Pitch = 0.0;
		UDangerUtils::CalcLaunchPitch(Dist.Size2D(), Dist.Z, CachedSpeed, CachedGravity, Pitch, true);

		MortarGoalRot.Yaw = Dist.Rotation().Yaw;
		MortarGoalRot.Pitch = Pitch;
		MortarGoalRot.Roll = 0.f;
		// 월드 기준 목표라, RInterp도 월드 회전으로 비교/적용할 것 (UpdatePivots에서)
	}

	for (FMortarShot& Shot : MortarShots)
	{
		if (Shot.Proj)  Shot.Proj->Destroy();
		if (Shot.Decal) Shot.Decal->DestroyComponent();
	}
	MortarShots.Empty();
	MortarShots.SetNum(MortarShotCount);

	// 데칼 그리기 + 올리기 동시 시작
	DecalCursor = 0;
	DrawDecal();
	bRaising = true;
	GetWorld()->GetTimerManager().SetTimer(
		MortarRaiseTimer, this, &UMortarWeaponComponent::OnRaised, MortarRaiseTime, false);
}

void UMortarWeaponComponent::DrawDecal()
{
	if (DecalCursor < MortarShotCount)
	{
		int32 Side = DecalCursor % 2;
		if (bMuzzleAlive[Side])
		{
			MortarShots[DecalCursor].Decal = UGameplayStatics::SpawnDecalAtLocation(
				this, DecalMaterial,
				FVector(200, 300, 300),
				CachedGoalLocs[DecalCursor],
				FRotator(-90, 0, 0),
				9999.f);
		}
		DecalCursor++;
		GetWorld()->GetTimerManager().SetTimer(
			DecalTwinkleTimer, this, &UMortarWeaponComponent::DrawDecal, DecalTwinkleTime, false);
	}
	else
	{
		DecalCursor = 0;
	}
}

void UMortarWeaponComponent::UpdatePivots(float DeltaTime)
{
	if (!bRaising && !bLowering) return;

	if (bRotateMode)
	{
		if (!YawPivot) return;
		
			// yaw → YawPivot (월드). 내릴 땐 yaw 유지(목표=현재)라 올릴 때만 보간
			if (bRaising)
			{
				FRotator NewYaw = FMath::RInterpTo(
					YawPivot->GetComponentRotation(),
					FRotator(0, MortarGoalRot.Yaw, 0),
					DeltaTime, MortarRaiseInterpSpeed);
				YawPivot->SetWorldRotation(FRotator(0, NewYaw.Yaw, 0));
			}

			// pitch → MortarPivot_L (상대)
			const float GoalPitch = bRaising ? MortarGoalRot.Pitch : 0;
			FRotator NewPitch = FMath::RInterpTo(
				MortarPivot_L->GetRelativeRotation(),
				FRotator(GoalPitch, 0, 0),
				DeltaTime, MortarRaiseInterpSpeed);
			MortarPivot_L->SetRelativeRotation(FRotator(NewPitch.Pitch, 0, 0));
		
		
	}
	else
	{
		// 위치 모드 (보스): 기존 그대로
		const FVector GoalL = bRaising ? MortarPivotGoal_L->GetRelativeLocation() : MortarDefaultPos_L;
		const FVector GoalR = bRaising ? MortarPivotGoal_R->GetRelativeLocation() : MortarDefaultPos_R;
		MortarPivot_L->SetRelativeLocation(
			FMath::VInterpTo(MortarPivot_L->GetRelativeLocation(), GoalL, DeltaTime, MortarRaiseInterpSpeed));
		MortarPivot_R->SetRelativeLocation(
			FMath::VInterpTo(MortarPivot_R->GetRelativeLocation(), GoalR, DeltaTime, MortarRaiseInterpSpeed));
	}
}

void UMortarWeaponComponent::OnRaised()
{
	bRaising = false;
	MortarCursor = 0;
	FireMortar();
}

void UMortarWeaponComponent::FireMortar()
{
	if (MortarCursor < MortarShotCount)
	{
		int32 Side = MortarCursor % 2;
		USceneComponent* Muzzle = (Side == 0) ? MortarSpawnPoint_L : MortarSpawnPoint_R;

		if (bMuzzleAlive[Side])
		{
			const FVector Origin = Muzzle->GetComponentLocation();
			const FVector Dist = CachedGoalLocs[MortarCursor] - Origin;
			UDangerUtils::CalcLaunchPitch(Dist.Size2D(), Dist.Z, CachedSpeed, CachedGravity, MortarGoalRotator.Pitch, true);
			Muzzle->SetWorldRotation(FRotator(MortarGoalRotator.Pitch, Dist.Rotation().Yaw, 0));

			AMortarProjectile* Proj = GetWorld()->SpawnActor<AMortarProjectile>(
				MortarProjectileClass, Muzzle->GetComponentLocation(), Muzzle->GetComponentRotation());
			if (Proj)
			{
				Proj->SetOwner(GetOwner());
				Proj->OnMortarHit.AddDynamic(this, &UMortarWeaponComponent::DestroyDecal);
				MortarShots[MortarCursor].Proj = Proj;
			}
		}
		else if (MortarShots[MortarCursor].Decal) // 죽은 쪽 차례 -> 데칼만 정리
		{
			MortarShots[MortarCursor].Decal->DestroyComponent();
			MortarShots[MortarCursor].Decal = nullptr;
		}

		MortarCursor++;
		GetWorld()->GetTimerManager().SetTimer(
			MortarTwinkleTimer, this, &UMortarWeaponComponent::FireMortar, MortarTwinkleTime, false);
	}
	else
	{
		MortarCursor = 0;
		bLowering = true;
		GetWorld()->GetTimerManager().SetTimer(
			MortarReloadTimer, this, &UMortarWeaponComponent::Reload, MortarReloadTime, false);
	}
}

void UMortarWeaponComponent::Reload()
{
	bReady = true;
	bLowering = false;
}
 void UMortarWeaponComponent::DestroyDecal()
{
	for (int32 i = 0; i < MortarShots.Num(); i++)
	{
		if (!MortarShots[i].Proj) continue;
		if (!MortarShots[i].Proj->bHasHit) continue;

		if (MortarShots[i].Decal)
		{
			MortarShots[i].Decal->DestroyComponent();
			MortarShots[i].Decal = nullptr;
		}
		MortarShots[i].Proj = nullptr;
		break;
	}
}

void UMortarWeaponComponent::StopSequence()
{
	FTimerManager& TM = GetWorld()->GetTimerManager();
	TM.ClearTimer(MortarRaiseTimer);
	TM.ClearTimer(DecalTwinkleTimer);
	TM.ClearTimer(MortarTwinkleTimer);
	TM.ClearTimer(MortarReloadTimer);

		for (int32 i = 0; i < MortarShots.Num(); i++)
	{
		if (MortarShots[i].Proj == nullptr && MortarShots[i].Decal)
		{
			MortarShots[i].Decal->DestroyComponent();
			MortarShots[i].Decal = nullptr;
		}
	}

	bRaising = false;
	bLowering = false;
	MortarCursor = 0;
	DecalCursor = 0;
	bReady = true;
}

void UMortarWeaponComponent::ApplyEnrage()
{
	MortarScatterRadius *= 2.f;
	MortarTwinkleTime *= 0.5f;
	DecalTwinkleTime *= 0.5f;
	MortarReloadTime *= 0.5f;
}

void UMortarWeaponComponent::SetMuzzleAlive(int32 Idx, bool bAlive)
{
	if (Idx >= 0 && Idx < 2) bMuzzleAlive[Idx] = bAlive;
}

void UMortarWeaponComponent::DestroyAllShots()
{
	StopSequence();
	for (FMortarShot& Shot : MortarShots)
	{

		if (Shot.Decal)
		{
			Shot.Decal->DestroyComponent();
			Shot.Decal = nullptr;
		}
		if (Shot.Proj)
		{
			Shot.Proj->Destroy();
			Shot.Proj = nullptr;
		}

	}
	MortarShots.Empty();
}