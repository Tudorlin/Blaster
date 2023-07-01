// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ElimAnnouncement.h"
#include "GameFramework/HUD.h"
#include "Util/ColorConstants.h"
#include "Blueprint/UserWidget.h"
#include "BlasterHUD.generated.h"

/**
 此类包含了十字准心，具体可以在Weapon类中定义
 */

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	float CrosshairSpread;    //动态准心参数
	FLinearColor CrosshairColor;
};

UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere,Category="Player Stats")
	TSubclassOf<class UUserWidget>CharacterOverlayClass;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	
	void AddCharacterOverlay();

	UPROPERTY(EditAnywhere,Category="Announcements")
	TSubclassOf<class UUserWidget>AnnouncementClass;

	UPROPERTY(EditAnywhere)
	float ElimAnnouncementTime = 2.5f;

	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToMove);

	UPROPERTY()
	TArray<UElimAnnouncement*>ElimMessages;

	UPROPERTY()
	class UAnnouncement* Announcement;

	void AddAnnouncement();

	void AddElimAnnouncement(FString Attacker , FString Victim);

protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY()
	class APlayerController* OwningPlayer;
	
	FHUDPackage HUDPackage;

	void DrawCrosshairs(UTexture2D* Texture,FVector2d ViewPortCenter,FVector2d Spread,FLinearColor CrosshaairsColor);        //绘制准心

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax=16.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UElimAnnouncement> ElimAnnouncementClass;
public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) {HUDPackage=Package;}  //设置包
};
