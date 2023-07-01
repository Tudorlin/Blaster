// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickups.h"

#include "Blaster/BlasterComponent/BuffComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

AHealthPickups::AHealthPickups()
{
	bReplicates = true;
}

void AHealthPickups::OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor,
	UPrimitiveComponent* Othercomp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlapComponent, OtherActor, Othercomp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		UBuffComponent * Buff = BlasterCharacter->GetBuff();
		if(Buff)
		{
			Buff->Heal(HealAmout,HealingTime);
		}
	}
	Destroy();
}

