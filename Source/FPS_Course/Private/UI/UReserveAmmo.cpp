// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UReserveAmmo.h"

#include "Character/ShooterCharacter.h"
#include "Combat/CombatComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UUReserveAmmo::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	Image_WeaponIcon->SetRenderOpacity(0.f);
	Text_Ammo->SetRenderOpacity(0.f);
	
	GetOwningPlayer()->OnPossessedPawnChanged.AddDynamic(this, &UUReserveAmmo::OnPossessedPawnChanged);
	
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetOwningPlayerPawn());
	if (!IsValid(ShooterCharacter)) return;
	
	OnPossessedPawnChanged(nullptr, ShooterCharacter);
	if (ShooterCharacter->HasWeaponFirstReplicated())
	{
		AWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(ShooterCharacter);
		if (IsValid(Weapon))
		{
			
			OnCurrentReserveAmmoChanged(IPlayerInterface::Execute_GetReserveAmmo(ShooterCharacter), Weapon->Ammo, Weapon->WeaponIcon);
		}
	}
	else
	{
		ShooterCharacter->OnWeaponFirstReplicated.AddDynamic(this, &ThisClass::OnWeaponFirstReplicated);
	}
	
	if (ShooterCharacter->HasAuthority())
	{
		AWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(ShooterCharacter);
		if (IsValid(Weapon))
		{
			OnCurrentReserveAmmoChanged(IPlayerInterface::Execute_GetReserveAmmo(ShooterCharacter), Weapon->Ammo, Weapon->WeaponIcon);
		}
	}
}

void UUReserveAmmo::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	UCombatComponent* OldPawnCombat = UCombatComponent::FindCombatComponent(OldPawn);
	if (IsValid(OldPawnCombat))
	{
		OldPawnCombat->OnCurrentReserveAmmoChanged.RemoveDynamic(this, &ThisClass::OnCurrentReserveAmmoChanged);
		OldPawnCombat->OnRoundFired.RemoveDynamic(this, &ThisClass::OnRoundFired);
	}
	UCombatComponent* NewPawnCombat = UCombatComponent::FindCombatComponent(NewPawn);
	if (IsValid(NewPawnCombat))
	{
		Image_WeaponIcon->SetRenderOpacity(1.f);
		Text_Ammo->SetRenderOpacity(1.f);
		
		NewPawnCombat->OnCurrentReserveAmmoChanged.AddDynamic(this, &ThisClass::OnCurrentReserveAmmoChanged);
		NewPawnCombat->OnRoundFired.AddDynamic(this, &ThisClass::OnRoundFired);
	}
}

void UUReserveAmmo::OnCurrentReserveAmmoChanged(int32 RoundsInReserve, int32 RoundsInWeapon, UMaterialInterface* WeaponIconMaterial)
{
	
	if (IsValid(Image_WeaponIcon) && IsValid(WeaponIconMaterial))
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(WeaponIconMaterial);
		Image_WeaponIcon->SetBrush(Brush);
	}
	
	if (IsValid(Text_Ammo))
	{
		FText AmmoText = FText::Format(NSLOCTEXT("AmmoText", "AmmoKey", "{0} / {1}"), RoundsInWeapon, RoundsInReserve);
		Text_Ammo->SetText(AmmoText);
	}
}

void UUReserveAmmo::OnRoundFired(int32 RoundsCurrent, int32 RoundsMax, int32 RoundsInReserve)
{
	if (IsValid(Text_Ammo))
	{
		FText AmmoText = FText::Format(NSLOCTEXT("AmmoText", "AmmoKey", "{0} / {1}"), RoundsCurrent, RoundsInReserve);
		Text_Ammo->SetText(AmmoText);
	}
}

void UUReserveAmmo::OnWeaponFirstReplicated(AWeapon* Weapon, bool bTargetingPlayer)
{
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetOwningPlayerPawn());
	if (!IsValid(ShooterCharacter)) return;
	OnCurrentReserveAmmoChanged(IPlayerInterface::Execute_GetReserveAmmo(ShooterCharacter), Weapon->Ammo, Weapon->WeaponIcon);
}
