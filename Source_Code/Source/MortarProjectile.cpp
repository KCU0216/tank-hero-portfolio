// Fill out your copyright notice in the Description page of Project Settings.

#include "MortarProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

AMortarProjectile::AMortarProjectile()
{
    ProjectileMoveComp->ProjectileGravityScale = 2.5f;
	ProjectileMoveComp->InitialSpeed = 3000.f;
    ProjectileMoveComp->MaxSpeed = 3000.f;

	Damage = 50.f;
}

void AMortarProjectile::OnHit(UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (bHasHit) return;
	if (!OtherActor ) return;

	bHasHit = true;
	AActor* MyOwner = GetOwner();
	AController* MyInstigator = MyOwner ? MyOwner->GetInstigatorController() : nullptr;
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(GetOwner());
	UGameplayStatics::ApplyRadialDamage(
		this,  // this
		Damage,              // float
		Hit.Location,              // FVector 폭발 중심
		DamageRadius,        // float
		UDamageType::StaticClass(),     // UDamageType::StaticClass()
		IgnoreActors,        // TArray<AActor*> 빈 배열 가능
		this,        // this
		nullptr, // nullptr 가능
		true        // false면 거리에 따라 감쇠, true면 범위 내 동일 데미지
	);

	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, HitParticles, GetActorLocation(), GetActorRotation());
	}

	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}

	OnMortarHit.Broadcast();

	ProjectileMesh->SetVisibility(false);
	TailMesh->SetVisibility(false);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TailMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMoveComp->StopMovementImmediately();
	TrailParticles->Deactivate();
	GetWorldTimerManager().SetTimer(DestroyTimerHandle,
		this,
		&AMortarProjectile::DestroyProjectile,
		1.f,
		false);
}

void AMortarProjectile::DestroyProjectile()
{
	this->Destroy();
}