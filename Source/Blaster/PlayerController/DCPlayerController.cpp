// Fill out your copyright notice in the Description page of Project Settings.


#include "DCPlayerController.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Blaster/HUD/DCHUD.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"


void ADCPlayerController::BeginPlay()
{
	Super::BeginPlay();

	DCHud = Cast<ADCHUD>(GetHUD());
}

void ADCPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* PlayerCharacter = Cast<ABlasterCharacter>(InPawn);
	if (PlayerCharacter)
	{
		SetHudHealth(PlayerCharacter->GetCurHealth(), PlayerCharacter->GetMaxHealth());
	}
}

void ADCPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetHUDTime();

	PassedTime += DeltaSeconds;
	if (PassedTime>DeltaTimeSyncFrequency && IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		PassedTime = 0.f;
	}
}

void ADCPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ADCPlayerController::SetHudHealth(float CurHealth, float MaxHealth)
{
	if(DCHud == nullptr) DCHud = Cast<ADCHUD>(GetHUD());

	if(DCHud && DCHud->CharacterOverlay && DCHud->CharacterOverlay->HealthBar && DCHud->CharacterOverlay->HealthText)
	{
		DCHud->CharacterOverlay->HealthBar->SetPercent(CurHealth/MaxHealth);

		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(CurHealth), FMath::CeilToInt(MaxHealth));
		DCHud->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void ADCPlayerController::SetHudScore(float Score)
{
	if(DCHud == nullptr) DCHud = Cast<ADCHUD>(GetHUD());

	if(DCHud && DCHud->CharacterOverlay && DCHud->CharacterOverlay->ScoreAmount)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		DCHud->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}

void ADCPlayerController::SetHUDDefeats(int32 Defeats)
{
	if(DCHud == nullptr) DCHud = Cast<ADCHUD>(GetHUD());

	if(DCHud && DCHud->CharacterOverlay && DCHud->CharacterOverlay->DefeatsAmount)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		DCHud->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
}

void ADCPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	if(DCHud == nullptr) DCHud = Cast<ADCHUD>(GetHUD());

	if(DCHud && DCHud->CharacterOverlay && DCHud->CharacterOverlay->WeaponAmmoAmount)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		DCHud->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ADCPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	if(DCHud == nullptr) DCHud = Cast<ADCHUD>(GetHUD());

	if(DCHud && DCHud->CharacterOverlay && DCHud->CharacterOverlay->CarriedAmmoAmount)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		DCHud->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

//tick call
void ADCPlayerController::SetHUDTime()
{
	uint32 Second = FMath::CeilToInt(LevelTime - GetServerTime());
	if (LelvelTimeInt != Second)
	{
		SetHUDLevelCountdownText(LevelTime - GetServerTime());
		LelvelTimeInt = Second;
	}
}
//one second call
void ADCPlayerController::SetHUDLevelCountdownText(float CountdownTime)
{
	if(DCHud == nullptr) DCHud = Cast<ADCHUD>(GetHUD());

	if(DCHud && DCHud->CharacterOverlay && DCHud->CharacterOverlay->LevelCountdownText)
	{
		int32 SumTime = FMath::FloorToInt(CountdownTime);
		int32 Minutes = SumTime / 60;
		int32 Seconds = SumTime % 60;
		FString LevelCountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		DCHud->CharacterOverlay->LevelCountdownText->SetText(FText::FromString(LevelCountdownText));
	}
}
void ADCPlayerController::ServerRequestServerTime_Implementation(float ClientTime)
{
	float ServerTime = GetWorld()->GetTimeSeconds();
	ClientReceiveServerTime(ClientTime, ServerTime);
}
void ADCPlayerController::ClientReceiveServerTime_Implementation(float ClientTime, float ServerTime)
{
	float OneRPCTime = GetWorld()->GetTimeSeconds() - ClientTime;
	float CurServerTime = ServerTime + OneRPCTime*0.5f;
	DeltaTimeOfClientServer = CurServerTime - GetWorld()->GetTimeSeconds();
}
float ADCPlayerController::GetServerTime()
{
	if(HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + DeltaTimeOfClientServer;
}