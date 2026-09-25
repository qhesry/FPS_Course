// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/CombatComponent.h"

#include "Weapon/Weapon.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void UCombatComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::Initiate_CycleWeapon()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Initiate_CycleWeapon"));
}

void UCombatComponent::Initiate_FireWeapon_Pressed()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Initiate_FireWeapon_Pressed"));
}

void UCombatComponent::Initiate_FireWeapon_Released()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Initiate_FireWeapon_Released"));
}

void UCombatComponent::Initiate_ReloadWeapon()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Initiate_ReloadWeapon"));
}

void UCombatComponent::Initiate_Aim_Pressed()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Initiate_Aim_Pressed"));
}

void UCombatComponent::Initiate_Aim_Released()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Initiate_Aim_Released"));
}

void UCombatComponent::SpawnInventory()
{
	AWeapon* NewWeapon = SpawnWeapon(DefaultWeaponClass);
	if (IsValid(NewWeapon))
	{
		NewWeapon->AttachToOwningPawn();	
	}
}

void UCombatComponent::DestroyInventory()
{
	// TODO : Destroy the inventory when we have one
}

AWeapon* UCombatComponent::SpawnWeapon(TSubclassOf<AWeapon> WeaponClass) const
{
	AActor* OwningActor = GetOwner();
	if (!IsValid(OwningActor)) return nullptr;
	if (OwningActor->GetLocalRole() < ROLE_Authority) return nullptr;

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = Cast<APawn>(OwningActor);
	SpawnInfo.Owner = OwningActor;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	return GetWorld()->SpawnActor<AWeapon>(WeaponClass, SpawnInfo);
}

