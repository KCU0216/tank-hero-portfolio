#include "Boss.h"
#include "DangerUtils.h"
#include "DangerIndicatorComponent.h"
#include "Projectile.h"
#include "MortarProjectile.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "HealthComponent.h"
#include "Engine/DamageEvents.h"
#include "CubeTankGameMode.h"
#include "LaserWeaponComponent.h"
#include "MortarWeaponComponent.h"
#include "NiagaraComponent.h"
#include "TankPlayerController.h"
#include "TankHUDWidget.h"
#include "Materials/MaterialInstanceDynamic.h"

// =====================================================================
//  생성자
// =====================================================================
ABoss::ABoss()
{
    // ----- 왼팔 -----
    TurretPivot_L = CreateDefaultSubobject<USceneComponent>(TEXT("TurretPivot_L"));
    BarrelPivot_L = CreateDefaultSubobject<USceneComponent>(TEXT("BarrelPivot_L"));
    CannonSpawnPoint_L1 = CreateDefaultSubobject<USceneComponent>(TEXT("CannonSpawnPoint_L1"));
    CannonSpawnPoint_L2 = CreateDefaultSubobject<USceneComponent>(TEXT("CannonSpawnPoint_L2"));
    AimRef_L = CreateDefaultSubobject<USceneComponent>(TEXT("AimRef_L"));
    TurretMesh_L = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh_L"));
    BarrelMesh_L1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh_L1"));
    BarrelMesh_L2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh_L2"));

    TurretPivot_L->SetupAttachment(BaseMesh);
    TurretMesh_L->SetupAttachment(TurretPivot_L);
    BarrelPivot_L->SetupAttachment(TurretPivot_L);
    BarrelMesh_L1->SetupAttachment(BarrelPivot_L);
    BarrelMesh_L2->SetupAttachment(BarrelPivot_L);
    CannonSpawnPoint_L1->SetupAttachment(BarrelPivot_L);
    CannonSpawnPoint_L2->SetupAttachment(BarrelPivot_L);
    AimRef_L->SetupAttachment(BarrelPivot_L);

    // ----- 오른팔 -----
    TurretPivot_R = CreateDefaultSubobject<USceneComponent>(TEXT("TurretPivot_R"));
    BarrelPivot_R = CreateDefaultSubobject<USceneComponent>(TEXT("BarrelPivot_R"));
    CannonSpawnPoint_R1 = CreateDefaultSubobject<USceneComponent>(TEXT("CannonSpawnPoint_R1"));
    CannonSpawnPoint_R2 = CreateDefaultSubobject<USceneComponent>(TEXT("CannonSpawnPoint_R2"));
    AimRef_R = CreateDefaultSubobject<USceneComponent>(TEXT("AimRef_R"));
    TurretMesh_R = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh_R"));
    BarrelMesh_R1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh_R1"));
    BarrelMesh_R2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh_R2"));

    TurretPivot_R->SetupAttachment(BaseMesh);
    TurretMesh_R->SetupAttachment(TurretPivot_R);
    BarrelPivot_R->SetupAttachment(TurretPivot_R);
    BarrelMesh_R1->SetupAttachment(BarrelPivot_R);
    BarrelMesh_R2->SetupAttachment(BarrelPivot_R);
    CannonSpawnPoint_R1->SetupAttachment(BarrelPivot_R);
    CannonSpawnPoint_R2->SetupAttachment(BarrelPivot_R);
    AimRef_R->SetupAttachment(BarrelPivot_R);

    // ----- 모탈 -----
    MortarPivot_L = CreateDefaultSubobject<USceneComponent>(TEXT("MortarPivot_L"));
    MortarPivot_L->SetupAttachment(BaseMesh);
    MortarPivot_R = CreateDefaultSubobject<USceneComponent>(TEXT("MortarPivot_R"));
    MortarPivot_R->SetupAttachment(BaseMesh);

    MortarSpawnPoint_L = CreateDefaultSubobject<USceneComponent>(TEXT("MortarSpawnPoint_L"));
    MortarSpawnPoint_L->SetupAttachment(MortarPivot_L);
    MortarSpawnPoint_R = CreateDefaultSubobject<USceneComponent>(TEXT("MortarSpawnPoint_R"));
    MortarSpawnPoint_R->SetupAttachment(MortarPivot_R);

    MortarPivotGoal_L = CreateDefaultSubobject<USceneComponent>(TEXT("MortarPivotGoal_L"));
    MortarPivotGoal_L->SetupAttachment(BaseMesh);
    MortarPivotGoal_R = CreateDefaultSubobject<USceneComponent>(TEXT("MortarPivotGoal_R"));
    MortarPivotGoal_R->SetupAttachment(BaseMesh);

    MortarMesh_L = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MortarMesh_L"));
    MortarMesh_L->SetupAttachment(MortarPivot_L);
    MortarMesh_R = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MortarMesh_R"));
    MortarMesh_R->SetupAttachment(MortarPivot_R);

    MortarComp = CreateDefaultSubobject<UMortarWeaponComponent>(TEXT("MortarComp"));
    // ----- 눈 -----
    LaserPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LaserPivot"));
    LaserPivot->SetupAttachment(RootComponent);

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

    // ----- 공통 -----
    DangerIndicator = CreateDefaultSubobject<UDangerIndicatorComponent>(TEXT("DangerIndicator"));
    DangerIndicator->SetupAttachment(RootComponent);

    CoreMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoreMesh"));
    CoreMesh->SetupAttachment(BaseMesh);

    ShieldHub = CreateDefaultSubobject<USceneComponent>(TEXT("ShieldHub"));
    ShieldHub->SetupAttachment(RootComponent);

    ShieldMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh1"));
    ShieldMesh1->SetupAttachment(ShieldHub);
    ShieldMesh1->ComponentTags.Add(TEXT("Shield"));
    ShieldMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh2"));
    ShieldMesh2->SetupAttachment(ShieldHub);
    ShieldMesh2->ComponentTags.Add(TEXT("Shield"));
    ShieldMesh3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh3"));
    ShieldMesh3->SetupAttachment(ShieldHub);
    ShieldMesh3->ComponentTags.Add(TEXT("Shield"));
}

// =====================================================================
//  BeginPlay
// =====================================================================
void ABoss::BeginPlay()
{
    Super::BeginPlay();

    // 캐논 CDO 캐싱
    if (ProjectileClass)
    {
        AProjectile* CDO = ProjectileClass->GetDefaultObject<AProjectile>();
        if (CDO && CDO->GetMoveComp() && GetWorld())
        {
            CachedSpeed = CDO->GetMoveComp()->InitialSpeed;
            CachedGravity = GetWorld()->GetGravityZ() * CDO->GetMoveComp()->ProjectileGravityScale;
        }
    }
    HealthComp->OnHealthChanged.AddDynamic(this, &ABoss::CheckCurrentPhase);
    HealthComp->OnDeath.AddDynamic(this, &ABoss::OnDeath);
    float cur, max;
    HealthComp->GetHealth(cur, max);
    CheckCurrentPhase(cur, max);

    // 보스 HP 바 연결 (위젯 준비 안 됐으면 다음 틱에 재시도)
    BindBossHealthBar();
    LaserComp->SetChargeComp(ChargeComp);
    LaserComp->SetBeamComp(BeamComp);

    // 보스 파트 배열 초기화 (인덱스 0~1 = L, 2~3 = R)
    FBossPart B;
    B.Mesh = BarrelMesh_L1; B.Muzzle = CannonSpawnPoint_L1; B.PartType = EBossPartType::Cannon; Parts.Add(B);
    B.Mesh = BarrelMesh_L2; B.Muzzle = CannonSpawnPoint_L2; B.PartType = EBossPartType::Cannon; Parts.Add(B);
    B.Mesh = BarrelMesh_R1; B.Muzzle = CannonSpawnPoint_R1; B.PartType = EBossPartType::Cannon; Parts.Add(B);
    B.Mesh = BarrelMesh_R2; B.Muzzle = CannonSpawnPoint_R2; B.PartType = EBossPartType::Cannon; Parts.Add(B);

    B.Mesh = MortarMesh_L; B.Muzzle = MortarSpawnPoint_L; B.PartType = EBossPartType::Mortar; Parts.Add(B);
    B.Mesh = MortarMesh_R; B.Muzzle = MortarSpawnPoint_R; B.PartType = EBossPartType::Mortar; Parts.Add(B);

    B.Mesh = LaserMesh; B.Health = 100.f; B.MaxHealth = 100.f; B.Muzzle = LaserSpawnPoint; B.PartType = EBossPartType::Laser; Parts.Add(B);

    // 반동 복귀용 원래 상대위치 캐싱 + 피격 발광용 DMI 생성
    for (FBossPart& P : Parts)
    {
        if (!P.Mesh) continue;
        P.BaseRelLoc = P.Mesh->GetRelativeLocation();
        P.DMI = P.Mesh->CreateDynamicMaterialInstance(0);   // 슬롯 0
    }

    LaserComp->Init(LaserPivot, LaserSpawnPoint, LaserMesh);
    MortarComp->Init(MortarPivot_L, MortarPivot_R, MortarPivotGoal_L, MortarPivotGoal_R, MortarSpawnPoint_L, MortarSpawnPoint_R);

    LaserMesh->SetVisibility(false);
    LaserMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetShieldActive(false);
}
void ABoss::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    UStaticMeshComponent* Shields[3] = { ShieldMesh1, ShieldMesh2, ShieldMesh3 };
    for (int i = 0; i < 3; i++)
    {
        if (!Shields[i]) continue;
        float Yaw = 120.f * i;
        Shields[i]->SetRelativeLocation(FRotator(0, Yaw, 0).RotateVector(FVector(ShieldRadius, 0, 0)));
        Shields[i]->SetRelativeRotation(FRotator(0, Yaw, 0));
    }
}

void ABoss::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdatePartRecoil(DeltaTime);
}



// =====================================================================
//  UpdateCombat — 매 틱 진입점
// =====================================================================
void ABoss::UpdateCombat(const FVector& PlayerLoc, float DeltaTime)
{
    if (bStun|| bTransitioning) return;

    if (bCanCannonFire)
        BeginCannonSequence();

    if (bCanCannonAim)
        CannonAim(PlayerLoc, DeltaTime);
    if (bCannonFiring)
        CannonSpread(PlayerLoc, DeltaTime);
    if (CurrentPhase >= EPhase::Phase2)
    {
        MortarComp->BeginFire(PlayerLoc);
        MortarComp->UpdatePivots(DeltaTime);
    }


    if (CurrentPhase >= EPhase::Phase3)
    {
        LaserComp->BeginFire();
        LaserComp->Aim(PlayerLoc, DeltaTime);
        LaserComp->Trace(PlayerLoc, DeltaTime);

    }
    
    if (bGuard)
        RotateShield(DeltaTime);

}

void ABoss::CheckCurrentPhase(float Health, float MaxHealth)
{

    float Temp = Health / MaxHealth * 100.f;
    EPhase NewPhase;
    if (Temp > 80.f)
        NewPhase = EPhase::Phase1;
    else if (Temp > 50.f)
        NewPhase = EPhase::Phase2;
    else if (Temp > 20.f)
        NewPhase = EPhase::Phase3;
    else
        NewPhase = EPhase::Phase4;

    if (bStun && NewPhase != EPhase::Phase4) return;

    if (CurrentPhase != NewPhase)
    {
        CurrentPhase = NewPhase;
        if (CurrentPhase <= Phase2)
        {
            if (CurrentPhase == Phase1) bCanCannonFire = true;
        }

        else
        {
            StopAllPatterns();
            bTransitioning = true;
            GetWorldTimerManager().SetTimer(TransitionTimer, this, &ABoss::EndTransition, TransitionTime, false);
        }
    }
}


void ABoss::BindBossHealthBar()
{
    ATankPlayerController* PC = Cast<ATankPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
    if (PC && PC->HUDWidget)
    {
        float cur, max;
        HealthComp->GetHealth(cur, max);
        HealthComp->OnHealthChanged.AddDynamic(PC->HUDWidget, &UTankHUDWidget::UpdateBossHealth);
        PC->HUDWidget->UpdateBossHealth(cur, max);   // 초기값 (꽉 찬 바)
        PC->HUDWidget->ShowBossHealthBar(true);
    }
    else
    {
        // 아직 위젯 준비 안 됨 → 다음 프레임 재시도
        GetWorldTimerManager().SetTimerForNextTick(this, &ABoss::BindBossHealthBar);
    }
}

void ABoss::ApplyPartRecoil(FBossPart& Part, const FVector& WorldDir)
{
    if (!Part.Mesh) return;
    USceneComponent* Parent = Part.Mesh->GetAttachParent();
    FVector LocalDir = Parent
        ? Parent->GetComponentTransform().InverseTransformVectorNoScale(WorldDir)
        : WorldDir;
    Part.RecoilOffset = LocalDir.GetSafeNormal() * HitPushDistance;
    Part.Mesh->SetRelativeLocation(Part.BaseRelLoc + Part.RecoilOffset);   // 즉시 킹

    Part.HitFlash = 1.f;
    if (Part.DMI) Part.DMI->SetScalarParameterValue(TEXT("HitFlash"), 1.f);
}

void ABoss::UpdatePartRecoil(float DeltaTime)
{
    for (FBossPart& Part : Parts)
    {
        if (!Part.Mesh || !Part.bAlive) continue;

        // 반동 복귀
        if (!Part.RecoilOffset.IsNearlyZero(0.05f))
        {
            Part.RecoilOffset = FMath::VInterpTo(Part.RecoilOffset, FVector::ZeroVector, DeltaTime, HitReturnSpeed);
            if (Part.RecoilOffset.IsNearlyZero(0.05f)) Part.RecoilOffset = FVector::ZeroVector;
            Part.Mesh->SetRelativeLocation(Part.BaseRelLoc + Part.RecoilOffset);
        }

        // 피격 발광 감쇠
        if (Part.HitFlash > 0.f)
        {
            Part.HitFlash = FMath::FInterpTo(Part.HitFlash, 0.f, DeltaTime, HitReturnSpeed);
            if (Part.HitFlash < 0.01f) Part.HitFlash = 0.f;
            if (Part.DMI) Part.DMI->SetScalarParameterValue(TEXT("HitFlash"), Part.HitFlash);
        }
    }
}

void ABoss::StopAllPatterns()
{

    GetWorldTimerManager().ClearAllTimersForObject(this);
    SetShieldActive(false);
    

    bCanCannonFire = false;
    bCanCannonAim = false;
    bCannonFiring = false;

    MortarComp->StopSequence();

    LaserComp->StopSequence();

    // 커서/카운터 리셋
    ShotCursor = 0;
    SpreadCount = 0;
}

void ABoss::EndTransition()
{
    StartPatterns(CurrentPhase);
}

void ABoss::StartPatterns(EPhase Phase)
{
    bStun = false;
    bTransitioning = false;

    switch (Phase)
    {
    case Phase1:
    {
        bCanCannonFire = true;
        break;
    }
    case Phase2:
    {
        break;
    }
    case Phase3:
    {
        LaserMesh->SetVisibility(true);
        LaserMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SetShieldActive(true);
        bCanCannonFire = true;

        break;
    }
    case Phase4:
    {
        if (!bEnraged)        // 중복 진입 방지
        {
            bEnraged = true;
            EnragedMod();
            GetWorldTimerManager().SetTimer(SelfDestructTimer, this, &ABoss::SelfDestruct, SelfDestructTime, false);
        }
        bCanCannonFire = true;

        break;
    }
    }

}

void ABoss::EnragedMod()
{
    AimDelay *= 0.5f;
    TwinkleTime *= 0.5f;
    ReloadTime *= 0.5f;
    SpreadRotationSpeed *= 1.5f;
    SpreadShotCount += 2;

    MortarComp->ApplyEnrage();
    LaserComp->ApplyEnrage();
}

// =====================================================================
//  캐논 — 조준
// =====================================================================
void ABoss::CannonAim(const FVector& Target, float DeltaTime)
{
    const FVector BaseDist = Target - GetActorLocation();
    const FVector GoalDist_L = Target - AimRef_L->GetComponentLocation();
    const FVector GoalDist_R = Target - AimRef_R->GetComponentLocation();

    BaseRotator.Yaw = BaseDist.Rotation().Yaw;

    switch (CurrentCannonType)
    {
    case ECannonType::Tracking:
    {
        if (!UDangerUtils::CalcLaunchPitch(GoalDist_L.Size2D(), GoalDist_L.Z, CachedSpeed, CachedGravity, GoalRotator_L.Pitch)) return;
        if (!UDangerUtils::CalcLaunchPitch(GoalDist_R.Size2D(), GoalDist_R.Z, CachedSpeed, CachedGravity, GoalRotator_R.Pitch)) return;
        GoalRotator_L.Yaw = GoalDist_L.Rotation().Yaw; GoalRotator_L.Roll = 0.f;
        GoalRotator_R.Yaw = GoalDist_R.Rotation().Yaw; GoalRotator_R.Roll = 0.f;
        break;
    }
    case ECannonType::Spread:
    {
        BaseRotator.Yaw += SpreadOffset;
        FVector MidAimRef = (AimRef_L->GetComponentLocation() + AimRef_R->GetComponentLocation()) * 0.5f;
        FVector GoalDist_Mid = Target - MidAimRef;
        double MidPitch;
        if (!UDangerUtils::CalcLaunchPitch(GoalDist_Mid.Size2D(), GoalDist_Mid.Z, CachedSpeed, CachedGravity, MidPitch)) return;
        GoalRotator_L.Pitch = GoalRotator_R.Pitch = MidPitch;
        break;
    }
    }

    const FRotator NewBaseRot = FMath::RInterpTo(
        BaseMesh->GetComponentRotation(), FRotator(0, BaseRotator.Yaw, 0), DeltaTime, RotationSpeed);
    BaseMesh->SetWorldRotation(FRotator(0, NewBaseRot.Yaw, 0));

    const FRotator NewTurretRot_L = FMath::RInterpTo(
        TurretPivot_L->GetComponentRotation(), FRotator(GoalRotator_L.Pitch, NewBaseRot.Yaw, 0), DeltaTime, RotationSpeed);
    TurretPivot_L->SetWorldRotation(FRotator(NewTurretRot_L.Pitch, NewBaseRot.Yaw, 0));

    const FRotator NewTurretRot_R = FMath::RInterpTo(
        TurretPivot_R->GetComponentRotation(), FRotator(GoalRotator_R.Pitch, NewBaseRot.Yaw, 0), DeltaTime, RotationSpeed);
    TurretPivot_R->SetWorldRotation(FRotator(NewTurretRot_R.Pitch, NewBaseRot.Yaw, 0));

    if (CurrentCannonType == ECannonType::Tracking)
    {
        const FRotator NewBarrelRot_L = FMath::RInterpTo(
            BarrelPivot_L->GetComponentRotation(), FRotator(0, GoalRotator_L.Yaw, 0), DeltaTime, RotationSpeed);
        BarrelPivot_L->SetWorldRotation(FRotator(NewTurretRot_L.Pitch, NewBarrelRot_L.Yaw, 0));

        const FRotator NewBarrelRot_R = FMath::RInterpTo(
            BarrelPivot_R->GetComponentRotation(), FRotator(0, GoalRotator_R.Yaw, 0), DeltaTime, RotationSpeed);
        BarrelPivot_R->SetWorldRotation(FRotator(NewTurretRot_R.Pitch, NewBarrelRot_R.Yaw, 0));
    }

    IsReady();
}

void ABoss::IsReady()
{

    const float YawDiff = FMath::Abs(FMath::FindDeltaAngleDegrees(BaseMesh->GetComponentRotation().Yaw, BaseRotator.Yaw));

    switch (CurrentCannonType)
    {
    case Tracking:
    {
        const float PitchDiff = FMath::Abs(FMath::FindDeltaAngleDegrees(TurretPivot_L->GetComponentRotation().Pitch, GoalRotator_L.Pitch));
        if (YawDiff < AimAngleTolerance && PitchDiff < AimAngleTolerance)
        {
            bCanCannonAim = false;
            TurretPivot_L->SetWorldRotation(FRotator(GoalRotator_L.Pitch, BaseMesh->GetComponentRotation().Yaw, 0));
            BarrelPivot_L->SetWorldRotation(FRotator(GoalRotator_L.Pitch, GoalRotator_L.Yaw, 0));
            TurretPivot_R->SetWorldRotation(FRotator(GoalRotator_R.Pitch, BaseMesh->GetComponentRotation().Yaw, 0));
            BarrelPivot_R->SetWorldRotation(FRotator(GoalRotator_R.Pitch, GoalRotator_R.Yaw, 0));

            // 위험 표시
            if (DangerIndicator)
            {
                DangerIndicator->Clear();
                for (FBossPart& B : Parts)
                {
                    if (B.PartType != EBossPartType::Cannon || !B.bAlive) continue;
                    TArray<FVector> Points = UDangerUtils::PredictTrajectory(
                        this, B.Muzzle->GetComponentLocation(), B.Muzzle->GetForwardVector() * CachedSpeed, CachedGravity);
                    DangerIndicator->ShowTrajectory(Points, AimDelay);
                }
            }
            GetWorldTimerManager().SetTimer(AimTimer, this, &ABoss::OnFired, AimDelay, false);
        }
        break;
    }
    case Spread:
    {
        if (YawDiff < AimAngleTolerance)
        {
            BaseMesh->SetWorldRotation(FRotator(0, BaseRotator.Yaw, 0));
            TurretPivot_L->SetWorldRotation(FRotator(GoalRotator_L.Pitch, BaseRotator.Yaw, 0));
            TurretPivot_R->SetWorldRotation(FRotator(GoalRotator_R.Pitch, BaseRotator.Yaw, 0));

            FVector MidAimRef = (CannonSpawnPoint_L2->GetComponentLocation() + CannonSpawnPoint_R2->GetComponentLocation()) * 0.5f;
            FVector Pivot = BaseMesh->GetComponentLocation();
            FVector Mid = Pivot + FRotator(0, 60.f * SpreadDir, 0).RotateVector(MidAimRef - Pivot);
            FVector GoalDist_Mid = PlayerPawn->GetActorLocation() - Mid;
            double MidPitch;
            if (!UDangerUtils::CalcLaunchPitch(GoalDist_Mid.Size2D(), GoalDist_Mid.Z, CachedSpeed, CachedGravity, MidPitch)) return;
            GoalRotator_L.Pitch = GoalRotator_R.Pitch = MidPitch;

            CannonSpawnPoint_L2->SetWorldRotation(FRotator(GoalRotator_L.Pitch, BaseRotator.Yaw, 0));
            CannonSpawnPoint_R2->SetWorldRotation(FRotator(GoalRotator_R.Pitch, BaseRotator.Yaw, 0));

            bCanCannonAim = false;

            if (DangerIndicator)
            {
                DangerIndicator->Clear();
                for (FBossPart& B : Parts)
                {
                    if (B.PartType != EBossPartType::Cannon || !B.bAlive || B.Muzzle == CannonSpawnPoint_L1 || B.Muzzle == CannonSpawnPoint_R1) continue;
                    float ShotYaw = 0;
                    FRotator YawOnly(0, ShotYaw, 0);
                    FVector WorldPos = Pivot + YawOnly.RotateVector(B.Muzzle->GetComponentLocation() - Pivot);
                    FVector Dir = YawOnly.RotateVector(B.Muzzle->GetForwardVector());
                    TArray<FVector> Points = UDangerUtils::PredictTrajectory(
                        this, WorldPos, Dir * CachedSpeed, CachedGravity);
                    DangerIndicator->ShowTrajectory(Points, 2.f);
                }


            }
            GetWorldTimerManager().SetTimer(AimTimer, this, &ABoss::OnSpreadFired, AimDelay, false);
        }

        break;
    }
    }

}

// =====================================================================
//  캐논 — 발사 시퀀스
// =====================================================================
void ABoss::BeginCannonSequence()
{
    bCanCannonFire = false;
    bCanCannonAim = true;


    int Rand = FMath::RandRange(0, 100);
    if (Rand > 75)
        CurrentCannonType = ECannonType::Spread;
    else 
        CurrentCannonType = ECannonType::Tracking;

    if (CurrentCannonType == ECannonType::Spread)
    {

        BarrelPivot_L->SetWorldRotation(TurretPivot_L->GetComponentRotation());
        BarrelPivot_R->SetWorldRotation(TurretPivot_R->GetComponentRotation());
        SpreadDir = FMath::RandBool() ? 1 : -1;
        SpreadOffset = FMath::Abs(SpreadOffset) * SpreadDir * -1;

    }

    else {  // Tracking
        CannonSpawnPoint_L2->SetRelativeRotation(FRotator::ZeroRotator);
        CannonSpawnPoint_R2->SetRelativeRotation(FRotator::ZeroRotator);
    }
    
}

void ABoss::OnFired()
{
    FireTick();
}

void ABoss::FireTick()
{
    // 인덱스: L=0,1 / R=2,3
    const int32 LeftIdx = ShotCursor;
    const int32 RightIdx = ShotCursor + 2;

    if (Parts[LeftIdx].bAlive)  FireOne(Parts[LeftIdx].Muzzle);
    if (Parts[RightIdx].bAlive) FireOne(Parts[RightIdx].Muzzle);

    ShotCursor++;
    if (ShotCursor < 2)
        GetWorldTimerManager().SetTimer(TwinkleTimer, this, &ABoss::FireTick, TwinkleTime, false);
    else
        GetWorldTimerManager().SetTimer(ReloadTimer, this, &ABoss::FinishReload, ReloadTime, false);
}

void ABoss::OnSpreadFired()
{
    bCannonFiring = true;

}

void ABoss::CannonSpread(const FVector& Target, float DeltaTime)
{

    BaseMesh->SetWorldRotation(FRotator(0, BaseMesh->GetComponentRotation().Yaw + SpreadDir * SpreadRotationSpeed * DeltaTime, 0));
    float CurrentYaw = BaseMesh->GetComponentRotation().Yaw;
    float YawDiff = FMath::FindDeltaAngleDegrees(BaseRotator.Yaw, CurrentYaw);
    float Interval = (FMath::Abs(SpreadOffset) * 2) / SpreadShotCount;
    if (SpreadDir > 0 ? YawDiff >= SpreadDir * Interval * ShotCursor : YawDiff <= SpreadDir * Interval * ShotCursor)
    {
        DangerIndicator->Clear();
        
        for (FBossPart& B : Parts)
        {
            if (B.PartType != EBossPartType::Cannon || !B.bAlive || B.Muzzle == CannonSpawnPoint_L1 || B.Muzzle == CannonSpawnPoint_R1) continue;
            if (B.Muzzle == CannonSpawnPoint_L2)
            FireOne(CannonSpawnPoint_L2);
            else
            FireOne(CannonSpawnPoint_R2);
        }
        ShotCursor++;

        if (ShotCursor > SpreadShotCount)
        {
            if (SpreadCount < 1)
            {
                SpreadDir = -SpreadDir;
                SpreadOffset = -SpreadOffset;
                BaseRotator.Yaw = BaseMesh->GetComponentRotation().Yaw;
                ShotCursor = 1;
                SpreadCount++;
            }
            else
            {
                SpreadCount = 0;
                bCannonFiring = false;
                FinishReload();
                return;
            }
        }

        if (DangerIndicator)
        {

            for (FBossPart& B : Parts)
            {
                if (B.PartType != EBossPartType::Cannon || !B.bAlive || B.Muzzle == CannonSpawnPoint_L1 || B.Muzzle == CannonSpawnPoint_R1) continue;
                float ShotYaw = Interval * SpreadDir;
                FRotator YawOnly(0, ShotYaw, 0);
                FVector Pivot = BaseMesh->GetComponentLocation();
                FVector WorldPos = Pivot + YawOnly.RotateVector(B.Muzzle->GetComponentLocation() - Pivot);
                FVector Dir = YawOnly.RotateVector(B.Muzzle->GetForwardVector());
                TArray<FVector> Points = UDangerUtils::PredictTrajectory(
                    this, WorldPos, Dir * CachedSpeed, CachedGravity);
                DangerIndicator->ShowTrajectory(Points, 2.f);
            }


        }
    }
}

void ABoss::FireOne(USceneComponent* Muzzle)
{
    if (!Muzzle) return;

    AProjectile* Proj = SpawnBullet(Muzzle->GetComponentTransform());
}

void ABoss::FinishReload()
{
    
    bCanCannonFire = true;
    ShotCursor = 0;
}

// =====================================================================
//  모탈 — 발사 시퀀스
// =====================================================================



// =====================================================================
//  피격 처리
// =====================================================================



float ABoss::TakeDamage(float DamageAmount,
    const FDamageEvent& DamageEvent,
    AController* EventInstigator,
    AActor* DamageCauser)
{
    float Actual = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
        const FPointDamageEvent& Point = (const FPointDamageEvent&)DamageEvent;

        for (FBossPart& Part : Parts)
        {

            if (Point.HitInfo.GetComponent() == Part.Mesh && Part.bAlive)
            {
                ApplyPartRecoil(Part, Point.ShotDirection);
                Part.Health -= Actual;
                if (Part.Health <= 0)
                {
                    if (Part.PartType == EBossPartType::Laser && CurrentPhase == Phase3)
                    {
                        EnterStun();
                    }
                    else
                    {
                        Part.bAlive = false;
                        Part.Mesh->SetVisibility(false);
                        Part.Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                        if (Part.PartType == EBossPartType::Mortar)
                        {
                            int32 idx = Part.Mesh == MortarMesh_L ? 0 : 1;
                            MortarComp->SetMuzzleAlive(idx, Part.bAlive);
                        }
                    }
                }
                break;
            }

        }

        if (Point.HitInfo.GetComponent() == CoreMesh)
            Actual *= 2;
        HealthComp->ApplyDamage(Actual);

    }

    else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
    {

        bool bCoreHit = false;
        const FRadialDamageEvent& Radial = (const FRadialDamageEvent&)DamageEvent;

        for (const FHitResult& Hit : Radial.ComponentHits)
        {
            UPrimitiveComponent* HitComp = Hit.GetComponent();
            if (HitComp == CoreMesh)
                bCoreHit = true;

            else
            {
                for (FBossPart& Part : Parts)
                {
                    if (Part.Mesh == HitComp && Part.bAlive)
                    {

                        Part.Health -= Actual;

                        if (Part.Health <= 0)
                        {
                            if (Part.PartType == EBossPartType::Laser && CurrentPhase == Phase3)
                            {
                                
                                EnterStun();   // 메시/콜리전 안 건드림
                            }
                            else
                            {
                                Part.bAlive = false;
                                Part.Mesh->SetVisibility(false);
                                Part.Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                                if (Part.PartType == EBossPartType::Mortar)
                                {
                                    int32 idx = Part.Mesh == MortarMesh_L ? 0 : 1;
                                    MortarComp->SetMuzzleAlive(idx,Part.bAlive);
                                }
                                
                            }
                        }

                        break;
                    }
                }
            }
        }
        if (bCoreHit)
            Actual *= 2;
        HealthComp->ApplyDamage(Actual);
    }

    return Actual;
}
void ABoss::SetShieldActive(bool Active)
{
    bGuard = Active;
    UStaticMeshComponent* Shield[3] = { ShieldMesh1,ShieldMesh2,ShieldMesh3 };
    for (UStaticMeshComponent* S: Shield)
    {
        if (!S) continue;
        S->SetVisibility(Active);
        S->SetCollisionEnabled(Active ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    }
}

void ABoss::RotateShield(float DeltaTime)
{
    float RotateYaw = ShieldHub->GetComponentRotation().Yaw + DeltaTime * ShieldRotationSpeed;
    ShieldHub->SetWorldRotation(FRotator(0, RotateYaw, 0));

}

void ABoss::EnterStun()
{

    bStun = true;
    StopAllPatterns();
    // 3) 스턴 타이머 → ExitStun
    GetWorldTimerManager().SetTimer(StunTimer, this, &ABoss::ExitStun, StunTime, false);
}



void ABoss::ExitStun()
{
    for (FBossPart& Part : Parts)
    {
        if (Part.Mesh == LaserMesh)
        {
            Part.Health = Part.MaxHealth;
            Part.bAlive = true;
            break;
        }
            
    }
    StartPatterns(CurrentPhase);
}

void ABoss::SelfDestruct()
{
    if (ACubeTankGameMode* GM = Cast<ACubeTankGameMode>(UGameplayStatics::GetGameMode(this)))
        GM->GameOver(false);
}


    void ABoss::OnDeath()
    {
        GetWorldTimerManager().ClearAllTimersForObject(this);

        // 보스 HP 바 숨김
        if (ATankPlayerController* PC = Cast<ATankPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
            if (PC->HUDWidget) PC->HUDWidget->ShowBossHealthBar(false);

        if (ACubeTankGameMode* GM = Cast<ACubeTankGameMode>(UGameplayStatics::GetGameMode(this)))
        {
            GM->GameOver(true);
        }
        MortarComp->DestroyAllShots();

        Destroy();
    }
