// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LineTraceWeapon.h"
#include "LineTraceShotgun.generated.h"

//参考资料 https://zh.wikipedia.org/wiki/09%E5%BC%8F%E5%86%9B%E7%94%A8%E9%9C%B0%E5%BC%B9%E6%9E%AA

/**
 * 这个Class有子弹离散的方法，以后如果需要进行重构
 */
UCLASS()
class BLASTER_API ALineTraceShotgun : public ALineTraceWeapon
{
	GENERATED_BODY()
public:
	virtual void WeaponTraceHit(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	uint32 NumberOfOnceShot = 14;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float HalfScatterDegree = 15;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter|Debug")
	bool bDebugDraw = false;
};
