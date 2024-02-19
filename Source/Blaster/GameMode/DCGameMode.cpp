// Fill out your copyright notice in the Description page of Project Settings.


#include "DCGameMode.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/DCPlayerController.h"
#include "Blaster/PlayerState/DCPlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void ADCGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ADCPlayerController* VictimController,
                                   ADCPlayerController* AttackerController)
{
	ADCPlayerState* AttackerPlayerState = AttackerController ? Cast<ADCPlayerState>(AttackerController->PlayerState) : nullptr;
	ADCPlayerState* VictimPlayerState = VictimController ? Cast<ADCPlayerState>(VictimController->PlayerState) : nullptr;
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
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
