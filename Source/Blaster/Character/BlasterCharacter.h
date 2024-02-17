// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "Blaster/Types/TurningInPlace.h"
#include "Blaster/Interfaces/CrosshairInteraction.h"

#include "BlasterCharacter.generated.h"


UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public ICrosshairInteraction
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	//注册要复制的变量
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	virtual void OnRep_ReplicatedMovement() override;
protected:
	virtual void BeginPlay() override;
	
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	
	void PlayHitReactMontage();
	void SimProxiesTurn();

	/*
	 * Player Health
	 */
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	void UpdateHUDHealth();
	
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,meta = (AllowPrivateAccess = "true"), Category = Widget)
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* CombatComponent;
	
	//复制    works only when the variable is changed
	//UPROPERTY(Replicated)
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);//可以有输入参数，但只能有一个被复制变量的类型的参数，此例中只能有一个 AWeapon* 类型的参数，指被复制前最后一个数据

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	float AO_Yaw;
	float InterpAOYaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = CombatComponent)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = CombatComponent)
	UAnimMontage* HitReactMontage;
	
	//相机靠近隐藏Character
	void HideCameraIfCharacterClose();
	float CameraThreashold = 200.f;

	//Simulate Proxy smooth rotate
	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	void CalculateAOPitch();
	float CalculateSpeed();

	/*
	 * Player Health
	 */
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Health, Category = "Player Stats")
	float CurHealth = 100.f;

	UFUNCTION()
	void OnRep_Health();

	class ADCPlayerController* DCPlayerController;
public:	
	//FORCEINLINE void SetOverlappingWeapon(AWeapon* Weapon){ OverlappingWeapon = Weapon; }
	void SetOverlappingWeapon(AWeapon* Weapon);
	
	void PlayFireMontage(bool bIsAiming);
	
	// UFUNCTION(NetMulticast, Unreliable)  //removed into ReceiveDamage, because ReplicatedNotify is better than RPC
	// void MultiHitReact();

	bool IsWeaponEquipped();
	bool IsAiming();

	FORCEINLINE float GetAOYaw() const { return AO_Yaw; }
	FORCEINLINE float GetAOPitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const{ return bRotateRootBone; }

	AWeapon* GetEquippedWeapon();
	FVector GetHitTarget() const;
};
