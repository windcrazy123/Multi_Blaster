// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WarmUpWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UWarmUpWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TitleText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WarmUpTime;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Info;
};
