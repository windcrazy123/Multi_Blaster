// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMeshComponent"));
	RocketMeshComponent->SetupAttachment(RootComponent);
	RocketMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& HitResult)
{
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(this,
				Damage, 10.f,
				GetActorLocation(),
				200.f, 500.f,
				1.f,
				UDamageType::StaticClass(),
				TArray<AActor*>(),
				this,
				FiringController//,ECollisionChannel::ECC_Visibility
			);
		}
	}
	
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, HitResult);
}
