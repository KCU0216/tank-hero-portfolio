// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"


// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	RootComponent = ProjectileMesh;

	TailMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tail Mesh"));
	TailMesh->SetupAttachment(ProjectileMesh);

	ProjectileMoveComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMoveComp"));
	ProjectileMoveComp->MaxSpeed = 3000.f;
	ProjectileMoveComp->InitialSpeed = 3000.f;
	ProjectileMoveComp->ProjectileGravityScale = 1.f;
	ProjectileMoveComp->bRotationFollowsVelocity = true;


	TrailParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Smoke Trail"));
	TrailParticles->SetupAttachment(RootComponent);

	InitialLifeSpan = 30.f;
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	ProjectileMesh->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);


	if (LaunchSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LaunchSound, GetActorLocation());
	}

}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::OnHit(UPrimitiveComponent* HitComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	
	if (bHasHit) return;
	if (!OtherActor) return;
	bHasHit = true;
	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, HitParticles, GetActorLocation(), GetActorRotation());
	}

	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}

	ProjectileMesh->SetVisibility(false);
	TailMesh->SetVisibility(false);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TailMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMoveComp->StopMovementImmediately();
	TrailParticles->Deactivate();
	GetWorldTimerManager().SetTimer(DestroyTimerHandle,
		this,
		&AProjectile::DestroyProjectile,
		1.f,
		false);


	AActor* MyOwner = GetOwner();
	if (OtherActor == MyOwner) return;
	if (OtherComp && OtherComp->ComponentHasTag(TEXT("Shield"))) return;   // 방패 = 무적, 딩 스킵 (탄은 위에서 이미 소멸 처리됨)
	AController* MyInstigator = MyOwner ? MyOwner->GetInstigatorController() : nullptr;
	UGameplayStatics::ApplyPointDamage(OtherActor,
		Damage,
		-Hit.ImpactNormal,
		Hit,
		MyInstigator,
		this,
		UDamageType::StaticClass());

}

void AProjectile::DestroyProjectile()
{
	this->Destroy();
}

