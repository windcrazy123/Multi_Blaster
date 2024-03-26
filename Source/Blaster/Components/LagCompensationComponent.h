// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"


USTRUCT(BlueprintType)
struct FBoxInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;
	UPROPERTY()
	FRotator Rotation;
	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInfo> HitBoxInfos;
};

USTRUCT(BlueprintType)
struct FServerRewindHitResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
	UPROPERTY()
	bool bBodyShot;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	friend class ABlasterCharacter;
	ULagCompensationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server, Reliable)
	void ServerRewindApplyDamage(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
	);

protected:
	virtual void BeginPlay() override;

	FFramePackage InterpFramePackages(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);

private:
	//在PostInitializeComponents初始化
	UPROPERTY()
	ABlasterCharacter* PlayerCharacter;
	UPROPERTY()
	class ADCPlayerController* PlayerController;

	void SaveFramePackage(FFramePackage& Package);
	void DebugFramePackage(const FFramePackage& Package);

	//无法随机访问，这样就会导致在Rewind访问时消耗时间为 O(n) 级别
	//TQueue<FFramePackage> FrameHistory;
	
	// TArray<FFramePackage> FrameHistory;//removeatswap
	// TMap<FFramePackage, int> FrameHistoryIndex;
	//放弃以上两种方案，因为想不出来！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！只能使用可以访问切插入删除方便的双链表了
	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxHistoryLength = 3.f;

	FServerRewindHitResult ServerRewind(ABlasterCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize& HitTarget, 
		float HitTime
	);

	bool LineCheckBox(const FBoxInfo& CheckBoxInfo, const FVector& Start, const FVector& End);
};
