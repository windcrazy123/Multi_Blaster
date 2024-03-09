// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UBuffComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	friend class ABlasterCharacter;

protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY()
	ABlasterCharacter* PlayerCharacter;

public:
	void BuffOfHealth(float NumOfHealth, float IncreasedMaxHealth);

	void BuffOfSpeed(float WalkSpeed, float CrouchSpeed, float BuffTime);

	void BuffOfShield(float NumOfShield, float MaxShield);

private:
	FTimerHandle SpeedBuffTimerHandle;

	UFUNCTION()
	void ResetSpeed();
	UFUNCTION(NetMulticast, Reliable)
	void MultiChangeSpeed(float WalkSpeed, float CrouchSpeed);

	float InitWalkSpeed;
	float InitCrouchSpeed;
		
};
