// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;             //一个安全的虚幻类，如果直接用class选择会让所有声明的类在选择框中，这里只会出现Projectile类

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ServerSdideReWindProjectileClass; //使用服务器倒带时的子弹类，不会被复制
};
