// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SInteractionComponent.h"
#include "SAttributeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SActionComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//创建组件以及设置
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>("SpringArmComp");
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComp");
	CameraComp->SetupAttachment(SpringArmComp);

	InteractionComp = CreateDefaultSubobject<USInteractionComponent>("InteractionComp");

	AttributeComp = CreateDefaultSubobject<USAttributeComponent>("AttributeComp");

	ActionComp = CreateDefaultSubobject<USActionComponent>("ActionComp");

	//如为true，旋转角色朝向加速方向，将RotationRate用作旋转的变化率。重载UseControllerDesiredRotation。
	GetCharacterMovement()->bOrientRotationToMovement = true;
	//通常需要确保将其他设置清除，如角色上的bUseControllerRotationYaw.
	bUseControllerRotationYaw = false;
	
	// Enabled on mesh to react to incoming projectiles
	//修改GetGenerateOverlapEvents（）返回的值为true,如果为true，则该组件将在与其他组件重叠时生成重叠事件
	GetMesh()->SetGenerateOverlapEvents(true);
	// 禁用胶囊碰撞，以避免在进入触发区等时出现双重倾斜和接收2个重叠。
	// 一次来自网格，第二次来自胶囊
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);

	TimeToHitParamName = "TimeToHit";
}

//重写PostInitializeComponents()函数，但仍旧想要使用父类的功能，所以调用父类的该函数
//该函数在Beginplay()之前调用
void ASCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();//相当于ACharacter::PostInitializeComponents()

	//用于将UObject实例和成员UFUNCTION绑定到动态多播委托的帮助器宏。
	//参数：
	//UserObject–UObject实例
	//FuncName–指向成员UFUNCTION的函数指针，通常形式为&UClassName::FunctionName
	//OnHealthChanged是一个委托变量
	AttributeComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);
}


FVector ASCharacter::GetPawnViewLocation() const
{
	return CameraComp->GetComponentLocation();
}


// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("PrimaryAttack", IE_Pressed, this, &ASCharacter::PrimaryAttack);
	// Used generic name 'SecondaryAttack' for binding
	PlayerInputComponent->BindAction("SecondaryAttack", IE_Pressed, this, &ASCharacter::BlackHoleAttack);
	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &ASCharacter::Dash);
	PlayerInputComponent->BindAction("PrimaryInteract", IE_Pressed, this, &ASCharacter::PrimaryInteract);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASCharacter::SprintStart);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASCharacter::SprintStop);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
}

//前后移动速度
void ASCharacter::MoveForward(float Value)
{
	FRotator ControlRot = GetControlRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;

	AddMovementInput(ControlRot.Vector(), Value);
}

//左右移动速度
void ASCharacter::MoveRight(float Value)
{
	FRotator ControlRot = GetControlRotation();
	ControlRot.Pitch = 0.0f;
	ControlRot.Roll = 0.0f;

	// X = Forward (Red)
	// Y = Right (Green)
	// Z = Up (Blue)

	FVector RightVector = FRotationMatrix(ControlRot).GetScaledAxis(EAxis::Y);

	AddMovementInput(RightVector, Value);
}

//冲刺
void ASCharacter::SprintStart()
{
	ActionComp->StartActionByName(this, "Sprint");
}
void ASCharacter::SprintStop()
{
	ActionComp->StopActionByName(this, "Sprint");
}

//攻击
void ASCharacter::PrimaryAttack()
{
	ActionComp->StartActionByName(this, "PrimaryAttack");
}
void ASCharacter::BlackHoleAttack()
{
	ActionComp->StartActionByName(this, "Blackhole");
}

//突进
void ASCharacter::Dash()
{
	ActionComp->StartActionByName(this, "Dash");
}

//初级交互
void ASCharacter::PrimaryInteract()
{
	if (InteractionComp)
	{
		InteractionComp->PrimaryInteract();
	}
}

//对玩家造成伤害/治愈
void ASCharacter::HealSelf(float Amount /* = 100 */)
{
	AttributeComp->ApplyHealthChange(this, Amount);
}

//玩家受到伤害/治疗
void ASCharacter::OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	// Damaged
	if (Delta < 0.0f)
	{
		GetMesh()->SetScalarParameterValueOnMaterials(TimeToHitParamName, GetWorld()->TimeSeconds);

		// Rage added equal to damage received (Abs to turn into positive rage number)
		float RageDelta = FMath::Abs(Delta);
		AttributeComp->ApplyRage(InstigatorActor, RageDelta);
	}

	// Died
	if (NewHealth <= 0.0f && Delta < 0.0f)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		DisableInput(PC);

		SetLifeSpan(5.0f);
	}
}