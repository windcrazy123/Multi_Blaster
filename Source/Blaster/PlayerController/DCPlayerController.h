// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DCPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ADCPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void OnPossess(APawn* InPawn) override;

	
	void SetHudHealth(float CurHealth, float MaxHealth);
	void SetHudScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	class ADCHUD* DCHud;
};
