# 更新日志

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [未发布]

## [0.2.4] - 2025-03-17

### 新增

- 适配 LeviLamina v1.1.1(BDS1.20.60)
- 命令: `/stats player <playerName:string> <StatsType>`

### 更改

- 将数据文件保存到存档目录下

### 修复

- 玩家被间接击杀时无法被记录的异常情况

## [0.2.3] - 2025-03-01

### 新增

- 统计玩家合成，交易物品数量
- 统计玩家与村民交易次数
- 统计玩家使用附魔台次数

## [0.2.2] - 2025-02-28

### 新增

- 适配 LeviLamina v1.1.0(BDS1.20.60)
- 统计玩家捕鱼次数
- 统计玩家移动距离(滑行、疾跑、游泳等)

### 修复

- 玩家造成伤害无法被记录的异常情况
- mob_kills 拼写错误

## [0.2.1] - 2025-02-23

### 新增

- 适配 LeviLamina v1.1.0.rc.1(BDS1.20.60)
- 统计玩家摔落高度(单位: 厘米)

## [0.1.12] - 2025-02-23

### 新增

- 统计玩家潜行时间(单位:Tick)
- 统计玩家游玩时间(单位:Tick)
- 世界打开时间(单位:Tick)
- 统计玩家繁殖动物次数
- 崩服上报(Sentry.io)

### 更改

- 重构方块交互事件

### 修复

- 插件初始化(未创建 Stats 目录)崩服
- 部分玩家进入游戏时崩服

## [0.1.9] - 2025-02-20

### 新增

- 玩家命令`/stats rank <StatsType> [type]`
- 排行榜功能

### 修复

- 部分玩家进入游戏时崩服

## [0.1.7] - 2025-02-12

### 新增

- 玩家命令`/stats gui [StatsType]`
- 玩家统计信息表单(Form)
- 支持多语言

### 更改

- 玩家造成伤害(只统计由玩家造成近战伤害)

## [0.1.5] - 2025-02-05

### 新增

- 统计玩家使用部分物品
- 统计玩家从熔炉取出物品

### 更改

- 重构部分代码

## [0.1.4] - 2025-01-31

### 修复

- 无法使用 lip 安装的问题(tooth 包)

## [0.1.3] - 2025-01-31

### 新增

- 统计玩家使用物品(音乐唱片)
- 统计物品(工具)破损
- 统计玩家触发袭击次数
- 统计玩家袭击胜利次数

## [0.1.2] - 2025-01-30

### 新增

- 统计玩家与工作台、音符盒、唱片机、蛋糕、营火、花盆交互次数
- 统计玩家击中标靶次数
- 统计玩家与炼药锅交互次数(装水,清洗染色物品)
- 统计玩家使用食物
- 统计玩家造成/承受伤害量(包括吸收、抵抗、真实伤害)
- 统计玩家用盾抵挡伤害量
- 统计玩家与村民交互次数

## [0.1.1] - 2025-01-27

### 新增

- 统计玩家放置/挖掘方块
- 统计玩家击杀实体/被实体击杀
- 统计玩家捡起/丢弃物品
- 统计玩家与床交互次数
- 统计玩家与部分方块交互次数

[未发布]: https://github.com/LeafKnife/Stats/compare/v0.2.4...dev
[0.2.4]: https://github.com/LeafKnife/Stats/compare/v0.2.3...v0.2.4
[0.2.3]: https://github.com/LeafKnife/Stats/compare/v0.2.2...v0.2.3
[0.2.2]: https://github.com/LeafKnife/Stats/compare/v0.2.1...v0.2.2
[0.2.1]: https://github.com/LeafKnife/Stats/compare/v0.1.12...v0.2.1
[0.1.12]: https://github.com/LeafKnife/Stats/compare/v0.1.9...v0.1.12
[0.1.9]: https://github.com/LeafKnife/Stats/compare/v0.1.7...v0.1.9
[0.1.7]: https://github.com/LeafKnife/Stats/compare/v0.1.5...v0.1.7
[0.1.5]: https://github.com/LeafKnife/Stats/compare/v0.1.4...v0.1.5
[0.1.4]: https://github.com/LeafKnife/Stats/compare/v0.1.3...v0.1.4
[0.1.3]: https://github.com/LeafKnife/Stats/compare/v0.1.2...v0.1.3
[0.1.2]: https://github.com/LeafKnife/Stats/compare/v0.1.1...v0.1.2
[0.1.1]: https://github.com/LeafKnife/Stats/releases/tag/v0.1.1
