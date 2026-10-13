// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/CombatComponent.h"

#include "Data/WeaponData.h"
#include "FPS_Course/FPS_Course.h"
#include "Interface/PlayerInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	TraceLength = 20000.f;
	bAiming = false;
	bTriggerPressed = false;
}

void UCombatComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwningPawn) || !OwningPawn->IsLocallyControlled()) return;
	
	APlayerController* PC = Cast<APlayerController>(OwningPawn->GetController());
	if (!IsValid(PC)) return;

	FVector EyesWorldLocation;
	FRotator EyesWorldRotation;
	PC->GetPlayerViewPoint(EyesWorldLocation, EyesWorldRotation);
	
	const FVector EyesWorldDirection = UKismetMathLibrary::GetForwardVector(EyesWorldRotation);
	const FVector Start = EyesWorldLocation;
	const FVector End = Start + EyesWorldDirection * TraceLength;
	
	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwningPawn);
	
	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse.SetAllChannels(ECollisionResponse::ECR_Ignore);
	ResponseParams.CollisionResponse.SetResponse(ECC_Pawn, ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECC_PhysicsBody, ECR_Block);
	
	GetWorld()->LineTraceSingleByChannel(Hit, Start, End, FPSTraceChannels::ECC_Weapon, QueryParams, ResponseParams);
	
	bHitPlayer = IsValid(Hit.GetActor()) && Hit.GetActor()->Implements<UPlayerInterface>();
	
	if (bHitPlayer != bHitPlayerLastFrame)
	{
		OnTargetingPlayerStatusChanged.Broadcast(bHitPlayer);
		
	}
	
	bHitPlayerLastFrame = bHitPlayer;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, Inventory);
	DOREPLIFETIME(ThisClass, CurrentWeapon);
	DOREPLIFETIME_CONDITION(ThisClass, bAiming, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ThisClass, CurrentReserveAmmo, COND_OwnerOnly);
}

void UCombatComponent::OnRep_CurrentWeapon(AWeapon* LastWeapon)
{
	if (!IsValid(CurrentWeapon)) return;
	CurrentWeapon->AttachToOwningPawn();
	IPlayerInterface::Execute_WeaponReplicated(GetOwner());
	InitializeWeaponWidgets();
}

void UCombatComponent::Initiate_CycleWeapon()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Initiate_CycleWeapon"));
}

void UCombatComponent::Initiate_FireWeapon_Pressed()
{
	if (!IsValid(CurrentWeapon)) return;
	bTriggerPressed = true;
	
	if (CurrentWeapon->Ammo > 0)
	{
		Local_FireWeapon();	
	}
}


void UCombatComponent::Local_FireWeapon()
{
	if (!IsValid(CurrentWeapon)) return;
	ensure(IsValid(WeaponData));
	
	UAnimMontage* Montage1P = WeaponData->FirstPersonMontages.FindChecked(CurrentWeapon->WeaponType).FireMontage;
	USkeletalMeshComponent* Mesh1P = IPlayerInterface::Execute_GetMesh1P(CurrentWeapon->GetOwner());
	
	if (IsValid(Montage1P) && IsValid(Mesh1P))
	{
		Mesh1P->GetAnimInstance()->Montage_Play(Montage1P);
	}
	
	FHitResult Hit;
	CurrentWeapon->WeaponTrace(Hit, TraceLength);
	
	EPhysicalSurface ImpactSurfaceType = Hit.PhysMaterial.IsValid(false) ? Hit.PhysMaterial->SurfaceType.GetValue() : EPhysicalSurface::SurfaceType1;
	CurrentWeapon->Local_Fire(Hit.ImpactPoint, Hit.ImpactNormal, ImpactSurfaceType, true);
	OnRoundFired.Broadcast(CurrentWeapon->Ammo, CurrentWeapon->MagCapacity, CurrentReserveAmmo);
	
	GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, CurrentWeapon->FireTime);
	
	Server_FireWeapon(Hit);
}

void UCombatComponent::FireTimerFinished()
{
	if (!IsValid(CurrentWeapon)) return;
	
	if (bTriggerPressed && CurrentWeapon->FireType == EFireType::Auto && CurrentWeapon->Ammo > 0)
	{
		Local_FireWeapon();
	}
}



void UCombatComponent::Server_FireWeapon_Implementation(const FHitResult& Hit)
{
	if (!IsValid(CurrentWeapon)) return;
	
	if (!Cast<APawn>(GetOwner())->IsLocallyControlled())
	{
		CurrentWeapon->Auth_Fire();
	}
	
	Multicast_FireWeapon(Hit, CurrentWeapon->Ammo);
}


void UCombatComponent::Multicast_FireWeapon_Implementation(const FHitResult& Hit, int32 AuthAmmo)
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (OwningPawn->IsLocallyControlled())
	{
		CurrentWeapon->Rep_Fire(AuthAmmo); 
	} 
	else 
	{
		ensure(IsValid(WeaponData));
	
		UAnimMontage* Montage3P = WeaponData->ThirdPersonMontages.FindChecked(CurrentWeapon->WeaponType).FireMontage;
		USkeletalMeshComponent* Mesh3P = IPlayerInterface::Execute_GetMesh3P(CurrentWeapon->GetOwner());
	
		EPhysicalSurface ImpactSurfaceType = Hit.PhysMaterial.IsValid(false) ? Hit.PhysMaterial->SurfaceType.GetValue() : EPhysicalSurface::SurfaceType1;
		CurrentWeapon->Local_Fire(Hit.ImpactPoint, Hit.ImpactNormal, ImpactSurfaceType, false);
		
		if (IsValid(Montage3P) && IsValid(Mesh3P))
		{
			Mesh3P->GetAnimInstance()->Montage_Play(Montage3P);
		}
	}
}

void UCombatComponent::Initiate_FireWeapon_Released()
{
	bTriggerPressed = false;
}

void UCombatComponent::Initiate_ReloadWeapon()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Initiate_ReloadWeapon"));
}

void UCombatComponent::Initiate_Aim_Pressed()
{
	Server_Aim(true);
	Local_Aim(true);
}

void UCombatComponent::Initiate_Aim_Released()
{
	Server_Aim(false);
	Local_Aim(false);
}

void UCombatComponent::Server_Aim_Implementation(bool bPressed)
{
	Local_Aim(bPressed);
}

void UCombatComponent::OnRep_CurrentReserveAmmo()
{
	if (!IsValid(CurrentWeapon)) return;
	
	OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, CurrentWeapon->Ammo, CurrentWeapon->WeaponIcon);
}

void UCombatComponent::Local_Aim(bool bPressed)
{
	bAiming = bPressed;
	OnAimingStatusChanged.Broadcast(bAiming);
	
}

void UCombatComponent::SpawnInventory()
{
	if (GetOwner()->GetLocalRole() < ROLE_Authority) return;
	for (TSubclassOf<AWeapon>& WeaponClass : DefaultWeaponClasses)
	{
		AWeapon* NewWeapon = SpawnWeapon(WeaponClass);
		Inventory.Add(NewWeapon);
		ReserveAmmo.Add(NewWeapon->WeaponType, NewWeapon->StartingCarriedAmmo);
	}
	
	if (Inventory.Num() > 0)
	{
		EquipWeapon(Inventory[0]);
		InitializeWeaponWidgets();
	}
}

void UCombatComponent::EquipWeapon(AWeapon* Weapon)
{
	CurrentWeapon = Weapon;
	CurrentWeapon->AttachToOwningPawn();
	
	CurrentReserveAmmo = ReserveAmmo.FindChecked(Weapon->WeaponType);
	OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, Weapon->Ammo, Weapon->WeaponIcon);
}

void UCombatComponent::DestroyInventory()
{
	for (AWeapon* Weapon : Inventory)
	{
		Weapon->Destroy();
	}
	Inventory.Empty();
}

void UCombatComponent::InitializeWeaponWidgets() const
{
	if (IsValid(CurrentWeapon))
	{
		OnReticleChanged.Broadcast(CurrentWeapon->GetReticleDynamicMaterialInstance(), CurrentWeapon->ReticleParams, bHitPlayer);
		OnAmmoCounterChanged.Broadcast(CurrentWeapon->GetAmmoCounterDynamicMaterialInstance(), CurrentWeapon->Ammo, CurrentWeapon->MagCapacity);
	}
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

