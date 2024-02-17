// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Blaster/Blaster.h"
//#include "Particles/ParticleSystemComponent.h"

//子弹类型说明：https://zh.wikipedia.org/wiki/5.8%C3%9742mm
AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	SetRootComponent(BoxCollision);
	BoxCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BoxCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BoxCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	BoxCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	BoxCollision->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);//blaster.h

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->InitialSpeed = 93000.f;
	ProjectileMovementComponent->MaxSpeed = 93000.f;
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			BoxCollision, FName(), GetActorLocation(), GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}

	// AddOnComponentHitEvent on server
	if (HasAuthority())
	{
		BoxCollision->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
}

//而所调用的Destroy方法广播给所有绑定的物体进行销毁，而我们的子弹是`bReplicates = true;`那么确实会传播给所有客户端，因此我们可以利用Destroy进行多播。
void AProjectile::Destroyed()
{
	Super::Destroyed();

	if (ImpactParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}

//Just on Server
void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                        FVector NormalImpulse, const FHitResult& HitResult)
{
	/* ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	// if (BlasterCharacter)
	// {
	// 	BlasterCharacter->MultiHitReact();
	// }*/
	
	Destroy();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

