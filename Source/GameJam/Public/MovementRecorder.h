// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovementRecorder.generated.h" 
class AMyCharacter;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FInputRecord
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    float Timestamp;

    UPROPERTY(BlueprintReadWrite)
    FVector MovementDirection=FVector::Zero();
    UPROPERTY(BlueprintReadWrite)
    bool bIsJumping=0;
};

UCLASS(config = Game)
class GAMEJAM_API AMovementRecorder : public AActor
{

    GENERATED_BODY()
public:
	AMovementRecorder();
	~AMovementRecorder();

    bool bIsRecording;

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    void StartRecording();
    void StopRecording();

    void DestroyGhostActor();

    void SaveInputRecord(const FInputRecord& InputRecord);
    void SpawnGhostAtStartingPosition();

    void PlaybackInputRecord();
    void ApplyNextInputRecord();
    void ResetPlayback();

    void SaveInputToText();

    TSubclassOf<AMyCharacter> GhostCharacterClass;

private:
    TArray<FInputRecord> RecordedInputs;
    TArray<FInputRecord> PlaybackInputs;

    int32 PlaybackIndex;

    AMyCharacter* GhostCharacter;
    FTimerHandle PlaybackTimerHandle;
    float PlaybackDelay=1.f;

    FTimerHandle ForwardMovementTimerHandle; 
};
