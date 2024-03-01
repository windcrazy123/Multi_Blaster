// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileRocket();
	virtual void BeginPlay() override;
	
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult) override;

private:
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* RocketMeshComponent;

	UPROPERTY(EditDefaultsOnly)
	USoundCue* FlyingLoopSound;
	UPROPERTY()
	UAudioComponent* FlyingLoopAudioComponent;

	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent;
};
