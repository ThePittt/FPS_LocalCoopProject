// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GEII_FPS_Weapons.h"
#include "GEII_FPS_Weapons_ShotgunPattern.generated.h"

/**
 * 
 */
UCLASS()
class GEII_FPSGAME_API AGEII_FPS_Weapons_ShotgunPattern : public AGEII_FPS_Weapons
{
	GENERATED_BODY()

protected:
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	//int32 PelletCount = 8; // Number of projectiles per shot

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	//float SpreadAngle = 15.0f; // Half-angle of the spread cone in degrees

public:
	
	virtual void DoFire() override;
};
