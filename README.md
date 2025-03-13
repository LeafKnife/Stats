# LK-Stats

![GitHub Tag](https://img.shields.io/github/v/tag/LeafKnife/Stats?include_prereleases&style=for-the-badge)
![GitHub Downloads (all assets, all releases)](https://img.shields.io/github/downloads/LeafKnife/Stats/total?style=for-the-badge)
<!-- ![GitHub License](https://img.shields.io/github/license/LeafKnife/Stats?style=for-the-badge) -->

Minecraft 基岩版服务器(BDS) 统计信息插件

## 安装

### 使用 Lip

```sh
lip install github.com/LeafKnife/Stats
```

### 手动安装

1. 从[发布页面](https://github.com/LeafKnife/Stats/releases)下载最新版本。
2. 解压`Stats-windows-x64.zip`文件。
3. 将`LK-Stats`文件夹复制到 BDS 安装目录下的 plugins 文件夹中。

## 使用

启动服务器即可

### 命令

- `/stats gui [StatsType]` 显示玩家统计信息表单(GUI)
- `/stats rank <StatsType> [type]` 显示玩家统计信息排行榜

StatsType: `custom`, `mined`, `broken`, `crafted`, `used`, `picked_up`, `dropped`, `killed`, `killed_by`

## 统计信息

### 统计类型和名称

| 统计类型         | 描述                                       | 命名空间            |
| ---------------- | ------------------------------------------ | ------------------- |
| CUSTOM           | 与玩家行为相关的大量通用统计信息           | minecraft:custom    |
| BLOCK_MINED      | 与玩家开采的方块数相关的统计信息           | minecraft:mined     |
| ITEM_BROKE       | 与玩家的物品耐久度相关的统计信息           | minecraft:broken    |
| ITEM_CRAFTED     | 与合成、熔炼、交易等物品数量有关的统计信息 | minecraft:crafted   |
| ITEM_USED        | 与使用的方块或物品的数量有关的统计信息     | minecraft:used      |
| ITEM_PICKED_UP   | 与玩家捡起的掉落物品数量有关的统计信息     | minecraft:picked_up |
| ITEM_DROPPED     | 与丢弃的物品数量有关的统计信息             | minecraft:dropped   |
| ENTITY_KILLED    | 与玩家杀死的实体数量相关的统计信息         | minecraft:killed    |
| ENTITY_KILLED_BY | 与玩家被实体杀死相关的统计信息             | minecraft:killed_by |

### 统计信息列表

| 统计名称           | 描述                                               | 命名空间                                  |
| ------------------ | -------------------------------------------------- | ----------------------------------------- |
| 繁殖动物次数       | 玩家通过给成对的动物喂食来使之繁殖的次数。         | minecraft:animals_bred                    |
| 清洗盔甲次数       | 玩家使用炼药锅洗去皮革盔甲染色的次数。             | minecraft:clean_armor                     |
| 清洗旗帜次数       | 玩家使用炼药锅洗去旗帜上的图案的次数。             | minecraft:clean_banner                    |
| 木桶打开次数       | 玩家打开木桶的次数。                               | minecraft:open_barrel                     |
| 鸣钟次数           | 玩家敲响钟的次数。                                 | minecraft:bell_ring                       |
| 吃掉的蛋糕片数     | 玩家吃下的蛋糕片数。                               | minecraft:eat_cake_slice                  |
| 炼药锅装水次数     | 玩家使用水桶将炼药锅装满的次数。                   | minecraft:fill_cauldron                   |
| 箱子打开次数       | 玩家打开箱子的次数。                               | minecraft:open_chest                      |
| 吸收的伤害         | 玩家吸收的伤害总量，单位为 1（♥）的 1⁄10。         | minecraft:damage_absorbed                 |
| 盾牌抵挡的伤害     | 玩家使用盾牌抵挡的伤害总量，单位为 1（♥）的 1⁄10。 | minecraft:damage_blocked_by_shield        |
| 造成伤害           | 玩家造成的伤害总量，单位为 1（♥）的 1⁄10           | minecraft:damage_dealt                    |
| 造成伤害（被吸收） | 玩家造成但被吸收的伤害总量，单位为 1（♥）的 1⁄10。 | minecraft:damage_dealt_absorbed           |
| 造成伤害（被抵挡） | 玩家造成但被抵挡的伤害总量，单位为 1（♥）的 1⁄10。 | minecraft:damage_dealt_resisted           |
| 抵挡的伤害         | 玩家抵挡的伤害总量，单位为 1（♥）的 1⁄10。         | minecraft:damage_resisted                 |
| 受到伤害           | 玩家承受的伤害总量，单位为 1（♥）的 1⁄10。         | minecraft:damage_taken                    |
| 搜查发射器次数     | 玩家和发射器交互的次数。                           | minecraft:inspect_dispenser               |
| 坐船移动距离       | 玩家乘船移动的总距离。                             | minecraft:boat_one_cm                     |
| 鞘翅滑行距离       | 玩家使用鞘翅滑行的总距离。                         | minecraft:aviate_one_cm                   |
| 骑马移动距离       | 玩家骑马移动的总距离。                             | minecraft:horse_one_cm                    |
| 坐矿车移动距离     | 玩家乘矿车移动的总距离。                           | minecraft:minecart_one_cm                 |
| 骑猪移动距离       | 玩家用鞍骑猪移动的总距离。                         | minecraft:pig_one_cm                      |
| 骑炽足兽移动距离   | 玩家用鞍骑炽足兽移动的总距离。                     | minecraft:strider_one_cm                  |
| 已攀爬距离         | 玩家通过梯子或藤蔓向上移动的总距离。               | minecraft:climb_one_cm                    |
| 潜行距离           | 玩家潜行时移动的总距离。                           | minecraft:crouch_one_cm                   |
| 摔落高度           | 玩家坠落的总距离，跳跃不计算在内。                 | minecraft:fall_one_cm                     |
| 飞行距离           | 玩家同时向上、向前移动的总距离                     | minecraft:fly_one_cm                      |
| 疾跑距离           | 玩家疾跑的总距离。                                 | minecraft:sprint_one_cm                   |
| 游泳距离           | 玩家游泳的总距离                                   | minecraft:swim_one_cm                     |
| 行走距离           | 玩家行走的总距离。                                 | minecraft:walk_one_cm                     |
| 水面行走距离       | 玩家在水面上下游动的总距离。                       | minecraft:walk_on_water_one_cm            |
| 水下行走距离       | 玩家在水下直立行走的总距离。                       | minecraft:walk_under_water_one_cm         |
| 搜查投掷器次数     | 玩家和投掷器交互的次数。                           | minecraft:inspect_dropper                 |
| 末影箱打开次数     | 玩家打开末影箱的次数。                             | minecraft:open_enderchest                 |
| 捕鱼数             | 玩家捕到的鱼的总数。                               | minecraft:fish_caught                     |
| 游戏退出次数       | 玩家点击“保存并退回到标题画面”按钮的次数。         | minecraft:leave_game                      |
| 搜查漏斗次数       | 玩家和漏斗交互的次数。                             | minecraft:inspect_hopper                  |
| 与铁砧交互次数     | 玩家和铁砧交互的次数。                             | minecraft:interact_with_anvil             |
| 与信标交互次数     | 玩家和信标交互的次数。                             | minecraft:interact_with_beacon            |
| 与高炉交互次数     | 玩家和高炉交互的次数。                             | minecraft:interact_with_blast_furnace     |
| 与酿造台交互次数   | 玩家和酿造台交互的次数。                           | minecraft:interact_with_brewingstand      |
| 与营火交互次数     | 玩家和营火交互的次数。                             | minecraft:interact_with_campfire          |
| 与制图台交互次数   | 玩家和制图台交互的次数。                           | minecraft:interact_with_cartography_table |
| 与工作台交互次数   | 玩家和工作台交互的次数。                           | minecraft:interact_with_crafting_table    |
| 与熔炉交互次数     | 玩家和熔炉交互的次数。                             | minecraft:interact_with_furnace           |
| 与砂轮交互次数     | 玩家和砂轮交互的次数。                             | minecraft:interact_with_grindstone        |
| 与讲台交互次数     | 玩家和讲台交互的次数。                             | minecraft:interact_with_lectern           |
| 与织布机交互次数   | 玩家和织布机交互的次数。                           | minecraft:interact_with_loom              |
| 与锻造台交互次数   | 玩家和锻造台交互的次数。                           | minecraft:interact_with_smithing_table    |
| 与烟熏炉交互次数   | 玩家和烟熏炉交互的次数。                           | minecraft:interact_with_smoker            |
| 与切石机交互次数   | 玩家和切石机交互的次数。                           | minecraft:interact_with_stonecutter       |
| 物品掉落           | 玩家掉落物品的数量，因死亡掉落的物品不计算在内。   | minecraft:drop                            |
| 物品附魔次数       | 玩家为物品使用附魔台附魔的次数。                   | minecraft:enchant_item                    |
| 跳跃次数           | 玩家进行跳跃的次数。                               | minecraft:jump                            |
| 生物击杀数         | 玩家击杀的生物总数。                               | minecraft:mob_kills                       |
| 播放唱片数         | 玩家用唱片机播放音乐唱片的次数。                   | minecraft:play_record                     |
| 音符盒播放次数     | 玩家击打音符盒的次数。                             | minecraft:play_noteblock                  |
| 音符盒调音次数     | 玩家和音符盒交互的次数。                           | minecraft:tune_noteblock                  |
| 死亡次数           | 玩家死亡的次数。                                   | minecraft:deaths                          |
| 盆栽种植数         | 玩家将植物种进花盆的次数。                         | minecraft:pot_flower                      |
| 玩家击杀数         | 玩家（在开启 PvP 的服务器中）击杀的玩家总数        | minecraft:player_kills                    |
| 触发袭击次数       | 玩家触发袭击的次数。                               | minecraft:raid_trigger                    |
| 袭击胜利次数       | 玩家战胜袭击的次数。                               | minecraft:raid_win                        |
| 潜影盒清洗次数     | 玩家使用炼药锅洗去潜影盒染色的次数。               | minecraft:clean_shulker_box               |
| 潜影盒打开次数     | 玩家打开潜影盒的次数。                             | minecraft:open_shulker_box                |
| ~~自上次死亡~~     | 玩家自上次死亡以来经过的游戏时长。                 | minecraft:time_since_death                |
| ~~自上次入眠~~     | 玩家自上次在床上睡眠以来经过的游戏时长             | minecraft:time_since_rest                 |
| 潜行时间           | 玩家潜行的时长。                                   | minecraft:sneak_time                      |
| 村民交互次数       | 玩家和村民交互（打开其 GUI）的次数。               | minecraft:talked_to_villager              |
| 击中标靶次数       | 玩家射中标靶的次数。                               | minecraft:target_hit                      |
| 游戏时长           | 玩家在游戏中经过的时长。                           | minecraft:play_time                       |
| 世界打开时间       | 玩家打开世界的总时长。                             | minecraft:total_world_time                |
| 躺在床上的次数     | 玩家在床上睡觉的次数。                             | minecraft:sleep_in_bed                    |
| 村民交易次数       | 玩家和村民进行交易的次数。                         | minecraft:traded_with_villager            |
| 陷阱箱触发次数     | 玩家打开陷阱箱的次数。                             | minecraft:trigger_trapped_chest           |
| 从炼药锅取水次数   | 玩家从炼药锅中装取水、熔岩和细雪的次数。           | minecraft:use_cauldron                    |

## 更新日志

查看 [CHANGELOG](CHANGELOG.md) 文件获取更多信息。

## 贡献

如果您有任何问题，请开启一个 [issue](https://github.com/LeafKnife/Stats/issues) 来讨论  
欢迎 PR

## 许可

AGPL-3.0 © LWenK
