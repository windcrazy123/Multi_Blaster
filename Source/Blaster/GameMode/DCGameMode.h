// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "DCGameMode.generated.h"

namespace MatchState
{
	extern BLASTER_API const FName Cooldown;	//游戏时间到达，显示结算，开始CooldownTimer
}

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

	//关卡游戏时间
	UPROPERTY(EditDefaultsOnly)
	float LevelTime = 80.f;

	//热身时间
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	//结算时间
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;

	//如果是主机直接取gamemode中的CountdownTime减少offset，但是已经有GetServerTime可以考虑注销这几行
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
	
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
