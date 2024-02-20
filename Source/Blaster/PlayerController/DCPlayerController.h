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

	
	void SetHudHealth(float CurHealth, float MaxHealth);
	void SetHudScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDLevelCountdownText(float CountdownTime);

	virtual float GetServerTime();

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
private:
	UPROPERTY()
	class ADCHUD* DCHud;

	float LevelTime = 80.f;
	uint32 LelvelTimeInt = 0;
};
