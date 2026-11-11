// Fill out your copyright notice in the Description page of Project Settings.


#include "TankPlayerController.h"
#include "TankHUDWidget.h"

void ATankPlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetThirdPersonMode();  // 게임 시작은 항상 3인칭

	if (HUDWidgetClass) 
	{
		HUDWidget = CreateWidget<UTankHUDWidget>(this, HUDWidgetClass);
		if (HUDWidget)
			HUDWidget->AddToViewport();
		HUDWidget->ShowFirstCrosshair(false);
		HUDWidget->ShowThirdCrosshair(false);

	}
}

void ATankPlayerController::SetFirstPersonMode()
{
	bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	if (HUDWidget) HUDWidget->ShowFirstCrosshair(true);
	if (HUDWidget) HUDWidget->ShowThirdCrosshair(false);
}

void ATankPlayerController::SetSecondPersonMode()
{
	bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	if (HUDWidget) HUDWidget->ShowThirdCrosshair(true);
	if (HUDWidget) HUDWidget->ShowFirstCrosshair(false);
}

void ATankPlayerController::SetThirdPersonMode()
{
	bShowMouseCursor = true;
	CurrentMouseCursor = EMouseCursor::Crosshairs;
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	if (HUDWidget) HUDWidget->ShowFirstCrosshair(false);
	if (HUDWidget) HUDWidget->ShowThirdCrosshair(false);
}

void ATankPlayerController::EnableTankInput(bool Active)
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn) return;

	if (Active) MyPawn->EnableInput(this);
	else MyPawn->DisableInput(this);
}
