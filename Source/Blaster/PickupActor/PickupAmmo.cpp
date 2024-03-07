// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupAmmo.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

void APickupAmmo::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* PlayerCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		PlayerCharacter->PickupAmmo(AmmoType, AmmoCount);
	}

	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,
			PickupSound,
			GetActorLocation()
		);
	}
	
	Destroy();
}
