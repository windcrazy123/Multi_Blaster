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

void APickupPotion::Destroyed()
{
	if (PickupEffect)
	{
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
		}
		
		if (PickupEffectComponent)
		{
			PickupEffectComponent->SetColorParameter(FName("UserColor"), EffectColor);
		}
	}

	Super::Destroyed();
}

//暂时只有血瓶，如果后期增加则使用PotionType进行switch
void APickupPotion::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	PlayerCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		PlayerCharacter->GetBuffComponent()->BuffOfHealth(PotionAmount, BuffOfPotion);
	}
	
	Destroy();
}
