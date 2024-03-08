// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

//on server
void UBuffComponent::BuffOfHealth(float NumOfHealth, float IncreasedMaxHealth)
{
	if (PlayerCharacter)
	{
		PlayerCharacter->SetMaxHealth(
			FMath::Clamp(
				PlayerCharacter->GetMaxHealth()+IncreasedMaxHealth,
				PlayerCharacter->GetMaxHealth(), 999.f
			)
		);
		PlayerCharacter->SetCurHealth(
			FMath::Clamp(
				PlayerCharacter->GetCurHealth()+NumOfHealth,
				PlayerCharacter->GetCurHealth(), PlayerCharacter->GetMaxHealth()
			)
		);
		
		if (PlayerCharacter->IsLocallyControlled())
        {
        	PlayerCharacter->UpdateHUDHealth();
        }
	}
	
}

void UBuffComponent::BuffOfSpeed(float NewWalkSpeed, float NewCrouchSpeed, float BuffTime)
{
	if (PlayerCharacter)
	{
		MultiChangeSpeed(NewWalkSpeed, NewCrouchSpeed);
		
		PlayerCharacter->GetWorldTimerManager().SetTimer(
			SpeedBuffTimerHandle,
			this, &UBuffComponent::ResetSpeed,
			BuffTime,
			false
		);
	}
}
void UBuffComponent::ResetSpeed()
{
	MultiChangeSpeed(InitWalkSpeed, InitCrouchSpeed);
}
void UBuffComponent::MultiChangeSpeed_Implementation(float WalkSpeed, float CrouchSpeed)
{
	if (PlayerCharacter && PlayerCharacter->GetCharacterMovement())
	{
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		PlayerCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	}
}



