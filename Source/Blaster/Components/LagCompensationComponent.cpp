// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/Weapon/Weapon.h"

#include "DrawDebugHelpers.h"


ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void ULagCompensationComponent::ServerRewindApplyDamage_Implementation(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	FServerRewindHitResult Confirm = ServerRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if (PlayerCharacter && HitCharacter && PlayerCharacter->GetEquippedWeapon() && Confirm.bHitConfirmed)
	{
		const float DamageMultiply = Confirm.bHeadShot ? 1.25f : (Confirm.bBodyShot ? 1.1f : 1.f);

		UGameplayStatics::ApplyDamage(
			HitCharacter,
			PlayerCharacter->GetEquippedWeapon()->GetDamage() * DamageMultiply,
			PlayerCharacter->Controller,
			PlayerCharacter->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

//on server
void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	if(PlayerCharacter == nullptr) PlayerCharacter = Cast<ABlasterCharacter>(GetOwner());
	if (PlayerCharacter)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		for(auto& HitBox : PlayerCharacter->HitBoxes)
		{
			FBoxInfo BoxInfo;
			BoxInfo.Location = HitBox.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBox.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBox.Value->GetScaledBoxExtent();
			Package.HitBoxInfos.Add(HitBox.Key, BoxInfo);
		}
	}
}

void ULagCompensationComponent::DebugFramePackage(const FFramePackage& Package)
{
	for(auto& Box : Package.HitBoxInfos)
	{
		DrawDebugBox(
			GetWorld(),
			Box.Value.Location, Box.Value.BoxExtent, FQuat(Box.Value.Rotation),
			FColor::Cyan, false, 5.f
		);
	}
}

FServerRewindHitResult ULagCompensationComponent::ServerRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitTarget, float HitTime)
{
	if(HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComp() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetTail() == nullptr)
	{
		return FServerRewindHitResult();
	}

	//=======================================
	//获取HitTime时的Package
	FFramePackage PackageToCheck;
	bool bShouldInterp = true;

	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComp()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	
	if (OldestHistoryTime > HitTime)
	{
		//延迟过高不予考虑
		UE_LOG(LogTemp, Warning, TEXT("延迟过高不予考虑"));
		return FServerRewindHitResult();
	}
	if (OldestHistoryTime == HitTime)
	{
		PackageToCheck = History.GetTail()->GetValue();
		bShouldInterp = false;
	}
	else if (NewestHistoryTime <= HitTime)
	{
		PackageToCheck = History.GetHead()->GetValue();
		bShouldInterp = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = History.GetTail();
	while (Older->GetValue().Time < Younger->GetValue().Time) // 一般Older是不会小于Younger时间的，只是为了防止意外进入死循环
	{
		if(Older->GetValue().Time >= HitTime || Younger->GetValue().Time <= HitTime) break;//这个是主要判断
		if (Younger->GetNextNode() == nullptr || Older->GetPrevNode() == nullptr) break;
		
		Younger = Younger->GetNextNode();
		Older = Older->GetPrevNode();
		
	}
	if (Older->GetValue().Time == HitTime)
	{
		PackageToCheck = Older->GetValue();
		bShouldInterp= false;
	}
	if(Younger->GetValue().Time == HitTime)
	{
		PackageToCheck = Younger->GetValue();
		bShouldInterp = false;
	}
	
	if (bShouldInterp)//找不到而进行 插值 是大概率的
	{
		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
			Older = Older->GetNextNode();
		}
		else if (Younger->GetValue().Time < HitTime)
		{
			Older = Younger;
			Younger = Younger->GetPrevNode();
		}
		PackageToCheck = InterpFramePackages(Older->GetValue(), Younger->GetValue(), HitTime);
	}

	//=======================================
	//检测射线是否穿过盒体GAMES101：Bounding Volume https://www.bilibili.com/video/BV1X7411F744?t=3369.5&p=13
	FServerRewindHitResult ServerRewindHitResult = {false, false, false};

	//头部
	FBoxInfo CheckBoxInfo = PackageToCheck.HitBoxInfos[FName("Head")];
	if (LineCheckBox(CheckBoxInfo, TraceStart, HitTarget))
	{
		UE_LOG(LogTemp, Warning, TEXT("Head"));
		ServerRewindHitResult = {true, true, false};
		DebugFramePackage(PackageToCheck);
		return ServerRewindHitResult;
	}
	//身体
	CheckBoxInfo = PackageToCheck.HitBoxInfos[FName("Body")];
	if (LineCheckBox(CheckBoxInfo, TraceStart, HitTarget))
	{
		UE_LOG(LogTemp, Warning, TEXT("Body"));
		ServerRewindHitResult = {true, false, true};
		DebugFramePackage(PackageToCheck);
		return ServerRewindHitResult;
	}
	//其他：四肢
	for(auto BoxPair : PackageToCheck.HitBoxInfos)
	{
		if(BoxPair.Key == FName("Head") || BoxPair.Key == FName("Body")) continue;
		CheckBoxInfo = BoxPair.Value;
		if (LineCheckBox(CheckBoxInfo, TraceStart, HitTarget))
		{
			UE_LOG(LogTemp, Warning, TEXT("other"));
			ServerRewindHitResult = {true, false, false};
			DebugFramePackage(PackageToCheck);
			return ServerRewindHitResult;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("not hit"));
	return ServerRewindHitResult;
}
bool ULagCompensationComponent::LineCheckBox(const FBoxInfo& CheckBoxInfo,
	const FVector& Start, const FVector& End)
{
	FBox CheckBox(-CheckBoxInfo.BoxExtent, CheckBoxInfo.BoxExtent);
	FTransform BoxTransform = FTransform(CheckBoxInfo.Rotation, CheckBoxInfo.Location, FVector(1.f));
	FRotator InverseRotator = UKismetMathLibrary::InverseTransformRotation(BoxTransform, FRotator(0.f));
	FVector StartVector = InverseRotator.RotateVector(Start - CheckBoxInfo.Location);
	FVector EndVector = InverseRotator.RotateVector(End - CheckBoxInfo.Location);
	EndVector = (EndVector - StartVector).GetSafeNormal()*1.25f + EndVector;
	//获取结果：是否命中
	DrawDebugBox(GetWorld(), CheckBoxInfo.Location,CheckBoxInfo.BoxExtent,FColor::Red,false, 40.f);
	//DrawDebugCone(GetWorld(), StartVector+CheckBoxInfo.Location,EndVector - StartVector, ((End - CheckBoxInfo.Location) * 1.25).Size(), 5.f, 5.f, 16, FColor::Red, false, 10.f);
	DrawDebugLine(GetWorld(), StartVector+CheckBoxInfo.Location, EndVector+CheckBoxInfo.Location,FColor::Red, false, 40.f);
	if(FMath::LineBoxIntersection(CheckBox, StartVector, EndVector, EndVector - StartVector))
	{
		return true;
	}
	return false;
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//server save history
	/*
	if(PlayerCharacter && PlayerCharacter->HasAuthority())
	{
		FFramePackage Package;
        SaveFramePackage(Package);
        if(FrameHistory.Enqueue(Package))
        {
        	FFramePackage TailPackage;
            if(FrameHistory.Peek(TailPackage))
            {
                float HistoryLength = Package.Time - TailPackage.Time;
                while(HistoryLength > MaxHistoryLength)
                {
                	FrameHistory.Dequeue(TailPackage);
                	FrameHistory.Peek(TailPackage);
                	HistoryLength = Package.Time - TailPackage.Time;
                	DebugFramePackage(TailPackage);
                }
            }
        }
	}
	*/
	if(PlayerCharacter == nullptr) PlayerCharacter = Cast<ABlasterCharacter>(GetOwner());
	if (PlayerCharacter == nullptr) return;
	if(!PlayerCharacter->HasAuthority())return;
	
	FFramePackage Package;
	SaveFramePackage(Package);
	FrameHistory.AddHead(Package);

	float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
	while (HistoryLength > MaxHistoryLength)
	{
		FrameHistory.RemoveNode(FrameHistory.GetTail());
		HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
	}

	// if(FrameHistory.GetTail())
	// {
	// 	DebugFramePackage(FrameHistory.GetTail()->GetValue());
	// }
}

FFramePackage ULagCompensationComponent::InterpFramePackages(const FFramePackage& OlderPackage,
	const FFramePackage& YoungerPackage, float HitTime)
{
	const float Distance = YoungerPackage.Time - OlderPackage.Time;
	//插在HitTime的位置 0~1
	const float InterpFraction = FMath::Clamp((HitTime - OlderPackage.Time) / Distance, 0.f, 1.f);

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	for (auto& YoungerHitBox : YoungerPackage.HitBoxInfos)
	{
		const FName& BoxInfoName = YoungerHitBox.Key;

		const FBoxInfo& OlderBox = OlderPackage.HitBoxInfos[BoxInfoName];
		const FBoxInfo& YoungerBox = YoungerPackage.HitBoxInfos[BoxInfoName];

		FBoxInfo InterpBoxInfo;

		//const FVector	DeltaMove = Dist * FMath::Clamp<float>(DeltaTime * InterpSpeed, 0.f, 1.f);
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location,
			1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation,
			1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterpFramePackage.HitBoxInfos.Add(BoxInfoName, InterpBoxInfo);
	}

	return InterpFramePackage;
}