#pragma once

#define FIRE_TRACE_LENGTH 80000.f
#define Shotgun_TRACE_LENGTH 10000.f

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};
