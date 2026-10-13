// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterTypes/ShooterTypes.h"
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
	FReticleParams CurrentReticleParams;
	float BaseCornerScaleFactor;
	float BaseShapeCutFactor;
	float _BaseCornerScaleFactor_RoundFired;
	float _BaseCornerScaleFactor_Aiming;
	float _BaseCornerScaleFactor_TargetingPlayer;
	
	float _BaseShapeCutFactor_RoundFired;
	float _BaseShapeCutFactor_Aiming;
	bool bAiming;
	bool bTargetingPlayer;
	
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	
	UFUNCTION()
	void OnWeaponFirstReplicated(AWeapon* Weapon, bool bHitPlayer);
	
	UFUNCTION()
	void OnReticleChanged(UMaterialInstanceDynamic* ReticleDynMatInst, const FReticleParams& ReticleParams, bool bTargeting);
	
	UFUNCTION()
	void OnAmmoCounterChanged(UMaterialInstanceDynamic* AmmoCounterDynMatInst, int32 RoundsCurrent, int32 RoundsMax);
	
	UFUNCTION()
	void OnRoundFired(int32 RoundsCurrent, int32 RoundsMax, int32 RoundsInReserve);
	
	UFUNCTION()
	void OnAimingStatusChanged(bool bIsAiming);
	
	UFUNCTION()
	void OnTargetingPlayerStatusChanged(bool bTargeting);
};
