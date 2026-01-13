#include "Weapon/AshWeapon.h"

AAshWeapon::AAshWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordOfPrideMesh"));
	RootComponent = WeaponMesh;

	// 初始设置无碰撞，攻击判定通常在蒙太奇中使用 BoxTrace
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}