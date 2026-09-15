#include "BackroomsLocRegistry.h"

// 한국어.
namespace BackroomsLoc
{
	void Register_KO(FLanguageTable& T)
	{
		T.Add(TEXT("Menu.NewGame"), TEXT("새 게임"));
		T.Add(TEXT("Menu.Continue"), TEXT("계속하기"));
		T.Add(TEXT("Menu.Levels"), TEXT("레벨"));
		T.Add(TEXT("Menu.Achievements"), TEXT("업적"));
		T.Add(TEXT("Menu.Settings"), TEXT("설정"));
		T.Add(TEXT("Menu.Controls"), TEXT("조작"));
		T.Add(TEXT("Menu.Quit"), TEXT("종료"));
		T.Add(TEXT("Menu.Subtitle"), TEXT("리미널 다이브"));
		T.Add(TEXT("Menu.Back"), TEXT("뒤로   >"));

		T.Add(TEXT("Tab.Screen"), TEXT("화면"));
		T.Add(TEXT("Tab.Quality"), TEXT("품질"));
		T.Add(TEXT("Tab.Camera"), TEXT("카메라"));
		T.Add(TEXT("Tab.Sound"), TEXT("사운드"));
		T.Add(TEXT("Tab.Game"), TEXT("게임"));
		T.Add(TEXT("Tab.Controls"), TEXT("조작"));
		T.Add(TEXT("Settings.Title"), TEXT("설 정"));
		T.Add(TEXT("Settings.Categories"), TEXT("분 류"));

		T.Add(TEXT("Set.Resolution"), TEXT("해상도"));
		T.Add(TEXT("Set.DisplayMode"), TEXT("화면 모드"));
		T.Add(TEXT("Set.VSync"), TEXT("수직 동기화"));
		T.Add(TEXT("Set.RenderScale"), TEXT("렌더 스케일"));
		T.Add(TEXT("Set.Gamma"), TEXT("감마"));
		T.Add(TEXT("Set.Language"), TEXT("언어"));

		T.Add(TEXT("Set.Shadows"), TEXT("그림자"));
		T.Add(TEXT("Set.Textures"), TEXT("텍스처"));
		T.Add(TEXT("Set.AntiAliasing"), TEXT("안티앨리어싱"));
		T.Add(TEXT("Set.PostProcess"), TEXT("후처리"));
		T.Add(TEXT("Set.Effects"), TEXT("효과"));
		T.Add(TEXT("Set.ViewDistance"), TEXT("시야 거리"));
		T.Add(TEXT("Set.DepthOfField"), TEXT("피사계 심도"));
		T.Add(TEXT("Set.Bloom"), TEXT("블룸"));
		T.Add(TEXT("Set.Lumen"), TEXT("Lumen (조명)"));
		T.Add(TEXT("Set.Upscale"), TEXT("DLSS / 업스케일"));

		T.Add(TEXT("Set.FOV"), TEXT("시야각"));
		T.Add(TEXT("Set.Sensitivity"), TEXT("감도"));
		T.Add(TEXT("Set.RecordingEffect"), TEXT("녹화 카메라 효과"));

		T.Add(TEXT("Set.Volume.Master"), TEXT("전체"));
		T.Add(TEXT("Set.Volume.Menu"), TEXT("음악 / 메뉴"));
		T.Add(TEXT("Set.Volume.Game"), TEXT("게임 사운드"));
		T.Add(TEXT("Set.Volume.Monster"), TEXT("몬스터 소리"));
		T.Add(TEXT("Set.Volume.Other"), TEXT("기타 소리"));

		T.Add(TEXT("Set.Difficulty"), TEXT("난이도"));
		T.Add(TEXT("Set.Difficulty.Hint"),
			TEXT("난이도는 생성에 영향을 줍니다: 벽 밀도, 통로 수, 보급품과 전리품의 풍족함."));

		T.Add(TEXT("Levels.Title"), TEXT("레 벨"));
		T.Add(TEXT("Levels.Locked"), TEXT("잠김"));
		T.Add(TEXT("Levels.UnlockedFmt"),
			TEXT("해금된 레벨: %d / 10. 새 레벨은 내려갈수록 열립니다 — 더 깊이 가서 미리보기를 확인하세요."));

		T.Add(TEXT("Ach.Title"), TEXT("업 적"));
		T.Add(TEXT("Ach.Unlocked"), TEXT("업적"));
		T.Add(TEXT("Ach.ProgressFmt"), TEXT("해금: %d / %d     점수: %d"));

		T.Add(TEXT("Diff.Peaceful.Name"), TEXT("평화"));
		T.Add(TEXT("Diff.Peaceful.Desc"), TEXT("몬스터와 압박이 없습니다. 탐험과 세계뿐."));
		T.Add(TEXT("Diff.Easy.Name"), TEXT("쉬움"));
		T.Add(TEXT("Diff.Easy.Desc"), TEXT("넓고 풍족한 장소. 몬스터는 늦게 나타납니다."));
		T.Add(TEXT("Diff.Normal.Name"), TEXT("보통"));
		T.Add(TEXT("Diff.Normal.Desc"), TEXT("의도된 균형: 좁은 복도, 굶주림, 단 하나의 존재."));
		T.Add(TEXT("Diff.Hard.Name"), TEXT("어려움"));
		T.Add(TEXT("Diff.Hard.Desc"), TEXT("벽은 많고 문은 적으며, 전리품은 부족하고, 두 존재."));
		T.Add(TEXT("Diff.Nightmare.Name"), TEXT("악몽"));
		T.Add(TEXT("Diff.Nightmare.Desc"), TEXT("함정 미로, 최소한의 보급, 세 존재, 죽음은 영원."));

		T.Add(TEXT("HUD.Sanity"), TEXT("정신력"));
		T.Add(TEXT("HUD.Flashlight"), TEXT("손전등"));
		T.Add(TEXT("HUD.Hunger"), TEXT("배고픔"));
		T.Add(TEXT("HUD.Thirst"), TEXT("갈증"));
		T.Add(TEXT("HUD.Health"), TEXT("체력"));
		T.Add(TEXT("HUD.Stamina"), TEXT("스태미나"));
		T.Add(TEXT("HUD.Inventory"), TEXT("인벤토리"));
		T.Add(TEXT("HUD.Empty"), TEXT("비어 있음"));
		T.Add(TEXT("Prog.Level"), TEXT("갈베"));
		T.Add(TEXT("Prog.LevelUp"), TEXT("갈베 업"));

		T.Add(TEXT("Interact.Inspect"), TEXT("조사"));
		T.Add(TEXT("Interact.Throw"), TEXT("던지기"));
		T.Add(TEXT("Interact.Drop"), TEXT("내려놓기"));
		T.Add(TEXT("Interact.Take"), TEXT("줍기"));
		T.Add(TEXT("Interact.Push"), TEXT("밀기"));
		T.Add(TEXT("Interact.Use"), TEXT("사용"));
		T.Add(TEXT("Interact.Pickup"), TEXT("집기"));

		T.Add(TEXT("Hint.Critical"), TEXT("위급 상태 — 즉시 약을 찾으세요"));
		T.Add(TEXT("Hint.Sanity"), TEXT("정신력이 바닥 — 숨어서 숨을 고르세요"));
		T.Add(TEXT("Hint.Hunger"), TEXT("심한 배고픔 — 음식을 찾으세요"));
		T.Add(TEXT("Hint.Thirst"), TEXT("심한 갈증 — 물을 마시세요"));
		T.Add(TEXT("Hint.Monster"), TEXT("근처에 있습니다 — 조용히 하세요"));
		T.Add(TEXT("Hint.Poison"), TEXT("중독되었습니다 — 아몬드 워터나 구급함을 찾으세요"));
		T.Add(TEXT("Hint.Radiation"), TEXT("방사선이 높습니다 — 여기서 나가세요"));
		T.Add(TEXT("Hint.Sleep"), TEXT("지쳤습니다 — 멈추고 쉬세요"));
		T.Add(TEXT("Hint.Battery"), TEXT("손전등이 거의 꺼져갑니다"));

		T.Add(TEXT("Ach.FirstSteps.Name"), TEXT("첫 걸음"));
		T.Add(TEXT("Ach.FirstSteps.Desc"), TEXT("Backrooms 로 내려가기."));
		T.Add(TEXT("Ach.Tourist.Name"), TEXT("관광객"));
		T.Add(TEXT("Ach.Tourist.Desc"), TEXT("서로 다른 3곳 방문."));
		T.Add(TEXT("Ach.Explorer.Name"), TEXT("탐험가"));
		T.Add(TEXT("Ach.Explorer.Desc"), TEXT("서로 다른 5곳 방문."));
		T.Add(TEXT("Ach.DeepDiver.Name"), TEXT("심연 속으로"));
		T.Add(TEXT("Ach.DeepDiver.Desc"), TEXT("서로 다른 7곳 방문."));
		T.Add(TEXT("Ach.AllLevels.Name"), TEXT("전체 층"));
		T.Add(TEXT("Ach.AllLevels.Desc"), TEXT("10곳 모두 방문."));
		T.Add(TEXT("Ach.Survivor.Name"), TEXT("생존자"));
		T.Add(TEXT("Ach.Survivor.Desc"), TEXT("Backrooms 에서 15분 버티기."));
		T.Add(TEXT("Ach.Marathon.Name"), TEXT("마라토너"));
		T.Add(TEXT("Ach.Marathon.Desc"), TEXT("지하에서 3킬로미터 이동."));
		T.Add(TEXT("Ach.Scavenger.Name"), TEXT("수집가"));
		T.Add(TEXT("Ach.Scavenger.Desc"), TEXT("아이템 10개 줍기."));
		T.Add(TEXT("Ach.Pharmacist.Name"), TEXT("약사"));
		T.Add(TEXT("Ach.Pharmacist.Desc"), TEXT("약 5회 사용."));
		T.Add(TEXT("Ach.Hydrated.Name"), TEXT("아몬드 맛"));
		T.Add(TEXT("Ach.Hydrated.Desc"), TEXT("아몬드 워터 5회 마시기."));
		T.Add(TEXT("Ach.MonsterAware.Name"), TEXT("근처에 있음"));
		T.Add(TEXT("Ach.MonsterAware.Desc"), TEXT("존재와 조우하고 생존."));
		T.Add(TEXT("Ach.EscapeArtist.Name"), TEXT("첫 출구"));
		T.Add(TEXT("Ach.EscapeArtist.Desc"), TEXT("출구를 찾기."));
		T.Add(TEXT("Ach.FrequentFlyer.Name"), TEXT("단골"));
		T.Add(TEXT("Ach.FrequentFlyer.Desc"), TEXT("출구 5개 통과."));
		T.Add(TEXT("Ach.DeepFear.Name"), TEXT("공포의 한계"));
		T.Add(TEXT("Ach.DeepFear.Desc"), TEXT("환경 압박을 최대로 올리기."));
		T.Add(TEXT("Ach.Dead.Name"), TEXT("그들 중 하나"));
		T.Add(TEXT("Ach.Dead.Desc"), TEXT("Backrooms 에서 죽기."));

		T.Add(TEXT("Bind.MoveForwardPlus"), TEXT("앞으로"));
		T.Add(TEXT("Bind.MoveForwardMinus"), TEXT("뒤로"));
		T.Add(TEXT("Bind.MoveRightPlus"), TEXT("오른쪽"));
		T.Add(TEXT("Bind.MoveRightMinus"), TEXT("왼쪽"));
		T.Add(TEXT("Bind.Jump"), TEXT("점프"));
		T.Add(TEXT("Bind.Sprint"), TEXT("달리기"));
		T.Add(TEXT("Bind.Flashlight"), TEXT("손전등"));
		T.Add(TEXT("Bind.View"), TEXT("1인칭 / 3인칭"));
		T.Add(TEXT("Bind.Attack"), TEXT("주먹 공격"));
		T.Add(TEXT("Bind.Grab"), TEXT("아이템 집기 / 놓기"));
		T.Add(TEXT("Bind.Push"), TEXT("아이템 밀기"));
		T.Add(TEXT("Bind.Throw"), TEXT("아이템 던지기"));
		T.Add(TEXT("Bind.Inspect"), TEXT("아이템 살펴보기"));
		T.Add(TEXT("Bind.Use"), TEXT("아이템 사용"));
		T.Add(TEXT("Bind.Slot1"), TEXT("슬롯 1"));
		T.Add(TEXT("Bind.Slot2"), TEXT("슬롯 2"));
		T.Add(TEXT("Bind.Slot3"), TEXT("슬롯 3"));
		T.Add(TEXT("Bind.Slot4"), TEXT("슬롯 4"));
		T.Add(TEXT("Bind.Inventory"), TEXT("인벤토리"));
		T.Add(TEXT("Bind.PauseMenu"), TEXT("일시정지 / 메뉴"));
		T.Add(TEXT("Bind.Mouse"), TEXT("마우스"));
		T.Add(TEXT("Bind.Look"), TEXT("둘러보기"));
		T.Add(TEXT("Bind.Change"), TEXT("변경"));
		T.Add(TEXT("Bind.PressKey"), TEXT("키를 눌러 지정하세요... (취소: Esc)"));
		T.Add(TEXT("Bind.Cancel"), TEXT("취 소  ×"));
		T.Add(TEXT("Bind.Reset"), TEXT("모든 키를 기본값으로 초기화"));

		T.Add(TEXT("Ach.WellFed.Name"), TEXT("배부름"));
		T.Add(TEXT("Ach.WellFed.Desc"), TEXT("음식을 10회 먹기."));
		T.Add(TEXT("Ach.Electrician.Name"), TEXT("전기공"));
		T.Add(TEXT("Ach.Electrician.Desc"), TEXT("손전등에 배터리 10개 넣기."));
		T.Add(TEXT("Ach.Sprinter.Name"), TEXT("스프린터"));
		T.Add(TEXT("Ach.Sprinter.Desc"), TEXT("달리기 50회 시작."));
		T.Add(TEXT("Ach.Cartographer.Name"), TEXT("지도 제작자"));
		T.Add(TEXT("Ach.Cartographer.Desc"), TEXT("방 200개 통과."));
		T.Add(TEXT("Ach.Speedrun.Name"), TEXT("스피드러너"));
		T.Add(TEXT("Ach.Speedrun.Desc"), TEXT("지하 5킬로미터 이동."));
		T.Add(TEXT("Ach.Gourmand.Name"), TEXT("미식가"));
		T.Add(TEXT("Ach.Gourmand.Desc"), TEXT("Backrooms에서 30분 버티기."));

		T.Add(TEXT("GameOver.Title"), TEXT("당 신 은 죽 었 다"));
		T.Add(TEXT("GameOver.Subtitle"), TEXT("Backrooms는 놓아주지 않는다. 다시 시도하라."));
		T.Add(TEXT("GameOver.Restart"), TEXT("다시 시작"));
		T.Add(TEXT("GameOver.Quit"), TEXT("종료"));

		T.Add(TEXT("Level.W0"), TEXT("L0 · 로비"));
		T.Add(TEXT("Level.W1"), TEXT("L1 · 거주 구역"));
		T.Add(TEXT("Level.W2"), TEXT("L2 · 상수도"));
		T.Add(TEXT("Level.W3"), TEXT("L3 · 발전소"));
		T.Add(TEXT("Level.W4"), TEXT("L4 · 사무실"));
		T.Add(TEXT("Level.W5"), TEXT("L5 · 호텔"));
		T.Add(TEXT("Level.W6"), TEXT("L6 · 어둠"));
		T.Add(TEXT("Level.W7"), TEXT("L7 · 바다"));
		T.Add(TEXT("Level.W8"), TEXT("L8 · 동굴"));
		T.Add(TEXT("Level.W9"), TEXT("L9 · 병원"));

		T.Add(TEXT("Item.AlmondWater.Name"), TEXT("아몬드 워터"));
		T.Add(TEXT("Item.AlmondWater.Desc"), TEXT("갈증을 해소하고 정신을 약간 회복한다."));
		T.Add(TEXT("Item.CanFood.Name"), TEXT("통조림"));
		T.Add(TEXT("Item.CanFood.Desc"), TEXT("통조림 고기. 배고픔을 해소한다."));
		T.Add(TEXT("Item.MedKit.Name"), TEXT("구급 상자"));
		T.Add(TEXT("Item.MedKit.Desc"), TEXT("출혈을 멈춘다. 체력을 회복한다."));
		T.Add(TEXT("Item.Pill.Name"), TEXT("진정제"));
		T.Add(TEXT("Item.Pill.Desc"), TEXT("신경을 진정시킨다. 정신을 회복한다."));
		T.Add(TEXT("Item.Energy.Name"), TEXT("에너지 드링크"));
		T.Add(TEXT("Item.Energy.Desc"), TEXT("기운을 돋우고 머리가 약간 맑아진다."));
		T.Add(TEXT("Item.Battery.Name"), TEXT("배터리"));
		T.Add(TEXT("Item.Battery.Desc"), TEXT("손전등용 전원."));

		T.Add(TEXT("Event.LightFlicker"), TEXT("불빛이 깜빡임"));
		T.Add(TEXT("Event.LightOutage"), TEXT("불이 나갔다!"));
		T.Add(TEXT("Event.BoxDisappear"), TEXT("상자가 사라졌다..."));
		T.Add(TEXT("Event.BoxAppear"), TEXT("상자가 나타났다!"));
		T.Add(TEXT("Event.EntityGrowl"), TEXT("어둠 속에서 으르렁거림..."));
		T.Add(TEXT("Event.EntityFootsteps"), TEXT("벽 너머의 발소리..."));
		T.Add(TEXT("Event.Whisper"), TEXT("속삭임..."));
		T.Add(TEXT("Event.DistantBang"), TEXT("저 멀리서 쾅!"));
		T.Add(TEXT("Event.WallDrawing"), TEXT("벽에 낙서..."));
		T.Add(TEXT("Event.PipeCreak"), TEXT("배관이 삐걱거림"));
		T.Add(TEXT("Event.DoorSlam"), TEXT("문이 쾅 닫힘!"));
		T.Add(TEXT("Event.EmergencyLight"), TEXT("비상등"));
		T.Add(TEXT("Event.FogIncrease"), TEXT("안개가 짙어진다..."));
		T.Add(TEXT("Event.StaticNoise"), TEXT("노이즈..."));
		T.Add(TEXT("Event.FootprintAppear"), TEXT("바닥의 발자국..."));
		T.Add(TEXT("Event.Unknown"), TEXT("알 수 없는 사건"));

		T.Add(TEXT("Loading.Level"), TEXT("레벨 로딩 중..."));
		T.Add(TEXT("Loading.World"), TEXT("Backrooms 로딩 중... %d%% (청크: %d)"));
		T.Add(TEXT("Loading.Ready"), TEXT("세계 준비 완료! 현실 붕괴..."));
		T.Add(TEXT("Loading.Loaded"), TEXT("레벨 로딩 완료"));
		T.Add(TEXT("Loading.Noclip"), TEXT("Noclip: Backrooms로 추락..."));
		T.Add(TEXT("Loading.Backrooms"), TEXT("Backrooms 불러오는 중..."));
		T.Add(TEXT("Loading.Chunks"), TEXT("청크"));
		T.Add(TEXT("Loading.WorldReady"), TEXT("세계 준비 완료! 현실 오류..."));
		T.Add(TEXT("Loading.LevelDone"), TEXT("레벨 로드 완료"));

		T.Add(TEXT("Keys.SpaceBar"), TEXT("스페이스"));
		T.Add(TEXT("Keys.LeftShift"), TEXT("Shift"));
		T.Add(TEXT("Keys.RightShift"), TEXT("Shift (오른쪽)"));
		T.Add(TEXT("Keys.LeftCtrl"), TEXT("Ctrl"));
		T.Add(TEXT("Keys.RightCtrl"), TEXT("Ctrl (오른쪽)"));
		T.Add(TEXT("Keys.LeftAlt"), TEXT("Alt"));
		T.Add(TEXT("Keys.RightAlt"), TEXT("AltGr"));
		T.Add(TEXT("Keys.Escape"), TEXT("Esc"));
		T.Add(TEXT("Keys.LeftMouse"), TEXT("왼쪽 버튼"));
		T.Add(TEXT("Keys.RightMouse"), TEXT("오른쪽 버튼"));
		T.Add(TEXT("Keys.MiddleMouse"), TEXT("휠 (클릭)"));
		T.Add(TEXT("Keys.ThumbMouse1"), TEXT("측면 버튼 1"));
		T.Add(TEXT("Keys.ThumbMouse2"), TEXT("측면 버튼 2"));
		T.Add(TEXT("Keys.MouseX"), TEXT("마우스 X"));
		T.Add(TEXT("Keys.MouseY"), TEXT("마우스 Y"));
		T.Add(TEXT("Keys.MouseWheel"), TEXT("마우스 휠"));
		T.Add(TEXT("Keys.Backspace"), TEXT("백스페이스"));
		T.Add(TEXT("Keys.Tab"), TEXT("Tab"));
		T.Add(TEXT("Keys.Enter"), TEXT("Enter"));
		T.Add(TEXT("Keys.On"), TEXT("켜짐"));
		T.Add(TEXT("Keys.Off"), TEXT("꺼짐"));

		T.Add(TEXT("Set.DisplayMode.Fullscreen"), TEXT("전체 화면"));
		T.Add(TEXT("Set.DisplayMode.Borderless"), TEXT("테두리 없음"));
		T.Add(TEXT("Set.DisplayMode.Windowed"), TEXT("창 모드"));
		T.Add(TEXT("Set.RecordingEffect.Off"), TEXT("꺼짐"));
		T.Add(TEXT("Set.RecordingEffect.Light"), TEXT("약한 필름 질감"));
		T.Add(TEXT("Set.RecordingEffect.Medium"), TEXT("중간 필름 질감"));
		T.Add(TEXT("Set.RecordingEffect.Strong"), TEXT("강한 필름 질감"));
		T.Add(TEXT("Set.Quality.Overall"), TEXT("전체 품질"));
		T.Add(TEXT("Set.PotatoMode"), TEXT("감자 모드"));
		T.Add(TEXT("Set.PotatoMode.Hint"), TEXT("그래픽을 희생하고 최대 FPS."));
		T.Add(TEXT("Set.Graphics"), TEXT("그래픽"));
		T.Add(TEXT("Q.Low"), TEXT("낮음"));
		T.Add(TEXT("Q.Medium"), TEXT("중간"));
		T.Add(TEXT("Q.High"), TEXT("높음"));
		T.Add(TEXT("Q.Epic"), TEXT("에픽"));
		T.Add(TEXT("Q.Ultra"), TEXT("울트라"));

		T.Add(TEXT("Pause.Resume"), TEXT("계속하기"));
		T.Add(TEXT("Pause.Restart"), TEXT("다시 시작"));
		T.Add(TEXT("Pause.Quit"), TEXT("게임 종료"));

		T.Add(TEXT("Interact.TakeOff"), TEXT("집어넣기"));

		T.Add(TEXT("Menu.StressTest"), TEXT("스트레스 테스트"));
		T.Add(TEXT("HUD.Fps"), TEXT("FPS"));
		T.Add(TEXT("HUD.FpsAvg"), TEXT("平均"));
		T.Add(TEXT("HUD.FpsOneLow"), TEXT("1% Low"));
		T.Add(TEXT("Loading.StressTest"), TEXT("스트레스 테스트..."));
	}
}
