// Fill out your copyright notice in the Description page of Project Settings.


#include "DCGameMode.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/DCPlayerController.h"
#include "Blaster/PlayerState/DCPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

namespace MatchState
{
	const FName Cooldown = FName(TEXT("Cooldown"));
}

ADCGameMode::ADCGameMode()
{
	/*
	 * Match State
	 */
	
	//MatchState:WaitingToStart, we need call StartMatch manually to transition from WaitingToStart to InProgress
	bDelayedStart = true;
	
}

void ADCGameMode::BeginPlay()
{
	Super::BeginPlay();

	/*
	 * Match State
	 */
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ADCGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	/*
	 * Match State
	 */

	//之后换成Timer
	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime+LevelTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime+WarmupTime+LevelTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;

		//结束一关结算后的动作，目前为 virtual RestartGame();
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void ADCGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ADCPlayerController* DCPlayerController = Cast<ADCPlayerController>(*It);
		if (DCPlayerController)
		{
			DCPlayerController->OnMatchStateSet(MatchState);
		}
	}
}

void ADCGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ADCPlayerController* VictimController,
                                   ADCPlayerController* AttackerController)
{
	ADCPlayerState* AttackerPlayerState = AttackerController ? Cast<ADCPlayerState>(AttackerController->PlayerState) : nullptr;
	ADCPlayerState* VictimPlayerState = VictimController ? Cast<ADCPlayerState>(VictimController->PlayerState) : nullptr;
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Eliminate();
	}
}

void ADCGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 RandIndex = FMath::RandRange(0, PlayerStarts.Num() - 1);
		//蓝图中Character的ClassDefault中Actor->SpawnCollisionHandling改为调整但总是生成，但是我也在CPP构造中改了
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[RandIndex]);
	}
}

