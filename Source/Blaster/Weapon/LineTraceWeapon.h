// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "LineTraceWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ALineTraceWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;

protected:
	virtual void WeaponTraceHit(const FVector& HitTarget);

	
	void SingleLineTrace(const FVector& TraceStart, const FVector& HitTarget);
private:
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* LaserParticles;
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;
    
	UPROPERTY(EditAnywhere)
	USoundCue* HitSound;

	UPROPERTY(EditAnywhere)
	float Damage = 10.f;
};
