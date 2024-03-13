// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupMaster.h"

#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

APickupMaster::APickupMaster()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	OverlapSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphereComponent"));
	OverlapSphereComponent->SetupAttachment(RootComponent);
	OverlapSphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	OverlapSphereComponent->SetRelativeLocation(FVector(0.f, 0.f, 64.f));

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMeshComponent"));
	PickupMesh->SetupAttachment(OverlapSphereComponent);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APickupMaster::BeginPlay()
{
	Super::BeginPlay();
	
	OverlapSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &APickupMaster::OnSphereBeginOverlap);
}

void APickupMaster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (OverlapSphereComponent)
	{
		OverlapSphereComponent->AddWorldRotation(FRotator(0.f, DeltaTime*RotateSpeed, 0.f));
		Offset += DeltaTime*OffsetSpeed;
		OverlapSphereComponent->SetRelativeLocation(FVector(0.f, 0.f, sin(Offset)*OffsetExtension));
	}
}

// void APickupMaster::Destroyed()
// {
// 	Super::Destroyed();
//
// 	
// }

//all client
void APickupMaster::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	PlayPickupSound();
}

void APickupMaster::PlayPickupSound()
{
	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PickupSound,
			GetActorLocation()
		);
	}
}

