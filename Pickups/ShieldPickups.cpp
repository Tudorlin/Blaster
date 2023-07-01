// Fill out your copyright notice in the Description page of Project Settings.


#include "ShieldPickups.h"

#include "Blaster/BlasterComponent/BuffComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

void AShieldPickups::OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor,
                                     UPrimitiveComponent* Othercomp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlapComponent, OtherActor, Othercomp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		UBuffComponent* Buff = BlasterCharacter->GetBuff();
		if (Buff)
		{
			Buff->ReplenishShield(ShieldReplenishAmount, ShieldReplenishTime);
		}
	}

	Destroy();
}
