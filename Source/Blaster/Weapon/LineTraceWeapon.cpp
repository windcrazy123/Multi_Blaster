// Fill out your copyright notice in the Description page of Project Settings.


#include "LineTraceWeapon.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Components/LagCompensationComponent.h"
#include "Blaster/PlayerController/DCPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void ALineTraceWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	WeaponTraceHit(HitTarget);
}

void ALineTraceWeapon::WeaponTraceHit(const FVector& HitTarget)
{
	// APawn* OwnerPawn = Cast<APawn>(GetOwner());
	// if (OwnerPawn == nullptr) return;
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket/* && InstigatorController 客户端只有自己的Controller*/)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25f;
		
		//LineTraceSingleByChannel
		SingleLineTrace(Start, End);
	}
	
}

void ALineTraceWeapon::SingleLineTrace(const FVector& TraceStart, const FVector& HitTarget)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	
	FHitResult FireHit;
	
	UWorld* World = GetWorld();
 	if (World)
 	{
 		World->LineTraceSingleByChannel(
 			FireHit,
 			TraceStart,
 			HitTarget,
 			ECollisionChannel::ECC_Visibility
 		);
 	}

	FVector LaserTarget = HitTarget;
	if(FireHit.bBlockingHit)
	{
		LaserTarget = FireHit.ImpactPoint;
			
		//ApplyDamage
		ABlasterCharacter* DamagedCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		AController* InstigatorController = OwnerPawn->GetController();
		if (DamagedCharacter && InstigatorController)
		{
			//on server
			if (HasAuthority() && (!bUseServerRewind || OwnerPawn->IsLocallyControlled()))
			{
				UGameplayStatics::ApplyDamage(
                   DamagedCharacter, Damage,
                   InstigatorController, this,
                   UDamageType::StaticClass()
                );
			}
			if (!HasAuthority() && bUseServerRewind)
			{
				if(OwnerCharacter == nullptr) OwnerCharacter = Cast<ABlasterCharacter>(OwnerPawn);
				if(OwnerController == nullptr) OwnerController = Cast<ADCPlayerController>(InstigatorController);
				if (OwnerCharacter && OwnerController && OwnerCharacter->GetLagCompensationComp())
				{
					OwnerCharacter->GetLagCompensationComp()->ServerRewindApplyDamage(
						DamagedCharacter,
						TraceStart, HitTarget,
						OwnerController->GetServerTime() - OwnerController->GetSingleTripTime()
					);
				}
			}
			
		}
     
		//multi Effect
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
			   GetWorld(),
			   ImpactParticles,
			   FireHit.ImpactPoint
			);
		}
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
			   this,
			   HitSound,
			   FireHit.ImpactPoint
			);
		}
	}

	if (LaserParticles)
	{
		UNiagaraComponent* Laser = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			LaserParticles,
			TraceStart
		);
		if (Laser)
		{
			LaserTarget = LaserTarget - TraceStart;
			Laser->SetNiagaraVariableVec3(FString("Target"), LaserTarget);
		}
	}
}

