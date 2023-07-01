// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

#include "Announcement.h"
#include"GameFramework/PlayerController.h"
#include "CharacterOverlay.h"
#include "ElimAnnouncement.h"
#include "Components/HorizontalBox.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "GameFramework/PlayerState.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABlasterHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::AddAnnouncement()
{
	APlayerController* PlayerController =GetOwningPlayerController();
	if(PlayerController&&AnnouncementClass)
	{
		Announcement=CreateWidget<UAnnouncement>(PlayerController,AnnouncementClass);
		Announcement->AddToViewport();
	}
}

void ABlasterHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	OwningPlayer = OwningPlayer ==nullptr? GetOwningPlayerController() : OwningPlayer;
	if(OwningPlayer&&AnnouncementClass)
	{
		UElimAnnouncement* ElimAnnouncement = CreateWidget<UElimAnnouncement>(OwningPlayer,ElimAnnouncementClass);
		if(ElimAnnouncement)
		{
			ElimAnnouncement->SetElimAnnouncementText(Attacker,Victim);
			ElimAnnouncement->AddToViewport();

			/*
			 *  UCanvasPanelSlot 是 Unreal Engine 4 中的一个类。它表示 UCanvasPanel 类的一个槽位，可以用来放置其它 UI 元素。
				UCanvasPanel 是一个画布面板，它可以用来在游戏屏幕上显示多个 UI 元素，并通过槽位来管理这些元素的布局和尺寸。例如，您可以在一个 UCanvasPanel 上添加多个 UTextBlock 元素，并使用 UCanvasPanelSlot 来控制它们的位置和大小。

				您可以通过 UCanvasPanel 类的 AddChildToCanvas 函数来向画布面板添加 UI 元素。该函数会返回一个 UCanvasPanelSlot 指针，您可以通过该指针来控制新添加的元素的布局和尺寸。例如：

				UCanvasPanel* MyCanvasPanel;
				UTextBlock* MyTextBlock;

				UCanvasPanelSlot* MyCanvasPanelSlot = MyCanvasPanel->AddChildToCanvas(MyTextBlock);
				MyCanvasPanelSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
				MyCanvasPanelSlot->SetOffsets(FMargin(10.0f, 10.0f, 10.0f, 10.0f));
			 */

			for(UElimAnnouncement* Msg : ElimMessages)
			{
				if (Msg && Msg->AnnouncementBox)
				{
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
					if (CanvasSlot)
					{
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(
							CanvasSlot->GetPosition().X,
							Position.Y - CanvasSlot->GetSize().Y
						);
						CanvasSlot->SetPosition(NewPosition);
					}
				}
			}

			ElimMessages.Add(ElimAnnouncement);

			FTimerHandle ElimMsgTimer;
			FTimerDelegate ElimMsgDelegate;
			ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncement);
			GetWorldTimerManager().SetTimer(
				ElimMsgTimer,
				ElimMsgDelegate,
				ElimAnnouncementTime,
				false
			);
		}
	}
}

void ABlasterHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToMove)
{
	if(MsgToMove)
	{
		MsgToMove->RemoveFromParent();
	}
}

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();
	FVector2d ViewportSize;
	if(GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2d ViewportCenter(ViewportSize.X/2.f,ViewportSize.Y/2.f);
		float SpreadScaled = CrosshairSpreadMax*HUDPackage.CrosshairSpread;

		if(HUDPackage.CrosshairsCenter)
		{
			FVector2d Spread(0.f,0.f);
			DrawCrosshairs(HUDPackage.CrosshairsCenter,ViewportCenter,Spread,HUDPackage.CrosshairColor);
		}

		if(HUDPackage.CrosshairsLeft)
		{
			FVector2d Spread(-SpreadScaled,0.f);
			DrawCrosshairs(HUDPackage.CrosshairsLeft,ViewportCenter,Spread,HUDPackage.CrosshairColor);
		}

		if(HUDPackage.CrosshairsRight)
		{
			FVector2d Spread(SpreadScaled,0.f);
			DrawCrosshairs(HUDPackage.CrosshairsRight,ViewportCenter,Spread,HUDPackage.CrosshairColor);
		}

		if(HUDPackage.CrosshairsTop)
		{
			FVector2d Spread(0.f,-SpreadScaled);
			DrawCrosshairs(HUDPackage.CrosshairsTop,ViewportCenter,Spread,HUDPackage.CrosshairColor);
		}

		if(HUDPackage.CrosshairsBottom)
		{
			FVector2d Spread(0.f,SpreadScaled);
			DrawCrosshairs(HUDPackage.CrosshairsBottom,ViewportCenter,Spread,HUDPackage.CrosshairColor);
		}
	}
}

void ABlasterHUD::DrawCrosshairs(UTexture2D* Texture, FVector2d ViewPortCenter,FVector2d Spread,FLinearColor CrosshairsColor)
{
	const float TextureWidth=Texture->GetSizeX();
	const float TextureHeight=Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
	      ViewPortCenter.X-(TextureWidth/2)+Spread.X,
	      ViewPortCenter.Y-(TextureHeight/2)+Spread.Y
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairsColor
	);
}
