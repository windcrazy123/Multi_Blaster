// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "Blaster/Types/TurningInPlace.h"
#include "Blaster/Interfaces/CrosshairInteraction.h"
#include "Blaster/Types/CombatState.h"
#include "Components/TimelineComponent.h"

#include "BlasterCharacter.generated.h"


enum class EWeaponType : uint8;

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
	void ReloadButtonPressed();
	
	void PlayHitReactMontage();
	void SimProxiesTurn();

	/*
	 * Player Health
	 */
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	/*
	 * Score
	 */
	//Poll for any relevant classes and init our HUD
	void PollInit();
	
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly,meta = (AllowPrivateAccess = "true"), Category = Widget)
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,meta = (AllowPrivateAccess = "true"))
	class UBuffComponent* BuffComponent;
	
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
	UPROPERTY(EditAnywhere, Replicated, Category = "Player Stats")
	float MaxHealth = 100.f;
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Health, Category = "Player Stats")
	float CurHealth = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	UPROPERTY()
	class ADCPlayerController* DCPlayerController;

	/*
	 * 血量归零时淘汰
	*/
	UPROPERTY(EditAnywhere, Category = CombatComponent)
	UAnimMontage* ElimMontage;

	bool bElimmed = false;

	FTimerHandle ElimTimer;
	void ElimTimerFinished();
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	/*
	 * Dissolve Effect
	 */
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	
	FOnTimelineFloat DissolveTrackEvent;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	UPROPERTY(EditAnywhere, Category = Elimination)
	UCurveFloat* DissolveCurve;

	// Dynamic instance that we can change at *runtime*
	UPROPERTY(VisibleAnywhere, Category = Elimination)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	// Material instance set on the *Blueprint*, used with the Dynamic Material Instance
	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* DissolveMaterialInstance;	

	/*
	 * Score
	 */
	UPROPERTY()//一方面防止访问垃圾指针另一方面通知虚幻构建工具自动生成对象与虚幻垃圾收集系统正常工作所需的代码，指针都要进行管理
	class ADCPlayerState* DCPlayerState;

	/*
	 * Reload
	 */
	UPROPERTY(EditAnywhere, Category = CombatComponent)
	UAnimMontage* ReloadMontage;
	
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

	/*
	 * 血量归零时淘汰
	 */
	void Eliminate();
	UFUNCTION(NetMulticast, Reliable)
	void MultiEliminate();

	void PlayElimMontage();
	FORCEINLINE bool IsElimmed() const{ return bElimmed; }

	/*
	 * Player Health
	 */
	FORCEINLINE float GetCurHealth() const{ return CurHealth; }
	FORCEINLINE void SetCurHealth(float Health) { CurHealth = Health; }
	FORCEINLINE float GetMaxHealth() const{ return MaxHealth; }
	FORCEINLINE void SetMaxHealth(float Health) { MaxHealth = Health; }
	
	void UpdateHUDHealth();

	/*
	 * Reload
	 */
	void PlayReloadMontage();

	ECombatState GetCombatState() const;

	void JumptoEndSection();

	/*
	 * RestartGame
	 */

	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; }
	void DisableInputMore(bool bRestartGame);

	//狙击枪
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);
	UFUNCTION(BlueprintImplementableEvent)
	void RemoveSniperScopeWidget();

	/*
	 * Pickup
	 */
public:
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoNum);

	FORCEINLINE UBuffComponent* GetBuffComponent() const { return BuffComponent; }
};
