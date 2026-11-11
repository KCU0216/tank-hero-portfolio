// Fill out your copyright notice in the Description page of Project Settings.


#include "DangerIndicatorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SplineMeshComponent.h"

// Sets default values for this component's properties
UDangerIndicatorComponent::UDangerIndicatorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UDangerIndicatorComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UDangerIndicatorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UDangerIndicatorComponent::ShowTrajectory(const TArray<FVector>& Points, float duration)
{
	for (int i = 0; i < Points.Num()-1; i++)
	{
		
			USplineMeshComponent* Spline = NewObject<USplineMeshComponent>(GetOwner());
			Spline->SetMobility(EComponentMobility::Movable);   // 런타임 이동 가능
			Spline->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
			Spline->SetStaticMesh(TrajectoryMesh);              // 에디터에서 지정한 메시
			Spline->SetForwardAxis(ESplineMeshAxis::Z);		// 메시 길쭉한 축 (메시에 따라 X/Y/Z)
			Spline->SetStartScale(FVector2D(0.2f, 0.2f));
			Spline->SetEndScale(FVector2D(0.2f, 0.2f));
			Spline->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Spline->RegisterComponent();                        // ← 필수! 안 부르면 안 보임
			if (TrajectoryMat)
				if (TrajectoryMat) Spline->SetMaterial(0, TrajectoryMat);
		

		FVector p0 = Points[i];
		FVector p1 = Points[i + 1];
		FVector tangent = p1 - p0;

		Spline->SetStartAndEnd(p0, tangent, p1, tangent);


		SpawnedMeshes.Add(Spline);   // Clear()용 저장
	}
	GetWorld()->GetTimerManager().SetTimer(
		ClearTimer, this, &UDangerIndicatorComponent::Clear, duration, false);
}

void UDangerIndicatorComponent::Clear()
{
	for (USplineMeshComponent* Mesh : SpawnedMeshes)
	{
		if (Mesh) Mesh->DestroyComponent();
	}
	SpawnedMeshes.Empty();

	GetWorld()->GetTimerManager().ClearTimer(ClearTimer);
}
