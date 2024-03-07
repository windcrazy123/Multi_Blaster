// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "PickupMaster.h"
#include "Blaster/Weapon/WeaponTypes.h"

#include "PickupAmmo.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API APickupAmmo : public APickupMaster
{
	GENERATED_BODY()
public:
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
private:
	
	UPROPERTY(EditAnywhere, Category = "PickupProperties")
	EWeaponType AmmoType;

	UPROPERTY(EditAnywhere, Category = "PickupProperties")
	int32 AmmoCount;
};
