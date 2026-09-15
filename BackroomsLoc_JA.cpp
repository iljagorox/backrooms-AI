#include "BackroomsLocRegistry.h"

// 日本語.
namespace BackroomsLoc
{
	void Register_JA(FLanguageTable& T)
	{
		T.Add(TEXT("Menu.NewGame"), TEXT("新しいゲーム"));
		T.Add(TEXT("Menu.Continue"), TEXT("続ける"));
		T.Add(TEXT("Menu.Levels"), TEXT("レベル"));
		T.Add(TEXT("Menu.Achievements"), TEXT("実績"));
		T.Add(TEXT("Menu.Settings"), TEXT("設定"));
		T.Add(TEXT("Menu.Controls"), TEXT("操作"));
		T.Add(TEXT("Menu.Quit"), TEXT("終了"));
		T.Add(TEXT("Menu.Subtitle"), TEXT("リミナル・ダイブ"));
		T.Add(TEXT("Menu.Back"), TEXT("戻る   >"));

		T.Add(TEXT("Tab.Screen"), TEXT("画面"));
		T.Add(TEXT("Tab.Quality"), TEXT("画質"));
		T.Add(TEXT("Tab.Camera"), TEXT("カメラ"));
		T.Add(TEXT("Tab.Sound"), TEXT("サウンド"));
		T.Add(TEXT("Tab.Game"), TEXT("ゲーム"));
		T.Add(TEXT("Tab.Controls"), TEXT("操作"));
		T.Add(TEXT("Settings.Title"), TEXT("設 定"));
		T.Add(TEXT("Settings.Categories"), TEXT("カ テ ゴ リ"));

		T.Add(TEXT("Set.Resolution"), TEXT("解像度"));
		T.Add(TEXT("Set.DisplayMode"), TEXT("表示モード"));
		T.Add(TEXT("Set.VSync"), TEXT("垂直同期"));
		T.Add(TEXT("Set.RenderScale"), TEXT("レンダースケール"));
		T.Add(TEXT("Set.Gamma"), TEXT("ガンマ"));
		T.Add(TEXT("Set.Language"), TEXT("言語"));

		T.Add(TEXT("Set.Shadows"), TEXT("影"));
		T.Add(TEXT("Set.Textures"), TEXT("テクスチャ"));
		T.Add(TEXT("Set.AntiAliasing"), TEXT("アンチエイリアス"));
		T.Add(TEXT("Set.PostProcess"), TEXT("ポストプロセス"));
		T.Add(TEXT("Set.Effects"), TEXT("エフェクト"));
		T.Add(TEXT("Set.ViewDistance"), TEXT("表示距離"));
		T.Add(TEXT("Set.DepthOfField"), TEXT("被写界深度"));
		T.Add(TEXT("Set.Bloom"), TEXT("ブルーム"));
		T.Add(TEXT("Set.Lumen"), TEXT("Lumen（照明）"));
		T.Add(TEXT("Set.Upscale"), TEXT("DLSS / アップスケール"));

		T.Add(TEXT("Set.FOV"), TEXT("視野角"));
		T.Add(TEXT("Set.Sensitivity"), TEXT("感度"));
		T.Add(TEXT("Set.RecordingEffect"), TEXT("録画カメラ効果"));

		T.Add(TEXT("Set.Volume.Master"), TEXT("全体"));
		T.Add(TEXT("Set.Volume.Menu"), TEXT("音楽 / メニュー"));
		T.Add(TEXT("Set.Volume.Game"), TEXT("ゲーム内サウンド"));
		T.Add(TEXT("Set.Volume.Monster"), TEXT("モンスターの音"));
		T.Add(TEXT("Set.Volume.Other"), TEXT("その他の音"));

		T.Add(TEXT("Set.Difficulty"), TEXT("難易度"));
		T.Add(TEXT("Set.Difficulty.Hint"),
			TEXT("難易度は生成に影響します：壁の密度、通路の数、物資と戦利品の量。"));

		T.Add(TEXT("Levels.Title"), TEXT("レ ベ ル"));
		T.Add(TEXT("Levels.Locked"), TEXT("ロック中"));
		T.Add(TEXT("Levels.UnlockedFmt"),
			TEXT("解放済みレベル：%d / 10。新しいレベルは降りるほど開きます——深く進んでプレビューを見よう。"));

		T.Add(TEXT("Ach.Title"), TEXT("実 績"));
		T.Add(TEXT("Ach.Unlocked"), TEXT("実績"));
		T.Add(TEXT("Ach.ProgressFmt"), TEXT("解放：%d / %d     ポイント：%d"));

		T.Add(TEXT("Diff.Peaceful.Name"), TEXT("平和"));
		T.Add(TEXT("Diff.Peaceful.Desc"), TEXT("モンスターも圧力もない。探索と世界だけ。"));
		T.Add(TEXT("Diff.Easy.Name"), TEXT("簡単"));
		T.Add(TEXT("Diff.Easy.Desc"), TEXT("広く裕福な場所。モンスターは遅れて現れる。"));
		T.Add(TEXT("Diff.Normal.Name"), TEXT("普通"));
		T.Add(TEXT("Diff.Normal.Desc"), TEXT("意図されたバランス：狭い廊下、飢え、孤独な存在。"));
		T.Add(TEXT("Diff.Hard.Name"), TEXT("難しい"));
		T.Add(TEXT("Diff.Hard.Desc"), TEXT("壁が多く扉が少ない、乏しい戦利品、二体の存在。"));
		T.Add(TEXT("Diff.Nightmare.Name"), TEXT("悪夢"));
		T.Add(TEXT("Diff.Nightmare.Desc"), TEXT("罠の迷宮、最低限の物資、三体の存在、死は永遠。"));

		T.Add(TEXT("HUD.Sanity"), TEXT("正気"));
		T.Add(TEXT("HUD.Flashlight"), TEXT("懐中電灯"));
		T.Add(TEXT("HUD.Hunger"), TEXT("空腹"));
		T.Add(TEXT("HUD.Thirst"), TEXT("喉の渇き"));
		T.Add(TEXT("HUD.Health"), TEXT("体力"));
		T.Add(TEXT("HUD.Stamina"), TEXT("スタミナ"));
		T.Add(TEXT("HUD.Inventory"), TEXT("インベントリ"));
		T.Add(TEXT("HUD.Empty"), TEXT("空"));
		T.Add(TEXT("Prog.Level"), TEXT("レベル"));
		T.Add(TEXT("Prog.LevelUp"), TEXT("レベルアップ"));

		T.Add(TEXT("Interact.Inspect"), TEXT("調べる"));
		T.Add(TEXT("Interact.Throw"), TEXT("投げる"));
		T.Add(TEXT("Interact.Drop"), TEXT("置く"));
		T.Add(TEXT("Interact.Take"), TEXT("拾う"));
		T.Add(TEXT("Interact.Push"), TEXT("押す"));
		T.Add(TEXT("Interact.Use"), TEXT("使用"));
		T.Add(TEXT("Interact.Pickup"), TEXT("拾い取る"));

		T.Add(TEXT("Hint.Critical"), TEXT("危急状態——すぐに薬を探せ"));
		T.Add(TEXT("Hint.Sanity"), TEXT("正気が尽きる——隠れて息を整えろ"));
		T.Add(TEXT("Hint.Hunger"), TEXT("ひどい空腹——食べ物を探せ"));
		T.Add(TEXT("Hint.Thirst"), TEXT("ひどい渇き——水を飲め"));
		T.Add(TEXT("Hint.Monster"), TEXT("近くにいる——音を立てるな"));
		T.Add(TEXT("Hint.Poison"), TEXT("毒を受けた——アーモンドウォーターか救急箱を探せ"));
		T.Add(TEXT("Hint.Radiation"), TEXT("放射線が高い——ここを離れろ"));
		T.Add(TEXT("Hint.Sleep"), TEXT("疲れ果てている——止まって休め"));
		T.Add(TEXT("Hint.Battery"), TEXT("懐中電灯がほぼ切れている"));

		T.Add(TEXT("Ach.FirstSteps.Name"), TEXT("最初の一歩"));
		T.Add(TEXT("Ach.FirstSteps.Desc"), TEXT("Backrooms へ降りる。"));
		T.Add(TEXT("Ach.Tourist.Name"), TEXT("観光客"));
		T.Add(TEXT("Ach.Tourist.Desc"), TEXT("3 つの異なる場所を訪れる。"));
		T.Add(TEXT("Ach.Explorer.Name"), TEXT("探検家"));
		T.Add(TEXT("Ach.Explorer.Desc"), TEXT("5 つの異なる場所を訪れる。"));
		T.Add(TEXT("Ach.DeepDiver.Name"), TEXT("深部へ"));
		T.Add(TEXT("Ach.DeepDiver.Desc"), TEXT("7 つの異なる場所を訪れる。"));
		T.Add(TEXT("Ach.AllLevels.Name"), TEXT("全フロア"));
		T.Add(TEXT("Ach.AllLevels.Desc"), TEXT("10 か所すべてを訪れる。"));
		T.Add(TEXT("Ach.Survivor.Name"), TEXT("生存者"));
		T.Add(TEXT("Ach.Survivor.Desc"), TEXT("Backrooms で 15 分間耐える。"));
		T.Add(TEXT("Ach.Marathon.Name"), TEXT("マラソン"));
		T.Add(TEXT("Ach.Marathon.Desc"), TEXT("地下を 3 キロ歩く。"));
		T.Add(TEXT("Ach.Scavenger.Name"), TEXT("収集家"));
		T.Add(TEXT("Ach.Scavenger.Desc"), TEXT("10 個のアイテムを拾う。"));
		T.Add(TEXT("Ach.Pharmacist.Name"), TEXT("薬剤師"));
		T.Add(TEXT("Ach.Pharmacist.Desc"), TEXT("薬を 5 回使う。"));
		T.Add(TEXT("Ach.Hydrated.Name"), TEXT("アーモンドの味"));
		T.Add(TEXT("Ach.Hydrated.Desc"), TEXT("アーモンドウォーターを 5 回飲む。"));
		T.Add(TEXT("Ach.MonsterAware.Name"), TEXT("近くにいる"));
		T.Add(TEXT("Ach.MonsterAware.Desc"), TEXT("存在と遭遇して生き延びる。"));
		T.Add(TEXT("Ach.EscapeArtist.Name"), TEXT("最初の出口"));
		T.Add(TEXT("Ach.EscapeArtist.Desc"), TEXT("出口を見つける。"));
		T.Add(TEXT("Ach.FrequentFlyer.Name"), TEXT("常連"));
		T.Add(TEXT("Ach.FrequentFlyer.Desc"), TEXT("5 つの出口を通る。"));
		T.Add(TEXT("Ach.DeepFear.Name"), TEXT("恐怖の限界"));
		T.Add(TEXT("Ach.DeepFear.Desc"), TEXT("環境圧力を最大にする。"));
		T.Add(TEXT("Ach.Dead.Name"), TEXT("彼らの一人"));
		T.Add(TEXT("Ach.Dead.Desc"), TEXT("Backrooms で死ぬ。"));

		T.Add(TEXT("Bind.MoveForwardPlus"), TEXT("前進"));
		T.Add(TEXT("Bind.MoveForwardMinus"), TEXT("後退"));
		T.Add(TEXT("Bind.MoveRightPlus"), TEXT("右"));
		T.Add(TEXT("Bind.MoveRightMinus"), TEXT("左"));
		T.Add(TEXT("Bind.Jump"), TEXT("ジャンプ"));
		T.Add(TEXT("Bind.Sprint"), TEXT("ダッシュ"));
		T.Add(TEXT("Bind.Flashlight"), TEXT("懐中電灯"));
		T.Add(TEXT("Bind.View"), TEXT("一人称 / 三人称"));
		T.Add(TEXT("Bind.Attack"), TEXT("パンチ"));
		T.Add(TEXT("Bind.Grab"), TEXT("アイテムを取る / 置く"));
		T.Add(TEXT("Bind.Push"), TEXT("アイテムを押す"));
		T.Add(TEXT("Bind.Throw"), TEXT("アイテムを投げる"));
		T.Add(TEXT("Bind.Inspect"), TEXT("アイテムを調べる"));
		T.Add(TEXT("Bind.Use"), TEXT("アイテムを使う"));
		T.Add(TEXT("Bind.Slot1"), TEXT("スロット 1"));
		T.Add(TEXT("Bind.Slot2"), TEXT("スロット 2"));
		T.Add(TEXT("Bind.Slot3"), TEXT("スロット 3"));
		T.Add(TEXT("Bind.Slot4"), TEXT("スロット 4"));
		T.Add(TEXT("Bind.Inventory"), TEXT("インベントリ"));
		T.Add(TEXT("Bind.PauseMenu"), TEXT("ポーズ / メニュー"));
		T.Add(TEXT("Bind.Mouse"), TEXT("マウス"));
		T.Add(TEXT("Bind.Look"), TEXT("見回す"));
		T.Add(TEXT("Bind.Change"), TEXT("変更"));
		T.Add(TEXT("Bind.PressKey"), TEXT("割り当てるキーを押してください...（Esc でキャンセル）"));
		T.Add(TEXT("Bind.Cancel"), TEXT("キ ャ ン セ ル  ×"));
		T.Add(TEXT("Bind.Reset"), TEXT("すべてのキーを初期化"));

		T.Add(TEXT("Ach.WellFed.Name"), TEXT("満腹"));
		T.Add(TEXT("Ach.WellFed.Desc"), TEXT("食べ物を 10 回食べる。"));
		T.Add(TEXT("Ach.Electrician.Name"), TEXT("電気技師"));
		T.Add(TEXT("Ach.Electrician.Desc"), TEXT("懐中電灯に電池を 10 回入れる。"));
		T.Add(TEXT("Ach.Sprinter.Name"), TEXT("スプリンター"));
		T.Add(TEXT("Ach.Sprinter.Desc"), TEXT("ダッシュを 50 回開始する。"));
		T.Add(TEXT("Ach.Cartographer.Name"), TEXT("製図家"));
		T.Add(TEXT("Ach.Cartographer.Desc"), TEXT("200 の部屋を通過する。"));
		T.Add(TEXT("Ach.Speedrun.Name"), TEXT("スピードランナー"));
		T.Add(TEXT("Ach.Speedrun.Desc"), TEXT("地下を 5 キロ歩く。"));
		T.Add(TEXT("Ach.Gourmand.Name"), TEXT("美食家"));
		T.Add(TEXT("Ach.Gourmand.Desc"), TEXT("Backrooms で 30 分耐える。"));

		T.Add(TEXT("GameOver.Title"), TEXT("あ な た は 死 ん だ"));
		T.Add(TEXT("GameOver.Subtitle"), TEXT("Backrooms は離してくれない。もう一度挑戦しよう。"));
		T.Add(TEXT("GameOver.Restart"), TEXT("やり直す"));
		T.Add(TEXT("GameOver.Quit"), TEXT("終了"));

		T.Add(TEXT("Level.W0"), TEXT("L0 · ロビー"));
		T.Add(TEXT("Level.W1"), TEXT("L1 · 居住エリア"));
		T.Add(TEXT("Level.W2"), TEXT("L2 · 水道設備"));
		T.Add(TEXT("Level.W3"), TEXT("L3 · 発電所"));
		T.Add(TEXT("Level.W4"), TEXT("L4 · オフィス"));
		T.Add(TEXT("Level.W5"), TEXT("L5 · ホテル"));
		T.Add(TEXT("Level.W6"), TEXT("L6 · 闇"));
		T.Add(TEXT("Level.W7"), TEXT("L7 · 海洋"));
		T.Add(TEXT("Level.W8"), TEXT("L8 · 洞窟"));
		T.Add(TEXT("Level.W9"), TEXT("L9 · 病院"));

		T.Add(TEXT("Item.AlmondWater.Name"), TEXT("アーモンドウォーター"));
		T.Add(TEXT("Item.AlmondWater.Desc"), TEXT("のどを潤し、正気を少し回復する。"));
		T.Add(TEXT("Item.CanFood.Name"), TEXT("缶詰"));
		T.Add(TEXT("Item.CanFood.Desc"), TEXT("缶詰の肉。空腹を満たす。"));
		T.Add(TEXT("Item.MedKit.Name"), TEXT("救急キット"));
		T.Add(TEXT("Item.MedKit.Desc"), TEXT("出血を止める。体力を回復する。"));
		T.Add(TEXT("Item.Pill.Name"), TEXT("鎮静剤"));
		T.Add(TEXT("Item.Pill.Desc"), TEXT("神経を落ち着かせる。正気を回復する。"));
		T.Add(TEXT("Item.Energy.Name"), TEXT("エナジードリンク"));
		T.Add(TEXT("Item.Energy.Desc"), TEXT("元気が出て、頭が少しすっきりする。"));
		T.Add(TEXT("Item.Battery.Name"), TEXT("バッテリー"));
		T.Add(TEXT("Item.Battery.Desc"), TEXT("懐中電灯用の電源。"));

		T.Add(TEXT("Event.LightFlicker"), TEXT("明かりがちらつく"));
		T.Add(TEXT("Event.LightOutage"), TEXT("明かりが消えた！"));
		T.Add(TEXT("Event.BoxDisappear"), TEXT("箱が消えた……"));
		T.Add(TEXT("Event.BoxAppear"), TEXT("箱が現れた！"));
		T.Add(TEXT("Event.EntityGrowl"), TEXT("暗闇からうなり声……"));
		T.Add(TEXT("Event.EntityFootsteps"), TEXT("壁の向こうの足音……"));
		T.Add(TEXT("Event.Whisper"), TEXT("ささやき……"));
		T.Add(TEXT("Event.DistantBang"), TEXT("遠くで大きな音！"));
		T.Add(TEXT("Event.WallDrawing"), TEXT("壁の落書き……"));
		T.Add(TEXT("Event.PipeCreak"), TEXT("配管がきしむ"));
		T.Add(TEXT("Event.DoorSlam"), TEXT("扉がバタンと閉まる！"));
		T.Add(TEXT("Event.EmergencyLight"), TEXT("非常灯"));
		T.Add(TEXT("Event.FogIncrease"), TEXT("霧が濃くなる……"));
		T.Add(TEXT("Event.StaticNoise"), TEXT("ノイズ……"));
		T.Add(TEXT("Event.FootprintAppear"), TEXT("床の足跡……"));
		T.Add(TEXT("Event.Unknown"), TEXT("未知のイベント"));

		T.Add(TEXT("Loading.Level"), TEXT("レベルを読み込み中……"));
		T.Add(TEXT("Loading.World"), TEXT("Backrooms を読み込み中…… %d%%（チャンク: %d）"));
		T.Add(TEXT("Loading.Ready"), TEXT("世界の準備完了！現実の異常……"));
		T.Add(TEXT("Loading.Loaded"), TEXT("レベル読み込み完了"));
		T.Add(TEXT("Loading.Noclip"), TEXT("Noclip: Backrooms へ落下……"));
		T.Add(TEXT("Loading.Backrooms"), TEXT("Backrooms を読み込み中……"));
		T.Add(TEXT("Loading.Chunks"), TEXT("チャンク"));
		T.Add(TEXT("Loading.WorldReady"), TEXT("世界の準備完了！現実の異常……"));
		T.Add(TEXT("Loading.LevelDone"), TEXT("レベルをロードしました"));

		T.Add(TEXT("Keys.SpaceBar"), TEXT("スペース"));
		T.Add(TEXT("Keys.LeftShift"), TEXT("Shift"));
		T.Add(TEXT("Keys.RightShift"), TEXT("Shift（右）"));
		T.Add(TEXT("Keys.LeftCtrl"), TEXT("Ctrl"));
		T.Add(TEXT("Keys.RightCtrl"), TEXT("Ctrl（右）"));
		T.Add(TEXT("Keys.LeftAlt"), TEXT("Alt"));
		T.Add(TEXT("Keys.RightAlt"), TEXT("AltGr"));
		T.Add(TEXT("Keys.Escape"), TEXT("Esc"));
		T.Add(TEXT("Keys.LeftMouse"), TEXT("左クリック"));
		T.Add(TEXT("Keys.RightMouse"), TEXT("右クリック"));
		T.Add(TEXT("Keys.MiddleMouse"), TEXT("ホイール（クリック）"));
		T.Add(TEXT("Keys.ThumbMouse1"), TEXT("サイドボタン 1"));
		T.Add(TEXT("Keys.ThumbMouse2"), TEXT("サイドボタン 2"));
		T.Add(TEXT("Keys.MouseX"), TEXT("マウス X"));
		T.Add(TEXT("Keys.MouseY"), TEXT("マウス Y"));
		T.Add(TEXT("Keys.MouseWheel"), TEXT("マウスホイール"));
		T.Add(TEXT("Keys.Backspace"), TEXT("バックスペース"));
		T.Add(TEXT("Keys.Tab"), TEXT("Tab"));
		T.Add(TEXT("Keys.Enter"), TEXT("Enter"));
		T.Add(TEXT("Keys.On"), TEXT("オン"));
		T.Add(TEXT("Keys.Off"), TEXT("オフ"));

		T.Add(TEXT("Set.DisplayMode.Fullscreen"), TEXT("フルスクリーン"));
		T.Add(TEXT("Set.DisplayMode.Borderless"), TEXT("ボーダーレス"));
		T.Add(TEXT("Set.DisplayMode.Windowed"), TEXT("ウィンドウ"));
		T.Add(TEXT("Set.RecordingEffect.Off"), TEXT("オフ"));
		T.Add(TEXT("Set.RecordingEffect.Light"), TEXT("フィルム風（弱）"));
		T.Add(TEXT("Set.RecordingEffect.Medium"), TEXT("フィルム風（中）"));
		T.Add(TEXT("Set.RecordingEffect.Strong"), TEXT("フィルム風（強）"));
		T.Add(TEXT("Set.Quality.Overall"), TEXT("全体の画質"));
		T.Add(TEXT("Set.PotatoMode"), TEXT("ポテトモード"));
		T.Add(TEXT("Set.PotatoMode.Hint"), TEXT("画質を犠牲にして最大FPS。"));
		T.Add(TEXT("Set.Graphics"), TEXT("グラフィック"));
		T.Add(TEXT("Q.Low"), TEXT("低"));
		T.Add(TEXT("Q.Medium"), TEXT("中"));
		T.Add(TEXT("Q.High"), TEXT("高"));
		T.Add(TEXT("Q.Epic"), TEXT("最高"));
		T.Add(TEXT("Q.Ultra"), TEXT("究極"));

		T.Add(TEXT("Pause.Resume"), TEXT("再開"));
		T.Add(TEXT("Pause.Restart"), TEXT("やり直す"));
		T.Add(TEXT("Pause.Quit"), TEXT("ゲームを終了"));

		T.Add(TEXT("Interact.TakeOff"), TEXT("しまう"));

		T.Add(TEXT("Menu.StressTest"), TEXT("ストレステスト"));
		T.Add(TEXT("HUD.Fps"), TEXT("FPS"));
		T.Add(TEXT("HUD.FpsAvg"), TEXT("平均"));
		T.Add(TEXT("HUD.FpsOneLow"), TEXT("1% Low"));
		T.Add(TEXT("Loading.StressTest"), TEXT("ストレステスト..."));
	}
}
