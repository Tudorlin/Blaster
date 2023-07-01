// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickups.h"

#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"

void AAmmoPickups::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		UCombatComponent* Combat= BlasterCharacter->GetCombat();
		if(Combat)
		{
			Combat->PickupAmmo(WeaponType,AmmoAmount);
		}
	}
	Destroy();
}
