// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeapon;
class UWeaponData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPS_COURSE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Cycle to the next weapon
	void Initiate_CycleWeapon();
	void Initiate_FireWeapon_Pressed();
	void Initiate_FireWeapon_Released();
	void Initiate_ReloadWeapon();
	void Initiate_Aim_Pressed();
	void Initiate_Aim_Released();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|Weapons")
	TObjectPtr<UWeaponData> WeaponData;
	
	void EquipWeapon(AWeapon* Weapon);
	void SpawnInventory();
	void DestroyInventory();
	
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bAiming;
	
	UPROPERTY(Transient, BlueprintReadOnly ,ReplicatedUsing=OnRep_CurrentWeapon)
	TObjectPtr<AWeapon> CurrentWeapon;
protected:
	
private:
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
	void Server_FireWeapon();
	
	UFUNCTION()
	void Local_Aim(bool bPressed);
	
	UFUNCTION()
	void Local_FireWeapon();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FireWeapon();
};
