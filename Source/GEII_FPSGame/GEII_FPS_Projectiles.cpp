// Fill out your copyright notice in the Description page of Project Settings.


#include "GEII_FPS_Projectiles.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"


// Sets default values
AGEII_FPS_Projectiles::AGEII_FPS_Projectiles()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(true);

    // Definition for the SphereComponent that will serve as the Root component for the projectile and its collision.
    SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
    SphereComponent->InitSphereRadius(37.5f);
    SphereComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    RootComponent = SphereComponent;

    // FIXED: Register collision on ALL machines, not just authority
    // The authority check should be INSIDE the function, not here
    SphereComponent->OnComponentHit.AddDynamic(this, &AGEII_FPS_Projectiles::OnProjectileImpact);

    // Definition for the Mesh that will serve as your visual representation.
    static ConstructorHelpers::FObjectFinder<UStaticMesh>
        DefaultMesh(TEXT("/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere"));
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    StaticMesh->SetupAttachment(RootComponent);

    // Set the Static Mesh and its position/scale if you successfully found a mesh asset to use.
    if (DefaultMesh.Succeeded())
    {
        StaticMesh->SetStaticMesh(DefaultMesh.Object);
        StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -37.5f));
        StaticMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));
    }

    // FIXED: Set mesh collision to NoCollision so only the sphere handles collision
    StaticMesh->SetCollisionProfileName(TEXT("NoCollision"));

    static ConstructorHelpers::FObjectFinder<UParticleSystem>
        DefaultExplosionEffect(TEXT("/Game/StarterContent/Particles/P_Explosion.P_Explosion"));
    if (DefaultExplosionEffect.Succeeded())
    {
        ExplosionEffect = DefaultExplosionEffect.Object;
    }

    // Definition for the Projectile Movement Component.
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovementComponent->SetUpdatedComponent(SphereComponent);
    ProjectileMovementComponent->InitialSpeed = 1500.0f;
    ProjectileMovementComponent->MaxSpeed = 1500.0f;
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->ProjectileGravityScale = 0.0f;

    DamageType = UDamageType::StaticClass();
    Damage = 10.0f;
}

void AGEII_FPS_Projectiles::OnProjectileImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // FIXED: Only apply damage and destroy on server/authority
    if (HasAuthority())
    {
        if (OtherActor && OtherActor != this && OtherActor != GetOwner())
        {
            UGameplayStatics::ApplyPointDamage(OtherActor, Damage, Hit.Location, Hit, GetInstigator() ? GetInstigator()->Controller : nullptr, this, DamageType);
        }

        // Server destroys the projectile, which will replicate to clients
        Destroy();
    }
}


void AGEII_FPS_Projectiles::Destroyed()
{
    Super::Destroyed();

    // FIXED: Only spawn effects on clients (not on dedicated server)
    if (GetWorld() && !GetWorld()->IsNetMode(NM_DedicatedServer))
    {
        FVector spawnLocation = GetActorLocation();
        UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionEffect, spawnLocation,
            FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);
    }
}


