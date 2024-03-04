// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"

#include "Blaster/Components/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Blaster.h"
#include "Blaster/GameMode/DCGameMode.h"
#include "Blaster/PlayerController/DCPlayerController.h"
#include "Blaster/PlayerState/DCPlayerState.h"


ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	//蓝图中Character的ClassDefault中Actor->SpawnCollisionHandling改为调整但总是生成
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("MyCameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("MyFollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);// component dont need GetLifetimeReplicatedProps

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);// blaster.h
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	GetCharacterMovement()->RotationRate = FRotator(0.f,850.f,0.f);

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// #include "Net/UnrealNetwork.h"
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	//DOREPLIFETIME(ABlasterCharacter, OverlappingWeapon);
	
	DOREPLIFETIME(ABlasterCharacter, CurHealth);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent)
	{
		CombatComponent->Character = this;
	}
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	//依旧需要在BeginPlay中调用这个，因为尽管在Controller中的OnPossess添加了SetHudHealth，
	//但是在游戏开始和OnPossess时并不是所有东西都初始化好了，可能DCHud已经初始化而DCHud->CharacterOverlay还没有
	UpdateHUDHealth();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//rep notify can replace it, not Replicated
	/* if(OverlappingWeapon)
	 {
	 	OverlappingWeapon->ShowPickupWidget(true);
	 }*/
	
	//AimOffset(DeltaTime);
	////add if(Simulate Proxy smooth rotate)
	if (/*GetLocalRole() > ENetRole::ROLE_SimulatedProxy && */IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	// else		//(move to OnRep_ReplicatedMovement)
	// {
	// 	SimProxiesTurn();
	// }
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.2f)//如果隔了这么长时间就直接调用
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAOPitch();
	}
	
	if (IsLocallyControlled())
	{
		HideCameraIfCharacterClose();
	}

	PollInit();
}

void ABlasterCharacter::PollInit()
{
	//不知道PlayerState什么时候能初始化，所以在tick中不断尝试进行初始化
	if (DCPlayerState == nullptr)
	{
		DCPlayerState = GetPlayerState<ADCPlayerState>();
		if (DCPlayerState)
		{
			DCPlayerState->AddToScore(0.f);
			DCPlayerState->AddToDefeats(0);
		}
	}
}
//Tick(Simulate Proxy)
void ABlasterCharacter::SimProxiesTurn()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	bRotateRootBone = false;

	if (CalculateSpeed() > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	//CalculateAOPitch();

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}
// Tick(AutonomousProxy and Authority)
void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (CombatComponent && CombatComponent->EquippedWeapon == nullptr) return;
	
	float Speed  = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)
	{
		bRotateRootBone = true;
		// GetBaseAimRotation 返回的数值是将pitch压缩至1字节并网络同步后解压缩的一个FRotator，此pitch取值范围是0~360
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAOYaw = AO_Yaw;
		}
		//bUseControllerRotationYaw = false;

		TurnInPlace(DeltaTime);
	}
	if (Speed>0.f || bIsInAir)
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		//bUseControllerRotationYaw = true;

		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	CalculateAOPitch();
}
void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAOYaw = FMath::FInterpTo(InterpAOYaw, 0.f, DeltaTime, 5.f);
		AO_Yaw = InterpAOYaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}
void ABlasterCharacter::CalculateAOPitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		AO_Pitch -= 360.f;
		// FVector2D InRange(270.f, 360.f);
		// FVector2D OutRange(-90.f, 0.f);
		// AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}
float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

//Tick(local)
void ABlasterCharacter::HideCameraIfCharacterClose()
{
	//if(!IsLocallyControlled()) return;

	//靠近则隐藏
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreashold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->SetOwnerNoSee(true);
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->SetOwnerNoSee(false);
		}
	}
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump",IE_Pressed,this,&ACharacter::Jump);
	PlayerInputComponent->BindAction("Equip",IE_Pressed,this,&ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch",IE_Pressed,this,&ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim",IE_Pressed,this,&ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim",IE_Released,this,&ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire",IE_Pressed,this,&ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire",IE_Released,this,&ABlasterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload",IE_Released,this,&ABlasterCharacter::ReloadButtonPressed);
	
	PlayerInputComponent->BindAxis("MoveForward",this,&ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",this,&ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn",this,&ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp",this,&ABlasterCharacter::LookUp);
}

void ABlasterCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f,Controller->GetControlRotation().Yaw,0.f);
		const FVector ForwardDirection(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(ForwardDirection, Value);
		// AddMovementInput(GetActorForwardVector(),Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f,Controller->GetControlRotation().Yaw,0.f);
		const FVector ForwardDirection(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(ForwardDirection, Value);
	}
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::EquipButtonPressed()
{
	if (CombatComponent)
	{
		if (HasAuthority())
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}
void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();// there is a Replicated boolean var "bIsCrouched"
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(false);
	}
}

void ABlasterCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	Super::Jump();
}

void ABlasterCharacter::FireButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(false);
	}
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->Reload();
	}
}

void ABlasterCharacter::PlayFireMontage(bool bIsAiming)
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bIsAiming ? FName("FireRifleAim") : FName("FireRifleHip");
		AnimInstance->Montage_JumpToSection(SectionName, FireWeaponMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	//暂时只有拿着武器的受击动画
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromLeft");
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}
/* void ABlasterCharacter::MultiHitReact_Implementation()
// {
// 	PlayHitReactMontage();
// }*/

//rep notify只有一条路，即：服务器到客户端，因此不会给服务器发送通知，也就是说服务器不会被调用到此方法
void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ABlasterCharacter::OnRep_Health()
{
	if(IsLocallyControlled())
	{
		UpdateHUDHealth();
	}
	
	PlayHitReactMontage();
}

//on server
void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	CurHealth = FMath::Clamp(CurHealth - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	/*
	 * 血量归零时淘汰
	 */
	if (CurHealth == 0.f)
	{
		ADCGameMode* DCGameMode = GetWorld()->GetAuthGameMode<ADCGameMode>();
        if (DCGameMode)
        {
        	if(DCPlayerController == nullptr) DCPlayerController = Cast<ADCPlayerController>(Controller);
	        //ADCPlayerController* AttackerController = Cast<ADCPlayerController>(GetInstigatorController());
	        ADCPlayerController* AttackerController = Cast<ADCPlayerController>(InstigatedBy);
        	//PlayerEliminated有检查
	        DCGameMode->PlayerEliminated(this, DCPlayerController, AttackerController);
        }
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	if(DCPlayerController == nullptr) DCPlayerController = Cast<ADCPlayerController>(Controller);
	if (DCPlayerController)
	{
		DCPlayerController->SetHudHealth(CurHealth, MaxHealth);
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)	// LastWeapon
		{
			OverlappingWeapon->ShowPickupWidget(false);
		}
	}
	OverlappingWeapon = Weapon;
	//此方法只有服务器会调用，因此在这里只处理好服务器即可，客户端通过rep notify方法通知
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (CombatComponent && CombatComponent->bAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if(CombatComponent == nullptr) return nullptr;
	return CombatComponent->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if(CombatComponent == nullptr) return FVector();
	return  CombatComponent->HitTarget;
}

//on server
void ABlasterCharacter::Eliminate()
{
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->EquippedWeapon->Drop();
	}
	//equipweapon利用之后的setowner的notify进行多播SetHUDAmmo();，淘汰就利用下面多播
	MultiEliminate();
	GetWorldTimerManager().SetTimer(ElimTimer, this, &ABlasterCharacter::ElimTimerFinished, ElimDelay);
}
void ABlasterCharacter::MultiEliminate_Implementation()
{
	//Ammo
	if (DCPlayerController)
	{
		DCPlayerController->SetHUDWeaponAmmo(0);
	}
	
	bElimmed = true;
	PlayElimMontage();

	// Dissolve Effect
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);

		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(FName("Dissolve"), 0.55f);
		//DynamicDissolveMaterialInstance->SetScalarParameterValue(FName("Glow"), 200.f);
	}
	StartDissolve();

	
	DisableInputMore(false);
	
	//
	//SetCanBeDamaged(false);

	//狙击枪
	if (IsLocallyControlled() && CombatComponent && CombatComponent->bAiming
		&& CombatComponent->EquippedWeapon
		&& CombatComponent->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		RemoveSniperScopeWidget();
	}
}
void ABlasterCharacter::DisableInputMore(bool bRestartGame)
{
	//disable character movement
	GetCharacterMovement()->DisableMovement();//MOVE_NONE 没办法wasd
	GetCharacterMovement()->StopMovementImmediately();//把旋转角色也禁用了
	if (DCPlayerController)
	{
		DisableInput(DCPlayerController);
	}
	//disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//stop fire
	CombatComponent->SetFireButtonPressed(false);
	
	//Restart Game
	if (bRestartGame)
	{
		//deEquipWeapon
		GetCharacterMovement()->bOrientRotationToMovement = true;
		bUseControllerRotationYaw = false;
		
		if (CombatComponent)
		{
			
			//Destroy weapon and stop equipment animation
			if(CombatComponent->EquippedWeapon)
			{
				CombatComponent->EquippedWeapon->Destroy();
				CombatComponent->EquippedWeapon = nullptr;
			}
		}
		//视角转到前方且禁止旋转
		if (CameraBoom)
		{
			CameraBoom->bUsePawnControlRotation = false;
			FRotator NewRotation(-10.f, -80.f, 0.f);
			CameraBoom->SetRelativeRotation(NewRotation);
			CameraBoom->bDoCollisionTest = false;
		}
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::PlayReloadMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (CombatComponent->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		default:
			SectionName = FName("Rifle");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName, ReloadMontage);
	}
}

//淘汰倒计时结束：重生
void ABlasterCharacter::ElimTimerFinished()
{
	ADCGameMode* DCGameMode = GetWorld()->GetAuthGameMode<ADCGameMode>();
	if (DCGameMode)
	{
		DCGameMode->RequestRespawn(this, Controller);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrackEvent.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);

	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrackEvent);//call back
		DissolveTimeline->Play();
	}
}
void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(FName("Dissolve"), DissolveValue);
	}
}
 
ECombatState ABlasterCharacter::GetCombatState() const
{
	if(CombatComponent == nullptr) return ECombatState::ECS_MAX;
	return  CombatComponent->CombatState;
}

