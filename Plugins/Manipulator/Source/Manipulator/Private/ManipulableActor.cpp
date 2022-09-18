// Fill out your copyright notice in the Description page of Project Settings.


#include "ManipulableActor.h"

#include "Components/StaticMeshComponent.h"

// Sets default values
AManipulableActor::AManipulableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName(TEXT("Mesh")));
	SetRootComponent(MeshComponent);
}

// Called when the game starts or when spawned
void AManipulableActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AManipulableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

