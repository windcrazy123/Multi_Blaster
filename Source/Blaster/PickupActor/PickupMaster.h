// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"

#include "PickupMaster.generated.h"

UCLASS()
class BLASTER_API APickupMaster : public AActor
{
	GENERATED_BODY()
public:
	APickupMaster();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

private:
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* OverlapSphereComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	UStaticMeshComponent* PickupMesh;

	float Offset = 0.f;
	UPROPERTY(EditAnywhere, Category = "PickupProperties|Movement")
	float RotateSpeed = 30.f;
	UPROPERTY(EditAnywhere, Category = "PickupProperties|Movement")
	float OffsetSpeed = 2.f;
	UPROPERTY(EditAnywhere, Category = "PickupProperties|Movement")
	float OffsetExtension = 10.f;
};
