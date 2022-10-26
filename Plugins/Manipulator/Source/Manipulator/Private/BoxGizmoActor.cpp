// Fill out your copyright notice in the Description page of Project Settings.


#include "BoxGizmoActor.h"

#include "Components/SphereComponent.h"

#include "PrimitiveGizmoRectComponent.h"
#include "PrimitiveGizmoCircleComponent.h"
#include "PrimitiveGizmoArrowComponent.h"
#include "PrimitiveGizmoRotateComponent.h"

ABoxGizmoActor::ABoxGizmoActor()
{
    // Root
    USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("GizmoCenter"));
    SphereComponent->InitSphereRadius(1.0f);
    SphereComponent->SetVisibility(false);
    SphereComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
    SetRootComponent(SphereComponent);

    //
    // Bounds Group
    //
    BoundsGroupComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GizmoBoundsGroup"));
    BoundsGroupComponent->SetupAttachment(GetRootComponent());

    const FLinearColor GizmoBoundsColor(0.0f, 0.2f, 0.2f);
    const float GizmoBoundsThickness = 10.0f;
    const float GizmoBoundsElevationRadius = 10.0f;
    const float GizmoBoundsElevationOffset = GizmoBoundsElevationRadius + GizmoBoundsThickness;

    // Elevation
    UPrimitiveGizmoCircleComponent* TempElevationComponent = CreateDefaultSubobject<UPrimitiveGizmoCircleComponent>(TEXT("GizmoElevation"));
    TempElevationComponent->Color = GizmoBoundsColor;
    TempElevationComponent->SetRadius(GizmoBoundsElevationRadius);
    TempElevationComponent->SetAudoScaleRadius(true);
    TempElevationComponent->SetCenterOffset(FVector(0.0f, 0.0f, GizmoBoundsElevationOffset));
    TempElevationComponent->SetThickness(GizmoBoundsThickness);
    TempElevationComponent->SetViewAligned(true);
    TempElevationComponent->SetupAttachment(BoundsGroupComponent);
    ElevationComponent = TempElevationComponent;

    // Plane top left
    UPrimitiveGizmoRectComponent* TempPlanTopLeftComponent = CreateDefaultSubobject<UPrimitiveGizmoRectComponent>(TEXT("GizmoPlanTopLeft"));
    TempPlanTopLeftComponent->Color = GizmoBoundsColor;
    TempPlanTopLeftComponent->SetOffsetX(TempPlanTopLeftComponent->GetLengthX() * -0.5f);
    TempPlanTopLeftComponent->SetOffsetY(TempPlanTopLeftComponent->GetLengthY() * -0.5f);
    TempPlanTopLeftComponent->SetThickness(GizmoBoundsThickness);
    TempPlanTopLeftComponent->SetSegmentFlags(0x1 | 0x8);
    TempPlanTopLeftComponent->SetupAttachment(BoundsGroupComponent);
    PlanTopLeftComponent = TempPlanTopLeftComponent;

    // Plane top right
    UPrimitiveGizmoRectComponent* TempPlanTopRightComponent = CreateDefaultSubobject<UPrimitiveGizmoRectComponent>(TEXT("GizmoPlanTopRight"));
    TempPlanTopRightComponent->Color = GizmoBoundsColor;
    TempPlanTopRightComponent->SetOffsetX(TempPlanTopRightComponent->GetLengthX() * -0.5f);
    TempPlanTopRightComponent->SetOffsetY(TempPlanTopRightComponent->GetLengthY() * -0.5f);
    TempPlanTopRightComponent->SetThickness(GizmoBoundsThickness);
    TempPlanTopRightComponent->SetSegmentFlags(0x1 | 0x2);
    TempPlanTopRightComponent->SetupAttachment(BoundsGroupComponent);
    PlanTopRightComponent = TempPlanTopRightComponent;

    // Plane bottom right
    UPrimitiveGizmoRectComponent* TempPlanBottomRightComponent = CreateDefaultSubobject<UPrimitiveGizmoRectComponent>(TEXT("GizmoPlanBottomRight"));
    TempPlanBottomRightComponent->Color = GizmoBoundsColor;
    TempPlanBottomRightComponent->SetOffsetX(TempPlanBottomRightComponent->GetLengthX() * -0.5f);
    TempPlanBottomRightComponent->SetOffsetY(TempPlanBottomRightComponent->GetLengthY() * -0.5f);
    TempPlanBottomRightComponent->SetThickness(GizmoBoundsThickness);
    TempPlanBottomRightComponent->SetSegmentFlags(0x2 | 0x4);
    TempPlanBottomRightComponent->SetupAttachment(BoundsGroupComponent);
    PlanBottomRightComponent = TempPlanBottomRightComponent;

    // Plane bottom left
    UPrimitiveGizmoRectComponent* TempPlanBottomLeftComponent = CreateDefaultSubobject<UPrimitiveGizmoRectComponent>(TEXT("GizmoPlanBottomLeft"));
    TempPlanBottomLeftComponent->Color = GizmoBoundsColor;
    TempPlanBottomLeftComponent->SetOffsetX(TempPlanBottomLeftComponent->GetLengthX() * -0.5f);
    TempPlanBottomLeftComponent->SetOffsetY(TempPlanBottomLeftComponent->GetLengthY() * -0.5f);
    TempPlanBottomLeftComponent->SetThickness(GizmoBoundsThickness);
    TempPlanBottomLeftComponent->SetSegmentFlags(0x4 | 0x8);
    TempPlanBottomLeftComponent->SetupAttachment(BoundsGroupComponent);
    PlanBottomLeftComponent = TempPlanBottomLeftComponent;

    //
    // Rotation Group
    //
    const float GizmoRotationThickness = GizmoBoundsThickness * 0.8f;
    const float GizmoRotationRadius = 30.0f;
    const float GizmoRotationResolution = 5.0f;
    const float GizmoRotationOffset = GizmoRotationRadius + GizmoRotationThickness;

    RotationGroupComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GizmoRotationGroup"));
    RotationGroupComponent->SetupAttachment(GetRootComponent());

    //
    // Axis X Rotate
    //
    const FLinearColor GizmoRotateXColor = FLinearColor(1.0f, 0.1f, 0.1f);
    const FVector GizmoRotateXFrontNormal = FVector(1.0f, 0.0f, 0.0f);
    const FVector GizmoRotateXBackNormal = FVector(-1.0f, 0.0f, 0.0f);

    // Axis X Rotate Front Socket
    RotateXFrontSocketComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GizmoRotateXFrontSocket"));
    RotateXFrontSocketComponent->SetupAttachment(RotationGroupComponent);

    // Axis X Rotate Front Indicator
    UPrimitiveGizmoCircleComponent* TempRotateXFrontIndicatorComponent = CreateDefaultSubobject<UPrimitiveGizmoCircleComponent>(TEXT("GizmoRotateXFrontIndicator"));
    TempRotateXFrontIndicatorComponent->Color = GizmoRotateXColor;
    TempRotateXFrontIndicatorComponent->SetNormal(GizmoRotateXFrontNormal);
    TempRotateXFrontIndicatorComponent->SetRadius(GizmoRotationRadius);
    TempRotateXFrontIndicatorComponent->SetAudoScaleRadius(true);
    TempRotateXFrontIndicatorComponent->SetResolution(GizmoRotationResolution);
    TempRotateXFrontIndicatorComponent->SetStartAngle(120.0f);
    TempRotateXFrontIndicatorComponent->SetEndAngle(240.0f);
    TempRotateXFrontIndicatorComponent->SetCullFace(true);
    TempRotateXFrontIndicatorComponent->SetCenterOffset(FVector(0.0f, 0.0f, GizmoRotationOffset));
    TempRotateXFrontIndicatorComponent->SetThickness(GizmoRotationThickness);
    TempRotateXFrontIndicatorComponent->SetupAttachment(RotateXFrontSocketComponent);
    RotateXFrontIndicatorComponent = TempRotateXFrontIndicatorComponent;

    // Axis X Rotate Front Dial
    UPrimitiveGizmoCircleComponent* TempRotateXFrontDialComponent = CreateDefaultSubobject<UPrimitiveGizmoCircleComponent>(TEXT("GizmoRotateXFrontDial"));
    TempRotateXFrontDialComponent->Color = GizmoRotateXColor;
    TempRotateXFrontDialComponent->SetNormal(GizmoRotateXFrontNormal);
    TempRotateXFrontDialComponent->SetRadius(GizmoRotationRadius);
    TempRotateXFrontDialComponent->SetAudoScaleRadius(false);
    TempRotateXFrontDialComponent->SetResolution(GizmoRotationResolution);
    TempRotateXFrontDialComponent->SetCullFace(true);
    TempRotateXFrontDialComponent->SetThickness(GizmoRotationThickness);
    TempRotateXFrontDialComponent->SetupAttachment(RotateXFrontSocketComponent);
    TempRotateXFrontDialComponent->SetVisibility(false);
    RotateXFrontDialComponent = TempRotateXFrontDialComponent;

    // Axis X Rotate Back Socket
    RotateXBackSocketComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GizmoRotateXBackSocket"));
    RotateXBackSocketComponent->SetupAttachment(RotationGroupComponent);

    // Axis X Rotate Back Indicator
    UPrimitiveGizmoCircleComponent* TempRotateXBackIndicatorComponent = CreateDefaultSubobject<UPrimitiveGizmoCircleComponent>(TEXT("GizmoRotateXBackIndicator"));
    TempRotateXBackIndicatorComponent->Color = GizmoRotateXColor;
    TempRotateXBackIndicatorComponent->SetNormal(GizmoRotateXBackNormal);
    TempRotateXBackIndicatorComponent->SetRadius(GizmoRotationRadius);
    TempRotateXBackIndicatorComponent->SetAudoScaleRadius(true);
    TempRotateXBackIndicatorComponent->SetResolution(GizmoRotationResolution);
    TempRotateXBackIndicatorComponent->SetStartAngle(-60.0f);
    TempRotateXBackIndicatorComponent->SetEndAngle(60.0f);
    TempRotateXBackIndicatorComponent->SetCullFace(true);
    TempRotateXBackIndicatorComponent->SetCenterOffset(FVector(0.0f, 0.0f, GizmoRotationOffset));
    TempRotateXBackIndicatorComponent->SetThickness(GizmoRotationThickness);
    TempRotateXBackIndicatorComponent->SetupAttachment(RotateXBackSocketComponent);
    RotateXBackIndicatorComponent = TempRotateXBackIndicatorComponent;

    // Axis X Rotate Back Dial
    UPrimitiveGizmoCircleComponent* TempRotateXBackDialComponent = CreateDefaultSubobject<UPrimitiveGizmoCircleComponent>(TEXT("GizmoRotateXBackDial"));
    TempRotateXBackDialComponent->Color = GizmoRotateXColor;
    TempRotateXBackDialComponent->SetNormal(GizmoRotateXBackNormal);
    TempRotateXBackDialComponent->SetRadius(GizmoRotationRadius);
    TempRotateXBackDialComponent->SetAudoScaleRadius(false);
    TempRotateXBackDialComponent->SetResolution(GizmoRotationResolution);
    TempRotateXBackDialComponent->SetCullFace(true);
    TempRotateXBackDialComponent->SetThickness(GizmoRotationThickness);
    TempRotateXBackDialComponent->SetupAttachment(RotateXBackSocketComponent);
    TempRotateXBackDialComponent->SetVisibility(false);
    RotateXBackDialComponent = TempRotateXBackDialComponent;

    //
    // Axis Y Rotate
    //
    const FLinearColor GizmoRotateYColor = FLinearColor(0.1f, 1.0f, 0.1f);
    const FVector GizmoRotateYFrontNormal = FVector(0.0f, 1.0f, 0.0f);
    const FVector GizmoRotateYBackNormal = FVector(0.0f, -1.0f, 0.0f);

    // Axis Y Rotate Front Socket
    RotateYFrontSocketComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GizmoRotateYFrontSocket"));
    RotateYFrontSocketComponent->SetupAttachment(RotationGroupComponent);

    // Axis Y Rotate Front Indicator
    UPrimitiveGizmoCircleComponent* TempRotateYFrontIndicatorComponent = CreateDefaultSubobject<UPrimitiveGizmoCircleComponent>(TEXT("GizmoRotateYFrontIndicator"));
    TempRotateYFrontIndicatorComponent->Color = GizmoRotateYColor;
    TempRotateYFrontIndicatorComponent->SetNormal(GizmoRotateYFrontNormal);
    TempRotateYFrontIndicatorComponent->SetRadius(GizmoRotationRadius);
    TempRotateYFrontIndicatorComponent->SetAudoScaleRadius(true);
    TempRotateYFrontIndicatorComponent->SetResolution(GizmoRotationResolution);
    TempRotateYFrontIndicatorComponent->SetStartAngle(210.0f);
    TempRotateYFrontIndicatorComponent->SetEndAngle(330.0f);
    TempRotateYFrontIndicatorComponent->SetCullFace(true);
    TempRotateYFrontIndicatorComponent->SetCenterOffset(FVector(0.0f, 0.0f, GizmoRotationOffset));
    TempRotateYFrontIndicatorComponent->SetThickness(GizmoRotationThickness);
    TempRotateYFrontIndicatorComponent->SetupAttachment(RotateYFrontSocketComponent);
    RotateYFrontIndicatorComponent = TempRotateYFrontIndicatorComponent;

    // Axis Y Rotate Front Dial
    UPrimitiveGizmoCircleComponent* TempRotateYFrontDialComponent = CreateDefaultSubobject<UPrimitiveGizmoCircleComponent>(TEXT("GizmoRotateYFrontDial"));
    TempRotateYFrontDialComponent->Color = GizmoRotateYColor;
    TempRotateYFrontDialComponent->SetNormal(GizmoRotateYFrontNormal);
    TempRotateYFrontDialComponent->SetRadius(GizmoRotationRadius);
    TempRotateYFrontDialComponent->SetAudoScaleRadius(false);
    TempRotateYFrontDialComponent->SetResolution(GizmoRotationResolution);
    TempRotateYFrontDialComponent->SetCullFace(true);
    TempRotateYFrontDialComponent->SetThickness(GizmoRotationThickness);
    TempRotateYFrontDialComponent->SetupAttachment(RotateYFrontSocketComponent);
    TempRotateYFrontDialComponent->SetVisibility(false);
    RotateYFrontDialComponent = TempRotateYFrontDialComponent;

    // Axis Y Rotate Back Socket
    RotateYBackSocketComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GizmoRotateYBackSocket"));
    RotateYBackSocketComponent->SetupAttachment(RotationGroupComponent);

    // Axis Y Rotate Back Indicator
    UPrimitiveGizmoCircleComponent* TempRotateYBackIndicatorComponent = CreateDefaultSubobject<UPrimitiveGizmoCircleComponent>(TEXT("GizmoRotateYBackIndicator"));
    TempRotateYBackIndicatorComponent->Color = GizmoRotateYColor;
    TempRotateYBackIndicatorComponent->SetNormal(GizmoRotateYBackNormal);
    TempRotateYBackIndicatorComponent->SetRadius(GizmoRotationRadius);
    TempRotateYBackIndicatorComponent->SetAudoScaleRadius(true);
    TempRotateYBackIndicatorComponent->SetResolution(GizmoRotationResolution);
    TempRotateYBackIndicatorComponent->SetStartAngle(30.0f);
    TempRotateYBackIndicatorComponent->SetEndAngle(150.0f);
    TempRotateYBackIndicatorComponent->SetCullFace(true);
    TempRotateYBackIndicatorComponent->SetCenterOffset(FVector(0.0f, 0.0f, GizmoRotationOffset));
    TempRotateYBackIndicatorComponent->SetThickness(GizmoRotationThickness);
    TempRotateYBackIndicatorComponent->SetupAttachment(RotateYBackSocketComponent);
    RotateYBackIndicatorComponent = TempRotateYBackIndicatorComponent;

    // Axis Y Rotate Back Dial
    UPrimitiveGizmoCircleComponent* TempRotateYBackDialComponent = CreateDefaultSubobject<UPrimitiveGizmoCircleComponent>(TEXT("GizmoRotateYBackDial"));
    TempRotateYBackDialComponent->Color = GizmoRotateYColor;
    TempRotateYBackDialComponent->SetNormal(GizmoRotateYBackNormal);
    TempRotateYBackDialComponent->SetRadius(GizmoRotationRadius);
    TempRotateYBackDialComponent->SetAudoScaleRadius(false);
    TempRotateYBackDialComponent->SetResolution(GizmoRotationResolution);
    TempRotateYBackDialComponent->SetCullFace(true);
    TempRotateYBackDialComponent->SetThickness(GizmoRotationThickness);
    TempRotateYBackDialComponent->SetupAttachment(RotateYBackSocketComponent);
    TempRotateYBackDialComponent->SetVisibility(false);
    RotateYBackDialComponent = TempRotateYBackDialComponent;

    // Rotation Proxy
    RotationProxyComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GizmoRotationProxy"));
    RotationProxyComponent->SetupAttachment(RotationGroupComponent);

    //
    // Translation Group
    //
    TranslationGroupComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GizmoTranslationGroup"));
    TranslationGroupComponent->SetupAttachment(GetRootComponent());

    const FLinearColor GizmoTranslationColor = FLinearColor(0.0f, 0.3f, 0.4f);
    const float GizmoTranslationThickness = GizmoBoundsThickness * 1.0f;

    // Translate Z
    UPrimitiveGizmoArrowComponent* TempTranslateZComponent = CreateDefaultSubobject<UPrimitiveGizmoArrowComponent>(TEXT("GizmoTranslateZ"));
    TempTranslateZComponent->Color = GizmoTranslationColor;
    TempTranslateZComponent->SetDirection(FVector(0.0f, 0.0f, 1.0f));
    TempTranslateZComponent->SetGap(60.0f);
    TempTranslateZComponent->SetLength(40.0f);
    TempTranslateZComponent->SetThickness(GizmoTranslationThickness);
    TempTranslateZComponent->SetupAttachment(TranslationGroupComponent);
    TranslateZComponent = TempTranslateZComponent;

    // Translation Proxy
    TranslationProxyComponent = CreateDefaultSubobject<USceneComponent>(TEXT("GizmoTranslationProxy"));
    TranslationProxyComponent->SetupAttachment(TranslationGroupComponent);
}

TArray<UPrimitiveComponent*> ABoxGizmoActor::GetBoundsSubComponents()
{
    TArray<UPrimitiveComponent*> Result;
    Result.Add(ElevationComponent);
    Result.Add(PlanTopLeftComponent);
    Result.Add(PlanTopRightComponent);
    Result.Add(PlanBottomRightComponent);
    Result.Add(PlanBottomLeftComponent);

    return Result;
}

UPrimitiveComponent* ABoxGizmoActor::GetPlanCornerComponent(int32 CornerIndex)
{
    check(CornerIndex >= 0 && CornerIndex < 4);

    UPrimitiveComponent* CornerComponent = nullptr;
    switch (CornerIndex)
    {
    case 0:
        CornerComponent = PlanTopLeftComponent;
        break;
    case 1:
        CornerComponent = PlanTopRightComponent;
        break;
    case 2:
        CornerComponent = PlanBottomRightComponent;
        break;
    case 3:
        CornerComponent = PlanBottomLeftComponent;
        break;
    default:
        break;
    }

    return CornerComponent;
}

int32 ABoxGizmoActor::GetPlanCornerDiagonalIndex(int32 CornerIndex) const
{
    check(CornerIndex >= 0 && CornerIndex < 4);

    return ((CornerIndex + 2) % 4);
}

TArray<int32> ABoxGizmoActor::GetPlanCornerNeighborIndices(int32 CornerIndex) const
{
    check(CornerIndex >= 0 && CornerIndex < 4);

    TArray<int32> Result;
    Result.Add((CornerIndex + 1) % 4);
    Result.Add((CornerIndex + 3) % 4);

    return Result;
}

USceneComponent* ABoxGizmoActor::GetRotateAxisSocketComponent(int32 AxisIndex, int32 FaceIndex)
{
    USceneComponent* Result = nullptr;
    switch (AxisIndex)
    {
    case 0:
        switch (FaceIndex)
        {
        case 0:
            Result = RotateXFrontSocketComponent;
            break;
        case 1:
            Result = RotateXBackSocketComponent;
            break;
        default:
            break;
        }
        break;
    case 1:
        switch (FaceIndex)
        {
        case 0:
            Result = RotateYFrontSocketComponent;
            break;
        case 1:
            Result = RotateYBackSocketComponent;
            break;
        default:
            break;
        }
        break;
    case 2:
        break;
    default:
        break;
    }

    return Result;
}

UPrimitiveComponent* ABoxGizmoActor::GetRotateAxisIndicatorComponent(int32 AxisIndex, int32 FaceIndex)
{
    UPrimitiveComponent* Result = nullptr;
    switch (AxisIndex)
    {
    case 0:
        switch (FaceIndex)
        {
        case 0:
            Result = RotateXFrontIndicatorComponent;
            break;
        case 1:
            Result = RotateXBackIndicatorComponent;
            break;
        default:
            break;
        }
        break;
    case 1:
        switch (FaceIndex)
        {
        case 0:
            Result = RotateYFrontIndicatorComponent;
            break;
        case 1:
            Result = RotateYBackIndicatorComponent;
            break;
        default:
            break;
        }
        break;
    case 2:
        break;
    default:
        break;
    }

    return Result;
}

UPrimitiveComponent* ABoxGizmoActor::GetRotateAxisDialComponent(int32 AxisIndex, int32 FaceIndex)
{
    UPrimitiveComponent* Result = nullptr;
    switch (AxisIndex)
    {
    case 0:
        switch (FaceIndex)
        {
        case 0:
            Result = RotateXFrontDialComponent;
            break;
        case 1:
            Result = RotateXBackDialComponent;
            break;
        default:
            break;
        }
        break;
    case 1:
        switch (FaceIndex)
        {
        case 0:
            Result = RotateYFrontDialComponent;
            break;
        case 1:
            Result = RotateYBackDialComponent;
            break;
        default:
            break;
        }
        break;
    case 2:
        break;
    default:
        break;
    }

    return Result;
}

UPrimitiveComponent* ABoxGizmoActor::GetTranslateXYComponent()
{
    UPrimitiveComponent* Result = nullptr;

    if (TranslateXYComponent.IsValid())
    {
        Result = TranslateXYComponent.Get();
    }

    return Result;
}

void ABoxGizmoActor::SetTranslateXYComponent(UPrimitiveComponent* InComponent)
{
    if (InComponent)
    {
        TranslateXYComponent = InComponent;
    }
    else
    {
        TranslateXYComponent.Reset();
    }
}
