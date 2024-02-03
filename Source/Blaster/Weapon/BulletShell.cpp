// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletShell.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ABulletShell::ABulletShell()
{
	PrimaryActorTick.bCanEverTick = false;

	ShellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletShellMesh"));
	SetRootComponent(ShellMesh);
	ShellMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	ShellMesh->SetSimulatePhysics(true);
	ShellMesh->SetEnableGravity(true);
	ShellMesh->SetNotifyRigidBodyCollision(true);//In Blueprint: Simulation Generates Hit Events
	ShellEjectionImpulse = 4.f;
}

void ABulletShell::BeginPlay()
{
	Super::BeginPlay();

	ShellMesh->OnComponentHit.AddDynamic(this,&ABulletShell::OnHit);
	ShellMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
}

void ABulletShell::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& HitResult)
{
	if (!bIsSoundPlayed)
	{
		if (ShellLandSound)
        {
        	UGameplayStatics::PlaySoundAtLocation(this, ShellLandSound, GetActorLocation());
        }
        FLatentActionInfo LatentActionInfo(0,36,TEXT("FinishHit"),this);
        UKismetSystemLibrary::Delay(this, 3.f, LatentActionInfo);
		bIsSoundPlayed = true;
	}
}
void ABulletShell::FinishHit()
{
	Destroy();
}
