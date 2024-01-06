// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"

#include "Components/TextBlock.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	//ENetRole RemoteRole = InPawn->GetRemoteRole();
	ENetRole LocalRole = InPawn->GetLocalRole();
	FString RoleText;
	switch (LocalRole)
	{
	case ENetRole::ROLE_Authority:
		RoleText = FString("Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		RoleText = FString("Autonomous Proxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		RoleText = FString("Simulated Proxy");
		break;
	case ENetRole::ROLE_None:
		RoleText = FString("None");
		break;
	default :
		RoleText = FString("unkown error");
		break;
	}
	SetDisplayText(FString::Printf(TEXT("Local Role is: %s"), *RoleText));
}

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}
