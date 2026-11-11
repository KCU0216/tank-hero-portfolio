// Fill out your copyright notice in the Description page of Project Settings.


#include "BreakableWall.h"
#include "HealthComponent.h"

// Sets default values
ABreakableWall::ABreakableWall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	HealthComp=CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

}

// Called when the game starts or when spawned
void ABreakableWall::BeginPlay()
{
	Super::BeginPlay();
	HealthComp->OnDeath.AddDynamic(this, &ABreakableWall::OnBreak);
	OnTakeAnyDamage.AddDynamic(this, &ABreakableWall::HandleDamage);

}

// Called every frame
void ABreakableWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABreakableWall::OnBreak()
{

	Destroy();
}

void ABreakableWall::HandleDamage(AActor* DamagedActor, float Damage,
	const class UDamageType* DamageType,
	class AController* InstigatedBy, AActor* DamageCauser)
{
	HealthComp->ApplyDamage(Damage);
}