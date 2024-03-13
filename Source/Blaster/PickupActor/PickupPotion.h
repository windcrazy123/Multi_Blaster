// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupMaster.h"
#include "PickupPotion.generated.h"

UENUM(BlueprintType)
enum class EPotionType : uint8
{
	EPT_Health UMETA(DisplayName = "Health Potion"),
	EPT_Speed UMETA(DisplayName = "Speed Potion"),
	EPT_Shield UMETA(DisplayName = "Shield Potion"),

	EPT_MAX UMETA(DisplayName = "DefaultMAX")
};

/**
 * 
 */
UCLASS()
class BLASTER_API APickupPotion : public APickupMaster
{
	GENERATED_BODY()
public:
	APickupPotion();
	//virtual void Destroyed() override;
	
protected:
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere, Category = "PotionProperties")
	class UNiagaraSystem* PickupEffect;
	UPROPERTY()
	class UNiagaraComponent* PickupEffectComponent;
	UPROPERTY(EditAnywhere, Category = "PotionProperties")
	FLinearColor EffectColor;

	UPROPERTY()
	class ABlasterCharacter* PlayerCharacter;

	UPROPERTY(EditAnywhere, Category = "PotionProperties")
	float PotionAmount;
	//药水额外技能，比如血瓶增加的血上限数量，速度的蹲下行走速度, 护盾的最大能量
	UPROPERTY(EditAnywhere, Category = "PotionProperties")
	float BuffOfPotion;

	//暂时有些是永久的，考虑以后添加布尔变量
	UPROPERTY(EditAnywhere, Category = "PotionProperties")
	float BuffTime;

	UPROPERTY(EditAnywhere)
	EPotionType PotionType;

	void PlayPickupEffect();
};
