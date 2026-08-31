// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "GEII_FPS_WeaponPickUp.generated.h"

class USphereComponent;
class UPrimitiveComponent;
class AGEII_FPS_Weapons;

UCLASS()
class GEII_FPSGAME_API AGEII_FPS_WeaponPickUp : public AActor
{
	GENERATED_BODY()

	/** Weapon class to grant on pickup */
	UPROPERTY(EditAnywhere)
	TSubclassOf<AGEII_FPS_Weapons> WeaponClass;

protected:


	/** Mesh to display on the pickup */
	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* ColisionSphere;

	/** Time to wait before respawning this pickup */
	UPROPERTY(EditAnywhere, Category = "Pickup")
	float RespawnTime = 4.0f;

	/** Timer to respawn the pickup */
	FTimerHandle RespawnTimer;


public:	
	// Sets default values for this actor's properties
	AGEII_FPS_WeaponPickUp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


};
