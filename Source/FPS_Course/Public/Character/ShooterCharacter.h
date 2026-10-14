// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/PlayerInterface.h"
#include "ShooterTypes/ShooterTypes.h"
#include "Weapon/Weapon.h"
#include "ShooterCharacter.generated.h"

struct FGameplayTag;
class UInputAction;
class UCombatComponent;
class UCameraComponent;
class USpringArmComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponFirstReplicated, AWeapon*, Weapon, bool, bHitPlayer);

UCLASS()
class FPS_COURSE_API AShooterCharacter : public ACharacter, public IPlayerInterface
{
	GENERATED_BODY()

public:
	AShooterCharacter();
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnRep_PlayerState() override;
	
	
	virtual void PossessedBy(AController* NewController) override;
	/** Player Interface */
	virtual FName GetWeaponAttachPoint_Implementation(const FGameplayTag& WeaponTag) override;
	virtual USkeletalMeshComponent* GetMesh1P_Implementation() const override;
	virtual USkeletalMeshComponent* GetMesh3P_Implementation() const override;
	virtual void WeaponReplicated_Implementation() override;
	virtual AWeapon* GetCurrentWeapon_Implementation() override;
	virtual int32 GetReserveAmmo_Implementation() override;
	virtual void NotifyCycleWeapon_Implementation() override;
	/** ~Player Interface */
	
	UFUNCTION(BlueprintCallable)
	FRotator GetFixedAimRotation() const;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|FABRIK")
	FTransform FABRIK_SocketTransform;
	
	UFUNCTION(BlueprintCallable, Category="FPS|FABRIK")
	bool HasCurrentWeapon() const; 
	
	UPROPERTY(BlueprintAssignable)
	FWeaponFirstReplicated OnWeaponFirstReplicated;

	bool HasWeaponFirstReplicated() const { return bWeaponFirstReplicated; }
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS|Combat");
	TObjectPtr<UCombatComponent> CombatComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS|Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Camera")
	float DefaultFOV;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|TurnInPlace")
	float AO_Yaw;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|Strafing")
	float MovementOffsetYaw;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|TurnInPlace")
	ETurningInPlace TurningStatus;
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnAim(bool bIsAiming);

private:
	void Input_CycleWeapon();
	void Input_ReloadWeapon();
	void Input_FireWeapon_Pressed();
	void Input_FireWeapon_Released();
	void Input_AimWeapon_Pressed();
	void Input_AimWeapon_Released();
	
	void CalculateFABRIKSocketTransform();
	void CalculateTurnInPlaceParameters(float DeltaTime);
	void TurnInPlace(float DeltaTime);
	
	bool bWeaponFirstReplicated;
	FRotator StartingAimRotation;
	float InterpAO_Yaw;

	// 1st person view (arms)
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> Mesh1P;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;
	
	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> CycleWeaponAction;
	
	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> FireWeaponAction;
	
	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> ReloadWeaponAction;
	
	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> AimWeaponAction;
};
