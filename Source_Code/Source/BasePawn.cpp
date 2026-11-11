// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePawn.h"

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Projectile.h"
#include "MortarProjectile.h"

// Sets default values
ABasePawn::ABasePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Collider"));
	RootComponent = CapsuleComp;

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(CapsuleComp);

	ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Spawn Point"));
	ProjectileSpawnPoint->SetupAttachment(BaseMesh);
}

// Called when the game starts or when spawned
void ABasePawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABasePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABasePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

AProjectile* ABasePawn::SpawnBullet(const FTransform& Transform)
{
	AProjectile* Projectile =GetWorld()->SpawnActor<AProjectile>(ProjectileClass, Transform.GetLocation(), Transform.Rotator());
	if (Projectile)
		Projectile->SetOwner(this);
	return Projectile;
}


AMortarProjectile* ABasePawn::SpawnMortarBullet(const FTransform& Transform)
{
	AMortarProjectile* Projectile = GetWorld()->SpawnActor<AMortarProjectile>(MortarProjectileClass, Transform.GetLocation(), Transform.Rotator());
	if (Projectile)
		Projectile->SetOwner(this);
	return Projectile;
}
