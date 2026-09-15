#include "BackroomsLocRegistry.h"

// Русский. Эталонный набор ключей: если ключа нет здесь — его не хватает и
// в остальных языках. Имена собственные (Backrooms, L0..L9) не переводятся.
namespace BackroomsLoc
{
	void Register_RU(FLanguageTable& T)
	{
		T.Add(TEXT("Menu.NewGame"), TEXT("Новая игра"));
		T.Add(TEXT("Menu.Continue"), TEXT("Продолжить"));
		T.Add(TEXT("Menu.Levels"), TEXT("Уровни"));
		T.Add(TEXT("Menu.Achievements"), TEXT("Достижения"));
		T.Add(TEXT("Menu.Settings"), TEXT("Настройки"));
		T.Add(TEXT("Menu.Controls"), TEXT("Управление"));
		T.Add(TEXT("Menu.Quit"), TEXT("Выход"));
		T.Add(TEXT("Menu.Subtitle"), TEXT("ЛИМИНАЛЬНОЕ ПОГРУЖЕНИЕ"));
		T.Add(TEXT("Menu.Back"), TEXT("<   Н А З А Д"));

		T.Add(TEXT("Tab.Screen"), TEXT("ЭКРАН"));
		T.Add(TEXT("Tab.Quality"), TEXT("КАЧЕСТВО"));
		T.Add(TEXT("Tab.Camera"), TEXT("КАМЕРА"));
		T.Add(TEXT("Tab.Sound"), TEXT("ЗВУК"));
		T.Add(TEXT("Tab.Game"), TEXT("ИГРА"));
		T.Add(TEXT("Tab.Controls"), TEXT("УПРАВЛЕНИЕ"));
		T.Add(TEXT("Settings.Title"), TEXT("Н А С Т Р О Й К И"));
		T.Add(TEXT("Settings.Categories"), TEXT("К А Т Е Г О Р И И"));

		T.Add(TEXT("Set.Resolution"), TEXT("Разрешение"));
		T.Add(TEXT("Set.DisplayMode"), TEXT("Режим экрана"));
		T.Add(TEXT("Set.VSync"), TEXT("VSync"));
		T.Add(TEXT("Set.RenderScale"), TEXT("Масштаб рендера"));
		T.Add(TEXT("Set.Gamma"), TEXT("Гамма"));
		T.Add(TEXT("Set.Language"), TEXT("Язык"));

		T.Add(TEXT("Set.Shadows"), TEXT("Тени"));
		T.Add(TEXT("Set.Textures"), TEXT("Текстуры"));
		T.Add(TEXT("Set.AntiAliasing"), TEXT("Сглаживание"));
		T.Add(TEXT("Set.PostProcess"), TEXT("Пост-обработка"));
		T.Add(TEXT("Set.Effects"), TEXT("Эффекты"));
		T.Add(TEXT("Set.ViewDistance"), TEXT("Дальность прорисовки"));
		T.Add(TEXT("Set.DepthOfField"), TEXT("Глубина резкости"));
		T.Add(TEXT("Set.Bloom"), TEXT("Свечение (Bloom)"));
		T.Add(TEXT("Set.Lumen"), TEXT("Lumen (освещение)"));
		T.Add(TEXT("Set.Upscale"), TEXT("DLSS / Апскейл"));

		T.Add(TEXT("Set.FOV"), TEXT("Угол обзора (FOV)"));
		T.Add(TEXT("Set.Sensitivity"), TEXT("Чувствительность"));
		T.Add(TEXT("Set.RecordingEffect"), TEXT("Эффект камеры записи"));

		T.Add(TEXT("Set.Volume.Master"), TEXT("Общая"));
		T.Add(TEXT("Set.Volume.Menu"), TEXT("Музыка / меню"));
		T.Add(TEXT("Set.Volume.Game"), TEXT("Звуки в игре"));
		T.Add(TEXT("Set.Volume.Monster"), TEXT("Звуки монстров"));
		T.Add(TEXT("Set.Volume.Other"), TEXT("Прочие звуки"));

		T.Add(TEXT("Set.Difficulty"), TEXT("Сложность"));
		T.Add(TEXT("Set.Difficulty.Hint"),
			TEXT("Сложность воздействует на генерацию: плотность стен, число проёмов, щедрость припасов и дроп."));

		T.Add(TEXT("Levels.Title"), TEXT("У Р О В Н И"));
		T.Add(TEXT("Levels.Locked"), TEXT("ЗАКРЫТО"));
		T.Add(TEXT("Levels.UnlockedFmt"),
			TEXT("Открыто локаций: %d из 10. Новые открываются по мере спуска — дойди вглубь, чтобы увидеть их превью."));

		T.Add(TEXT("Ach.Title"), TEXT("Д О С Т И Ж Е Н И Я"));
		T.Add(TEXT("Ach.Unlocked"), TEXT("ДОСТИЖЕНИЕ"));
		T.Add(TEXT("Ach.ProgressFmt"), TEXT("Открыто: %d / %d     Очков: %d"));

		T.Add(TEXT("Diff.Peaceful.Name"), TEXT("Мирный"));
		T.Add(TEXT("Diff.Peaceful.Desc"), TEXT("Без монстров и давления. Только исследование и вид мира."));
		T.Add(TEXT("Diff.Easy.Name"), TEXT("Лёгкий"));
		T.Add(TEXT("Diff.Easy.Desc"), TEXT("Свободные, щедрые локации. Один монстр появляется поздно."));
		T.Add(TEXT("Diff.Normal.Name"), TEXT("Обычный"));
		T.Add(TEXT("Diff.Normal.Desc"), TEXT("Задуманный баланс: тесные коридоры, голод, одиночная сущность."));
		T.Add(TEXT("Diff.Hard.Name"), TEXT("Сложный"));
		T.Add(TEXT("Diff.Hard.Desc"), TEXT("Больше стен и меньше дверей, скудный лут, две сущности."));
		T.Add(TEXT("Diff.Nightmare.Name"), TEXT("Кошмар"));
		T.Add(TEXT("Diff.Nightmare.Desc"), TEXT("Лабиринт-ловушка, минимум припасов, три сущности, смерть окончательна."));

		T.Add(TEXT("HUD.Sanity"), TEXT("РАССУДОК"));
		T.Add(TEXT("HUD.Flashlight"), TEXT("ФОНАРИК"));
		T.Add(TEXT("HUD.Hunger"), TEXT("ГОЛОД"));
		T.Add(TEXT("HUD.Thirst"), TEXT("ЖАЖДА"));
		T.Add(TEXT("HUD.Health"), TEXT("ЗДОРОВЬЕ"));
		T.Add(TEXT("HUD.Stamina"), TEXT("ВЫНОСЛИВОСТЬ"));
		T.Add(TEXT("HUD.Inventory"), TEXT("ИНВЕНТАРЬ"));
		T.Add(TEXT("HUD.Empty"), TEXT("пусто"));
		T.Add(TEXT("Prog.Level"), TEXT("Уровень"));
		T.Add(TEXT("Prog.LevelUp"), TEXT("Новый уровень"));

		T.Add(TEXT("Interact.Inspect"), TEXT("Осмотреть"));
		T.Add(TEXT("Interact.Throw"), TEXT("Бросить"));
		T.Add(TEXT("Interact.Drop"), TEXT("Положить"));
T.Add(TEXT("Interact.Take"), TEXT("Взять"));
		T.Add(TEXT("Interact.Push"), TEXT("Толкнуть"));
		T.Add(TEXT("Interact.Use"), TEXT("Использовать"));
		T.Add(TEXT("Interact.Pickup"), TEXT("Подобрать"));

		T.Add(TEXT("Hint.Critical"), TEXT("Критическое состояние — срочно найди лекарство"));
		T.Add(TEXT("Hint.Sanity"), TEXT("Рассудок на исходе — спрячься и отдышись"));
		T.Add(TEXT("Hint.Hunger"), TEXT("Сильный голод — найди еду"));
		T.Add(TEXT("Hint.Thirst"), TEXT("Сильная жажда — выпей воды"));
		T.Add(TEXT("Hint.Monster"), TEXT("Оно где-то рядом — не шуми"));
		T.Add(TEXT("Hint.Poison"), TEXT("Ты отравился — нужна миндальная вода или аптечка"));
		T.Add(TEXT("Hint.Radiation"), TEXT("Фон облучения высокий — уходи отсюда"));
		T.Add(TEXT("Hint.Sleep"), TEXT("Ты вымотан — остановись и передохни"));
		T.Add(TEXT("Hint.Battery"), TEXT("Фонарик почти разряжен"));

		// --- Достижения (имена и описания) ---
		T.Add(TEXT("Ach.FirstSteps.Name"), TEXT("Первые шаги"));
		T.Add(TEXT("Ach.FirstSteps.Desc"), TEXT("Спуститься в Бэкрумс."));
		T.Add(TEXT("Ach.Tourist.Name"), TEXT("Турист"));
		T.Add(TEXT("Ach.Tourist.Desc"), TEXT("Побывать в 3 разных локациях."));
		T.Add(TEXT("Ach.Explorer.Name"), TEXT("Исследователь"));
		T.Add(TEXT("Ach.Explorer.Desc"), TEXT("Побывать в 5 разных локациях."));
		T.Add(TEXT("Ach.DeepDiver.Name"), TEXT("На глубине"));
		T.Add(TEXT("Ach.DeepDiver.Desc"), TEXT("Побывать в 7 разных локациях."));
		T.Add(TEXT("Ach.AllLevels.Name"), TEXT("Весь этаж"));
		T.Add(TEXT("Ach.AllLevels.Desc"), TEXT("Побывать во всех 10 локациях."));
		T.Add(TEXT("Ach.Survivor.Name"), TEXT("Выживший"));
		T.Add(TEXT("Ach.Survivor.Desc"), TEXT("Продержаться в Бэкрумсе 15 минут."));
		T.Add(TEXT("Ach.Marathon.Name"), TEXT("Марафонец"));
		T.Add(TEXT("Ach.Marathon.Desc"), TEXT("Пройти 3 километра под землёй."));
		T.Add(TEXT("Ach.Scavenger.Name"), TEXT("Собиратель"));
		T.Add(TEXT("Ach.Scavenger.Desc"), TEXT("Подобрать 10 предметов."));
		T.Add(TEXT("Ach.Pharmacist.Name"), TEXT("Фармацевт"));
		T.Add(TEXT("Ach.Pharmacist.Desc"), TEXT("Использовать 5 медикаментов."));
		T.Add(TEXT("Ach.Hydrated.Name"), TEXT("Миндальный вкус"));
		T.Add(TEXT("Ach.Hydrated.Desc"), TEXT("Выпить 5 порций миндальной воды."));
		T.Add(TEXT("Ach.MonsterAware.Name"), TEXT("Оно рядом"));
		T.Add(TEXT("Ach.MonsterAware.Desc"), TEXT("Встретить сущность и выжить."));
		T.Add(TEXT("Ach.EscapeArtist.Name"), TEXT("Первый выход"));
		T.Add(TEXT("Ach.EscapeArtist.Desc"), TEXT("Найти выход из локации."));
		T.Add(TEXT("Ach.FrequentFlyer.Name"), TEXT("Частый гость"));
		T.Add(TEXT("Ach.FrequentFlyer.Desc"), TEXT("Пройти через 5 выходов."));
		T.Add(TEXT("Ach.DeepFear.Name"), TEXT("Предел страха"));
		T.Add(TEXT("Ach.DeepFear.Desc"), TEXT("Довести давление среды до максимума."));
		T.Add(TEXT("Ach.Dead.Name"), TEXT("Один из них"));
		T.Add(TEXT("Ach.Dead.Desc"), TEXT("Погибнуть в Бэкрумсе."));
		T.Add(TEXT("Bind.MoveForwardPlus"), TEXT("Вперёд"));
		T.Add(TEXT("Bind.MoveForwardMinus"), TEXT("Назад"));
		T.Add(TEXT("Bind.MoveRightPlus"), TEXT("Вправо"));
		T.Add(TEXT("Bind.MoveRightMinus"), TEXT("Влево"));
		T.Add(TEXT("Bind.Jump"), TEXT("Прыжок"));
		T.Add(TEXT("Bind.Sprint"), TEXT("Бег"));
		T.Add(TEXT("Bind.Flashlight"), TEXT("Фонарик"));
		T.Add(TEXT("Bind.View"), TEXT("Вид 1-е / 3-е лицо"));
		T.Add(TEXT("Bind.Attack"), TEXT("Удар кулаком"));
		T.Add(TEXT("Bind.Grab"), TEXT("Взять / положить предмет"));
		T.Add(TEXT("Bind.Push"), TEXT("Толкнуть предмет"));
		T.Add(TEXT("Bind.Throw"), TEXT("Бросить предмет"));
		T.Add(TEXT("Bind.Inspect"), TEXT("Осмотреть предмет"));
		T.Add(TEXT("Bind.Use"), TEXT("Использовать предмет"));
		T.Add(TEXT("Bind.Slot1"), TEXT("Слот 1"));
		T.Add(TEXT("Bind.Slot2"), TEXT("Слот 2"));
		T.Add(TEXT("Bind.Slot3"), TEXT("Слот 3"));
		T.Add(TEXT("Bind.Slot4"), TEXT("Слот 4"));
		T.Add(TEXT("Bind.Inventory"), TEXT("Инвентарь"));
		T.Add(TEXT("Bind.PauseMenu"), TEXT("Пауза / меню"));
		T.Add(TEXT("Bind.Mouse"), TEXT("Мышь"));
		T.Add(TEXT("Bind.Look"), TEXT("Обзор"));
		T.Add(TEXT("Bind.Change"), TEXT("изменить"));
		T.Add(TEXT("Bind.PressKey"), TEXT("Нажмите клавишу для назначения... (Esc — отмена)"));
		T.Add(TEXT("Bind.Cancel"), TEXT("ОТМЕНА  ×"));
		T.Add(TEXT("Bind.Reset"), TEXT("Сбросить все кнопки на клавиши по умолчанию"));
		T.Add(TEXT("Ach.WellFed.Name"), TEXT("Сытый"));
		T.Add(TEXT("Ach.WellFed.Desc"), TEXT("Съесть 10 порций еды."));
		T.Add(TEXT("Ach.Electrician.Name"), TEXT("Электрик"));
		T.Add(TEXT("Ach.Electrician.Desc"), TEXT("Вставить 10 батареек в фонарик."));
		T.Add(TEXT("Ach.Sprinter.Name"), TEXT("Бегун"));
		T.Add(TEXT("Ach.Sprinter.Desc"), TEXT("Начать бег 50 раз."));
		T.Add(TEXT("Ach.Cartographer.Name"), TEXT("Картограф"));
		T.Add(TEXT("Ach.Cartographer.Desc"), TEXT("Пройти 200 комнат."));
		T.Add(TEXT("Ach.Speedrun.Name"), TEXT("Скороход"));
		T.Add(TEXT("Ach.Speedrun.Desc"), TEXT("Пройти 5 километров под землёй."));
T.Add(TEXT("Ach.Gourmand.Name"), TEXT("Гурман"));
		T.Add(TEXT("Ach.Gourmand.Desc"), TEXT("Продержаться 30 минут в Бэкрумсе."));

		// --- Экран смерти ---
		T.Add(TEXT("GameOver.Title"), TEXT("ВЫ ПОГИБЛИ"));
		T.Add(TEXT("GameOver.Subtitle"), TEXT("Бэкрумс не отпускает. Попробуйте снова."));
		T.Add(TEXT("GameOver.Restart"), TEXT("Заново"));
		T.Add(TEXT("GameOver.Quit"), TEXT("Выход"));

		// --- Уровни (книга уровней) ---
		T.Add(TEXT("Level.W0"), TEXT("L0 · Лобби"));
		T.Add(TEXT("Level.W1"), TEXT("L1 · Обитаемая зона"));
		T.Add(TEXT("Level.W2"), TEXT("L2 · Водопровод"));
		T.Add(TEXT("Level.W3"), TEXT("L3 · Электростанция"));
		T.Add(TEXT("Level.W4"), TEXT("L4 · Офисы"));
		T.Add(TEXT("Level.W5"), TEXT("L5 · Отель"));
		T.Add(TEXT("Level.W6"), TEXT("L6 · Тьма"));
		T.Add(TEXT("Level.W7"), TEXT("L7 · Океан"));
		T.Add(TEXT("Level.W8"), TEXT("L8 · Пещеры"));
		T.Add(TEXT("Level.W9"), TEXT("L9 · Больница"));

		// --- Предметы ---
		T.Add(TEXT("Item.AlmondWater.Name"), TEXT("Миндальная вода"));
		T.Add(TEXT("Item.AlmondWater.Desc"), TEXT("Утоляет жажду и слегка возвращает рассудок."));
		T.Add(TEXT("Item.CanFood.Name"), TEXT("Тушёнка"));
		T.Add(TEXT("Item.CanFood.Desc"), TEXT("Тушёное мясо. Утоляет голод."));
		T.Add(TEXT("Item.MedKit.Name"), TEXT("Аптечка"));
		T.Add(TEXT("Item.MedKit.Desc"), TEXT("Останавливает кровь. Восстанавливает здоровье."));
		T.Add(TEXT("Item.Pill.Name"), TEXT("Успокоительное"));
		T.Add(TEXT("Item.Pill.Desc"), TEXT("Успокаивает нервы. Восстанавливает рассудок."));
		T.Add(TEXT("Item.Energy.Name"), TEXT("Энергетик"));
		T.Add(TEXT("Item.Energy.Desc"), TEXT("Взбадривает и слегка проясняет сознание."));
		T.Add(TEXT("Item.Battery.Name"), TEXT("Батарейка"));
		T.Add(TEXT("Item.Battery.Desc"), TEXT("Элемент питания для фонарика."));

		// --- События Бэкрумса ---
		T.Add(TEXT("Event.LightFlicker"), TEXT("Мерцание света"));
		T.Add(TEXT("Event.LightOutage"), TEXT("Погас свет!"));
		T.Add(TEXT("Event.BoxDisappear"), TEXT("Ящик исчез..."));
		T.Add(TEXT("Event.BoxAppear"), TEXT("Ящик появился!"));
		T.Add(TEXT("Event.EntityGrowl"), TEXT("Рык из темноты..."));
		T.Add(TEXT("Event.EntityFootsteps"), TEXT("Шаги за стеной..."));
		T.Add(TEXT("Event.Whisper"), TEXT("Шёпот..."));
		T.Add(TEXT("Event.DistantBang"), TEXT("Грохот издалека!"));
		T.Add(TEXT("Event.WallDrawing"), TEXT("Рисунок на стене..."));
		T.Add(TEXT("Event.PipeCreak"), TEXT("Скрип трубы"));
		T.Add(TEXT("Event.DoorSlam"), TEXT("Хлопок двери!"));
		T.Add(TEXT("Event.EmergencyLight"), TEXT("Аварийный свет"));
		T.Add(TEXT("Event.FogIncrease"), TEXT("Туман густеет..."));
		T.Add(TEXT("Event.StaticNoise"), TEXT("Помехи..."));
		T.Add(TEXT("Event.FootprintAppear"), TEXT("Следы на полу..."));
		T.Add(TEXT("Event.Unknown"), TEXT("Неизвестное событие"));

		// --- Загрузка ---
		T.Add(TEXT("Loading.Level"), TEXT("Загрузка уровня..."));
		T.Add(TEXT("Loading.World"), TEXT("Загрузка Бэкрумса... %d%% (чанков: %d)"));
		T.Add(TEXT("Loading.Ready"), TEXT("Мир готов! Сбой реальности..."));
		T.Add(TEXT("Loading.Loaded"), TEXT("Уровень загружен"));
		T.Add(TEXT("Loading.Noclip"), TEXT("Noclip: Проваливание в Бэкрумс..."));
		T.Add(TEXT("Loading.Backrooms"), TEXT("Загрузка Бэкрумса..."));
		T.Add(TEXT("Loading.Chunks"), TEXT("чанков"));
		T.Add(TEXT("Loading.WorldReady"), TEXT("Мир готов! Сбой реальности..."));
		T.Add(TEXT("Loading.LevelDone"), TEXT("Уровень загружен"));

		// --- Названия клавиш ---
		T.Add(TEXT("Keys.SpaceBar"), TEXT("Пробел"));
		T.Add(TEXT("Keys.LeftShift"), TEXT("Shift"));
		T.Add(TEXT("Keys.RightShift"), TEXT("Shift (прав.)"));
		T.Add(TEXT("Keys.LeftCtrl"), TEXT("Ctrl"));
		T.Add(TEXT("Keys.RightCtrl"), TEXT("Ctrl (прав.)"));
		T.Add(TEXT("Keys.LeftAlt"), TEXT("Alt"));
		T.Add(TEXT("Keys.RightAlt"), TEXT("AltGr"));
		T.Add(TEXT("Keys.Escape"), TEXT("Esc"));
		T.Add(TEXT("Keys.LeftMouse"), TEXT("ЛКМ"));
		T.Add(TEXT("Keys.RightMouse"), TEXT("ПКМ"));
		T.Add(TEXT("Keys.MiddleMouse"), TEXT("Колесо (кнопка)"));
		T.Add(TEXT("Keys.ThumbMouse1"), TEXT("Боковая кнопка 1"));
		T.Add(TEXT("Keys.ThumbMouse2"), TEXT("Боковая кнопка 2"));
		T.Add(TEXT("Keys.MouseX"), TEXT("Мышь X"));
		T.Add(TEXT("Keys.MouseY"), TEXT("Мышь Y"));
		T.Add(TEXT("Keys.MouseWheel"), TEXT("Колесо мыши"));
		T.Add(TEXT("Keys.Backspace"), TEXT("Backspace"));
		T.Add(TEXT("Keys.Tab"), TEXT("Tab"));
		T.Add(TEXT("Keys.Enter"), TEXT("Enter"));
		T.Add(TEXT("Keys.On"), TEXT("ВКЛ"));
		T.Add(TEXT("Keys.Off"), TEXT("ВЫКЛ"));

		// --- Значения настроек ---
		T.Add(TEXT("Set.DisplayMode.Fullscreen"), TEXT("Полный экран"));
		T.Add(TEXT("Set.DisplayMode.Borderless"), TEXT("Безрамочный"));
		T.Add(TEXT("Set.DisplayMode.Windowed"), TEXT("Окно"));
		T.Add(TEXT("Set.RecordingEffect.Off"), TEXT("Выкл"));
		T.Add(TEXT("Set.RecordingEffect.Light"), TEXT("Лёгкая плёнка"));
		T.Add(TEXT("Set.RecordingEffect.Medium"), TEXT("Средняя плёнка"));
		T.Add(TEXT("Set.RecordingEffect.Strong"), TEXT("Сильная плёнка"));
		T.Add(TEXT("Set.Quality.Overall"), TEXT("Общее качество"));
		T.Add(TEXT("Set.PotatoMode"), TEXT("Картофельный режим"));
		T.Add(TEXT("Set.PotatoMode.Hint"), TEXT("Максимальный FPS в ущерб графике."));
		T.Add(TEXT("Set.Graphics"), TEXT("Графика"));
		T.Add(TEXT("Q.Low"), TEXT("Низкое"));
		T.Add(TEXT("Q.Medium"), TEXT("Среднее"));
		T.Add(TEXT("Q.High"), TEXT("Высокое"));
		T.Add(TEXT("Q.Epic"), TEXT("Эпическое"));
		T.Add(TEXT("Q.Ultra"), TEXT("Ультра"));

		// --- Пауза ---
		T.Add(TEXT("Pause.Resume"), TEXT("Продолжить"));
		T.Add(TEXT("Pause.Restart"), TEXT("Заново"));
		T.Add(TEXT("Pause.Quit"), TEXT("Выход из игры"));

		// --- Действия игрока ---
		T.Add(TEXT("Interact.TakeOff"), TEXT("Снять"));

		T.Add(TEXT("Menu.StressTest"), TEXT("СТРЕСС-ТЕСТ"));
		T.Add(TEXT("HUD.Fps"), TEXT("FPS"));
		T.Add(TEXT("HUD.FpsAvg"), TEXT("Средн"));
		T.Add(TEXT("HUD.FpsOneLow"), TEXT("1% мин"));
		T.Add(TEXT("Loading.StressTest"), TEXT("Стресс-тест — жарим Бэкрумс..."));
	}
}
