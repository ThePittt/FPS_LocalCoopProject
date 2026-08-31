// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "GEII_FPS_Weapons.h"
#include "GEII_FPSGameCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class AGEII_FPS_Weapons;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AGEII_FPSGameCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh3P;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess= "true"))
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SwitchWeaponAction;

	bool IsReloading;

	UPROPERTY(EditAnywhere, Category = "Weapons")
	FTimerHandle TimerHandle;

	UPROPERTY(EditAnywhere, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* FireMontage3P;

	UPROPERTY(EditAnywhere, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* ReloadMontage;



	
public:
	AGEII_FPSGameCharacter();

	/** Property replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


public:

	void AttachWeaponMeshes(AGEII_FPS_Weapons* Weapon);

	void AddWeaponClass(const TSubclassOf<AGEII_FPS_Weapons>& WeaponClass);

	/** Activates the passed weapon */
	UFUNCTION(NetMulticast, Reliable)
	void OnWeaponActivated(AGEII_FPS_Weapons* Weapon);

	/** Deactivates the passed weapon */
	void OnWeaponDeactivated(AGEII_FPS_Weapons* Weapon);
	
protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Response to health being updated. Called on the server immediately after
	modification, and on clients in response to a RepNotify*/
	void OnHealthUpdate();

	void Reload();

	UFUNCTION()
	void OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted);


protected:

	/** The player's maximum health. This is the highest value of their health can be.
	This value is a value of the player's health, which starts at when spawned.*/
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHealth;

	/** The player's current health. When reduced to 0, they are considered dead.*/
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	/** RepNotify for changes made to current health.*/
	UFUNCTION()
	void OnRep_CurrentHealth();

	TArray<AGEII_FPS_Weapons*> OwnedWeapons;

	TObjectPtr<AGEII_FPS_Weapons> CurrentWeapon;

protected:

	/** Returns true if the character already owns a weapon of the given class */
	AGEII_FPS_Weapons* FindWeaponOfType(TSubclassOf<AGEII_FPS_Weapons> WeaponClass) const
	{
		// check each owned weapon
		for (AGEII_FPS_Weapons* Weapon : OwnedWeapons)
		{
			if (Weapon->IsA(WeaponClass))
			{
				return Weapon;
			}
		}
		//weapon not found
		return nullptr;
	}

protected:
	// APawn interface
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

	USkeletalMeshComponent* GetMesh3p() const { return Mesh3P; }

	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }


	/** Getter for Max Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	/** Getter for Current Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

	/** Setter for Current Health. Clamps the value between 0 and MaxHealth and calls
	OnHealthUpdate. Should only be called on the server.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetCurrentHealth(float healthValue);

	/** Event for taking damage. Overridden from APawn.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void PlayFireAnimation();

protected:


	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Projectile")
	TSubclassOf<class AGEII_FPS_Projectiles> ProjectileClass;

	/** Delay between shots in seconds. Used to control fire rate for your test
	projectile, but also to prevent an overflow of server functions from binding
	SpawnProjectile directly to input.*/
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	float FireRate;

	/** If true, you are in the process of firing projectiles. */
	bool bIsFiringWeapon;

	/** Function for beginning weapon fire.*/
	UFUNCTION(NetMulticast, Reliable)
	void StartFire();

	/** Function for ending weapon fire. Once this is called, the player can use
	StartFire again.*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void StopFire();

	/** Server function for spawning projectiles.*/
	UFUNCTION(Server, Reliable)
	void HandleFire();

	UFUNCTION(Server, Reliable)
	void DoSwitchWeapon();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSwitchWeapon(int32 WeaponIndex);


	/** A timer handle used for providing the fire rate delay in-between spawns.*/
	FTimerHandle FiringTimer;


};

