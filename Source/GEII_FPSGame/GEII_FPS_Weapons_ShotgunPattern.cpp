// Fill out your copyright notice in the Description page of Project Settings.


#include "GEII_FPS_Weapons_ShotgunPattern.h"
#include "GEII_FPS_Projectiles.h"
#include "GEII_FPS_Weapons.h"


void AGEII_FPS_Weapons_ShotgunPattern::DoFire()
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
}
