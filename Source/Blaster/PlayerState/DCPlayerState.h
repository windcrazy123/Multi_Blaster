// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "DCPlayerState.generated.h"

/**
 * APlayerState有很多东西
 */
UCLASS()
class BLASTER_API ADCPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	/*
	 * Score
	 */
	//update server score
	void AddToScore(float ScoreAmount);
	//rep client score
	virtual void OnRep_Score() override;

private:
	//UPROPERTY() or TWeakObjectPtr防止由于访问GC悬空 Actor 指针而导致的崩溃
	//TWeakObjectPtr<class ABlasterCharacter> Character;
	//class ABlasterCharacter* Character;
	//class ADCPlayerController* Controller;
	//FWeakObjectPtr<class ADCPlayerController> Controller;
	//TWeakObjectPtr<class ADCPlayerController> Controller;
	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ADCPlayerController* Controller;
};
