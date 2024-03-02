// Fill out your copyright notice in the Description page of Project Settings.


#include "LineTraceWeapon.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

void ALineTraceWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket && InstigatorController)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		//LineTraceSingleByChannel
		WeaponTraceHit(Start, HitTarget, FireHit);

		if(!FireHit.bBlockingHit) return;
		
		ABlasterCharacter* DamagedCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
        if (DamagedCharacter && HasAuthority())
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
}

void ALineTraceWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;

		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
	}
}