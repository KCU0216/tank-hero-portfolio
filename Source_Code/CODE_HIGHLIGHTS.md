# Tank Hero — 핵심 코드 하이라이트

> 이 문서는 전체 소스 중 **설계 의도가 담긴 핵심 부분만** 발췌한 것입니다.
> 각 항목은 "무엇을 / 왜 그렇게 했는지"를 코드와 함께 정리했습니다.
> 제공된 CPP 소스는 [Source 폴더](Source/) 참고. 헤더와 프로젝트 설정은 포함하지 않았습니다.

---

## 1. 포물선 방정식 직접 구현 — 두 해 분기 + 판별식 가드

**파일:** `DangerUtils.cpp · CalcLaunchPitch()`

엔진 내장 `PredictProjectilePath`(궤적 시뮬레이션)는 **예측 시각화**에만 쓰고,
실제 발사각은 **포물선 운동방정식을 직접 풀어** 산출했습니다.

```cpp
bool UDangerUtils::CalcLaunchPitch(float x, float y, float v, float g,
                                   double& OutPitch, bool bHighArc)
{
    if (FMath::IsNearlyZero(x)) return false;   // 바로 위/아래 방어
    if (FMath::IsNearlyZero(g)) return false;

    // 판별식 D = v⁴ - g²x² + 2gv²y   (g는 음수)
    float D = v*v*v*v - g*g*x*x + 2*g*v*v*y;
    if (D < 0.f) return false;                  // ★ 도달 불가 → 실패 반환

    float Discriminate = FMath::Sqrt(D);
    // ★ 두 해: 고각(박격포) / 저각(캐논)을 bHighArc로 분기
    Discriminate = bHighArc ? -Discriminate : Discriminate;

    float tanTheta = (-v*v + Discriminate) / (g * x);
    OutPitch = FMath::RadiansToDegrees(FMath::Atan(tanTheta));
    return true;
}
```

**설계 포인트**
- 2차 방정식의 **두 해**가 곧 무기 정체성: 고각 해 = 박격포(곡사), 저각 해 = 캐논(직사).
- **판별식 `D < 0` → `return false`**: 중력·속도·거리 조합으로 물리적 도달이 불가능하면 발사를 막는 안전망. 반환값을 확인하는 호출부에서 발사를 건너뛸 수 있도록 구성.
- 캐논 조준 경로는 실패 반환값을 검사합니다. **현재 박격포 호출부는 반환값을 검사하지 않아 같은 발사 차단이 보장되지 않습니다.**

---

## 2. 무기 컴포넌트 — 단방향 의존 + 파라미터 주입

**파일:** `MortarWeaponComponent.cpp`

박격포 시퀀스를 박격포 미니포탑·보스가 공유하므로
`UActorComponent`로 추상화. 소유자가 피벗·발사점·파츠 생존 여부를 주입합니다. 컴포넌트도 `GetOwner()`와 플레이어 조회를 사용하므로, 액터 참조 자체가 없다는 의미는 아닙니다.

```cpp
UMortarWeaponComponent::UMortarWeaponComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // 소유자(보스)가 매 틱 지휘
}

void UMortarWeaponComponent::BeginPlay()
{
    // CDO에서 발사 물리 파라미터 캐싱 — 단일 소스
    if (MortarProjectileClass)
    {
        AMortarProjectile* CDO = MortarProjectileClass->GetDefaultObject<AMortarProjectile>();
        if (CDO && CDO->GetMoveComp())
        {
            CachedSpeed   = CDO->GetMoveComp()->MaxSpeed;
            CachedGravity = GetWorld()->GetGravityZ()
                          * CDO->GetMoveComp()->ProjectileGravityScale;
        }
    }
}

// 소유자가 피벗/발사점 등 씬 컴포넌트를 '주입'
void UMortarWeaponComponent::Init(USceneComponent* InPivotL, /* ... */) { /* ... */ }
```

**설계 포인트**
- 발사 물리(속도·중력)는 ProjectileMovement에서 노출 → BeginPlay에서 **CDO 캐싱**. calc가 이 캐싱값으로 도달 가능성을 판정.
- 컴포넌트가 `GetOwner()`로 보스 상태를 역참조하지 않음 = **단방향 의존**.

---

## 3. 파츠 생존 게이팅 — 인덱스 + bool 주입

**파일:** `MortarWeaponComponent.cpp` · `Boss::TakeDamage`

보스가 파츠 파괴를 감지하면, **머즐 인덱스 + 생존 여부(bool)만** 컴포넌트에 주입.
컴포넌트는 메시를 모르고 bool 배열만 봅니다.

```cpp
// 컴포넌트: bool 배열만 보유 (메시 포인터 X)
void UMortarWeaponComponent::SetMuzzleAlive(int32 Idx, bool bAlive)
{
    if (Idx >= 0 && Idx < 2) bMuzzleAlive[Idx] = bAlive;
}

// 발사 시점: 죽은 머즐이면 발사 스킵 + 그 머즐 데칼만 선별 정리
void UMortarWeaponComponent::FireMortar()
{
    int32 Side = MortarCursor % 2;
    if (bMuzzleAlive[Side]) { /* 발사 */ }
    else if (MortarShots[MortarCursor].Decal)  // 죽은 쪽 → 데칼만 제거
    {
        MortarShots[MortarCursor].Decal->DestroyComponent();
        MortarShots[MortarCursor].Decal = nullptr;
    }
}
```

**설계 포인트**
- 머즐을 "추상 슬롯(인덱스)"으로만 다뤄 메시 의존을 끊음 — 단방향 원칙의 일관된 적용.
- 데칼-총알 1:1 매칭이라, 발사한 머즐이 죽어도 **그 머즐의 데칼만** 골라 정리하고 포인터를 nullptr로(댕글링 방지).

---

## 4. 보스 파트별 피격 처리 — 파괴형 vs 기절형 + 코어 2배딜

**파일:** `Boss::TakeDamage` (PointDamage 분기)

```cpp
if (Point.HitInfo.GetComponent() == Part.Mesh && Part.bAlive)
{
    ApplyPartRecoil(Part, Point.ShotDirection);
    Part.Health -= Actual;
    if (Part.Health <= 0)
    {
        // ★ 레이저 파츠(3페이즈) = 기절형: 메시/콜리전 유지, 복귀 시 부활
        if (Part.PartType == EBossPartType::Laser && CurrentPhase == Phase3)
        {
            EnterStun();
        }
        else  // ★ 캐논/박격포 = 파괴형: 영구 제거
        {
            Part.bAlive = false;
            Part.Mesh->SetVisibility(false);
            Part.Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            if (Part.PartType == EBossPartType::Mortar)
            {
                int32 idx = Part.Mesh == MortarMesh_L ? 0 : 1;
                MortarComp->SetMuzzleAlive(idx, Part.bAlive);   // 컴포넌트에 주입
            }
        }
    }
}
// ★ 코어(약점) 직격 → 2배 데미지 (하이리스크 보상)
if (Point.HitInfo.GetComponent() == CoreMesh)
    Actual *= 2;
HealthComp->ApplyDamage(Actual);
```

**설계 포인트**
- **파괴형(캐논/박격포)**: 영구 파괴 + 콜리전/시각 off → 패턴 영구 감소(안전 루트의 기반).
- **기절형(레이저)**: 일시 무력화 후 부활(체력 초기화) → 코어를 열려면 반복 공략 필요(하이리스크 루트의 난이도 유지).
- **코어 2배딜**: 트레이드오프의 보상. 코어는 회전하는 몸통 안에 있어 조준 난이도가 있음.

---

## 5. 레이저 — 패턴 텔레그래프 & 충전 중 고정

**파일:** `LaserWeaponComponent.cpp`

```cpp
void ULaserWeaponComponent::BeginFire()
{
    // ★ 패턴을 발사 진입 시 1회만 확정 (충전 중 안 바뀜 → 예측 가능)
    CurrentLaserType = bRandomType
        ? (FMath::RandBool() ? ELaserType::LaserSweep : ELaserType::LaserTracking)
        : ELaserType::LaserTracking;

    if (CurrentLaserType == ELaserType::LaserSweep)
        SetupSweep();   // Sweep만 조준 오프셋 부여 → 텔레그래프
}

void ULaserWeaponComponent::Aim(const FVector& Target, float DeltaTime)
{
    FRotator GoalRotator = (Target - LaserPivot->GetComponentLocation()).Rotation();
    if (CurrentLaserType == ELaserType::LaserSweep)
    {
        // ★ Sweep은 플레이어 + 랜덤 오프셋으로 조준 = "빗나간 조준"으로 패턴 예고
        GoalRotator.Pitch += SweepOffsetPitch;
        GoalRotator.Yaw   += SweepOffsetYaw;
    }
    // ... RInterpTo로 조준 정렬 보간
}
```

**설계 포인트**
- 정조준 = Tracking, 빗나간 조준 = Sweep. 플레이어가 충전 중 조준 방향을 보고 패턴을 **미리 구분**(텔레그래프).
- 패턴은 진입 시 1회 확정 후 고정 — 충전 중 바뀌면 예측이 무효화되므로 "공정한 보스" 원칙에 맞춤.

---

## 6. 레이저 데미지 — 2층 중복 제거(dedup)

**파일:** `LaserWeaponComponent.cpp · Trace()`

```cpp
TSet<AActor*> DamagedActors;  // ★ 공간 dedup: 한 틱 내 한 액터당 1회
for (FHitResult& Hit : OutHits)
{
    AActor* HitActor = Hit.GetActor();
    if (Hit.GetComponent()->ComponentHasTag(TEXT("Shield"))) continue; // 방패=무적
    if (DamagedActors.Contains(HitActor)) continue;  // 관통 트레이스 중복 부위 제거

    float Dmg;
    if (CurrentLaserType == ELaserType::LaserTracking)
        Dmg = LaserDamage * DeltaTime;        // 추적형 = 지속딜 (시간 dedup 안 함 = 의도)
    else  // Sweep
    {
        if (SweepHitActors.Contains(HitActor)) continue;  // ★ 시간 dedup: 한 스윕 1회
        SweepHitActors.Add(HitActor);
        Dmg = LaserDamage;                    // 스윕형 = 1회 풀데미지
    }
    DamagedActors.Add(HitActor);
    UGameplayStatics::ApplyPointDamage(HitActor, Dmg, /* ... */);
}
```

**설계 포인트**
- **공간 dedup (`DamagedActors`)**: 관통 트레이스가 한 액터의 여러 부위를 잡아도 데미지 1회. (액터 단위 — 다른 액터 관통은 그대로 적용)
- **시간 dedup (`SweepHitActors`)**: 스윕이 여러 틱에 걸쳐 닿아도 한 패스에 1회.
- **패턴별 데미지 모델**: Tracking은 지속(시간 dedup 안 함 = 의도), Sweep은 1회. 공정성 기준으로 데미지를 분리.

---

## 7. 레이저 — 카메라/포신 2중 트레이스

**파일:** `Tank.cpp · GetLaserAimPoint()` + `LaserWeaponComponent::Trace()`

카메라 조준점과 포신 발사점을 나누어 트레이스합니다. 탑다운은 커서 피격 지점, 그 외 모드는 `FirstPersonCamera`의 트레이스 결과를 사용합니다. 현재 코드에서는 `FullFPCamera`가 활성화된 경우도 `FirstPersonCamera`를 참조합니다.

```cpp
// 1차: 카메라 중앙에서 트레이스 → 플레이어가 조준한 실제 지점 확정
FVector ATank::GetLaserAimPoint()
{
    FVector CamLoc = FirstPersonCamera->GetComponentLocation();
    FVector CamDir = FirstPersonCamera->GetForwardVector();
    FVector End    = CamLoc + CamDir * Reach;

    FHitResult Hit;
    if (GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, End, ECC_Visibility, Params))
        return Hit.ImpactPoint;   // 맞은 지점
    return End;                   // 허공이면 끝점 fallback
}

// 2차: 포신에서 그 지점으로 재트레이스 → 실제 발사/데미지
//      (LaserWeaponComponent::Trace, bAimAtTarget=true일 때 Target으로 발사)
```

**설계 포인트**
- 카메라-포신 시차(parallax)를 보정해 "조준한 곳에 맞는다"는 조준감 확보.
- 1차 트레이스 허공 시 **끝점 fallback**으로 방향 깨짐 방지.

---

## 8. 전용 트레이스 채널 — 벽 차단 / 적 관통

**파일:** `LaserWeaponComponent::Trace()` + 콜리전 채널 설정

레이저 전용 채널(`ECC_GameTraceChannel4`)로, **벽=Block(차단) / 적=Overlap(관통)** 분기.
충돌 채널 레벨에서 "관통"이라는 무기 정체성을 구현.

```cpp
GetWorld()->LineTraceMultiByChannel(OutHits, Start, End, ECC_GameTraceChannel4, Params);
// 적들은 Overlap → 줄줄이 관통 hit (각 액터는 dedup으로 1회)
// 벽은 Block → 빔이 거기서 끊김
FVector BeamEnd = End;
if (OutHits.Num() > 0 && OutHits.Last().bBlockingHit)
    BeamEnd = OutHits.Last().ImpactPoint;   // 벽에서 빔 종료
```

---

## 9. HealthComponent — broadcast만 담당 (SRP)

**파일:** `HealthComponent.cpp`

```cpp
void UHealthComponent::ApplyDamage(float Dmg)
{
    if (bDead) return;
    Health -= Dmg;
    OnHealthChanged.Broadcast(Health, MaxHealth);  // 체력은 알림만
    if (Health <= 0.f)
    {
        bDead = true;
        OnDeath.Broadcast();    // 죽음도 알림만 — 처리는 액터가
    }
}
```

**설계 포인트**
- 체력 컴포넌트는 상태 변경을 **broadcast만** 하고, bind & handle은 액터 책임(SRP).
- 보스 페이즈 전환도 이 broadcast를 신호로 받아 처리 — 매 틱 폴링이 아닌 **이벤트 기반**.

---

## 10. 강제 중단 — StopSequence (진행 중 시퀀스 정리)

**파일:** `LaserWeaponComponent.cpp` · `MortarWeaponComponent.cpp`

페이즈 전환·파괴 시 진행 중이던 타이머·상태를 한 번에 정리.

```cpp
void ULaserWeaponComponent::StopSequence()
{
    FTimerManager& TM = GetWorld()->GetTimerManager();
    TM.ClearTimer(LaserChargeTimer);
    TM.ClearTimer(LaserFireTimer);
    TM.ClearTimer(LaserReloadTimer);
    if (BeamComp)   BeamComp->Deactivate();
    if (ChargeComp) ChargeComp->Deactivate();
    bCharging = bLaserFiring = bChargedReady = false;
    SweepCount = 0;
    SweepHitActors.Empty();
    bReady = true;
}
```

**설계 포인트**
- `EndFire` 등 다중 경로 호출 함수는 진입 시 `if (!bLaserFiring) return;`로 **멱등성(idempotency)** 보장.
- 페이즈 전환에서 기믹이 바뀌면 `StopSequence → 기믹 세팅 → 재시작` 순으로 깨끗이 정리.


## 제공 코드와 설명의 범위

- 레이저 컴포넌트는 탱크·레이저 미니포탑·보스에 연결되어 있습니다. 박격포 컴포넌트는 박격포 미니포탑·보스에 연결되어 있습니다.
- `CannonWeapon.cpp`는 수록되어 있으나, 제공된 미니포탑·보스 CPP에서는 해당 클래스를 생성·사용하는 연결이 확인되지 않습니다. 캐논 전체가 이 컴포넌트를 공유한다고 설명하지 않습니다.
- 박격포의 `CalcLaunchPitch` 실패 반환, 활성 카메라별 조준 기준, 중단 후 남은 투사체의 데칼 정리는 추가 확인·개선 항목입니다.
- 위 항목은 문서 설명을 제공 코드의 범위에 맞춘 기록입니다. 원본 CPP는 수정하지 않았습니다.
