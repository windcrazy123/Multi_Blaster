// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "DCGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ADCGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	/*
	 * 血量归零时淘汰
	 */
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ADCPlayerController* VictimController, ADCPlayerController* AttackerController);
};
