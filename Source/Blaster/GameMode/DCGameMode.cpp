// Fill out your copyright notice in the Description page of Project Settings.


#include "DCGameMode.h"

#include "Blaster/Character/BlasterCharacter.h"

void ADCGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ADCPlayerController* VictimController,
                                   ADCPlayerController* AttackerController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Eliminate();
	}
}
