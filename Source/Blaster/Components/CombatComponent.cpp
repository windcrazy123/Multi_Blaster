// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/PlayerController/DCPlayerController.h"
//#include "Blaster/HUD/DCHUD.h"
#include "Camera/CameraComponent.h"

//#include "DrawDebugHelpers.h"


// Set this component to be initialized when the game starts, and to be ticked every frame.
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	AimWalkSpeed = 450.f;
	AimCrouchSpeed = 200.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		BaseWalkSpeed = Character->GetCharacterMovement()->MaxWalkSpeed;
		BaseCrouchSpeed = Character->GetCharacterMovement()->MaxWalkSpeedCrouched;

		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		//carried Ammo
		if (Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(Character && Character->IsLocallyControlled())
	{
		
		SetHUDCrosshairs(DeltaTime);

		//correct weapon rotation at local 为了美观
		FHitResult HitResult;
        TraceUnderCrosshairs(HitResult);
        HitTarget = HitResult.ImpactPoint;

		// FOV
		InterpFOV(DeltaTime);
	}
}

//Tick(local) HUD
void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if(Character == nullptr || Character->Controller == nullptr) return;

	//Controller = Controller == nullptr ? Cast<ADCPlayerController>(Character->Controller) : Controller;
	if(Controller == nullptr) Controller = Cast<ADCPlayerController>(Character->Controller);
	if (Controller)
	{
		if(HUD == nullptr) HUD = Cast<ADCHUD>(Controller->GetHUD());
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
			}

			//Calculate Crosshair Spread Scale
			FVector2D WalkSpeedRange(0.f, BaseWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairsVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairsInAirFactor = FMath::FInterpTo(CrosshairsInAirFactor, 1.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairsInAirFactor = FMath::FInterpTo(CrosshairsInAirFactor, 0.f, DeltaTime, 15.f);
			}
			if (bAiming)
			{
				CrosshairsAimFactor = FMath::FInterpTo(CrosshairsAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairsAimFactor = FMath::FInterpTo(CrosshairsAimFactor, 0.f, DeltaTime, 30.f);
			}

			if(!FMath::IsNearlyZero(CrosshairsShootingFactor, 0.01f))
			{
				CrosshairsShootingFactor = FMath::FInterpTo(CrosshairsShootingFactor, 0.f, DeltaTime, 20.f);
			}
			
			//基础离散值加其他因素影响
			HUDPackage.CrosshairsSpreadScale = 0.5f +
				CrosshairsVelocityFactor + CrosshairsInAirFactor - CrosshairsAimFactor + CrosshairsShootingFactor;
			
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

//Tick(local) FOV
void UCombatComponent::InterpFOV(float DeltaTime)
{
	if(EquippedWeapon == nullptr)return;
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	//first set in locate, then set this boolean by RPC, so that we don't need wait for network delay
	bAiming = bIsAiming;
	//if(!Character->HasAuthority())
	ServerSetAiming(bIsAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = bIsAiming ? AimCrouchSpeed : BaseCrouchSpeed;
	}
}
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = bIsAiming ? AimCrouchSpeed : BaseCrouchSpeed;
	}
}

//Fire
void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	Fire();
}
void UCombatComponent::Fire()
{
	/*if(bCanFire){
		bCanFire = false;
		if (bFireButtonPressed && EquippedWeapon)
		{
			// FHitResult HitResult;
			// TraceUnderCrosshairs(HitResult);
			ServerFire(HitTarget);
			CrosshairsShootingFactor += 0.7f;
		}

		//不论自动还是非自动
		StartFireTimer();
	}*/
	// if (bFireButtonPressed)
	// {
		if(CanFire())
		{
			bCanFire = false;
			ServerFire(HitTarget);
			CrosshairsShootingFactor += 0.7f;

			//不论自动还是非自动
			StartFireTimer();
		}
	//}
}
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(EquippedWeapon == nullptr) return;
	if (Character /*&& CombatState == ECombatState::ECS_Unoccupied*/)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}
void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(FireTimerHandle, this, &UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay);
}
void UCombatComponent::FireTimerFinished()
{
	bCanFire = true;
	if(EquippedWeapon == nullptr) return;
	if(EquippedWeapon->bAutomaticFire){
		Fire();
	}
}

//Tick(local) 获取屏幕中心Hit到物体的Result，距离为 FIRE_TRACE_LENGTH
void UCombatComponent::TraceUnderCrosshairs(FHitResult& HitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X/2.f, ViewportSize.Y/2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this,0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		if (Character)
		{
			float DistanceToCharacter = (Start - Character->GetActorLocation()).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 80.f);// 80.f一个可能合适的数字，为了离开Character一点
		}
		FVector End = Start + CrosshairWorldDirection * FIRE_TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(HitResult,Start, End,ECollisionChannel::ECC_Visibility);
		if (!HitResult.bBlockingHit)
		{
			HitResult.ImpactPoint = End;
		}
		// else
		// {
		// 	DrawDebugSphere(GetWorld(),HitResult.ImpactPoint, 12.f, 12, FColor::Green, true);
		// }

		//检测射线是否命中可与十字准星交互的物体（一般为可伤害的物体）
		if (HitResult.GetActor() && HitResult.GetActor()->Implements<UCrosshairInteraction>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}
	}
}

//server
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr)
		return;
	if (EquippedWeapon)
	{
		EquippedWeapon->Drop();
	}
	EquippedWeapon = WeaponToEquip;

	/*网络和attachactor不一定
	 * 设置武器状态和附着Actor会传播到客户端，但是不能保证是哪一个先到达客户端，为了避免这种假设，可以在OnRep_EquippedWeapon中AttachActor以直接在客户端调用
	 * 但是，SetWeaponState中会处理武器模拟物理的状态，如果在模拟物理时，是没有办法捡起武器的
	 * 所以SetWeaponState和AttachActor这两种函数都要放在OnRep_EquippedWeapon中
	 */
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	
	/* Actor.h
	 *UPROPERTY(ReplicatedUsing=OnRep_Owner)
	AActor* Owner;*/
	EquippedWeapon->SetOwner(Character);
	//Server SetOwner then SetHUDAmmo, use OnRep_Owner on weapon call SetHUDAmmo on all client
	EquippedWeapon->SetHUDAmmo();

	//CarriedAmmo
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	if (Controller == nullptr) Controller = Cast<ADCPlayerController>(Character->Controller);
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		//网络和attachactor不一定，所以在这里复制了
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}
		
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon
		/*&& !EquippedWeapon->IsFull() && !bLocallyReloading*/)
	{
		ServerReload();
		//HandleReload();
		//bLocallyReloading = true;
	}
}
void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_Reloading;
	//if (!Character->IsLocallyControlled())
		HandleReload();
}
void UCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;
	//bLocallyReloading = false;
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if (bFireButtonPressed)
	{
		Fire();
	}
}
void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		//if (Character && !Character->IsLocallyControlled())
			HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	// case ECombatState::ECS_ThrowingGrenade:
	// 	if (Character && !Character->IsLocallyControlled())
	// 	{
	// 		Character->PlayThrowGrenadeMontage();
	// 		AttachActorToLeftHand(EquippedWeapon);
	// 		ShowAttachedGrenade(true);
	// 	}
	// 	break;
	// case ECombatState::ECS_SwappingWeapons:
	// 	if (Character && !Character->IsLocallyControlled())
	// 	{
	// 		Character->PlaySwapMontage();
	// 	}
	// 	break;
	default:
		break;
	}
}
void UCombatComponent::HandleReload()
{
	if (Character)
	{
		Character->PlayReloadMontage();
	}
}
int32 UCombatComponent::ReloadAmmoAmount()
{
	if(EquippedWeapon == nullptr) return 0;
	int32 IntoMagAmmo = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 CarriedAmmoNum = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 LeastNum = FMath::Min(IntoMagAmmo, CarriedAmmoNum);
		//return LeastNum;
		return FMath::Clamp(IntoMagAmmo, 0, LeastNum);
	}
	return 0;
}
void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	int32 ReloadAmmoNum = ReloadAmmoAmount();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmmoNum;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	EquippedWeapon->AddAmmo(ReloadAmmoNum);
	if (Controller == nullptr) Controller = Cast<ADCPlayerController>(Character->Controller);
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}


bool UCombatComponent::CanFire()
{
	if(EquippedWeapon == nullptr) return false;
	return !EquippedWeapon->IsEmpty() && bCanFire && bFireButtonPressed && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	if (Controller == nullptr) Controller = Cast<ADCPlayerController>(Character->Controller);
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
}
