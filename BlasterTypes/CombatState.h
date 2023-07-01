#pragma once

UENUM(BlueprintType)

enum class ECombtState : uint8
{
	ECS_Unoccupied UMETA(DisPlayName="Unoccupied"),
	ECS_Reloading UMETA(DiisPlayName="Reloading"),
	ECS_ThrowingGrenade UMETA(DiisPlayName="Throwing Grenade"),
	ECS_SwappingWeapons UMETA(DisplayName = "Swapping Weapons"),

	ECS_MAX UMETA(DisPlayName="DefaultMAX")
};