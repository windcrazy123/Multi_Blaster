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

//all client
void APickupPotion::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	PlayerCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (PlayerCharacter && HasAuthority())
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
	}

	PlayPickupEffect();
	if(GetMesh())
		GetMesh()->SetVisibility(false);
	
	if(HasAuthority())
		Destroy();
}

/*
* 原来是Destroy();后使用Destroyed()传播PlayPickupEffect();但由于SpawnSystemAttached中PlayerCharacter获取不到，后改为
* Server：PlayPickupEffect();SetOwner(PlayerCharacter);Destroy();  OnRep_Owner：PlayPickupEffect();
* 但是可能由于Destroy()被标记PendingKill而导致OnRep_Owner没有触发（注：AActor::SetOwner），因此需要改变最初逻辑，所有客户端都可以overlap
* 但是需要将视觉效果和逻辑分主机和客户端，因此对OnSphereBeginOverlap从if (PlayerCharacter)改为if (PlayerCharacter && HasAuthority())
 */
void APickupPotion::PlayPickupEffect()
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

	
}
