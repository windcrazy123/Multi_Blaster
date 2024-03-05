// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& HitResult);

	/*
	 * Player Health
	 */
	
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	void RadialDamage();
	void PointDamage(AActor* OtherActor);
private:
	UPROPERTY(EditAnywhere)
	class UBoxComponent* BoxCollision;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;

	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticle;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	//RadialDamage
	UPROPERTY(EditDefaultsOnly, Category = "RadialDamage")
	float MinimumDamage = 10.f;
	UPROPERTY(EditDefaultsOnly, Category = "RadialDamage")
	float DamageInnerRadius = 200.f;
	UPROPERTY(EditDefaultsOnly, Category = "RadialDamage")
	float DamageOuterRadius = 500.f;
};
