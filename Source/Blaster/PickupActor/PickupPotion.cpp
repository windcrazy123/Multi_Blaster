// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupPotion.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Components/BuffComponent.h"

APickupPotion::APickupPotion()
{
	bReplicates = true;
	
	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
	PickupEffectComponent->SetupAttachment(RootComponent);
}

void APickupPotion::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	PlayerCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		switch (PotionType)
		{
		case EPotionType::EPT_Health:
			PlayerCharacter->GetBuffComponent()->BuffOfHealth(PotionAmount, BuffOfPotion);
			break;
		case  EPotionType::EPT_Speed:
			PlayerCharacter->GetBuffComponent()->BuffOfSpeed(PotionAmount, BuffOfPotion, BuffTime);
			break;
		case EPotionType::EPT_Shield:
			PlayerCharacter->GetBuffComponent()->BuffOfShield(PotionAmount, BuffOfPotion);
			break;
		default:
			break;
		}

		SetOwner(PlayerCharacter);
	}

	PlayPickupEffect();
	
	Destroy();
}

void APickupPotion::OnRep_Owner()
{
	Super::OnRep_Owner();

	PlayPickupEffect();

	UE_LOG(LogTemp, Warning, TEXT("OnRep_Owner is vaild"));
}

void APickupPotion::PlayPickupEffect()
{
	if (PickupEffect)
	{
		if (!HasAuthority())
		{
			PlayerCharacter = Cast<ABlasterCharacter>(GetOwner());
		}
		
		UE_LOG(LogTemp, Warning, TEXT("PickupEffect is vaild"));
		if (PlayerCharacter)
		{
			PickupEffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				PickupEffect,
				PlayerCharacter->GetMesh(),
				FName(),
				PlayerCharacter->GetActorLocation(), GetActorRotation(),
				EAttachLocation::KeepWorldPosition,
				true
			);
			UE_LOG(LogTemp, Warning, TEXT("hhh"));
		}
		
		if (PickupEffectComponent)
		{
			PickupEffectComponent->SetColorParameter(FName("UserColor"), EffectColor);
		}
	}
}
