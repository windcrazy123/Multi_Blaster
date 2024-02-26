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
	ADCGameMode();
	virtual void Tick(float DeltaSeconds) override;

	/*
	 * Match State
	 */
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	float LevelStartingTime = 0.f;
	
	/*
	 * 血量归零时淘汰
	 */
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ADCPlayerController* VictimController, ADCPlayerController* AttackerController);
	//重生
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	
private:
	/*
	 * Match State
	 */
	float CountdownTime = 10.f;
};
