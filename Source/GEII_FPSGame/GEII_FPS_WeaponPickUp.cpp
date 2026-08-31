// Fill out your copyright notice in the Description page of Project Settings.


#include "GEII_FPS_WeaponPickUp.h"
#include "Components/SkeletalMeshComponent.h"
#include "GEII_FPSGameCharacter.h"
#include "Components/SphereComponent.h"

// Sets default values
AGEII_FPS_WeaponPickUp::AGEII_FPS_WeaponPickUp()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	ColisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SetRootComponent(ColisionSphere);

	StaticMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponComponent"));
	StaticMesh->SetupAttachment(this->GetRootComponent());

	ColisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AGEII_FPS_WeaponPickUp::OnSphereBeginOverlap);
}

// Called when the game starts or when spawned
void AGEII_FPS_WeaponPickUp::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGEII_FPS_WeaponPickUp::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if(AGEII_FPSGameCharacter* PlayerCharacter = Cast<AGEII_FPSGameCharacter>(OtherActor))
	{
		FString healthMessage = FString::Printf(TEXT(" now has health remaining."));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, healthMessage);
		PlayerCharacter->AddWeaponClass(WeaponClass);
	}
}


