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
	void SetHudHealth(float CurHealth, float MaxHealth);

protected:
	virtual void BeginPlay() override;

private:
	class ADCHUD* DCHud;
};
