// Fill out your copyright notice in the Description page of Project Settings.


#include "DangerUtils.h"
#include "Kismet/GameplayStatics.h"

TArray<FVector> UDangerUtils::PredictTrajectory(UObject* WorldContext, FVector start, FVector velocity,
	float gravity)
{

	FPredictProjectilePathParams Params;
	Params.StartLocation = start;
	Params.LaunchVelocity = velocity;
	Params.OverrideGravityZ = gravity;
	Params.bTraceWithCollision = true;
	Params.TraceChannel = ECollisionChannel::ECC_GameTraceChannel1;
	Params.MaxSimTime = 2.f;
	Params.ProjectileRadius = 10.f;
	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(WorldContext, Params, PathResult);

	TArray<FVector> Points;
	for (const auto& P : PathResult.PathData)
	{
		Points.Add(P.Location);
	}

	return Points;
}

bool UDangerUtils::CalcLaunchPitch(float x, float y, float v, float g, double& OutPitch, bool bHighArc)
{
	if (FMath::IsNearlyZero(x)) return false;  
	if (FMath::IsNearlyZero(g)) return false;   // ← 이 줄 추가// 바로 위/아래 방어

	float D = v * v * v * v - g * g * x * x + 2 * g * v * v * y;           // g 음수
	if (D < 0.f) return false;                          // 도달 불가

	float Discriminate = FMath::Sqrt(D);

	Discriminate = (bHighArc) ? -Discriminate : Discriminate;

	float tanTheta = (-v * v + Discriminate) / (g * x); // 낮은 포물선
	OutPitch = FMath::RadiansToDegrees(FMath::Atan(tanTheta));
	return true;
}