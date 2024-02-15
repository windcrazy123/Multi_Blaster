// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Blaster/HUD/DCHUD.h"

#include "CombatComponent.generated.h"

#define FIRE_TRACE_LENGTH 80000.f

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class ABlasterCharacter;// ABlasterCharacter has full access
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	

	void EquipWeapon(class AWeapon* WeaponToEquip);
protected:
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireButtonPressed(bool bPressed);
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& HitResult);

	void SetHUDCrosshairs(float DeltaTime);
private:
	void Fire();
	
	class ABlasterCharacter* Character;
	class ADCPlayerController* Controller;
	class ADCHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	float BaseWalkSpeed;
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;
	
	float BaseCrouchSpeed;
	UPROPERTY(EditAnywhere)
	float AimCrouchSpeed;

	bool bFireButtonPressed;

	/*
	 * HUD and Crosshairs
	 */
	float CrosshairsVelocityFactor;
	float CrosshairsInAirFactor;
	float CrosshairsAimFactor;
	float CrosshairsShootingFactor;
	
	FHUDPackage HUDPackage;

	//correct weapon rotation at local
	FVector HitTarget;

	/*
	 * Aiming and FOV
	 * 武器停止aim时都是这个zoom的速度
	 */
	//Field of view when not aiming; set to the camera's base FOV in BeginPlay
	float DefaultFOV;
	float CurrentFOV;
	//UPROPERTY(EditAnywhere, Category = CombatComponent)
	//float ZoomedFOV = 30.f;
	UPROPERTY(EditAnywhere, Category = CombatComponent)
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/*
	 * Automatic Fire
	 */
	FTimerHandle FireTimerHandle;

	//在firedelay未结束时禁止开火(手动或自动)  同时这样也使非自动的武器有一个强制的禁止开火时间
	bool bCanFire = true;
	//设置并启动计时器
	void StartFireTimer();
	//计时器回调
	void FireTimerFinished();
};
