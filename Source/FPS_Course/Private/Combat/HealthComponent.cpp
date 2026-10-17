
#include "Combat/HealthComponent.h"

#include "Net/UnrealNetwork.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	DeathState = EDeathState::NotDead;
	Health = 100.0f;
	MaxHealth = 100.0f;
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, DeathState);
	DOREPLIFETIME_CONDITION(ThisClass, Health, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, MaxHealth, COND_OwnerOnly);
}

float UHealthComponent::GetHealthNormalized() const
{
	return (MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f;
}

bool UHealthComponent::ChangeHealthByAmount(float Amount, AActor* Instigator)
{
	float OldValue = Health;
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(this, OldValue, Health, Instigator);
	
	if (Health <= 0.f)
	{
		StartDeath();
	}
	
	return false;
}

void UHealthComponent::StartDeath()
{
	if (DeathState != EDeathState::NotDead) return;
	
	DeathState = EDeathState::DeathStarted;
	OnDeathStarted.Broadcast();
	GetOwner()->ForceNetUpdate();
}

void UHealthComponent::ChangeMaxHealthByAmount(float Amount, AActor* Instigator)
{
	float OldValue = MaxHealth;
	MaxHealth = FMath::Clamp(MaxHealth + Amount, 0.0f, MAX_FLT);
	OnMaxHealthChanged.Broadcast(this, OldValue, MaxHealth, Instigator);
}


void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHealthComponent::OnRep_DeathState(EDeathState OldDeathState)
{
	if (DeathState == EDeathState::DeathStarted)
	{
		OnDeathStarted.Broadcast();
	}
}

void UHealthComponent::OnRep_Health(float OldHealth)
{
	OnHealthChanged.Broadcast(this, OldHealth, Health, nullptr);
}

void UHealthComponent::OnRep_MaxHealth(float OldMaxHealth)
{
	OnMaxHealthChanged.Broadcast(this, OldMaxHealth, MaxHealth, nullptr);
}