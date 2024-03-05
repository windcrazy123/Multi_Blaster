// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileGrenade.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileGrenade();
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
private:
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* GrenadeMeshComponent;

	UPROPERTY(EditDefaultsOnly)
	class USoundCue* BounceSound;

	FTimerHandle DestroyTimerHandle;

	void DestroyTimerFinished();

	UPROPERTY(EditDefaultsOnly)
	float DestroyTime = 5.f;

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& HitResult) override;
};
