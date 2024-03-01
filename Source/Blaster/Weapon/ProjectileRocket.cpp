// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "RocketMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMeshComponent"));
	RocketMeshComponent->SetupAttachment(RootComponent);
	RocketMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->InitialSpeed = 3000.f;
	RocketMovementComponent->MaxSpeed = 3000.f;
	RocketMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if (FlyingLoopSound)
	{
		FlyingLoopAudioComponent = UGameplayStatics::SpawnSoundAttached(
			FlyingLoopSound, GetRootComponent(), FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			true
		);
	}
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& HitResult)
{
	//由于添加的是RocketMovementComponent，因此撞到东西不会停止，直到撞到其他东西触发Super中的Destroy
	if (OtherActor == GetOwner())
	{
		return;
	}
	
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
