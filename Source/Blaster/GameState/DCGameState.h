// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "DCGameState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ADCGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&) const override;


	/*
	 * 分数排名
	 */

	//保存最高分
	void UpdateTopScore(class ADCPlayerState* ScoreingPlayerState);
	
	UPROPERTY(Replicated)
	TArray<ADCPlayerState*> TopScorePlayerStates;

	float TopScore = 0.f;
};
