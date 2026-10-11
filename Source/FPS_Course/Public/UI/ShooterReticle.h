// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterReticle.generated.h"
class AWeapon;
class UImage;

UCLASS()
class FPS_COURSE_API UShooterReticle : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Reticle;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_AmmoCounter;
	
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
private:
	TWeakObjectPtr<UMaterialInstanceDynamic> CurrentReticle_DynMatInst;
	TWeakObjectPtr<UMaterialInstanceDynamic> CurrentAmmoCounter_DynMatInst;
	
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	
	UFUNCTION()
	void OnWeaponFirstReplicated(AWeapon* Weapon);
	
	UFUNCTION()
	void OnReticleChanged(UMaterialInstanceDynamic* ReticleDynMatInst);
	
	UFUNCTION()
	void OnAmmoCounterChanged(UMaterialInstanceDynamic* AmmoCounterDynMatInst, int32 RoundsCurrent, int32 RoundsMax);
	
};
