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
	UE_LOG(LogTemp, Warning, TEXT("SetHUDWeaponAmmo"));
	if(DCHud == nullptr) DCHud = Cast<ADCHUD>(GetHUD());

	if(DCHud && DCHud->CharacterOverlay && DCHud->CharacterOverlay->WeaponAmmoAmount)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		DCHud->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}
