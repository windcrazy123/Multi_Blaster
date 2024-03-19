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
	virtual void Tick(float DeltaSeconds) override;
	//Update DeltaTimeOfClientServer as soon as possible
	virtual void ReceivedPlayer() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
	void SetHudHealth(float CurHealth, float MaxHealth);
	void SetHudShield(float CurShield, float MaxShield);
	void SetHudScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDLevelCountdownText(float CountdownTime);
	void SetHUDWarmUpTimeCountdown(float CountdownTime);

	virtual float GetServerTime();

	/*
	 * MatchState
	 */

	void OnMatchStateSet(FName MatchState);
	
protected:
	virtual void BeginPlay() override;

	void SetHUDTime();

	/*
	 * Sync time between client and server
	 */
	
	//Server_RequestServerTime客户端请求服务器当前时间，并传入客户端当前时间以用来之后计算RPC往返时间差
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float ClientTime);
	//Client_ReceiveServerTime服务器返回当前服务器时间
	UFUNCTION(Client, Reliable)
	void ClientReceiveServerTime(float ClientTime, float ServerTime);

	//Client进入游戏后Server已经运行的时间
	float DeltaTimeOfClientServer = 0.f;

	UPROPERTY(EditAnywhere, Category = DeltaTime)
	float DeltaTimeSyncFrequency = 5.f;
	float PassedTime = 0.f;

	/*
	 * MatchState poll init
	 */
	//void PollInit();

	//warm up
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinGoingGame(float TimeOfWarmUp, float TimeOfLevel, float TimeOfLevelStarting, FName StateOfMatch, float TimeOfCooldown);
	
private:
	UPROPERTY()
	class ADCHUD* DCHud;

	//init with game mode
	float LevelTime = 0.f;
	uint32 LelvelTimeInt = 0;
	float LevelStartingTime = 0.f;

	//warm up, init with game mode
	float WarmUpTime = 0.f;

	//cool down
	float CooldownTime = 0.f;

	/*
	 * MatchState
	 */

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;
	UFUNCTION()
	void OnRep_MatchState();

	void HandleMatchHasStarted();
	void HandleMatchCooldown();

	// poll init
	/*UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	bool bInitCharacterOverlay = false;
	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;*/

	//如果是主机直接取gamemode中的CountdownTime减少offset，但是已经有GetServerTime可以考虑注销这几行
	UPROPERTY()
	class ADCGameMode* DCGameMode;

	/*
	 * HighPingWarning
	 */
	void StartHighPingWarning();
	void StopHighPingWarning();

	float Ping = 0.f;
	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 5.f;
	UPROPERTY(EditAnywhere)
	float WarningHighPingThreshold = 100.f;

	void CheckPing(float DeltaSeconds);
};
