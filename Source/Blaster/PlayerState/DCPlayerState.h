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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	/*
	 * Score
	 */
	//update server score
	void AddToScore(float ScoreAmount);
	//rep client score
	virtual void OnRep_Score() override;
	// Defeats 类似Score
	UFUNCTION()
	virtual void OnRep_Defeats();
	void AddToDefeats(int32 DefeatsAmount);
	
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

	// Defeats 类似Score
	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

	
};
