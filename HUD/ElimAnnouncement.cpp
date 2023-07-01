// Fill out your copyright notice in the Description page of Project Settings.


#include "ElimAnnouncement.h"

#include "Components/TextBlock.h"

void UElimAnnouncement::SetElimAnnouncementText(FString AttackerName, FString VicmName)
{
	FString ElimAnnouncementText = FString::Printf(TEXT("%s elimmed %s!"),*AttackerName,*VicmName);
	if(AnnouncementText)
	{
		AnnouncementText->SetText(FText::FromString(ElimAnnouncementText)); 
	}
}
