// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileGrenade::AProjectileGrenade()
{
	GrenadeMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMeshComponent"));
	GrenadeMeshComponent->SetupAttachment(RootComponent);
	GrenadeMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->InitialSpeed = 2000.f;
	ProjectileMovementComponent->MaxSpeed = 2000.f;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;
}

void AProjectileGrenade::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			DestroyTimerHandle,
			this, &AProjectileGrenade::DestroyTimerFinished,
			DestroyTime
		);
		
		ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::AProjectileGrenade::OnBounce);
	}
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,
			BounceSound, GetActorLocation()
		);
	}
}

void AProjectileGrenade::DestroyTimerFinished()
{
	Destroy();
	RadialDamage();
}

void AProjectileGrenade::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& HitResult)
{
	//碰撞不调用Destroy();
}
