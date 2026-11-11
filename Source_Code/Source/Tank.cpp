// Fill out your copyright notice in the Description page of Project Settings.
#include "Tank.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "math.h"
#include "Projectile.h"
#include "TankPlayerController.h"
#include "HealthComponent.h"
#include "TankHUDWidget.h"
#include "CubeTankGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "LaserWeaponComponent.h"
#include "NiagaraComponent.h"



ATank::ATank()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm-> SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera-> SetupAttachment(SpringArm);

	TurretPivot = CreateDefaultSubobject<USceneComponent>(TEXT("TurretPivot"));
	TurretPivot->SetupAttachment(BaseMesh);
	
	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh"));
	TurretMesh->SetupAttachment(TurretPivot);

	BarrelPivot = CreateDefaultSubobject<USceneComponent>(TEXT("BarrelPivot"));
	BarrelPivot->SetupAttachment(TurretPivot);

	BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh"));
	BarrelMesh->SetupAttachment(BarrelPivot);  // 피벗에 부착

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(BarrelPivot);  // 카메라도 피벗에
	FirstPersonCamera->SetActive(false);

	FullFPCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FullFPCamera"));
	FullFPCamera->SetupAttachment(BarrelPivot);  // 카메라도 피벗에
	FullFPCamera->SetActive(false);

	ProjectileSpawnPoint->SetupAttachment(BarrelPivot);  // 발사지점도 피벗에

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));

	LaserComp = CreateDefaultSubobject<ULaserWeaponComponent>(TEXT("LaserComp"));

	ChargeComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ChargeComp"));
	ChargeComp->SetupAttachment(ProjectileSpawnPoint);
	ChargeComp->SetAutoActivate(false);

	BeamComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BeamComp"));
	BeamComp->SetupAttachment(ProjectileSpawnPoint);
	BeamComp->SetAutoActivate(false);
}

void ATank::BeginPlay()
{
	Super::BeginPlay();
	TankPC = Cast<ATankPlayerController>(GetController());
	HealthComp->OnDeath.AddDynamic(this, &ATank::OnDeath);
	OnTakeAnyDamage.AddDynamic(this, &ATank::HandleDamage);

	if (TankPC && TankPC->HUDWidget && HealthComp)
	{
		HealthComp->OnHealthChanged.AddDynamic(TankPC->HUDWidget, &UTankHUDWidget::UpdateHealth);
		TankPC->HUDWidget->SetLaserComp(LaserComp);

		float cur, max;
		HealthComp->GetHealth(cur, max);
		TankPC->HUDWidget->UpdateHealth(cur, max);
	}

	LaserComp->SetChargeTime(2.f);
	LaserComp->Init(nullptr, ProjectileSpawnPoint, nullptr);
	LaserComp->SetRandomType(false);   // Tracking 고정 (Sweep 안 나오게)
	LaserComp->SetAimAtTarget(true);   // 총구 forward 말고 받은 타겟 방향으로

	LaserComp->SetChargeComp(ChargeComp);
	LaserComp->SetBeamComp(BeamComp);
	LaserComp->SetAutoFire(false);

}
void ATank::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!MoveDirection.IsNearlyZero())
	{
		MoveDirection.Normalize();
		AddActorLocalOffset(DeltaTime * Speed * MoveDirection,true);
		
	}
	if (!MoveRotation.IsNearlyZero())
	{
		AddActorLocalRotation(DeltaTime * RotationSpeed * MoveRotation);
	}

	if (!TankPC) return;
	switch (CurrentViewMode)
	{
	case EViewMode::TopDown:
	{
		FHitResult HitResult;
		if (TankPC->GetHitResultUnderCursor(
			ECollisionChannel::ECC_Visibility,
			false, HitResult)) 
		{
		
		FRotator NewRotation = (HitResult.ImpactPoint - GetActorLocation()).Rotation();
		NewRotation.Roll = 0;
		TurretYaw3P = NewRotation.Yaw;
		BarrelPitch3P = NewRotation.Pitch;
		TurretPivot->SetWorldRotation(FRotator(0, TurretYaw3P, 0));
		BarrelPivot->SetRelativeRotation(FRotator(BarrelPitch3P, 0, 0));
		}
	}
		break;
	case EViewMode::BehindAim:
	case EViewMode::FullFP:
	{
		
		TurretPivot->SetWorldRotation(FRotator(0, TurretYaw, 0));
		BarrelPivot->SetRelativeRotation(FRotator(BarrelPitch, 0.f, 0.f));
		
		
	}
		break;
	default:
		break;
	} 


	FVector AimPoint = GetLaserAimPoint();
	LaserComp->Trace(AimPoint,DeltaTime);

}

void ATank::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("FB"), this, &ATank::FBMove);
	PlayerInputComponent->BindAxis(TEXT("LR"), this, &ATank::LRRotation);
	PlayerInputComponent->BindAction(TEXT("ChangeView"), IE_Pressed, this, &ATank::ChangeView);
	PlayerInputComponent->BindAxis(TEXT("MouseX"), this, &ATank::MouseMoveX);
	PlayerInputComponent->BindAxis(TEXT("MouseY"), this, &ATank::MouseMoveY);

	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &ATank::Fire);

	PlayerInputComponent->BindAction(TEXT("Laser"), IE_Pressed, this, &ATank::LaserPressed);
	PlayerInputComponent->BindAction(TEXT("Laser"), IE_Released, this, &ATank::LaserReleased);
}

void ATank::FBMove(float Value)
{
	MoveDirection.X = Value;
}

void ATank::LRRotation(float Value)
{
	MoveRotation.Yaw = Value;
}

void ATank::MouseMoveX(float Value)
{
	float DT = GetWorld()->GetDeltaSeconds();
	BarrelPitch += Value * DT * MouseSensitivity;
	BarrelPitch = FMath::Clamp(BarrelPitch, MinBarrelPitch, MaxBarrelPitch);
}

void ATank::MouseMoveY(float Value)
{
	float DT = GetWorld()->GetDeltaSeconds();
	TurretYaw += Value * DT * MouseSensitivity;
}

void ATank::ChangeView()
{

	int Next = ((int)CurrentViewMode + 1) % (int)EViewMode::MAX;
	CurrentViewMode = (EViewMode)Next;
	ApplyViewMode(CurrentViewMode);
}

void ATank::ApplyViewMode(EViewMode Mode)
{
	if (!TankPC) return;

	Camera->SetActive(false);
	FirstPersonCamera->SetActive(false);
	FullFPCamera->SetActive(false);

	switch (Mode)
	{
	case EViewMode::TopDown:
	{
		Camera->SetActive(true);
		BarrelMesh->SetVisibility(true);
		TankPC->SetThirdPersonMode();

		break;
	}
	case EViewMode::BehindAim:
	{
		FirstPersonCamera->SetActive(true);
		TurretYaw = TurretYaw3P;       // 3인칭→1인칭 진입 시 방향 이어받기
		BarrelPitch = BarrelPitch3P;
		TankPC->SetSecondPersonMode();

		break;
	}
	case EViewMode::FullFP:
	{
		FullFPCamera->SetActive(true);
		BarrelMesh->SetVisibility(false);
		// BehindAim에서 넘어오니까 TurretYaw/BarrelPitch 그대로 사용
		TankPC->SetFirstPersonMode();
		break;
	}
	default:
		break;
	}
}


void ATank::Fire()
{
	if (!bCanFire)return;
	AProjectile* proj = SpawnBullet(ProjectileSpawnPoint->GetComponentTransform());
	if(proj)
	proj->SetOwner(this);
	bCanFire = false;
	if (!GetWorldTimerManager().IsTimerActive(FireTimer))
		GetWorldTimerManager().SetTimer(FireTimer, this, &ATank::ResetFire, FireRate, false);
}

void ATank::ResetFire()
{
	bCanFire = true;
}

void ATank::LaserPressed()
{
	LaserComp->BeginFire();
}

void ATank::LaserReleased()
{
	LaserComp->Release();
}

FVector  ATank::GetLaserAimPoint()
{	
	const float Reach = 100000.f; // 충분히 먼 거리 (조준선 끝점용)

	if (CurrentViewMode == EViewMode::TopDown)
	{
		// 탑뷰: 커서가 가리키는 지점
		FHitResult Hit;
		if (TankPC && TankPC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
			return Hit.ImpactPoint;
		// 커서가 허공 → 배럴 forward 끝점 fallback
		return ProjectileSpawnPoint->GetComponentLocation()
			+ ProjectileSpawnPoint->GetForwardVector() * Reach;
	}
	else
	{
		// 1인칭 2개: 카메라에서 라인트레이스
		FVector CamLoc = FirstPersonCamera->GetComponentLocation();
		FVector CamDir = FirstPersonCamera->GetForwardVector();
		FVector End = CamLoc + CamDir * Reach;

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		if (GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, End, ECC_Visibility, Params))
			return Hit.ImpactPoint;   // 맞은 지점
		return End;                   // 안 맞으면 끝점 (카메라 방향 그대로)
	}
	

}

void ATank::HandleDamage(AActor* DamagedActor, float Damage,
	const class UDamageType* DamageType,
	class AController* InstigatedBy, AActor* DamageCauser)
{
	HealthComp->ApplyDamage(Damage);
}

void ATank::OnDeath()
{
	ACubeTankGameMode* GM = Cast<ACubeTankGameMode>(UGameplayStatics::GetGameMode(this));
	if (GM) GM->ActorDied(this);
	Destroy();
}
