// Copyright Epic Games, Inc. All Rights Reserved.

#include "GEII_FPSGamePickUpComponent.h"

UGEII_FPSGamePickUpComponent::UGEII_FPSGamePickUpComponent()
{
	// Setup the Sphere Collision
	SphereRadius = 32.f;
}

void UGEII_FPSGamePickUpComponent::BeginPlay()
{
	Super::BeginPlay();

	// Register our Overlap Event
	OnComponentBeginOverlap.AddDynamic(this, &UGEII_FPSGamePickUpComponent::OnSphereBeginOverlap);
}

void UGEII_FPSGamePickUpComponent::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Checking if it is a First Person Character overlapping
	AGEII_FPSGameCharacter* Character = Cast<AGEII_FPSGameCharacter>(OtherActor);
	if(Character != nullptr)
	{
		// Notify that the actor is being picked up
		OnPickUp.Broadcast(Character);

		// Unregister from the Overlap Event so it is no longer triggered
		OnComponentBeginOverlap.RemoveAll(this);
	}
}
