// Fill out your copyright notice in the Description page of Project Settings.


#include "SExplosiveBarrel.h"

#include "DrawDebugHelpers.h"

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp=CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetSimulatePhysics(true);
	RootComponent=MeshComp;

	RadialForceComp=CreateDefaultSubobject<URadialForceComponent>("RadialForceComp");
	RadialForceComp->SetupAttachment(MeshComp);

	RadialForceComp->SetAutoActivate(false);

	RadialForceComp->Radius=750.0f;
	RadialForceComp->ImpulseStrength=2500.0f;
	RadialForceComp->bImpulseVelChange=true;

	RadialForceComp->AddCollisionChannelToAffect(ECC_WorldDynamic);


}

void ASExplosiveBarrel::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	MeshComp->OnComponentHit.AddDynamic(this,&ASExplosiveBarrel::OnActorHit);
}


//撞击后，调用冲击力函数
void ASExplosiveBarrel::OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	RadialForceComp->FireImpulse();

	UE_LOG(LogTemp,Log,TEXT("OnActorHit in Explosive Barrel"));

	//%s = string;
	//%f = float;
	//logs：OtherActor: MyActor_1,at game time: 124.4
	UE_LOG(LogTemp,Warning,TEXT("OtherActor: %s,at game time: %f"),*GetNameSafe(OtherActor),GetWorld()->TimeSeconds);

	FString CombinedString=FString::Printf(TEXT("Hit at location:%s"),*Hit.ImpactPoint.ToString());
	DrawDebugString(GetWorld(),Hit.ImpactPoint,CombinedString,nullptr,FColor::Green,2.0f,true);
}



