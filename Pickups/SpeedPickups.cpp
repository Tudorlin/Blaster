// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedPickups.h"

#include "Blaster/BlasterComponent/BuffComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

void ASpeedPickups::OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor,
                                    UPrimitiveComponent* Othercomp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlapComponent, OtherActor, Othercomp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter*Character = Cast<ABlasterCharacter>(OtherActor);
	if(Character)
	{
		UBuffComponent* Buff = Character->GetBuff();
		if(Buff)
		{
			Buff->BuffSpeed(BaseBuffSpeed,CrouchBuffSpeed,SpeedBuffTime);
		}
	}
	Destroy();
}
