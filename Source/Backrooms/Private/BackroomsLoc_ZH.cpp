#include "BackroomsLocRegistry.h"

// 中文（简体）.
namespace BackroomsLoc
{
	void Register_ZH(FLanguageTable& T)
	{
		T.Add(TEXT("Menu.NewGame"), TEXT("新游戏"));
		T.Add(TEXT("Menu.Continue"), TEXT("继续"));
		T.Add(TEXT("Menu.Levels"), TEXT("关卡"));
		T.Add(TEXT("Menu.Achievements"), TEXT("成就"));
		T.Add(TEXT("Menu.Settings"), TEXT("设置"));
		T.Add(TEXT("Menu.Controls"), TEXT("操作"));
		T.Add(TEXT("Menu.Quit"), TEXT("退出"));
		T.Add(TEXT("Menu.Subtitle"), TEXT("阈限下潜"));
		T.Add(TEXT("Menu.Back"), TEXT("返回   >"));

		T.Add(TEXT("Tab.Screen"), TEXT("屏幕"));
		T.Add(TEXT("Tab.Quality"), TEXT("画质"));
		T.Add(TEXT("Tab.Camera"), TEXT("镜头"));
		T.Add(TEXT("Tab.Sound"), TEXT("声音"));
		T.Add(TEXT("Tab.Game"), TEXT("游戏"));
		T.Add(TEXT("Tab.Controls"), TEXT("操作"));
		T.Add(TEXT("Settings.Title"), TEXT("设 置"));
		T.Add(TEXT("Settings.Categories"), TEXT("分 类"));

		T.Add(TEXT("Set.Resolution"), TEXT("分辨率"));
		T.Add(TEXT("Set.DisplayMode"), TEXT("显示模式"));
		T.Add(TEXT("Set.VSync"), TEXT("垂直同步"));
		T.Add(TEXT("Set.RenderScale"), TEXT("渲染缩放"));
		T.Add(TEXT("Set.Gamma"), TEXT("亮度"));
		T.Add(TEXT("Set.Language"), TEXT("语言"));

		T.Add(TEXT("Set.Shadows"), TEXT("阴影"));
		T.Add(TEXT("Set.Textures"), TEXT("材质"));
		T.Add(TEXT("Set.AntiAliasing"), TEXT("抗锯齿"));
		T.Add(TEXT("Set.PostProcess"), TEXT("后处理"));
		T.Add(TEXT("Set.Effects"), TEXT("特效"));
		T.Add(TEXT("Set.ViewDistance"), TEXT("视距"));
		T.Add(TEXT("Set.DepthOfField"), TEXT("景深"));
		T.Add(TEXT("Set.Bloom"), TEXT("泛光"));
		T.Add(TEXT("Set.Lumen"), TEXT("Lumen（光照）"));
		T.Add(TEXT("Set.Upscale"), TEXT("DLSS / 超分辨率"));

		T.Add(TEXT("Set.FOV"), TEXT("视场角"));
		T.Add(TEXT("Set.Sensitivity"), TEXT("灵敏度"));
		T.Add(TEXT("Set.RecordingEffect"), TEXT("录像机效果"));

		T.Add(TEXT("Set.Volume.Master"), TEXT("总音量"));
		T.Add(TEXT("Set.Volume.Menu"), TEXT("音乐 / 菜单"));
		T.Add(TEXT("Set.Volume.Game"), TEXT("游戏音效"));
		T.Add(TEXT("Set.Volume.Monster"), TEXT("怪物音效"));
		T.Add(TEXT("Set.Volume.Other"), TEXT("其他音效"));

		T.Add(TEXT("Set.Difficulty"), TEXT("难度"));
		T.Add(TEXT("Set.Difficulty.Hint"),
			TEXT("难度会影响生成：墙体密度、通道数量、物资与战利品的丰富程度。"));

		T.Add(TEXT("Levels.Title"), TEXT("关 卡"));
		T.Add(TEXT("Levels.Locked"), TEXT("未解锁"));
		T.Add(TEXT("Levels.UnlockedFmt"),
			TEXT("已解锁关卡：%d / 10。新关卡随下潜逐步开放——走得更深即可看到它们的预览。"));

		T.Add(TEXT("Ach.Title"), TEXT("成 就"));
		T.Add(TEXT("Ach.Unlocked"), TEXT("成就"));
		T.Add(TEXT("Ach.ProgressFmt"), TEXT("已解锁：%d / %d     积分：%d"));

		T.Add(TEXT("Diff.Peaceful.Name"), TEXT("和平"));
		T.Add(TEXT("Diff.Peaceful.Desc"), TEXT("没有怪物与压力，只有探索和世界。"));
		T.Add(TEXT("Diff.Easy.Name"), TEXT("简单"));
		T.Add(TEXT("Diff.Easy.Desc"), TEXT("开阔宽裕的场所，一只怪物出现得很晚。"));
		T.Add(TEXT("Diff.Normal.Name"), TEXT("普通"));
		T.Add(TEXT("Diff.Normal.Desc"), TEXT("预期的平衡：狭窄走廊、饥饿、一只孤立的实体。"));
		T.Add(TEXT("Diff.Hard.Name"), TEXT("困难"));
		T.Add(TEXT("Diff.Hard.Desc"), TEXT("更多墙、更少门，战利品稀少，两只实体。"));
		T.Add(TEXT("Diff.Nightmare.Name"), TEXT("噩梦"));
		T.Add(TEXT("Diff.Nightmare.Desc"), TEXT("陷阱迷宫、物资极少、三只实体，死亡即终结。"));

		T.Add(TEXT("HUD.Sanity"), TEXT("理智"));
		T.Add(TEXT("HUD.Flashlight"), TEXT("手电筒"));
		T.Add(TEXT("HUD.Hunger"), TEXT("饥饿"));
		T.Add(TEXT("HUD.Thirst"), TEXT("口渴"));
		T.Add(TEXT("HUD.Health"), TEXT("生命"));
		T.Add(TEXT("HUD.Stamina"), TEXT("耐力"));
		T.Add(TEXT("HUD.Inventory"), TEXT("背包"));
		T.Add(TEXT("HUD.Empty"), TEXT("空"));
		T.Add(TEXT("Prog.Level"), TEXT("等级"));
		T.Add(TEXT("Prog.LevelUp"), TEXT("升级"));

		T.Add(TEXT("Interact.Inspect"), TEXT("查看"));
		T.Add(TEXT("Interact.Throw"), TEXT("投掷"));
		T.Add(TEXT("Interact.Drop"), TEXT("放下"));
		T.Add(TEXT("Interact.Take"), TEXT("拾取"));
		T.Add(TEXT("Interact.Push"), TEXT("推动"));
		T.Add(TEXT("Interact.Use"), TEXT("使用"));
		T.Add(TEXT("Interact.Pickup"), TEXT("拾起"));

		T.Add(TEXT("Hint.Critical"), TEXT("状态危急——立刻找到药物"));
		T.Add(TEXT("Hint.Sanity"), TEXT("理智濒临崩溃——躲起来喘口气"));
		T.Add(TEXT("Hint.Hunger"), TEXT("极度饥饿——去找食物"));
		T.Add(TEXT("Hint.Thirst"), TEXT("极度口渴——喝点水"));
		T.Add(TEXT("Hint.Monster"), TEXT("它就在附近——别出声"));
		T.Add(TEXT("Hint.Poison"), TEXT("你中毒了——需要杏仁水或急救包"));
		T.Add(TEXT("Hint.Radiation"), TEXT("辐射过高——离开这里"));
		T.Add(TEXT("Hint.Sleep"), TEXT("你已精疲力竭——停下休息"));
		T.Add(TEXT("Hint.Battery"), TEXT("手电筒快没电了"));

		T.Add(TEXT("Ach.FirstSteps.Name"), TEXT("第一步"));
		T.Add(TEXT("Ach.FirstSteps.Desc"), TEXT("进入 Backrooms。"));
		T.Add(TEXT("Ach.Tourist.Name"), TEXT("游客"));
		T.Add(TEXT("Ach.Tourist.Desc"), TEXT("到访 3 个不同的地点。"));
		T.Add(TEXT("Ach.Explorer.Name"), TEXT("探索者"));
		T.Add(TEXT("Ach.Explorer.Desc"), TEXT("到访 5 个不同的地点。"));
		T.Add(TEXT("Ach.DeepDiver.Name"), TEXT("深处"));
		T.Add(TEXT("Ach.DeepDiver.Desc"), TEXT("到访 7 个不同的地点。"));
		T.Add(TEXT("Ach.AllLevels.Name"), TEXT("整层"));
		T.Add(TEXT("Ach.AllLevels.Desc"), TEXT("到访全部 10 个地点。"));
		T.Add(TEXT("Ach.Survivor.Name"), TEXT("幸存者"));
		T.Add(TEXT("Ach.Survivor.Desc"), TEXT("在 Backrooms 中坚持 15 分钟。"));
		T.Add(TEXT("Ach.Marathon.Name"), TEXT("马拉松"));
		T.Add(TEXT("Ach.Marathon.Desc"), TEXT("在地下行走 3 公里。"));
		T.Add(TEXT("Ach.Scavenger.Name"), TEXT("拾荒者"));
		T.Add(TEXT("Ach.Scavenger.Desc"), TEXT("拾取 10 件物品。"));
		T.Add(TEXT("Ach.Pharmacist.Name"), TEXT("药剂师"));
		T.Add(TEXT("Ach.Pharmacist.Desc"), TEXT("使用 5 次药物。"));
		T.Add(TEXT("Ach.Hydrated.Name"), TEXT("杏仁味"));
		T.Add(TEXT("Ach.Hydrated.Desc"), TEXT("喝下 5 份杏仁水。"));
		T.Add(TEXT("Ach.MonsterAware.Name"), TEXT("它就在附近"));
		T.Add(TEXT("Ach.MonsterAware.Desc"), TEXT("遭遇实体并存活。"));
		T.Add(TEXT("Ach.EscapeArtist.Name"), TEXT("首次出口"));
		T.Add(TEXT("Ach.EscapeArtist.Desc"), TEXT("找到出口。"));
		T.Add(TEXT("Ach.FrequentFlyer.Name"), TEXT("常客"));
		T.Add(TEXT("Ach.FrequentFlyer.Desc"), TEXT("通过 5 个出口。"));
		T.Add(TEXT("Ach.DeepFear.Name"), TEXT("恐惧极限"));
		T.Add(TEXT("Ach.DeepFear.Desc"), TEXT("将环境压力推至最高。"));
		T.Add(TEXT("Ach.Dead.Name"), TEXT("他们中的一员"));
		T.Add(TEXT("Ach.Dead.Desc"), TEXT("死在 Backrooms 中。"));

		T.Add(TEXT("Bind.MoveForwardPlus"), TEXT("前进"));
		T.Add(TEXT("Bind.MoveForwardMinus"), TEXT("后退"));
		T.Add(TEXT("Bind.MoveRightPlus"), TEXT("向右"));
		T.Add(TEXT("Bind.MoveRightMinus"), TEXT("向左"));
		T.Add(TEXT("Bind.Jump"), TEXT("跳跃"));
		T.Add(TEXT("Bind.Sprint"), TEXT("奔跑"));
		T.Add(TEXT("Bind.Flashlight"), TEXT("手电筒"));
		T.Add(TEXT("Bind.View"), TEXT("第一 / 第三人称"));
		T.Add(TEXT("Bind.Attack"), TEXT("挥拳攻击"));
		T.Add(TEXT("Bind.Grab"), TEXT("拿起 / 放下物品"));
		T.Add(TEXT("Bind.Push"), TEXT("推动物品"));
		T.Add(TEXT("Bind.Throw"), TEXT("投掷物品"));
		T.Add(TEXT("Bind.Inspect"), TEXT("检查物品"));
		T.Add(TEXT("Bind.Use"), TEXT("使用物品"));
		T.Add(TEXT("Bind.Slot1"), TEXT("栏位 1"));
		T.Add(TEXT("Bind.Slot2"), TEXT("栏位 2"));
		T.Add(TEXT("Bind.Slot3"), TEXT("栏位 3"));
		T.Add(TEXT("Bind.Slot4"), TEXT("栏位 4"));
		T.Add(TEXT("Bind.Inventory"), TEXT("物品栏"));
		T.Add(TEXT("Bind.PauseMenu"), TEXT("暂停 / 菜单"));
		T.Add(TEXT("Bind.Mouse"), TEXT("鼠标"));
		T.Add(TEXT("Bind.Look"), TEXT("环视"));
		T.Add(TEXT("Bind.Change"), TEXT("更改"));
		T.Add(TEXT("Bind.PressKey"), TEXT("按任意键进行绑定...（Esc 取消）"));
		T.Add(TEXT("Bind.Cancel"), TEXT("取 消  ×"));
		T.Add(TEXT("Bind.Reset"), TEXT("将所有按键恢复默认"));

		T.Add(TEXT("Ach.WellFed.Name"), TEXT("温饱"));
		T.Add(TEXT("Ach.WellFed.Desc"), TEXT("食用 10 份食物。"));
		T.Add(TEXT("Ach.Electrician.Name"), TEXT("电工"));
		T.Add(TEXT("Ach.Electrician.Desc"), TEXT("为手电筒安装 10 节电池。"));
		T.Add(TEXT("Ach.Sprinter.Name"), TEXT("短跑选手"));
		T.Add(TEXT("Ach.Sprinter.Desc"), TEXT("开始奔跑 50 次。"));
		T.Add(TEXT("Ach.Cartographer.Name"), TEXT("制图师"));
		T.Add(TEXT("Ach.Cartographer.Desc"), TEXT("穿过 200 个房间。"));
		T.Add(TEXT("Ach.Speedrun.Name"), TEXT("速通玩家"));
		T.Add(TEXT("Ach.Speedrun.Desc"), TEXT("在地下行走 5 公里。"));
		T.Add(TEXT("Ach.Gourmand.Name"), TEXT("美食家"));
		T.Add(TEXT("Ach.Gourmand.Desc"), TEXT("在 Backrooms 中坚持 30 分钟。"));

		T.Add(TEXT("GameOver.Title"), TEXT("你 死 了"));
		T.Add(TEXT("GameOver.Subtitle"), TEXT("Backrooms 不会放手。再试一次。"));
		T.Add(TEXT("GameOver.Restart"), TEXT("重新开始"));
		T.Add(TEXT("GameOver.Quit"), TEXT("退出"));

		T.Add(TEXT("Level.W0"), TEXT("L0 · 大堂"));
		T.Add(TEXT("Level.W1"), TEXT("L1 · 居住区"));
		T.Add(TEXT("Level.W2"), TEXT("L2 · 水管系统"));
		T.Add(TEXT("Level.W3"), TEXT("L3 · 发电站"));
		T.Add(TEXT("Level.W4"), TEXT("L4 · 办公室"));
		T.Add(TEXT("Level.W5"), TEXT("L5 · 酒店"));
		T.Add(TEXT("Level.W6"), TEXT("L6 · 黑暗"));
		T.Add(TEXT("Level.W7"), TEXT("L7 · 海洋"));
		T.Add(TEXT("Level.W8"), TEXT("L8 · 洞穴"));
		T.Add(TEXT("Level.W9"), TEXT("L9 · 医院"));

		T.Add(TEXT("Item.AlmondWater.Name"), TEXT("杏仁水"));
		T.Add(TEXT("Item.AlmondWater.Desc"), TEXT("解渴并略微恢复理智。"));
		T.Add(TEXT("Item.CanFood.Name"), TEXT("罐头"));
		T.Add(TEXT("Item.CanFood.Desc"), TEXT("罐头肉。充饥。"));
		T.Add(TEXT("Item.MedKit.Name"), TEXT("急救包"));
		T.Add(TEXT("Item.MedKit.Desc"), TEXT("止血。恢复生命。"));
		T.Add(TEXT("Item.Pill.Name"), TEXT("镇静剂"));
		T.Add(TEXT("Item.Pill.Desc"), TEXT("舒缓神经。恢复理智。"));
		T.Add(TEXT("Item.Energy.Name"), TEXT("能量饮料"));
		T.Add(TEXT("Item.Energy.Desc"), TEXT("提神并略微清醒头脑。"));
		T.Add(TEXT("Item.Battery.Name"), TEXT("电池"));
		T.Add(TEXT("Item.Battery.Desc"), TEXT("手电筒的电源。"));

		T.Add(TEXT("Event.LightFlicker"), TEXT("灯光闪烁"));
		T.Add(TEXT("Event.LightOutage"), TEXT("灯灭了！"));
		T.Add(TEXT("Event.BoxDisappear"), TEXT("箱子消失了……"));
		T.Add(TEXT("Event.BoxAppear"), TEXT("出现了一个箱子！"));
		T.Add(TEXT("Event.EntityGrowl"), TEXT("黑暗中传来低吼……"));
		T.Add(TEXT("Event.EntityFootsteps"), TEXT("墙后有脚步声……"));
		T.Add(TEXT("Event.Whisper"), TEXT("低语……"));
		T.Add(TEXT("Event.DistantBang"), TEXT("远处传来巨响！"));
		T.Add(TEXT("Event.WallDrawing"), TEXT("墙上的涂鸦……"));
		T.Add(TEXT("Event.PipeCreak"), TEXT("管道吱嘎作响"));
		T.Add(TEXT("Event.DoorSlam"), TEXT("门砰地关上！"));
		T.Add(TEXT("Event.EmergencyLight"), TEXT("应急灯"));
		T.Add(TEXT("Event.FogIncrease"), TEXT("雾气变浓……"));
		T.Add(TEXT("Event.StaticNoise"), TEXT("杂音……"));
		T.Add(TEXT("Event.FootprintAppear"), TEXT("地上的脚印……"));
		T.Add(TEXT("Event.Unknown"), TEXT("未知事件"));

		T.Add(TEXT("Loading.Level"), TEXT("正在加载关卡……"));
		T.Add(TEXT("Loading.World"), TEXT("正在加载 Backrooms…… %d%%（区块：%d）"));
		T.Add(TEXT("Loading.Ready"), TEXT("世界就绪！现实故障……"));
		T.Add(TEXT("Loading.Loaded"), TEXT("关卡已加载"));
		T.Add(TEXT("Loading.Noclip"), TEXT("穿墙模式：坠入 Backrooms……"));
		T.Add(TEXT("Loading.Backrooms"), TEXT("正在加载 Backrooms……"));
		T.Add(TEXT("Loading.Chunks"), TEXT("区块"));
		T.Add(TEXT("Loading.WorldReady"), TEXT("世界就绪！现实错乱……"));
		T.Add(TEXT("Loading.LevelDone"), TEXT("已加载关卡"));

		T.Add(TEXT("Keys.SpaceBar"), TEXT("空格"));
		T.Add(TEXT("Keys.LeftShift"), TEXT("Shift"));
		T.Add(TEXT("Keys.RightShift"), TEXT("Shift（右）"));
		T.Add(TEXT("Keys.LeftCtrl"), TEXT("Ctrl"));
		T.Add(TEXT("Keys.RightCtrl"), TEXT("Ctrl（右）"));
		T.Add(TEXT("Keys.LeftAlt"), TEXT("Alt"));
		T.Add(TEXT("Keys.RightAlt"), TEXT("AltGr"));
		T.Add(TEXT("Keys.Escape"), TEXT("Esc"));
		T.Add(TEXT("Keys.LeftMouse"), TEXT("左键"));
		T.Add(TEXT("Keys.RightMouse"), TEXT("右键"));
		T.Add(TEXT("Keys.MiddleMouse"), TEXT("滚轮（按键）"));
		T.Add(TEXT("Keys.ThumbMouse1"), TEXT("侧键 1"));
		T.Add(TEXT("Keys.ThumbMouse2"), TEXT("侧键 2"));
		T.Add(TEXT("Keys.MouseX"), TEXT("鼠标 X"));
		T.Add(TEXT("Keys.MouseY"), TEXT("鼠标 Y"));
		T.Add(TEXT("Keys.MouseWheel"), TEXT("鼠标滚轮"));
		T.Add(TEXT("Keys.Backspace"), TEXT("退格"));
		T.Add(TEXT("Keys.Tab"), TEXT("Tab"));
		T.Add(TEXT("Keys.Enter"), TEXT("回车"));
		T.Add(TEXT("Keys.On"), TEXT("开"));
		T.Add(TEXT("Keys.Off"), TEXT("关"));

		T.Add(TEXT("Set.DisplayMode.Fullscreen"), TEXT("全屏"));
		T.Add(TEXT("Set.DisplayMode.Borderless"), TEXT("无边框"));
		T.Add(TEXT("Set.DisplayMode.Windowed"), TEXT("窗口化"));
		T.Add(TEXT("Set.RecordingEffect.Off"), TEXT("关闭"));
		T.Add(TEXT("Set.RecordingEffect.Light"), TEXT("轻度胶片颗粒"));
		T.Add(TEXT("Set.RecordingEffect.Medium"), TEXT("中度胶片颗粒"));
		T.Add(TEXT("Set.RecordingEffect.Strong"), TEXT("重度胶片颗粒"));
		T.Add(TEXT("Set.Quality.Overall"), TEXT("总体质量"));
		T.Add(TEXT("Set.PotatoMode"), TEXT("土豆模式"));
		T.Add(TEXT("Set.PotatoMode.Hint"), TEXT("牺牲画质换取更高帧数。"));
		T.Add(TEXT("Set.Graphics"), TEXT("图形"));
		T.Add(TEXT("Q.Low"), TEXT("低"));
		T.Add(TEXT("Q.Medium"), TEXT("中"));
		T.Add(TEXT("Q.High"), TEXT("高"));
		T.Add(TEXT("Q.Epic"), TEXT("史诗"));
		T.Add(TEXT("Q.Ultra"), TEXT("极致"));

		T.Add(TEXT("Pause.Resume"), TEXT("继续"));
		T.Add(TEXT("Pause.Restart"), TEXT("重新开始"));
		T.Add(TEXT("Pause.Quit"), TEXT("退出游戏"));

		T.Add(TEXT("Interact.TakeOff"), TEXT("收起"));

		T.Add(TEXT("Menu.StressTest"), TEXT("压力测试"));
		T.Add(TEXT("HUD.Fps"), TEXT("FPS"));
		T.Add(TEXT("HUD.FpsAvg"), TEXT("平均"));
		T.Add(TEXT("HUD.FpsOneLow"), TEXT("1% 低"));
		T.Add(TEXT("Loading.StressTest"), TEXT("压力测试..."));
	}
}
