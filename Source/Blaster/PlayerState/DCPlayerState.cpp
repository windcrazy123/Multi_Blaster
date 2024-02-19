// Fill out your copyright notice in the Description page of Project Settings.


#include "DCPlayerState.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/DCPlayerController.h"
#include "Net/UnrealNetwork.h"

void ADCPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADCPlayerState, Defeats);
}

void ADCPlayerState::AddToScore(float ScoreAmount)
{
	//Score += ScoreAmount;This member will be made private.Use GetScore or SetScore instead.
	SetScore(GetScore()+ScoreAmount);

	if(Character == nullptr) Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		//if(Controller == nullptr) Controller = Cast<ADCPlayerController>(Character->Controller);
		Controller = Controller ? Controller : Cast<ADCPlayerController>(Character->Controller);
		if (Controller)
		{
			Controller->SetHudScore(GetScore());
		}
	}
}
void ADCPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	if(Character == nullptr) Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		Controller = Controller ? Controller : Cast<ADCPlayerController>(Character->Controller);
		if (Controller)
		{
			Controller->SetHudScore(GetScore());
		}
	}
}

void ADCPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;

	if(Character == nullptr) Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		Controller = Controller ? Controller : Cast<ADCPlayerController>(Character->Controller);
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}
void ADCPlayerState::OnRep_Defeats()
{
	if(Character == nullptr) Character = Cast<ABlasterCharacter>(GetPawn());
	if (Character)
	{
		Controller = Controller ? Controller : Cast<ADCPlayerController>(Character->Controller);
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}