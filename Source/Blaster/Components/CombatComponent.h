// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Blaster/HUD/DCHUD.h"
#include "Blaster/Types/CombatState.h"
#include "Blaster/Weapon/WeaponTypes.h"

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
	void Reload();
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	/*
	 * Disable Input
	 */

	void SetFireButtonPressed(bool bIsCanFire);
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
	/*
	 * Reload
	 */
	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	//返回从背包取出对应类型子弹装载到武器弹夹中的子弹数量
	int32 ReloadAmmoAmount();
private:
	void Fire();

	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ADCPlayerController* Controller;
	UPROPERTY()
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

	//检查EquippedWeapon和子弹和bCanFire和bFireButtonPressed和CombatState
	//子弹用完时禁止开火
	bool CanFire();

	/*
	 * 携带子弹数
	 */
	//指携带的当前武器种类所属子弹类型的数量
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;
	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	void InitializeCarriedAmmo();

	/*
	 * Reloading CombaState
	 */
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	//update ammo and carried ammo number
	void UpdateAmmoValues();
};
