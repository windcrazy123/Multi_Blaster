* content
{:toc #markdown-toc}

这是随手记得，很杂，在做的过程中一直改，还未整理，仅供参考

<!-- more -->

# 网络概述

![](.\ReadMeImg\GameFramework.png)

遇到问题：UE版本5.2.1，character movement组件的Character Movement(Networking)中Network Smoothing Mode为Linear或Exponential时调用蓝图函数Set Capsule Half Height 会使得Mesh相对于根节点的位置重置

https://docs.unrealengine.com/4.27/zh-CN/InteractiveExperiences/Networking/Overview/

相关性和优先级

**相关性** 用于决定是否需要在多人游戏期间复制Actor。复制期间将剔除被认为不相关的actor。此操作可节约带宽，以便相关Actor可更加高效地复制。若Actor未被玩家拥有，且不在玩家附近，将其被视为不相关，而不会进行复制。不相关Actor会存在于服务器上，且会影响授权游戏状态，但在玩家靠近前不会向客户端发送信息。覆盖 IsNetRelevantFor 函数以手动控制相关性，并可使用 NetCullDistanceSquared 属性决定成为相关Actor所需距离。

有时在游戏单帧内，没有足够带宽供复制所有相关Actor。因此，Actor拥有 **优先级（Priority）** 值，用于决定优先复制的Actor。Pawn和PlayerController的 NetPriority 默认为 **3.0**，从而使其成为游戏中最高优先级的Actor，而基础Actor的 NetPriority 为 **1.0**。Actor在被复制前经历的时间越久，每次成功通过时所处的优先级便越高。

在游戏中实现高效、稳定多人游戏系统的基本指南如下。

基本复制Actor清单

按照以下步骤，可创建复制Actor：

- 将Actor的复制设置设为True。
- 若复制Actor需要移动，将复制移动（Replicates Movement）设为True。
- 生成或销毁复制Actor时，确保在服务器上执行该操作。
- 设置必须在机器间共享的变量，以便进行复制。这通常适用于以gameplay为基础的变量。
- 尽量使用虚幻引擎的预制移动组件，其已针对复制进行构建。
- 若使用服务器授权模型，需确保玩家可执行的新操作均由服务器函数触发。

网络提示

- 尽可能少用RPC或复制蓝图函数。在合适情况下改用RepNotify。
- 组播函数会导致会话中各连接客户端的额外网络流量，需尤其少用。
- 若能保证非复制函数仅在服务器上执行，则服务器RPC中无需包含纯服务器逻辑。
- 将可靠RPC绑定到玩家输入时需谨慎。玩家可能会快速反复点击按钮，导致可靠RPC队列溢出。应采取措施限制玩家激活此项的频率。
- 若游戏频繁调用RPC或复制函数，如tick时，则应将其设为不可靠。
- 部分函数可重复使用。调用其响应游戏逻辑，然后调用其响应RepNotify，确保客户端和服务器拥有并列执行即可。
- 检查Actor的网络角色可查看其是否为 ROLE_Authority。此方法适用于过滤函数中的执行，该函数同时在服务器和客户端上激活。
- 使用C++中的 IsLocallyControlled 函数或蓝图中的Is     Locally Controlled函数，可检查Pawn是否受本地控制。基于执行是否与拥有客户端相关来过滤函数时，此方法十分拥有。
- 构造期间Pawn可能未被指定控制器，因此避免在构造函数脚本中使用 IsLocallyControlled。

# RPC

参考：[虚幻引擎文档](https://docs.unrealengine.com/5.2/en-US/rpcs-in-unreal-engine/)

**从服务器调用的 RPC**

| Actor 所有权       | 未复制         | NetMulticast               | Server         | Client                      |
| ------------------ | -------------- | -------------------------- | -------------- | --------------------------- |
| Client-owned actor | 在服务器上运行 | 在服务器和所有客户端上运行 | 在服务器上运行 | 在 actor 的所属客户端上运行 |
| Server-owned actor | 在服务器上运行 | 在服务器和所有客户端上运行 | 在服务器上运行 | 在服务器上运行              |
| Unowned actor      | 在服务器上运行 | 在服务器和所有客户端上运行 | 在服务器上运行 | 在服务器上运行              |

**从客户端调用的 RPC**

| Actor 所有权                | 未复制                   | NetMulticast             | Server         | Client                   |
| --------------------------- | ------------------------ | ------------------------ | -------------- | ------------------------ |
| Owned by invoking client    | 在执行调用的客户端上运行 | 在执行调用的客户端上运行 | 在服务器上运行 | 在执行调用的客户端上运行 |
| Owned by a different client | 在执行调用的客户端上运行 | 在执行调用的客户端上运行 | 丢弃           | 在执行调用的客户端上运行 |
| Server-owned actor          | 在执行调用的客户端上运行 | 在执行调用的客户端上运行 | 丢弃           | 在执行调用的客户端上运行 |
| Unowned actor               | 在执行调用的客户端上运行 | 在执行调用的客户端上运行 | 丢弃           | 在执行调用的客户端上运行 |

**但是我们在编写多人游戏的时候,我们需要确保添加 RPC 不仅仅是为了进行复制一些属性、Actor等。我们需要考虑可能有更有效的办法。利用已经有的RPC或ReplicatedNotify等是一个很好的办法。**比如：子弹在服务器射中目标播放声音和特效，需要添加一个Multicast方法吗？不必要，由于射中物体子弹销毁，而所调用的Destroy方法广播给所有绑定的物体进行销毁，而我们的子弹是`bReplicates = true;`那么确实会传播给所有客户端，因此我们可以利用Destroy进行多播。

```c++
void AActor::Destroyed()
{
	RouteEndPlay(EEndPlayReason::Destroyed);

	ReceiveDestroyed();
	OnDestroyed.Broadcast(this);
}
```
```c++
void AProjectile::Destroyed()
{
	Super::Destroyed();

	if (ImpactParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                        FVector NormalImpulse, const FHitResult& HitResult)
{
	Destroy();
}
```







插件：插件由一个或多个模块组成，每个模块都有自己的C++代码和.Build.cs构建文件游戏模块，模块只有代码不能包含uassets文件如mesh、纹理等，封装使其井井有条；我们游戏项目本身就是一个模块，当我们启用插件时，会添加到我们的.uproject文件，一个插件可以引用其他插件，UE支持相互依赖的模块和插件，但是需要注意：插件和模块都有层次结构，只能依赖于其他相同或更高层级的插件和模块，如：我们游戏模块可以依赖引擎模块，引擎模块不能依赖我们的游戏，引擎的许多模块可以引用是因为处在同一层级。

PublicDependencyModuleNames和PrivateDependencyModelNames唯一区别是：将他们添加到私有依赖模块名称意味着这些模块仅在私有源文件中可用，如果想在项目中使用这些模块就需要添加到PublicDependencyModuleNames中

在.uproject和.uplugin文件中可以更改此模块的属性和启用的插件



# FLatentActionInfo

参考：[How does the FLatentActionInfo Linkage variable work?](https://forums.unrealengine.com/t/how-does-the-flatentactioninfo-linkage-variable-work/440636)

```c++
// Trigger any pending execution links
	for (FLatentResponse::FExecutionInfo& LinkInfo : Response.LinksToExecute)
	{
        // INDEX_NONE == -1; LinkID : LatentActionInfo.Linkage
		if (LinkInfo.LinkID != INDEX_NONE)
		{
			if (UObject* CallbackTarget = LinkInfo.CallbackTarget.Get())
			{
				check(CallbackTarget == InObject);

				if (UFunction* ExecutionFunction = CallbackTarget->FindFunction(LinkInfo.ExecutionFunction))
				{
					CallbackTarget->ProcessEvent(ExecutionFunction, &(LinkInfo.LinkID));
				}
			}
		}
	}
```





UE中虽然可以前置声明，但是枚举类的前置声明较为特殊格式类似于：

```c++
enum class ETurningInPlace : uint8;
```

那么此时编译会报错：

```ABAP
Unrecognized type 'ETurningInPlace' - type must be a UCLASS, USTRUCT or UENUM
```

因此，如果不想在头文件引用体积大u的头文件，一般会另建头文件去写此枚举类，然后再引用枚举类的头文件

# UE4移动的网络同步

参考：[UE4移动的网络同步](https://zhuanlan.zhihu.com/p/114341957)
[《Exploring in UE4》移动组件详解[原理分析]](https://zhuanlan.zhihu.com/p/34257208)


## **Simulate客户端预测**

Simulate客户端收到的移动数据，相比移动刚发生时，理论上位置已经延迟了一个RTT，收到即落后。如果在此基础上进行平滑移动，平滑目标为服务器同步最新位置，那么又落后了一个平滑周期的时间。因此为了使Simulate客户端上角色更接近游戏实时状态，UE4允许Simulate角色进行本地预测移动。在每帧TickComponent时，对Simulate客户端会执行SimulateMovement，获取当前速度，大部分情况当前速度就是移动同步下来的玩家速度，然后把加速度设为和速度方向一样，大小为1，因此rotation并不会改变。移动会忽略MovementMode，且忽略摩擦力等外力对速度的影响。直到下一次收到服务器位置更新，再进行新的预测移动。

预测和插值会同时进行，预测改变的是世界transform，插值改变的是相对transform，不冲突。

加入预测后，Simulate客户端上角色移动落后就只有一个RTT了。

![img](.\ReadMeImg\Simulate客户端预测.png)

**一个实现细节：关于FVector_NetQuantize**

移动同步涉及到很多实现细节，没法面面俱到的介绍，在此仅把FVector_NetQuantize作为其中一个示例进行细致分析。

移动同步中的位置、加速度、速度都可以用FVector表示，内含3个float类型变量，总共12个Byte。相对于游戏中大部分使用场景，float的表示范围是大大溢出的，但是float遵循IEEE754标准，在网络上传输float无法使用自适应Bit流来减小数据量，导致直接传输FVector就必须用12个Byte，有些浪费。

FVector_NetQuantize数据结构就是为了解决这个问题，它使用了有损压缩技术。在进行网络序列化时，会把向量中的float转换成int传输，减小数据量，反序列化时，再把int恢复成float，再组合成FVector，会损失一些精度，但大部分情况都可接受。

FVector_NetQuantize数据结构有多个版本，我们常用的有FVector_NetQuantize10和FVector_NetQuantize100，后面的数字代表float转换成int时，精确到小数点的位数，显然后者更高。它们表示范围有限制，比如FVector_NetQuantize10中数字表示范围是2^24/10，即+/-1677721.6，至于为什么会在接下来介绍，使用时留意此范围即可。如果FVector代表角色位置，长度单位是厘米，那么精确到1/100厘米对游戏而言是完全OK的。

**序列化**

FVector_NetQuantize序列化会使用SerializePackedVector函数

```c++
template<uint32 ScaleFactor, int32 MaxBitsPerComponent>
bool SerializePackedVector(FVector &Vector, FArchive& Ar)
{
	if (Ar.IsSaving())
	{
		return  WritePackedVector<ScaleFactor, MaxBitsPerComponent>(Vector, Ar);
	}

	ReadPackedVector<ScaleFactor, MaxBitsPerComponent>(Vector, Ar);
	return true;
}
```

函数接受两个模版参数，第一个是缩放大小，即10、100，第二个是转换成int后可使用的最大bit数，限制了表示范围。FVector_NetQuantize10在序列化时，两个参数分别为10和24，因此表示范围是2^24/10。

序列化步骤如下

1. FVector乘以ScaleFactor，进行放大
2. 把float转换成int
3. 计算表示三个int绝对值+1（正数）所需最大位数，记为Bits，在MaxBitsPerComponent处截断
4. 计算偏移Bias=1<<(Bits+1)，然后把三个int都加上Bias，这是为了把负数都变成正数传输，这样才能用自适应Bit流
5. 计算上限Max=1<<(Bits+2)，并用Max-1对int数值进行截断
6. 先向数据流写入Bits，表示后续数字的最大位数，再依次写入三个int值，每个int值要求所用bit位数相同，不满的用0填充，完成序列化



例子：使用FVector_NetQuantize10传输(-1.0, 5.1, 100.0)

首先，把它们乘以ScaleFactor，得到(-10, 51, 1000)

然后计算表示它们需要的最大位数，为10，2^10=1024，因此Bias = 2048

之后三个int都加上Bias，得到(2038, 2099, 3048)

依次向数据流写入10和(2038, 2099, 3048)，此时数据流为：

![img](.\ReadMeImg\数据流.png)

总共使用了41位，原始的三个float需要128位，仅为其32%大小。留意10和2038的第一位0填充。



**反序列化**

了解了序列化过程，反序列化也不难理解了。

首先知道这是FVector_NetQuantize10类型数据，因此10和24这两个数字是确定的。于是可以从Bit流头部取出5位，得到“Bits“，从而得到上限“Max”和”Bias“，以及之后每个数字所用位数。剩下就是依次获取三个int值，然后减去Bias，并除以ScaleFactor进行还原。

实际案例：ServerMove接口

```c++
void UCharacterMovementComponent::ServerMove_Implementation(
	float TimeStamp,
	FVector_NetQuantize10 InAccel,
	FVector_NetQuantize100 ClientLoc,
```

加速度InAccel使用FVector_NetQuantize10存储，位置ClientLoc使用FVector_NetQuantize100存储。

## Simulate Proxy的旋转

在做AimOffset的时候发现，骨骼的旋转在Simulate Proxy上有点抖，这是因为AimOffset在动画蓝图里旋转了骨骼，而动画蓝图在本地机子和服务器上是每帧调用的这样看起来没有问题，但是实际上Simulate Proxy并不是每帧更新的，并且它的更新还有网络更新的影响，这样就慢的多，因此Simulate Proxy会抖来抖去。

# HUD 和 PlayerController

APlayerController::GetHUD()
AHUD::DrawHUD()
AHUD::DrawTexture()

# Rider使用

## UE 编译时 PCH 虚拟内存不足的错误

在多核 CPU 上编译 UE 时，可能比较频繁的遇到以下错误，尤其是编译的数量较多的时候：

```ABAP
0>c1xx: Error C3859 : 未能创建 PCH 的虚拟内存
	c1xx: note: 系统返回代码 1455: 页面文件太小，无法完成操作。
	c1xx: note: 请访问 https://aka.ms/pch-help 了解更多详情
0>c1xx: Error C1076 : 编译器限制: 达到内部堆限制
  Microsoft.MakeFile.targets(51, 5): [MSB3073] 命令“D:\UE_4.26\Engine\Build\BatchFiles\Rebuild.bat MyProjectEditor Win64 Development -Project="D:\UEProject\MultiGame\Multi_Test\MyProject.uproject" -WaitMutex -FromMsBuild”已退出，代码为 -1。
```

默认情况下，UE 会拉起系统最大线程数量的编译进程。就比如我电脑是16个

![](.\ReadMeImg\16processes.png)

UE 里每个编译任务的 `/zm` 值为 1000：[VCToolChain.cs?q=%2Fzm#L354](https://cs.github.com/EpicGames/UnrealEngine/blob/d11782b9046e9d0b130309591e4efc57f4b8b037/Engine/Source/Programs/UnrealBuildTool/Platform/Windows/VCToolChain.cs?q=/zm#L354)
表示每个 cl 进程会分配 750M 的虚拟内存：[/Zm (Specify precompiled header memory allocation limit)](https://docs.microsoft.com/en-us/cpp/build/reference/zm-specify-precompiled-header-memory-allocation-limit?view=msvc-160)

| Value of *`factor`* | Memory allocation limit |
| :------------------ | :---------------------- |
| 10                  | 7.5 MB                  |
| 100                 | 75 MB                   |
| 200                 | 150 MB                  |
| 1000                | 750 MB                  |
| 2000                | 1500 MB                 |

如果ue使用的虚拟内存大于系统的虚拟内存，就会导致分配失败
TaskCount ∗ PCHMemoryAllocationFactor > SystemVirtualMemery
那么就有两种办法可以解决：一个是减少`TaskCount`，另一个就是减少`PCHMemoryAllocationFactor`。这两种办法都可以通过编辑`BuildConfiguration.xml`进行更改

1. 减少TaskCount：[本地执行器和并行执行器](https://docs.unrealengine.com/4.27/en-US/ProductionPipelines/BuildTools/UnrealBuildTool/BuildConfiguration/#localexecutor)

   ```xml
   <LocalExecutor>
   	<MaxProcessorCount>8</MaxProcessorCount>
   </LocalExecutor>
   <ParallelExecutor>
   	<MaxProcessorCount>8</MaxProcessorCount>
   </ParallelExecutor>
   ```

2. 减少PCHMemoryAllocationFactor：[WindowsPlatform->PCHMemoryAllocationFactor](https://docs.unrealengine.com/4.27/en-US/ProductionPipelines/BuildTools/UnrealBuildTool/BuildConfiguration/#windowsplatform)

   ```xml
   <WindowsPlatform>
   	<PCHMemoryAllocationFactor>200</PCHMemoryAllocationFactor>
   </WindowsPlatform>
   ```

### BuildConfiguration.xml

除添加到 `Config/UnrealBuildTool` 文件夹中已生成UE4项目外，虚幻编译工具还会从以下位置（Windows系统）的XML配置文件读取设置：

- Engine/Saved/UnrealBuildTool/BuildConfiguration.xml
- User Folder/AppData/Roaming/Unreal Engine/UnrealBuildTool/BuildConfiguration.xml
- My Documents/Unreal Engine/UnrealBuildTool/BuildConfiguration.xml

如果是Linux和Mac，则会从以下路径读取：

- /Users//.config//Unreal Engine/UnrealBuildTool/BuildConfiguration.xml
- /Users//Unreal Engine/UnrealBuildTool/BuildConfiguration.xml

### Finished

```xml
<?xml version="1.0" encoding="utf-8" ?>
<Configuration xmlns="https://www.unrealengine.com/BuildConfiguration">
    <WindowsPlatform>
        <PCHMemoryAllocationFactor>200</PCHMemoryAllocationFactor>
    </WindowsPlatform>
    <LocalExecutor>
        <MaxProcessorCount>8</MaxProcessorCount>
    </LocalExecutor>
    <ParallelExecutor>
        <MaxProcessorCount>8</MaxProcessorCount>
    </ParallelExecutor>
</Configuration>
```

8*150MB = 1200MB（虽然我改为16也没有超过我系统的虚拟内存大小但还是报错了，**所以此内容可能有误**，但是我如此改完后确实不报错了）

> 注：虚拟内存查看：我的电脑->属性->高级系统设置->高级->性能->设置->高级->虚拟内存
>
> 并且需要注意的是：如果 `/zm` 值设置的太小，可能无法满足 UE 合并翻译单元的要求，导致编译错误，所以，最好还是修改系统虚拟内存大小或者控制并行的任务数量。
>
> 参考：https://ue5wiki.com/wiki/5cc4f8a/