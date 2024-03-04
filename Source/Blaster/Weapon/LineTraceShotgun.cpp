// Fill out your copyright notice in the Description page of Project Settings.


#include "LineTraceShotgun.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"

#include "DrawDebugHelpers.h"


void ALineTraceShotgun::WeaponTraceHit(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket/* && InstigatorController 客户端只有自己的Controller*/)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		//FVector End = Start + (HitTarget - Start) * 1.25f;
		
		FVector ToTargetNormalized = (HitTarget - Start).GetSafeNormal();

		for(uint32 i = 0; i < NumberOfOnceShot; ++i)
        {
			//离散子弹
			FVector RandomNormal = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(ToTargetNormalized, HalfScatterDegree);
			FVector End = Start + RandomNormal * Shotgun_TRACE_LENGTH;

			SingleLineTrace(Start, End);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			if (bDebugDraw)
			{
				DrawDebugCone(GetWorld(), Start, ToTargetNormalized,
					500, HalfScatterDegree*PI/180.f, HalfScatterDegree*PI/180.f,
					16, FColor::Red, true
				);
				DrawDebugLine(GetWorld(), Start, End,
					FColor::Cyan, true
				);
			}
#endif
			
		}
	}
}
