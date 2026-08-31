// Copyright Epic Games, Inc. All Rights Reserved.

#include "GEII_FPSGameCharacter.h"
#include "GEII_FPSGameProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "GEII_FPS_Projectiles.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GEII_FPS_Weapons.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AGEII_FPSGameCharacter

AGEII_FPSGameCharacter::AGEII_FPSGameCharacter()
{

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh2"));
	Mesh3P->SetOwnerNoSee(true);
	Mesh3P->SetupAttachment(FirstPersonCameraComponent);

	//Initialize the player's Health
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	//Initialize the player's Health
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;

	ProjectileClass = AGEII_FPSGameProjectile::StaticClass();
	FireRate = 0.25f;
	bIsFiringWeapon = false;
}

void AGEII_FPSGameCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGEII_FPSGameCharacter, CurrentHealth);
}


//////////////////////////////////////////////////////////////////////////// Input

void AGEII_FPSGameCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AGEII_FPSGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGEII_FPSGameCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGEII_FPSGameCharacter::Look);

		// Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AGEII_FPSGameCharacter::StartFire);

		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AGEII_FPSGameCharacter::Reload);


		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AGEII_FPSGameCharacter::DoSwitchWeapon);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AGEII_FPSGameCharacter::SetCurrentHealth(float healthValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}

float AGEII_FPSGameCharacter::TakeDamage(float DamageTaken, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float damageApplied = CurrentHealth - DamageTaken;
	SetCurrentHealth(damageApplied);
	return damageApplied;
}


void AGEII_FPSGameCharacter::AttachWeaponMeshes(AGEII_FPS_Weapons* Weapon)
{
	UAnimInstance* PlayerFPMeshAnim = GetMesh1P()->GetAnimInstance();
	if (PlayerFPMeshAnim)
	{
		// Method 1: Using FindFProperty (Recommended)
		FBoolProperty* BoolProperty = FindFProperty<FBoolProperty>(PlayerFPMeshAnim->GetClass(), TEXT("HasRifle"));
		if (BoolProperty)
		{
			BoolProperty->SetPropertyValue_InContainer(PlayerFPMeshAnim, true);
		}

	}
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
	Weapon->AttachToActor(this, AttachmentRule);

	Weapon->GetFirstPersonMesh()->AttachToComponent(GetMesh1P(), AttachmentRule, FName("GripPoint"));
	Weapon->GetThirdPersonMesh()->AttachToComponent(GetMesh(), AttachmentRule, FName("hand_r"));
	Weapon->GetThirdPersonMesh()->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
	Weapon->GetThirdPersonMesh()->SetRelativeLocation(FVector(-7.0f, 0.f, 0.f));
}

void AGEII_FPSGameCharacter::AddWeaponClass(const TSubclassOf<AGEII_FPS_Weapons>& WeaponClass)
{
	AGEII_FPS_Weapons* OwnedWeapon = FindWeaponOfType(WeaponClass);

	if (!OwnedWeapon)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

		AGEII_FPS_Weapons* AddedWeapon = GetWorld()->SpawnActor<AGEII_FPS_Weapons>(WeaponClass, GetActorTransform(), SpawnParams);

		AttachWeaponMeshes(AddedWeapon);

		if (AddedWeapon)
		{
			OwnedWeapons.Add(AddedWeapon);
			if (CurrentWeapon)
			{
				CurrentWeapon->DeactivateWeapon();
			}
			CurrentWeapon = AddedWeapon;
			CurrentWeapon->ActivateWeapon();
		}
	}
	if (OwnedWeapon)
	{
		CurrentWeapon->AddBulletsToReserve();
	}
}

void AGEII_FPSGameCharacter::OnWeaponActivated_Implementation(AGEII_FPS_Weapons* Weapon)
{
	if (GetWorld()->GetTimerManager().IsTimerActive(TimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		UAnimInstance* AnimInstance = GetMesh1P()->GetAnimInstance();
		AnimInstance->StopAllMontages(.4f);
		IsReloading = false;
	}
}

void AGEII_FPSGameCharacter::OnWeaponDeactivated(AGEII_FPS_Weapons* Weapon)
{
}

void AGEII_FPSGameCharacter::StartFire_Implementation()
{
	if (!IsReloading)
	{
		FVector spawnLocation = GetActorLocation() + (GetControlRotation().Vector() * 100.f) + (GetActorUpVector() * 60.f);
		FRotator spawnRotation = GetControlRotation();

		if (CurrentWeapon)
		{
			CurrentWeapon->StartFiring(spawnLocation,spawnRotation);
		}
	}
	
	
}

void AGEII_FPSGameCharacter::PlayFireAnimation()
{
	UAnimInstance* PlayerFPMeshAnim = GetMesh1P()->GetAnimInstance();
	PlayerFPMeshAnim->Montage_Play(FireMontage);

	UAnimInstance* PlayerFPMeshAnim2 = GetMesh()->GetAnimInstance();
	PlayerFPMeshAnim2->Montage_Play(FireMontage3P);
}

void AGEII_FPSGameCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFiring();
	}
}

void AGEII_FPSGameCharacter::MulticastSwitchWeapon_Implementation(int32 WeaponIndex)
{
	CurrentWeapon->DeactivateWeapon();
	CurrentWeapon = OwnedWeapons[WeaponIndex];
	CurrentWeapon->ActivateWeapon();
}

void AGEII_FPSGameCharacter::DoSwitchWeapon_Implementation()
{
	// ensure we have at least two weapons two switch between
	if (OwnedWeapons.Num() > 1)
	{
		// deactivate the old weapon
		CurrentWeapon->DeactivateWeapon();

		// find the index of the current weapon in the owned list
		int32 WeaponIndex = OwnedWeapons.Find(CurrentWeapon);

		// is this the last weapon?
		if (WeaponIndex == OwnedWeapons.Num() - 1)
		{
			// loop back to the beginning of the array
			WeaponIndex = 0;
		}
		else {
			// select the next weapon index
			++WeaponIndex;
		}

		//CurrentWeapon = OwnedWeapons[WeaponIndex];
		//CurrentWeapon->ActivateWeapon();
		MulticastSwitchWeapon(WeaponIndex);
	}
}




void AGEII_FPSGameCharacter::HandleFire_Implementation()
{
	UAnimInstance* PlayerFPMeshAnim = GetMesh1P()->GetAnimInstance();
	PlayerFPMeshAnim->Montage_Play(FireMontage);
}

void AGEII_FPSGameCharacter::Reload()
{
	if (!CurrentWeapon)
	{
		return;
	}

	int32 BulletAmount = CurrentWeapon->GetBulletCount();
	int32 MagSize = CurrentWeapon->GetMagazineSize();

	if (BulletAmount == MagSize)
	{
		return;
	}

	if (CurrentWeapon)
	{
		CurrentWeapon->StopFiring();
	}

	IsReloading = true;

	UAnimInstance* PlayerFPMeshAnim = GetMesh1P()->GetAnimInstance();
	float AnimationLength = PlayerFPMeshAnim->Montage_Play(ReloadMontage);

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			CurrentWeapon->ReloadMagazine();
			IsReloading = false;
		}, AnimationLength, false);
}

void AGEII_FPSGameCharacter::OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	CurrentWeapon->ReloadMagazine();
}

void AGEII_FPSGameCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AGEII_FPSGameCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AGEII_FPSGameCharacter::OnHealthUpdate()
{
	//Client-specific functionality
	if (IsLocallyControlled())
	{
		FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."),
			CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
		if (CurrentHealth <= 0)
		{
			FString deathMessage = FString::Printf(TEXT("You have been killed."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
		}
	}
	//Server-specific functionality
	if (GetLocalRole() == ROLE_Authority)
	{
		FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."),
			*GetFName().ToString(), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	}
	//Functions that occur on all machines.
	/*
	Any special functionality that should occur as a result of damage or death should
	be placed here.
	*/
}

void AGEII_FPSGameCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}