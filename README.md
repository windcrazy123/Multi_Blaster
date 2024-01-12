## 网络概述

遇到问题：UE版本5.2.1，character movement组件的Character Movement(Networking)中Network Smoothing Mode为Linear或Exponential时调用蓝图函数Set Capsule Half Height 会使得Mesh相对于根节点的位置重置

https://docs.unrealengine.com/4.27/zh-CN/InteractiveExperiences/Networking/Overview/

相关性和优先级

**相关性** 用于决定是否需要在多人游戏期间复制Actor。复制期间将剔除被认为不相关的actor。此操作可节约带宽，以便相关Actor可更加高效地复制。若Actor未被玩家拥有，且不在玩家附近，将其被视为不相关，而不会进行复制。不相关Actor会存在于服务器上，且会影响授权游戏状态，但在玩家靠近前不会向客户端发送信息。覆盖 IsNetRelevantFor 函数以手动控制相关性，并可使用 NetCullDistanceSquared 属性决定成为相关Actor所需距离。

有时在游戏单帧内，没有足够带宽供复制所有相关Actor。因此，Actor拥有 **优先级（Priority****）** 值，用于决定优先复制的Actor。Pawn和PlayerController的 NetPriority 默认为 **3.0**，从而使其成为游戏中最高优先级的Actor，而基础Actor的 NetPriority 为 **1.0**。Actor在被复制前经历的时间越久，每次成功通过时所处的优先级便越高。

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

 

 

插件：插件由一个或多个模块组成，每个模块都有自己的C++代码和.Build.cs构建文件游戏模块，模块只有代码不能包含uassets文件如mesh、纹理等，封装使其井井有条；我们游戏项目本身就是一个模块，当我们启用插件时，会添加到我们的.uproject文件，一个插件可以引用其他插件，UE支持相互依赖的模块和插件，但是需要注意：插件和模块都有层次结构，只能依赖于其他相同或更高层级的插件和模块，如：我们游戏模块可以依赖引擎模块，引擎模块不能依赖我们的游戏，引擎的许多模块可以引用是因为处在同一层级。

PublicDependencyModuleNames和PrivateDependencyModelNames唯一区别是：将他们添加到私有依赖模块名称意味着这些模块仅在私有源文件中可用，如果想在项目中使用这些模块就需要添加到PublicDependencyModuleNames中

在.uproject和.uplugin文件中可以更改此模块的属性和启用的插件