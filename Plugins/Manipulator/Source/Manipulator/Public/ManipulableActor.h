// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Manipulable.h"
#include "ManipulableActor.generated.h"

UCLASS()
class MANIPULATOR_API AManipulableActor : public AActor, public IManipulable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AManipulableActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//
	// Components
	//
protected:
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* MeshComponent;
};
