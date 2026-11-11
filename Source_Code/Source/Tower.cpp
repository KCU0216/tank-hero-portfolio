#include "Tower.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Projectile.h"
#include "Item.h"
#include "CubeTankGameMode.h"

ATower::ATower()
{
    HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("Health Component"));
}

void ATower::BeginPlay()
{
    Super::BeginPlay(); 
   
}

void ATower::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!PlayerPawn)
    {
        PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
        if (!PlayerPawn) return;
    }

    const FVector PlayerLoc = PlayerPawn->GetActorLocation();
    const float Dist = FVector::Dist(PlayerLoc, GetActorLocation());

    // 감지 범위 밖이면 끝
    if (Dist > PerceptionRange) return;

    UpdateCombat(PlayerLoc, DeltaTime);

}

void ATower::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}



void ATower::OnDeath()
{
    FVector SpawnPoint = GetActorLocation();
    SpawnPoint.Z = 50.f;
    GetWorld()->SpawnActor<AItem>(ItemClass, SpawnPoint, GetActorRotation());

    if (ACubeTankGameMode* GM = Cast<ACubeTankGameMode>(UGameplayStatics::GetGameMode(this)))
        GM->ActorDied(this);

    Destroy();
}

void ATower::HandleDamage(AActor* DamagedActor, float Damage,
    const class UDamageType* DamageType,
    class AController* InstigatedBy, AActor* DamageCauser)
{
    HealthComp->ApplyDamage(Damage);
}