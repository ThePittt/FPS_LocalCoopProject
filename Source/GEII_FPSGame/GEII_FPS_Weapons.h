// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GEII_FPS_Weapons.generated.h"

class AGEII_FPSGameCharacter;

UCLASS()
class GEII_FPSGAME_API AGEII_FPS_Weapons : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* SceneRootComponent;

	/** First person perspective mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** Third person perspective mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ThirdPersonMesh;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AGEII_FPS_Projectiles> ProjectileClass;

	AGEII_FPSGameCharacter* WeaponOwner;

	/** Number of bullets in a magazine */
	UPROPERTY(EditAnywhere, Category = "Ammo", Replicated)
	int32 MagazineSize = 10;

	/** Number of bullets in the current magazine */
	UPROPERTY(EditAnywhere, Category = "Ammo", Replicated)
	int32 CurrentBullets = 0;

	/** Number of bullets in the current magazine */
	UPROPERTY(EditAnywhere, Category = "Ammo", Replicated)
	int32 AmoReserve = 10;

	/** Amount of firing recoil to apply to the owner */
	UPROPERTY(EditAnywhere, Category = "Aim")
	float FiringRecoil = 0.0f;

	/** If true, this weapon will automatically fire at the refire rate */
	UPROPERTY(EditAnywhere, Category = "Refire")
	bool bFullAuto = false;

	/** Time between shots for this weapon. Affects both full auto and semi auto modes */
	UPROPERTY(EditAnywhere, Category = "Refire")
	float RefireRate = 0.5f;

	/** Game time of last shot fired, used to enforce refire rate on semi auto */
	float TimeOfLastShot = 0.f;

	UPROPERTY(Replicated)
	bool bIsFiring = false;

	/** Timer to handle full auto refiring */
	FTimerHandle RefireTimer;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	FVector SpawnLocation;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	FRotator SpawnRotator;

public:	
	// Sets default values for this actor's properties
	AGEII_FPS_Weapons();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(NetMulticast, Reliable)
	void ActivateWeapon();

	UFUNCTION(NetMulticast, Reliable)
	void DeactivateWeapon();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(FVector SpawnPlace, FRotator SpawnRotation);

	void AddBulletsToReserve();

	/** Start firing this weapon */
	void StartFiring(FVector SpawnPlace, FRotator SpawnRotation);

	/** Stop firing this weapon */
	void StopFiring();

	void ReloadMagazine();
	
private:

	UFUNCTION(NetMulticast, Reliable)
	virtual void DoFire();

	/** Server function for spawning projectiles.*/
	UFUNCTION(NetMulticast, Reliable)
	void HandleFire();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 PelletCount = 8; // Number of projectiles per shot

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float SpreadAngle = 15.0f; // Half-angle of the spread cone in degrees
	
	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool IsShotgun = false;

public:

	UFUNCTION(BlueprintPure, Category = "Weapon")
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; };

	UFUNCTION(BlueprintPure, Category = "Weapon")
	USkeletalMeshComponent* GetThirdPersonMesh() const { return ThirdPersonMesh; };

	int32 GetMagazineSize() const { return MagazineSize; };

	/** Returns the current bullet count */
	int32 GetBulletCount() const { return CurrentBullets; };

};
