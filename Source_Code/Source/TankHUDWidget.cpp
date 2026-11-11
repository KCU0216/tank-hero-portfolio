// Fill out your copyright notice in the Description page of Project Settings.


#include "TankHUDWidget.h"
#include "Components/Image.h" 
#include "Components/ProgressBar.h"
#include "LaserWeaponComponent.h"

void UTankHUDWidget::ShowFirstCrosshair(bool bShow)
{
    if (!FirstCrosshair) return;
    FirstCrosshair->SetVisibility(
        bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
    );
}

void UTankHUDWidget::ShowThirdCrosshair(bool bShow)
{
    if (!ThirdCrosshair) return;
    ThirdCrosshair->SetVisibility(
        bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
    );
}

void UTankHUDWidget::UpdateHealth(float Current, float Max)
{
    HealthBar->SetPercent(Current/Max);
}

void UTankHUDWidget::UpdateBossHealth(float Current, float Max)
{
    if (!BossHealthBar) return;
    BossHealthBar->SetPercent(Max > 0.f ? Current / Max : 0.f);
}

void UTankHUDWidget::ShowBossHealthBar(bool bShow)
{
    if (!BossHealthBar) return;
    BossHealthBar->SetVisibility(
        bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
    );
}

void UTankHUDWidget::SetLaserComp(ULaserWeaponComponent* Comp)
{
    LaserComp = Comp;
}

void UTankHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!LaserComp || !LaserBar) return;
    LaserBar->SetPercent(LaserComp->GetReloadProgress());
}