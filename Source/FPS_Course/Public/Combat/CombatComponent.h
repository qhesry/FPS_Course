// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeapon;
class UWeaponData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FReticleChanged, UMaterialInstanceDynamic*, ReticleDynMatInst, const FReticleParams&, ReticleParams, bool, bCurrentlyTargetingPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAmmoCounterChanged, UMaterialInstanceDynamic*, AmmoCounterDynMatInst, int32, RoundsCurrent, int32, RoundsMax);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRoundFired, int32, RoundsCurrent, int32, RoundsMax, int32, RoundsInReserve);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAimingStatusChanged, bool, bIsAiming);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTargetingPlayerStatusChanged, bool, bStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCurrentReserveAmmoChanged, int32, RoundsInReserve, int32, RoundsInWeapon, UMaterialInterface*, WeaponIconMaterial );

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPS_COURSE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintPure, Category="FPS|Combat")
	static UCombatComponent* FindCombatComponent(const AActor* Actor) { return (IsValid(Actor) ? Actor->FindComponentByClass<UCombatComponent>() : nullptr); }
	
	// Cycle to the next weapon
	void Initiate_CycleWeapon();
	void Initiate_FireWeapon_Pressed();
	void Initiate_FireWeapon_Released();
	void Initiate_ReloadWeapon();
	void Initiate_Aim_Pressed();
	void Initiate_Aim_Released();
	
	UPROPERTY(BlueprintAssignable)
	FReticleChanged OnReticleChanged;
	
	UPROPERTY(BlueprintAssignable)
	FAmmoCounterChanged OnAmmoCounterChanged;
	
	UPROPERTY(BlueprintAssignable)
	FRoundFired OnRoundFired;
	
	UPROPERTY(BlueprintAssignable)
	FAimingStatusChanged OnAimingStatusChanged;
	
	UPROPERTY(BlueprintAssignable)
	FTargetingPlayerStatusChanged OnTargetingPlayerStatusChanged;
	
	UPROPERTY(BlueprintAssignable)
	FCurrentReserveAmmoChanged OnCurrentReserveAmmoChanged;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|Weapons")
	TObjectPtr<UWeaponData> WeaponData;
	
	void EquipWeapon(AWeapon* Weapon);
	void SpawnInventory();
	void DestroyInventory();
	
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bAiming;
	
	bool bHitPlayer;
	
	UPROPERTY(Transient, BlueprintReadOnly ,ReplicatedUsing=OnRep_CurrentWeapon)
	TObjectPtr<AWeapon> CurrentWeapon;
	
	void InitializeWeaponWidgets() const;
	
	UPROPERTY(ReplicatedUsing = OnRep_CurrentReserveAmmo)
	int32 CurrentReserveAmmo;
protected:
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapons")
	float TraceLength;
	
private:
	TMap<FGameplayTag, int32> ReserveAmmo;
	bool bHitPlayerLastFrame;
	bool bTriggerPressed;
	FTimerHandle FireTimer;
	void FireTimerFinished();
	
	UFUNCTION()
	void OnRep_CurrentWeapon(AWeapon* LastWeapon);
	
	UPROPERTY(Transient, Replicated)
	TArray<AWeapon*> Inventory;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapons")
	TArray<TSubclassOf<AWeapon>> DefaultWeaponClasses;
	
	AWeapon* SpawnWeapon(TSubclassOf<AWeapon> WeaponClass) const;
	
	UFUNCTION(Server, Reliable)
	void Server_Aim(bool bPressed);
		
	UFUNCTION(Server, Reliable)
	void Server_FireWeapon(const FHitResult& Hit);
	
	UFUNCTION()
	void OnRep_CurrentReserveAmmo();
	
	UFUNCTION()
	void Local_Aim(bool bPressed);
	
	UFUNCTION()
	void Local_FireWeapon();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FireWeapon(const FHitResult& Hit, int32 AuthAmmo);
};
