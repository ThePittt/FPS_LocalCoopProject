// Fill out your copyright notice in the Description page of Project Settings.


#include "GEII_FPS_Weapons.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "GEII_FPS_Projectiles.h"
#include "GEII_FPSGameCharacter.h"

// Sets default values
AGEII_FPS_Weapons::AGEII_FPS_Weapons()
{
	SetReplicates(true);
	SetReplicateMovement(true);

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Uscene"));
	SetRootComponent(SceneRootComponent);

	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonWeapon"));
	FirstPersonMesh->SetupAttachment(RootComponent);
	FirstPersonMesh->SetMobility(EComponentMobility::Movable);
	FirstPersonMesh->bEditableWhenInherited = true;

	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	FirstPersonMesh->bOnlyOwnerSee = true;


	ThirdPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonWeapon"));
	ThirdPersonMesh->SetupAttachment(RootComponent);
	ThirdPersonMesh->SetMobility(EComponentMobility::Movable);
	ThirdPersonMesh->bEditableWhenInherited = true;

	ThirdPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	ThirdPersonMesh->bOwnerNoSee = true;

	RefireRate = 0.25f;
	bIsFiring = false;
}

void AGEII_FPS_Weapons::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Register your replicated properties
	DOREPLIFETIME(AGEII_FPS_Weapons, MagazineSize);
	DOREPLIFETIME(AGEII_FPS_Weapons, CurrentBullets);
	DOREPLIFETIME(AGEII_FPS_Weapons, AmoReserve);
	DOREPLIFETIME(AGEII_FPS_Weapons, bIsFiring);
}

// Called when the game starts or when spawned
void AGEII_FPS_Weapons::BeginPlay()
{
	Super::BeginPlay();

	CurrentBullets = MagazineSize;
	AmoReserve = MagazineSize;

	WeaponOwner = Cast<AGEII_FPSGameCharacter>(GetOwner());

	WeaponOwner->AttachWeaponMeshes(this);
}

void AGEII_FPS_Weapons::ActivateWeapon_Implementation()
{
	SetActorHiddenInGame(false);

	WeaponOwner->OnWeaponActivated(this);
}

void AGEII_FPS_Weapons::DeactivateWeapon_Implementation()
{
	StopFiring();

	SetActorHiddenInGame(true);

	WeaponOwner->OnWeaponDeactivated(this);
}

void AGEII_FPS_Weapons::ServerFire_Implementation(FVector SpawnPlace, FRotator SpawnRotation)
{
	SpawnLocation = SpawnPlace;
	SpawnRotator = SpawnRotation;
	DoFire_Implementation();
}

void AGEII_FPS_Weapons::AddBulletsToReserve()
{
	AmoReserve += MagazineSize;
}

void AGEII_FPS_Weapons::StartFiring(FVector SpawnPlace, FRotator SpawnRotation)
{
	SpawnLocation = SpawnPlace;
	SpawnRotator = SpawnRotation;
	if (CurrentBullets > 0)
	{
		bIsFiring = true;
		const float TimeSinceLastShot = GetWorld()->GetTimeSeconds() - TimeOfLastShot;

		if (TimeSinceLastShot >= RefireRate && !bFullAuto)
		{
			// Do immediate local fire for responsiveness
			ServerFire_Implementation(SpawnPlace, SpawnRotation);
			WeaponOwner->PlayFireAnimation();

			// Also tell server to fire (for other clients to see)
			if (WeaponOwner && WeaponOwner->IsLocallyControlled())
			{
				ServerFire(SpawnPlace, SpawnRotation);
			}
			StopFiring();
			return;
		}
		if (TimeSinceLastShot >= RefireRate)
		{
			// Do immediate local fire for responsiveness
			ServerFire_Implementation(SpawnPlace, SpawnRotation);
			WeaponOwner->PlayFireAnimation();

			// Also tell server to fire (for other clients to see)
			if (WeaponOwner && WeaponOwner->IsLocallyControlled())
			{
				ServerFire(SpawnPlace, SpawnRotation);
			}
			return;
		}
	}
}


void AGEII_FPS_Weapons::StopFiring()
{
	bIsFiring = false;

	// clear the refire timer
	GetWorld()->GetTimerManager().ClearTimer(RefireTimer);
}

void AGEII_FPS_Weapons::ReloadMagazine()
{
	CurrentBullets = MagazineSize;
}

void AGEII_FPS_Weapons::DoFire_Implementation()
{
	if (IsShotgun)
	{
		for (int32 i = 0; i < PelletCount; i++)
		{
			// Update the time of last shot FIRST
			TimeOfLastShot = GetWorld()->GetTimeSeconds();

			// Decrease bullet count

			// Spawn the projectile
			FActorSpawnParameters spawnParameters;
			spawnParameters.Instigator = GetInstigator();
			spawnParameters.Owner = this;

			float RandomYaw = FMath::RandRange(-SpreadAngle, SpreadAngle);
			float RandomPitch = FMath::RandRange(-SpreadAngle, SpreadAngle);

			FRotator ShotgunPalletRotation = FRotator(SpawnRotator.Pitch + RandomPitch, SpawnRotator.Yaw + RandomYaw, SpawnRotator.Roll);

			FTransform ProjectileTransform = FTransform(ShotgunPalletRotation, SpawnLocation, GetActorScale());

			AGEII_FPS_Projectiles* spawnedProjectile = GetWorld()->SpawnActor<AGEII_FPS_Projectiles>(ProjectileClass, ProjectileTransform, spawnParameters);
		}
		CurrentBullets--;
		return;
	}
	else
	{
		// Keep all your existing code exactly as it was:
		TimeOfLastShot = GetWorld()->GetTimeSeconds();
		CurrentBullets--;

		FActorSpawnParameters spawnParameters;
		spawnParameters.Instigator = GetInstigator();
		spawnParameters.Owner = this;
		AGEII_FPS_Projectiles* spawnedProjectile = GetWorld()->SpawnActor<AGEII_FPS_Projectiles>(ProjectileClass, SpawnLocation, SpawnRotator, spawnParameters);
	}
	

}

void AGEII_FPS_Weapons::HandleFire_Implementation()
{
	if (!bIsFiring || CurrentBullets <= 0)
	{
		return;
	}

	DoFire();

	if (CurrentBullets <= 0)
	{
		// Stop firing if out of bullets
		StopFiring();
	}
}

bool AGEII_FPS_Weapons::ServerFire_Validate(FVector SpawnPlace, FRotator SpawnRotation)
{
	return WeaponOwner && WeaponOwner == GetOwner();
}




