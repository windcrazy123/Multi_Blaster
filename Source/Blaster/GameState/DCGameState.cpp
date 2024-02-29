// Fill out your copyright notice in the Description page of Project Settings.


#include "DCGameState.h"

#include "Blaster/PlayerState/DCPlayerState.h"
#include "Net/UnrealNetwork.h"

void ADCGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADCGameState, TopScorePlayerStates);
}

void ADCGameState::UpdateTopScore(ADCPlayerState* ScoreingPlayerState)
{
	if (TopScorePlayerStates.Num() == 0)
	{
		TopScorePlayerStates.Add(ScoreingPlayerState);
		TopScore = ScoreingPlayerState->GetScore();
	}
	else if (ScoreingPlayerState->GetScore() == TopScore)
	{
		TopScorePlayerStates.AddUnique(ScoreingPlayerState);
	}
	else if (ScoreingPlayerState->GetScore() > TopScore)
	{
		TopScorePlayerStates.Empty();
		TopScorePlayerStates.AddUnique(ScoreingPlayerState);
		TopScore = ScoreingPlayerState->GetScore();
	}
	UE_LOG(LogTemp, Warning, TEXT("%f"), TopScore);
}
