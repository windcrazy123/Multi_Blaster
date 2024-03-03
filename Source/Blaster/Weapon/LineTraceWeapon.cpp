// Fill out your copyright notice in the Description page of Project Settings.


#include "LineTraceWeapon.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void ALineTraceWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket/* && InstigatorController 客户端只有自己的Controller*/)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25f;

		FHitResult FireHit;
		//LineTraceSingleByChannel
		WeaponTraceHit(Start, HitTarget, FireHit, End);

		FVector LaserTarget = End;
		if(FireHit.bBlockingHit)
		{
			LaserTarget = FireHit.ImpactPoint;
			
			//ApplyDamage
			ABlasterCharacter* DamagedCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
            AController* InstigatorController = OwnerPawn->GetController();
             if (DamagedCharacter && HasAuthority() && InstigatorController)
             {
                 UGameplayStatics::ApplyDamage(
                    DamagedCharacter, Damage,
                    InstigatorController, this,
                    UDamageType::StaticClass()
                 );
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
				SocketTransform.GetLocation()
			);
			if (Laser)
			{
				LaserTarget = LaserTarget - SocketTransform.GetLocation();
				Laser->SetNiagaraVariableVec3(FString("Target"), LaserTarget);
			}
		}
	}
}

void ALineTraceWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit, FVector& End)
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
	}
}