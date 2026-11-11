#include "Minion.h"
#include "DangerUtils.h"
#include "DangerIndicatorComponent.h"
#include "Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "HealthComponent.h"

AMinion::AMinion()
{
    TurretPivot = CreateDefaultSubobject<USceneComponent>(TEXT("TurretPivot"));
    TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh"));
    BarrelPivot = CreateDefaultSubobject<USceneComponent>(TEXT("BarrelPivot"));
    BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh"));
    SpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnPoint"));
    DangerIndicator = CreateDefaultSubobject<UDangerIndicatorComponent>(TEXT("DangerIndicator"));

    TurretPivot->SetupAttachment(BaseMesh);
    TurretMesh->SetupAttachment(TurretPivot);
    BarrelPivot->SetupAttachment(TurretPivot);
    BarrelMesh->SetupAttachment(BarrelPivot);
    SpawnPoint->SetupAttachment(BarrelPivot);
    DangerIndicator->SetupAttachment(BarrelPivot);
}

void AMinion::BeginPlay()
{
    Super::BeginPlay();
    OnTakeAnyDamage.AddDynamic(this, &ATower::HandleDamage);
    HealthComp->OnDeath.AddDynamic(this, &ATower::OnDeath);
    if (!ProjectileClass) return;

    AProjectile* CDO = ProjectileClass->GetDefaultObject<AProjectile>();
    if (CDO && CDO->GetMoveComp() && GetWorld())
    {
        CachedSpeed = CDO->GetMoveComp()->InitialSpeed;
        CachedGravity = GetWorld()->GetGravityZ() * CDO->GetMoveComp()->ProjectileGravityScale;
    }
}

void AMinion::UpdateCombat(const FVector& PlayerLoc, float DeltaTime)
{
    if (!bCanAim) return;
    if (!Aim(PlayerLoc, DeltaTime)) return;
    if (IsReady()) BeginFireSequence();
}

bool AMinion::Aim(const FVector& Target, float DeltaTime)
{
    const FVector AimOrigin = SpawnPoint->GetComponentLocation();
    const FVector GoalDist = Target - AimOrigin;

    if (!UDangerUtils::CalcLaunchPitch(GoalDist.Size2D(), GoalDist.Z, CachedSpeed, CachedGravity, GoalRotator.Pitch))
        return false;

    GoalRotator.Yaw = GoalDist.Rotation().Yaw;
    GoalRotator.Roll = 0.f;

    const FRotator NewTurretRot = FMath::RInterpTo(
        TurretPivot->GetComponentRotation(), FRotator(0, GoalRotator.Yaw, 0), DeltaTime, RotationSpeed);
    TurretPivot->SetWorldRotation(FRotator(0, NewTurretRot.Yaw, 0));

    const FRotator NewBarrelRot = FMath::RInterpTo(
        BarrelPivot->GetComponentRotation(), FRotator(GoalRotator.Pitch, 0, 0), DeltaTime, RotationSpeed);
    BarrelPivot->SetWorldRotation(FRotator(NewBarrelRot.Pitch, NewTurretRot.Yaw, 0));

    return true;
}

bool AMinion::IsReady() const
{
    const FVector Forward = SpawnPoint->GetForwardVector();
    const FVector Goal = GoalRotator.Vector();
    const float Dot = FVector::DotProduct(Forward, Goal);
    return (Dot >= AimThreshold);
}

void AMinion::BeginFireSequence()
{
    bCanAim = false;

    // 조준 스냅 (떨림 방지)
    TurretPivot->SetWorldRotation(FRotator(0, GoalRotator.Yaw, 0));
    BarrelPivot->SetWorldRotation(FRotator(GoalRotator.Pitch, GoalRotator.Yaw, 0));

    // 위험 표시
    if (DangerIndicator)
    {
        FVector SpawnLoc = SpawnPoint->GetComponentLocation();
        FVector Velocity = SpawnPoint->GetForwardVector() * CachedSpeed;
        TArray<FVector> Points = UDangerUtils::PredictTrajectory(this, SpawnLoc, Velocity, CachedGravity);
        DangerIndicator->ShowTrajectory(Points, AimDelay);
    }

    GetWorldTimerManager().SetTimer(AimTimer, this, &AMinion::OnFired, AimDelay, false);
}

void AMinion::OnFired()
{
    Fire();
    GetWorldTimerManager().SetTimer(ReloadTimer, this, &AMinion::FinishReload, ReloadTime, false);
}

void AMinion::FinishReload()
{
    bCanAim = true;
}

void AMinion::Fire()
{
    AProjectile* Proj = SpawnBullet(SpawnPoint->GetComponentTransform());
    if (Proj) Proj->SetOwner(this);
}