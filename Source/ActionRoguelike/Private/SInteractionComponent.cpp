// Fill out your copyright notice in the Description page of Project Settings.


#include "SInteractionComponent.h"

#include "DrawDebugHelpers.h"
#include "SGameplayInterface.h"
#include "Curves/CurveLinearColor.h"


// Sets default values for this component's properties
USInteractionComponent::USInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USInteractionComponent::PrimaryInteract()
{
	FCollisionObjectQueryParams   ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	AActor* MyOwner=GetOwner();

	FVector EyeLocation;
	FRotator  EyeRotation;
	MyOwner->GetActorEyesViewPoint(EyeLocation,EyeRotation);

	FVector End=EyeLocation+EyeRotation.Vector()*1000;

	/*FHitResult  Hit;
	bool  bBlockingHit=GetWorld()->LineTraceSingleByObjectType(Hit,EyeLocation,End,ObjectQueryParams);//射线检测是否碰撞到物体 */
	
	//球状检测是否碰撞到 物体
	TArray<FHitResult> Hits;

	float Radius=30.0f;

	FCollisionShape Shape;
	Shape.SetSphere(Radius);

	//将球状碰撞到的物体保存到Hits中
	bool bBlockingHit=GetWorld()->SweepMultiByObjectType(Hits,EyeLocation,End,FQuat::Identity,ObjectQueryParams,Shape);

	//如果碰撞到物体为绿色反之为红色
	FColor  LineColor=bBlockingHit?FColor::Green  : FColor::Red;

	for(FHitResult Hit:Hits)
	{
		AActor* HitActor=Hit.GetActor();
		if(HitActor)
		{
			//实现在蓝图中完成的交互（开关宝箱）
			if(HitActor->Implements<USGameplayInterface>())
			{
				APawn* MyPawn=Cast<APawn>(MyOwner);

				ISGameplayInterface::Execute_Interact(HitActor,MyPawn);
				break;
			}
		}

		DrawDebugSphere(GetWorld(),Hit.ImpactPoint,Radius,32,LineColor,false,2.0f);
	}
	
	

	DrawDebugLine(GetWorld(),EyeLocation,End,LineColor,false,2.0f,0,2.0f);//画一条线段  来 debug
}