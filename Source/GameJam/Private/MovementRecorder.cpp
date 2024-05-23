// Fill out your copyright notice in the Description page of Project Settings.


#include "GameJam/Public/MovementRecorder.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameJam/MyCharacter.h"

AMovementRecorder::AMovementRecorder()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsRecording = false;
    PlaybackIndex = 0;
    GhostCharacter = nullptr;
}

void AMovementRecorder::Tick(float DeltaTime)
{
    AActor::Tick(DeltaTime);

    if (bIsRecording)
    {
        APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
        if (PlayerController)
        {
            FInputRecord InputRecord;
            InputRecord.Timestamp = GetWorld()->GetTimeSeconds(); // Record timestamp

            UCharacterMovementComponent* CharacterMovement = PlayerController->GetCharacter()->GetCharacterMovement();
            if (CharacterMovement)
            {
                FVector MovementInput = CharacterMovement->GetCurrentAcceleration();
                if (MovementInput == FVector::Zero())
                    return;
                InputRecord.MovementDirection = MovementInput;
            }

            InputRecord.bIsJumping = PlayerController->GetCharacter()->bWasJumping;;

            SaveInputRecord(InputRecord);
        }
    }
}

AMovementRecorder::~AMovementRecorder()
{
}

void AMovementRecorder::BeginPlay()
{
    AActor::BeginPlay();
}

void AMovementRecorder::StartRecording()
{
    UE_LOG(LogTemp, Warning, TEXT("RECORDING STARTED"));
    //DestroyGhostActor();
    RecordedInputs.Empty();
    bIsRecording = true;
}

void AMovementRecorder::StopRecording()
{
    UE_LOG(LogTemp, Warning, TEXT("RECORDING STOPPED"));
    DestroyGhostActor();
    bIsRecording = false;
    SaveInputToText();

    PlaybackInputRecord();
}

void AMovementRecorder::DestroyGhostActor()
{
    if (GhostCharacter)
    {
        GhostCharacter->Destroy();
        GhostCharacter = nullptr;
    }
}

void AMovementRecorder::SaveInputRecord(const FInputRecord& InputRecord)
{
    RecordedInputs.Add(InputRecord);
}

void AMovementRecorder::SpawnGhostAtStartingPosition()
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController && GhostCharacterClass)
    {
        FVector SpawnLocation = Cast<AMyCharacter>(PlayerController->GetCharacter())->StartPos;;

        FTransform SpawnTransform(SpawnLocation);
        GhostCharacter = GetWorld()->SpawnActor<AMyCharacter>(GhostCharacterClass, SpawnTransform);
    }
}

void AMovementRecorder::PlaybackInputRecord()
{
    PlaybackInputs.Empty();
    PlaybackInputs.Append(RecordedInputs);
    RecordedInputs.Empty();
    if (PlaybackInputs.Num() > 0)
    {
        DestroyGhostActor();
        SpawnGhostAtStartingPosition();

        PlaybackIndex = 0;

        GetWorldTimerManager().SetTimer(PlaybackTimerHandle, this, &AMovementRecorder::ApplyNextInputRecord, 
            PlaybackDelay*GetWorld()->DeltaTimeSeconds, true, 1.f);
    }
}

void AMovementRecorder::ApplyNextInputRecord()
{
    if (PlaybackIndex < PlaybackInputs.Num() && GhostCharacter)
    {
        const FInputRecord& InputRecord = PlaybackInputs[PlaybackIndex];
        GhostCharacter->AddMovementInput(InputRecord.MovementDirection);

        if (!InputRecord.MovementDirection.IsNearlyZero())
        {
            FRotator NewRotation = InputRecord.MovementDirection.Rotation();
            GhostCharacter->SetActorRotation(NewRotation);
        }
        if (InputRecord.bIsJumping)
            GhostCharacter->Jump();

        //UE_LOG(LogTemp, Warning, TEXT("Timestamp: %f:  Movement applied : %d  %d"), InputRecord.Timestamp, (int)InputRecord.MovementDirection.X, (int)InputRecord.MovementDirection.Y);
        PlaybackIndex++;
        //all moves done
        if (PlaybackIndex >= PlaybackInputs.Num())
        {
            GetWorldTimerManager().ClearTimer(PlaybackTimerHandle);
            FVector NewLocation = GhostCharacter->GetActorLocation() +  GhostCharacter->GetActorForwardVector() * 50;
            GhostCharacter->SetActorLocation(NewLocation);
            DestroyGhostActor();
        }
    }
}


void AMovementRecorder::ResetPlayback()
{
    PlaybackIndex = 0;
}

void AMovementRecorder::SaveInputToText()
{
    FString RecordString;
    for (const FInputRecord& InputRecord : RecordedInputs)
    {
        RecordString += FString::Printf(TEXT("Timestamp: %f  MovementX: %d  MovementY: %d  isJumping: %d\n\n"),
            InputRecord.Timestamp,
            (int)InputRecord.MovementDirection.X,
            (int)InputRecord.MovementDirection.Y,
            InputRecord.bIsJumping ? 1 : 0);
    }

    // Save record string to text file
    FString FileName = FPaths::ProjectSavedDir() + TEXT("InputRecord.txt");
    FFileHelper::SaveStringToFile(RecordString, *FileName);
    UE_LOG(LogTemp, Warning, TEXT("saved to %s"), *FPaths::ProjectSavedDir());
}
