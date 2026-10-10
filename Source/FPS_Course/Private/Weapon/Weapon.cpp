// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/Weapon.h"

#include "KismetTraceUtils.h"
#include "FPS_Course/FPS_Course.h"
#include "Interface/PlayerInterface.h"
#include "Kismet/KismetMathLibrary.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bNetUseOwnerRelevancy = true;
	
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh1P"));
	Mesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh1P->bReceivesDecals = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetHiddenInGame(true);
	
	SetRootComponent(Mesh1P);
	
	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh3P"));
	Mesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh3P->bReceivesDecals = false;
	Mesh3P->SetupAttachment(Mesh1P);
	Mesh3P->SetHiddenInGame(true);
	
	AimFOV = 65.f;
	TraceRadius = 5.0f;
	FireTime = 0.1f;
	
	MagCapacity = 10;
	Ammo = 5;
	StartingCarriedAmmo = 10;
	Sequence = 0;
}

void AWeapon::OnRep_Instigator()
{
	Super::OnRep_Instigator();
	
	AttachToOwningPawn();
}

UMaterialInstanceDynamic* AWeapon::GetReticleDynamicMaterialInstance()
{
	if (!IsValid(DynMatInst_Reticle))
	{
		DynMatInst_Reticle = UMaterialInstanceDynamic::Create(ReticleMaterial, this);
	}
	return DynMatInst_Reticle;
}

UMaterialInstanceDynamic* AWeapon::GetAmmoCounterDynamicMaterialInstance()
{
	if (!IsValid(DynMatInst_AmmoCounter))
	{
		DynMatInst_AmmoCounter = UMaterialInstanceDynamic::Create(AmmoCounterMaterial, this);
	}
	return DynMatInst_AmmoCounter;
}

void AWeapon::AttachToOwningPawn() const
{
	APawn* OwningPawn = GetInstigator();
	if (!IsValid(OwningPawn) || !OwningPawn->Implements<UPlayerInterface>()) return;
	
	SetMeshVisibilities(OwningPawn);
	
	const FName AttachPoint = IPlayerInterface::Execute_GetWeaponAttachPoint(OwningPawn, WeaponType);
	USkeletalMeshComponent* PawnMesh1P = IPlayerInterface::Execute_GetMesh1P(OwningPawn);
	USkeletalMeshComponent* PawnMesh3P = IPlayerInterface::Execute_GetMesh3P(OwningPawn);
	
	Mesh1P->AttachToComponent(PawnMesh1P, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
	Mesh3P->AttachToComponent(PawnMesh3P, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
}

void AWeapon::WeaponTrace(FHitResult& OutHitResult, float TraceLength)
{
	FCollisionQueryParams Params;
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActor(GetOwner());
	
	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse.SetAllChannels(ECollisionResponse::ECR_Ignore);
	ResponseParams.CollisionResponse.SetResponse(ECC_Pawn, ECollisionResponse::ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECC_WorldStatic, ECollisionResponse::ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECC_WorldDynamic, ECollisionResponse::ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECC_PhysicsBody, ECollisionResponse::ECR_Block);

	ensure(GetInstigator());
	if (APlayerController* PC = Cast<APlayerController>(GetInstigatorController()); IsValid(PC))
	{
		FVector EyesWorldLocation;
		FRotator EyesWorldRotation;
		
		PC->GetActorEyesViewPoint(EyesWorldLocation, EyesWorldRotation);
		const FVector EyesWorldDirection = UKismetMathLibrary::GetForwardVector(EyesWorldRotation);
		
		const FVector TraceStart = EyesWorldLocation;
		const FVector TraceEnd = TraceStart + EyesWorldDirection * TraceLength;
		
		const bool bHit = GetWorld()->SweepSingleByChannel(
			OutHitResult, 
			TraceStart, 
			TraceEnd,
			FQuat::Identity, 
			FPSTraceChannels::ECC_Weapon, 
			FCollisionShape::MakeSphere(TraceRadius), 
			Params,
			ResponseParams
		);
		
		if (!bHit)
		{
			OutHitResult.ImpactPoint = TraceEnd;
		}
	}
}

void AWeapon::Local_Fire(const FVector& ImpactPoint, const FVector& ImpactNormal, TEnumAsByte<EPhysicalSurface> ImpactSurfaceType, bool bIsFirstPerson)
{
	FireEffects(ImpactPoint, ImpactNormal, ImpactSurfaceType, bIsFirstPerson);
	
	if (GetInstigator()->IsLocallyControlled())
	{
		Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
		++Sequence;
	}
}

void AWeapon::Auth_Fire()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
}

void AWeapon::Rep_Fire(int32 AuthAmmo)
{
	if (GetInstigator()->IsLocallyControlled())
	{
		Ammo = AuthAmmo;
		--Sequence;
		Ammo -= Sequence;
	}
}

void AWeapon::SetMeshVisibilities(APawn* OwningPawn) const
{
	if (OwningPawn->IsLocallyControlled())
	{
		Mesh1P->SetHiddenInGame(false);
		Mesh3P->SetHiddenInGame(true);
	}
	else
	{
		Mesh1P->SetHiddenInGame(true);
		Mesh3P->SetHiddenInGame(false);
	}
}
