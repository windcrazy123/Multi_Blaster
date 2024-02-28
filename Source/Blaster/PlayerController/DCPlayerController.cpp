// Fill out your copyright notice in the Description page of Project Settings.


#include "DCPlayerController.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameMode/DCGameMode.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Blaster/HUD/DCHUD.h"
#include "Blaster/HUD/WarmUpWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


void ADCPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ServerCheckMatchState();

	DCHud = Cast<ADCHUD>(GetHUD());

	if (DCHud)
	{
		DCHud->AddWarmUpWidget();
	}
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

	/*
	 * MatchState
	 */
	//PollInit();
}

void ADCPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ADCPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADCPlayerController, MatchState);
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
	/*
	 * MatchState pollinit
	 */
	// else
	// {
	// 	bInitCharacterOverlay = true;
	// 	HUDHealth = CurHealth;
	// 	HUDMaxHealth = MaxHealth;
	// }
}

void ADCPlayerController::SetHudScore(float Score)
{
	if(DCHud == nullptr) DCHud = Cast<ADCHUD>(GetHUD());

	if(DCHud && DCHud->CharacterOverlay && DCHud->CharacterOverlay->ScoreAmount)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		DCHud->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	/*
	 * MatchState pollinit
	 */
	// else
	// {
	// 	bInitCharacterOverlay = true;
	// 	HUDScore = Score;
	// }
}

void ADCPlayerController::SetHUDDefeats(int32 Defeats)
{
	if(DCHud == nullptr) DCHud = Cast<ADCHUD>(GetHUD());

	if(DCHud && DCHud->CharacterOverlay && DCHud->CharacterOverlay->DefeatsAmount)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		DCHud->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	/*
	 * MatchState pollinit
	 */
	// else
	// {
	// 	bInitCharacterOverlay = true;
	// 	HUDDefeats = Defeats;
	// }
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
	float LeftTime = 0.f;
	if (MatchState == MatchState::WaitingToStart)
	{
		LeftTime = WarmUpTime - GetServerTime() + LevelStartingTime;
	}else if(MatchState == MatchState::InProgress)
	{
		LeftTime = WarmUpTime+LevelTime - GetServerTime() + LevelStartingTime;
	}
	
	uint32 Second = FMath::CeilToInt(LeftTime);
	if (LelvelTimeInt != Second)
	{
		if (MatchState == MatchState::WaitingToStart)
		{
			SetHUDWarmUpTimeCountdown(LeftTime);
		}else if(MatchState == MatchState::InProgress)
		{
			SetHUDLevelCountdownText(LeftTime);
		}
		
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

void ADCPlayerController::SetHUDWarmUpTimeCountdown(float CountdownTime)
{
	if(DCHud == nullptr) DCHud = Cast<ADCHUD>(GetHUD());

	if(DCHud && DCHud->WarmUpWidget && DCHud->WarmUpWidget->WarmUpTime)
	{
		int32 SumTime = FMath::FloorToInt(CountdownTime);
		int32 Minutes = SumTime / 60;
		int32 Seconds = SumTime % 60;
		FString LevelCountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		DCHud->WarmUpWidget->WarmUpTime->SetText(FText::FromString(LevelCountdownText));
	}
}

//called in ReceivedPlayer and per DeltaTimeSyncFrequency one called
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

//server
void ADCPlayerController::OnMatchStateSet(FName StateOfMatch)
{
	MatchState = StateOfMatch;

	if (MatchState == MatchState::InProgress)
	{
		if(DCHud == nullptr) DCHud = Cast<ADCHUD>(GetHUD());
		if (DCHud)
		{
			DCHud->AddCharacterOverlay();

			//warm up
			if (DCHud->WarmUpWidget)
			{
				DCHud->WarmUpWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
}
void ADCPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		if(DCHud == nullptr) DCHud = Cast<ADCHUD>(GetHUD());
		if (DCHud)
		{
			DCHud->AddCharacterOverlay();
			
			//warm up
			if (DCHud->WarmUpWidget)
			{
				DCHud->WarmUpWidget->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
}

// poll init
/*void ADCPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (DCHud && DCHud->CharacterOverlay)
		{
			CharacterOverlay = DCHud->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHudHealth(HUDHealth, HUDMaxHealth);
				SetHudScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
}*/

void ADCPlayerController::ServerCheckMatchState_Implementation()
{
	ADCGameMode* GameMode = Cast<ADCGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmUpTime = GameMode->WarmupTime;
		LevelTime = GameMode->LevelTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();

		ClientJoinGoingGame(WarmUpTime, LevelTime, LevelStartingTime, MatchState);
	}
}

void ADCPlayerController::ClientJoinGoingGame_Implementation(float TimeOfWarmUp, float TimeOfLevel,
	float TimeOfLevelStarting, FName StateOfMatch)
{
	WarmUpTime = TimeOfWarmUp;
	LevelTime = TimeOfLevel;
	LevelStartingTime = TimeOfLevelStarting;
	MatchState = StateOfMatch;

	OnMatchStateSet(MatchState);
}

