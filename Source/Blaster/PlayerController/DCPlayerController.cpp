// Fill out your copyright notice in the Description page of Project Settings.


#include "DCPlayerController.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameMode/DCGameMode.h"
#include "Blaster/GameState/DCGameState.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Blaster/HUD/DCHUD.h"
#include "Blaster/HUD/WarmUpWidget.h"
#include "Blaster/PlayerState/DCPlayerState.h"
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

	CheckPing(DeltaSeconds);

	/*
	 * MatchState
	 */
	//PollInit();
}
void ADCPlayerController::CheckPing(float DeltaSeconds)
{
	if(HasAuthority()) return;
	
	Ping += DeltaSeconds;
	if (Ping>CheckPingFrequency)
	{
		if(PlayerState == nullptr) PlayerState = GetPlayerState<APlayerState>();
		if (PlayerState)
		{
			//literal value of Ping  1/4Ping
			if (PlayerState->GetPing() * 4 > WarningHighPingThreshold)
			{
				StartHighPingWarning();
			}
			else
			{
				StopHighPingWarning();
			}
			Ping = 0.f;
		}
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

void ADCPlayerController::SetHudShield(float CurShield, float MaxShield)
{
	if(DCHud == nullptr) DCHud = Cast<ADCHUD>(GetHUD());

	if(DCHud && DCHud->CharacterOverlay && DCHud->CharacterOverlay->ShieldBar && DCHud->CharacterOverlay->ShieldText)
	{
		DCHud->CharacterOverlay->ShieldBar->SetPercent(CurShield/MaxShield);

		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(CurShield), FMath::CeilToInt(MaxShield));
		DCHud->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
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
	}else if (MatchState == MatchState::Cooldown)
	{
		LeftTime = CooldownTime+WarmUpTime+LevelTime - GetServerTime() + LevelStartingTime;
	}
	
	uint32 Second = 0;

	//如果是主机直接取gamemode中的CountdownTime减少offset，但是已经有GetServerTime可以考虑注销这几行
	if (HasAuthority())
	{
		if(DCGameMode == nullptr) DCGameMode = Cast<ADCGameMode>(UGameplayStatics::GetGameMode(this));
		if (DCGameMode)
		{
			Second = FMath::CeilToInt(DCGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}
	else
	{
		Second = FMath::CeilToInt(LeftTime);
	}
	
	if (LelvelTimeInt != Second)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
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
		if (CountdownTime < 0.f)
		{
			DCHud->CharacterOverlay->LevelCountdownText->SetText(FText());
			return;
		}
		
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
		if (CountdownTime < 0.f)
		{
			DCHud->WarmUpWidget->WarmUpTime->SetText(FText());
			return;
		}
		
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
	SingleTripTime = OneRPCTime * 0.5f;
	float CurServerTime = ServerTime + SingleTripTime;
	DeltaTimeOfClientServer = CurServerTime - GetWorld()->GetTimeSeconds();
}
float ADCPlayerController::GetServerTime()
{
	if(HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + DeltaTimeOfClientServer;
}

//server, 仿照GameMode但是处理HUD
void ADCPlayerController::OnMatchStateSet(FName StateOfMatch)
{
	MatchState = StateOfMatch;

	if (MatchState == MatchState::InProgress)
	{
		//delayed start = false 这个会被调用两次
		HandleMatchHasStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleMatchCooldown();
	}
}
void ADCPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleMatchCooldown();
	}
}
void ADCPlayerController::HandleMatchHasStarted()
{
	if(DCHud == nullptr) DCHud = Cast<ADCHUD>(GetHUD());
	if (DCHud)
	{
		//delayed start = false 这个会被调用两次,因此检查一下
		if(DCHud->CharacterOverlay == nullptr) DCHud->AddCharacterOverlay();

		//warm up
		if (DCHud->WarmUpWidget)
		{
			DCHud->WarmUpWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
void ADCPlayerController::HandleMatchCooldown()
{
	if(DCHud == nullptr) DCHud = Cast<ADCHUD>(GetHUD());
	if (DCHud)
	{
		if (DCHud->CharacterOverlay)
		{
			DCHud->CharacterOverlay->RemoveFromParent();
		}

		//cool down
		if (DCHud->WarmUpWidget && DCHud->WarmUpWidget->TitleText && DCHud->WarmUpWidget->Info)
		{
			DCHud->WarmUpWidget->SetVisibility(ESlateVisibility::Visible);
			FString TitleText("Settlement/Cooldown");
			DCHud->WarmUpWidget->TitleText->SetText(FText::FromString(TitleText));

			ADCGameState* DCGameState = Cast<ADCGameState>(UGameplayStatics::GetGameState(this));
			ADCPlayerState* DCPlayerState = GetPlayerState<ADCPlayerState>();
			if (DCGameState && DCPlayerState)
			{
				TArray<ADCPlayerState*> TopPlayerStates = DCGameState->TopScorePlayerStates;
				FString InfoText;
				if (TopPlayerStates.Num() == 0)
				{
					InfoText = FString("Narrowly Escaped");
				}else if (TopPlayerStates.Num() == 1 && TopPlayerStates[0] == DCPlayerState)
				{
					InfoText = FString::Printf(TEXT("You are the MVP \n Get %f points in this game"), DCPlayerState->GetScore());
				}else if (TopPlayerStates.Num() == 1)
				{
					InfoText = FString::Printf(TEXT("%s is the MVP\n You Get %f points in this game"),
						*TopPlayerStates[0]->GetPlayerName(), DCPlayerState->GetScore());
					//UE_LOG(LogTemp, Warning, TEXT("dddd%f"), DCGameState->TopScore);DCGameState on server
				}else if (TopPlayerStates.Num() > 1)
				{
					InfoText = FString("Player tied for the win:\n");
					for(auto TiedPlayer : TopPlayerStates)
					{
						InfoText.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
					InfoText.Append(FString::Printf(TEXT("You Get %f points in this game"), DCPlayerState->GetScore()));
				}
				DCHud->WarmUpWidget->Info->SetText(FText::FromString(InfoText));
			}
		}
	}

	//Disable Input
	ABlasterCharacter* DCCharacter = Cast<ABlasterCharacter>(GetPawn());
	if (DCCharacter)
	{
		DCCharacter->DisableInputMore(true);
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
		CooldownTime = GameMode->CooldownTime;

		ClientJoinGoingGame(WarmUpTime, LevelTime, LevelStartingTime, MatchState, CooldownTime);
	}
}
void ADCPlayerController::ClientJoinGoingGame_Implementation(float TimeOfWarmUp, float TimeOfLevel,
	float TimeOfLevelStarting, FName StateOfMatch, float TimeOfCooldown)
{
	WarmUpTime = TimeOfWarmUp;
	LevelTime = TimeOfLevel;
	LevelStartingTime = TimeOfLevelStarting;
	MatchState = StateOfMatch;
	CooldownTime = TimeOfCooldown;

	OnMatchStateSet(MatchState);
}

void ADCPlayerController::StartHighPingWarning()
{
	if(DCHud == nullptr) DCHud = Cast<ADCHUD>(GetHUD());

	if(DCHud && DCHud->CharacterOverlay && DCHud->CharacterOverlay->HighPingWarning &&
		DCHud->CharacterOverlay->HighPingWarning->GetVisibility() == ESlateVisibility::Hidden)
	{
		DCHud->CharacterOverlay->HighPingWarning->SetVisibility(ESlateVisibility::Visible);
	}
}

void ADCPlayerController::StopHighPingWarning()
{
	if(DCHud == nullptr) DCHud = Cast<ADCHUD>(GetHUD());

	if(DCHud && DCHud->CharacterOverlay && DCHud->CharacterOverlay->HighPingWarning &&
		DCHud->CharacterOverlay->HighPingWarning->GetVisibility() == ESlateVisibility::Visible)
	{
		DCHud->CharacterOverlay->HighPingWarning->SetVisibility(ESlateVisibility::Hidden);
	}
}

