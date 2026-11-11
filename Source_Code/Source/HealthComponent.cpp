// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
		Health = MaxHealth;
	// ...
	
}


void UHealthComponent::SetMaxHealth(float NewMaxHealth)
{
	MaxHealth = NewMaxHealth;
	Health = NewMaxHealth;
}

// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHealthComponent::ApplyDamage(float Dmg)
{
	if (bDead) return;
	Health -= Dmg;
	OnHealthChanged.Broadcast(Health, MaxHealth);
	if (Health <= 0.f)
	{
		bDead = true;
		OnDeath.Broadcast();
	}
}

void UHealthComponent::GetHealth(float& currentHp, float& maxHp)
{
	maxHp = this->MaxHealth;
	currentHp = this->Health;
}

