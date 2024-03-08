// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupMaster.h"
#include "PickupPotion.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API APickupPotion : public APickupMaster
{
	GENERATED_BODY()
public:
	APickupPotion();
	virtual void Destroyed() override;
protected:
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffect;
	UPROPERTY()
	class UNiagaraComponent* PickupEffectComponent;
	UPROPERTY(EditAnywhere)
	FLinearColor EffectColor;

	UPROPERTY()
	class ABlasterCharacter* PlayerCharacter;

	UPROPERTY(EditAnywhere)
	float PotionAmount;
	//药水额外技能，比如血瓶增加的血上限数量
	UPROPERTY(EditAnywhere)
	float BuffOfPotion;
};
