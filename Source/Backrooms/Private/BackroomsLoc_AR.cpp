#include "BackroomsLocRegistry.h"

// العربية (فلسطين).
namespace BackroomsLoc
{
	void Register_AR(FLanguageTable& T)
	{
		T.Add(TEXT("Menu.NewGame"), TEXT("لعبة جديدة"));
		T.Add(TEXT("Menu.Continue"), TEXT("متابعة"));
		T.Add(TEXT("Menu.Levels"), TEXT("المراحل"));
		T.Add(TEXT("Menu.Achievements"), TEXT("الإنجازات"));
		T.Add(TEXT("Menu.Settings"), TEXT("الإعدادات"));
		T.Add(TEXT("Menu.Controls"), TEXT("التحكم"));
		T.Add(TEXT("Menu.Quit"), TEXT("خروج"));
		T.Add(TEXT("Menu.Subtitle"), TEXT("الهبوط الليمينالي"));
		T.Add(TEXT("Menu.Back"), TEXT("رجوع   >"));

		T.Add(TEXT("Tab.Screen"), TEXT("الشاشة"));
		T.Add(TEXT("Tab.Quality"), TEXT("الجودة"));
		T.Add(TEXT("Tab.Camera"), TEXT("الكاميرا"));
		T.Add(TEXT("Tab.Sound"), TEXT("الصوت"));
		T.Add(TEXT("Tab.Game"), TEXT("اللعبة"));
		T.Add(TEXT("Tab.Controls"), TEXT("التحكم"));
		T.Add(TEXT("Settings.Title"), TEXT("الإعدادات"));
		T.Add(TEXT("Settings.Categories"), TEXT("الفئات"));

		T.Add(TEXT("Set.Resolution"), TEXT("الدقة"));
		T.Add(TEXT("Set.DisplayMode"), TEXT("وضع العرض"));
		T.Add(TEXT("Set.VSync"), TEXT("VSync"));
		T.Add(TEXT("Set.RenderScale"), TEXT("مقياس العرض"));
		T.Add(TEXT("Set.Gamma"), TEXT("جاما"));
		T.Add(TEXT("Set.Language"), TEXT("اللغة"));

		T.Add(TEXT("Set.Shadows"), TEXT("الظلال"));
		T.Add(TEXT("Set.Textures"), TEXT("الخامات"));
		T.Add(TEXT("Set.AntiAliasing"), TEXT("تنعيم الحواف"));
		T.Add(TEXT("Set.PostProcess"), TEXT("المعالجة اللاحقة"));
		T.Add(TEXT("Set.Effects"), TEXT("التأثيرات"));
		T.Add(TEXT("Set.ViewDistance"), TEXT("مسافة الرؤية"));
		T.Add(TEXT("Set.DepthOfField"), TEXT("عمق الميدان"));
		T.Add(TEXT("Set.Bloom"), TEXT("التوهج"));
		T.Add(TEXT("Set.Lumen"), TEXT("Lumen (الإضاءة)"));
		T.Add(TEXT("Set.Upscale"), TEXT("DLSS / تحسين الدقة"));

		T.Add(TEXT("Set.FOV"), TEXT("زاوية الرؤية"));
		T.Add(TEXT("Set.Sensitivity"), TEXT("الحساسية"));
		T.Add(TEXT("Set.RecordingEffect"), TEXT("تأثير كاميرا التسجيل"));

		T.Add(TEXT("Set.Volume.Master"), TEXT("الرئيسية"));
		T.Add(TEXT("Set.Volume.Menu"), TEXT("الموسيقى / القائمة"));
		T.Add(TEXT("Set.Volume.Game"), TEXT("أصوات اللعبة"));
		T.Add(TEXT("Set.Volume.Monster"), TEXT("أصوات الوحوش"));
		T.Add(TEXT("Set.Volume.Other"), TEXT("أصوات أخرى"));

		T.Add(TEXT("Set.Difficulty"), TEXT("الصعوبة"));
		T.Add(TEXT("Set.Difficulty.Hint"),
			TEXT("الصعوبة تؤثر على التوليد: كثافة الجدران، عدد الأبواب، وكرم المؤن والغنائم."));

		T.Add(TEXT("Levels.Title"), TEXT("المراحل"));
		T.Add(TEXT("Levels.Locked"), TEXT("مقفل"));
		T.Add(TEXT("Levels.UnlockedFmt"),
			TEXT("المراحل المفتوحة: %d من 10. تُفتح الجديدة كلما نزلت أعمق — انزل لترى صورها."));

		T.Add(TEXT("Ach.Title"), TEXT("الإنجازات"));
		T.Add(TEXT("Ach.Unlocked"), TEXT("إنجاز"));
		T.Add(TEXT("Ach.ProgressFmt"), TEXT("المفتوحة: %d / %d     النقاط: %d"));

		T.Add(TEXT("Diff.Peaceful.Name"), TEXT("مسالم"));
		T.Add(TEXT("Diff.Peaceful.Desc"), TEXT("لا وحوش ولا ضغط. فقط استكشاف وعالم."));
		T.Add(TEXT("Diff.Easy.Name"), TEXT("سهل"));
		T.Add(TEXT("Diff.Easy.Desc"), TEXT("أماكن واسعة وكريمة. يظهر وحش واحد متأخراً."));
		T.Add(TEXT("Diff.Normal.Name"), TEXT("عادي"));
		T.Add(TEXT("Diff.Normal.Desc"), TEXT("التوازن المقصود: ممرات ضيقة، جوع، كائن وحيد."));
		T.Add(TEXT("Diff.Hard.Name"), TEXT("صعب"));
		T.Add(TEXT("Diff.Hard.Desc"), TEXT("جدران أكثر وأبواب أقل، غنائم شحيحة، كائنان."));
		T.Add(TEXT("Diff.Nightmare.Name"), TEXT("كابوس"));
		T.Add(TEXT("Diff.Nightmare.Desc"), TEXT("متاهة فخاخ، مؤن قليلة، ثلاثة كائنات، والموت نهائي."));

		T.Add(TEXT("HUD.Sanity"), TEXT("العقل"));
		T.Add(TEXT("HUD.Flashlight"), TEXT("المصباح"));
		T.Add(TEXT("HUD.Hunger"), TEXT("الجوع"));
		T.Add(TEXT("HUD.Thirst"), TEXT("العطش"));
		T.Add(TEXT("HUD.Health"), TEXT("الصحة"));
		T.Add(TEXT("HUD.Stamina"), TEXT("التحمّل"));
		T.Add(TEXT("HUD.Inventory"), TEXT("الحقيبة"));
		T.Add(TEXT("HUD.Empty"), TEXT("فارغ"));
		T.Add(TEXT("Prog.Level"), TEXT("المستوى"));
		T.Add(TEXT("Prog.LevelUp"), TEXT("ترقية"));

		T.Add(TEXT("Interact.Inspect"), TEXT("افحص"));
		T.Add(TEXT("Interact.Throw"), TEXT("ارمِ"));
		T.Add(TEXT("Interact.Drop"), TEXT("ضَع"));
		T.Add(TEXT("Interact.Take"), TEXT("خُذ"));
		T.Add(TEXT("Interact.Push"), TEXT("ادفع"));
		T.Add(TEXT("Interact.Use"), TEXT("استخدم"));
		T.Add(TEXT("Interact.Pickup"), TEXT("التقط"));

		T.Add(TEXT("Hint.Critical"), TEXT("حالة حرجة — ابحث عن دواء فوراً"));
		T.Add(TEXT("Hint.Sanity"), TEXT("عقلك على وشك الانهيار — اختبئ والتقط أنفاسك"));
		T.Add(TEXT("Hint.Hunger"), TEXT("جوع شديد — ابحث عن طعام"));
		T.Add(TEXT("Hint.Thirst"), TEXT("عطش شديد — اشرب ماءً"));
		T.Add(TEXT("Hint.Monster"), TEXT("إنه قريب — لا تُحدث ضجيجاً"));
		T.Add(TEXT("Hint.Poison"), TEXT("أنت مسموم — تحتاج ماء اللوز أو حقيبة إسعاف"));
		T.Add(TEXT("Hint.Radiation"), TEXT("الإشعاع مرتفع — اخرج من هنا"));
		T.Add(TEXT("Hint.Sleep"), TEXT("أنت منهك — توقف واسترح"));
		T.Add(TEXT("Hint.Battery"), TEXT("المصباح شبه فارغ"));

		T.Add(TEXT("Ach.FirstSteps.Name"), TEXT("الخطوات الأولى"));
		T.Add(TEXT("Ach.FirstSteps.Desc"), TEXT("النزول إلى Backrooms."));
		T.Add(TEXT("Ach.Tourist.Name"), TEXT("سائح"));
		T.Add(TEXT("Ach.Tourist.Desc"), TEXT("زيارة 3 أماكن مختلفة."));
		T.Add(TEXT("Ach.Explorer.Name"), TEXT("مستكشف"));
		T.Add(TEXT("Ach.Explorer.Desc"), TEXT("زيارة 5 أماكن مختلفة."));
		T.Add(TEXT("Ach.DeepDiver.Name"), TEXT("في الأعماق"));
		T.Add(TEXT("Ach.DeepDiver.Desc"), TEXT("زيارة 7 أماكن مختلفة."));
		T.Add(TEXT("Ach.AllLevels.Name"), TEXT("الطابق كاملاً"));
		T.Add(TEXT("Ach.AllLevels.Desc"), TEXT("زيارة الأماكن العشرة كلها."));
		T.Add(TEXT("Ach.Survivor.Name"), TEXT("ناجٍ"));
		T.Add(TEXT("Ach.Survivor.Desc"), TEXT("الصمود 15 دقيقة في Backrooms."));
		T.Add(TEXT("Ach.Marathon.Name"), TEXT("ماراثون"));
		T.Add(TEXT("Ach.Marathon.Desc"), TEXT("السير 3 كيلومترات تحت الأرض."));
		T.Add(TEXT("Ach.Scavenger.Name"), TEXT("جامع"));
		T.Add(TEXT("Ach.Scavenger.Desc"), TEXT("التقاط 10 أشياء."));
		T.Add(TEXT("Ach.Pharmacist.Name"), TEXT("صيدلي"));
		T.Add(TEXT("Ach.Pharmacist.Desc"), TEXT("استخدام 5 أدوية."));
		T.Add(TEXT("Ach.Hydrated.Name"), TEXT("طعم اللوز"));
		T.Add(TEXT("Ach.Hydrated.Desc"), TEXT("شرب 5 حصص من ماء اللوز."));
		T.Add(TEXT("Ach.MonsterAware.Name"), TEXT("إنه قريب"));
		T.Add(TEXT("Ach.MonsterAware.Desc"), TEXT("لقاء كائن والنجاة."));
		T.Add(TEXT("Ach.EscapeArtist.Name"), TEXT("المخرج الأول"));
		T.Add(TEXT("Ach.EscapeArtist.Desc"), TEXT("إيجاد مخرج."));
		T.Add(TEXT("Ach.FrequentFlyer.Name"), TEXT("زائر متكرر"));
		T.Add(TEXT("Ach.FrequentFlyer.Desc"), TEXT("المرور من 5 مخارج."));
		T.Add(TEXT("Ach.DeepFear.Name"), TEXT("حدّ الخوف"));
		T.Add(TEXT("Ach.DeepFear.Desc"), TEXT("رفع ضغط البيئة إلى أقصاه."));
		T.Add(TEXT("Ach.Dead.Name"), TEXT("واحد منهم"));
		T.Add(TEXT("Ach.Dead.Desc"), TEXT("الموت في Backrooms."));

		T.Add(TEXT("Bind.MoveForwardPlus"), TEXT("أمام"));
		T.Add(TEXT("Bind.MoveForwardMinus"), TEXT("خلف"));
		T.Add(TEXT("Bind.MoveRightPlus"), TEXT("يمين"));
		T.Add(TEXT("Bind.MoveRightMinus"), TEXT("يسار"));
		T.Add(TEXT("Bind.Jump"), TEXT("قفز"));
		T.Add(TEXT("Bind.Sprint"), TEXT("ركض"));
		T.Add(TEXT("Bind.Flashlight"), TEXT("مصباح يدوي"));
		T.Add(TEXT("Bind.View"), TEXT("منظور أول / ثالث شخص"));
		T.Add(TEXT("Bind.Attack"), TEXT("لكمة"));
		T.Add(TEXT("Bind.Grab"), TEXT("التقاط / إسقاط جسم"));
		T.Add(TEXT("Bind.Push"), TEXT("دفع جسم"));
		T.Add(TEXT("Bind.Throw"), TEXT("رمي جسم"));
		T.Add(TEXT("Bind.Inspect"), TEXT("فحص جسم"));
		T.Add(TEXT("Bind.Use"), TEXT("استخدام جسم"));
		T.Add(TEXT("Bind.Slot1"), TEXT("خانة 1"));
		T.Add(TEXT("Bind.Slot2"), TEXT("خانة 2"));
		T.Add(TEXT("Bind.Slot3"), TEXT("خانة 3"));
		T.Add(TEXT("Bind.Slot4"), TEXT("خانة 4"));
		T.Add(TEXT("Bind.Inventory"), TEXT("الحقيبة"));
		T.Add(TEXT("Bind.PauseMenu"), TEXT("إيقاف مؤقت / قائمة"));
		T.Add(TEXT("Bind.Mouse"), TEXT("فأرة"));
		T.Add(TEXT("Bind.Look"), TEXT("نظر"));
		T.Add(TEXT("Bind.Change"), TEXT("تغيير"));
		T.Add(TEXT("Bind.PressKey"), TEXT("اضغط مفتاحًا لتعيينه... (Esc للإلغاء)"));
		T.Add(TEXT("Bind.Cancel"), TEXT("إلغاء  ×"));
		T.Add(TEXT("Bind.Reset"), TEXT("إعادة جميع المفاتيح إلى الافتراضي"));

		T.Add(TEXT("Ach.WellFed.Name"), TEXT("شبعان"));
		T.Add(TEXT("Ach.WellFed.Desc"), TEXT("تناول 10 حصص من الطعام."));
		T.Add(TEXT("Ach.Electrician.Name"), TEXT("كهربائي"));
		T.Add(TEXT("Ach.Electrician.Desc"), TEXT("إدخال 10 بطاريات في المصباح."));
		T.Add(TEXT("Ach.Sprinter.Name"), TEXT("عدّاء"));
		T.Add(TEXT("Ach.Sprinter.Desc"), TEXT("بدء الركض 50 مرة."));
		T.Add(TEXT("Ach.Cartographer.Name"), TEXT("رسّام خرائط"));
		T.Add(TEXT("Ach.Cartographer.Desc"), TEXT("عبور 200 غرفة."));
		T.Add(TEXT("Ach.Speedrun.Name"), TEXT("متسابق سرعة"));
		T.Add(TEXT("Ach.Speedrun.Desc"), TEXT("قطع 5 كيلومترات تحت الأرض."));
		T.Add(TEXT("Ach.Gourmand.Name"), TEXT("ذوّاقة"));
		T.Add(TEXT("Ach.Gourmand.Desc"), TEXT("الصمود 30 دقيقة في Backrooms."));

		T.Add(TEXT("GameOver.Title"), TEXT("لقد مت"));
		T.Add(TEXT("GameOver.Subtitle"), TEXT("Backrooms لا يتركك. حاول مرة أخرى."));
		T.Add(TEXT("GameOver.Restart"), TEXT("إعادة المحاولة"));
		T.Add(TEXT("GameOver.Quit"), TEXT("خروج"));

		T.Add(TEXT("Level.W0"), TEXT("L0 · ردهة"));
		T.Add(TEXT("Level.W1"), TEXT("L1 · منطقة مأهولة"));
		T.Add(TEXT("Level.W2"), TEXT("L2 · أنابيب المياه"));
		T.Add(TEXT("Level.W3"), TEXT("L3 · محطة الطاقة"));
		T.Add(TEXT("Level.W4"), TEXT("L4 · مكاتب"));
		T.Add(TEXT("Level.W5"), TEXT("L5 · فندق"));
		T.Add(TEXT("Level.W6"), TEXT("L6 · ظلام"));
		T.Add(TEXT("Level.W7"), TEXT("L7 · محيط"));
		T.Add(TEXT("Level.W8"), TEXT("L8 · كهوف"));
		T.Add(TEXT("Level.W9"), TEXT("L9 · مستشفى"));

		T.Add(TEXT("Item.AlmondWater.Name"), TEXT("ماء اللوز"));
		T.Add(TEXT("Item.AlmondWater.Desc"), TEXT("يروي العطش ويستعيد شيئًا من العقل."));
		T.Add(TEXT("Item.CanFood.Name"), TEXT("طعام معلّب"));
		T.Add(TEXT("Item.CanFood.Desc"), TEXT("لحم معلّب. يطفئ الجوع."));
		T.Add(TEXT("Item.MedKit.Name"), TEXT("حقيبة إسعاف"));
		T.Add(TEXT("Item.MedKit.Desc"), TEXT("يوقف النزيف. يستعيد الصحة."));
		T.Add(TEXT("Item.Pill.Name"), TEXT("مهدّئ"));
		T.Add(TEXT("Item.Pill.Desc"), TEXT("يهدئ الأعصاب. يستعيد العقل."));
		T.Add(TEXT("Item.Energy.Name"), TEXT("مشروب طاقة"));
		T.Add(TEXT("Item.Energy.Desc"), TEXT("ينشط الجسد ويصفو الذهن قليلًا."));
		T.Add(TEXT("Item.Battery.Name"), TEXT("بطارية"));
		T.Add(TEXT("Item.Battery.Desc"), TEXT("خلايا طاقة للمصباح اليدوي."));

		T.Add(TEXT("Event.LightFlicker"), TEXT("وميض الضوء"));
		T.Add(TEXT("Event.LightOutage"), TEXT("انطفأ الضوء!"));
		T.Add(TEXT("Event.BoxDisappear"), TEXT("اختفى صندوق..."));
		T.Add(TEXT("Event.BoxAppear"), TEXT("ظهر صندوق!"));
		T.Add(TEXT("Event.EntityGrowl"), TEXT("هدير من الظلام..."));
		T.Add(TEXT("Event.EntityFootsteps"), TEXT("خطوات خلف الجدار..."));
		T.Add(TEXT("Event.Whisper"), TEXT("همس..."));
		T.Add(TEXT("Event.DistantBang"), TEXT("دويّ من بعيد!"));
		T.Add(TEXT("Event.WallDrawing"), TEXT("رسم على الجدار..."));
		T.Add(TEXT("Event.PipeCreak"), TEXT("صرير أنبوب"));
		T.Add(TEXT("Event.DoorSlam"), TEXT("إغلاق عنيف لباب!"));
		T.Add(TEXT("Event.EmergencyLight"), TEXT("ضوء الطوارئ"));
		T.Add(TEXT("Event.FogIncrease"), TEXT("يغتاظ الضباب..."));
		T.Add(TEXT("Event.StaticNoise"), TEXT("تشويش..."));
		T.Add(TEXT("Event.FootprintAppear"), TEXT("آثار أقدام على الأرض..."));
		T.Add(TEXT("Event.Unknown"), TEXT("حدث مجهول"));

		T.Add(TEXT("Loading.Level"), TEXT("جارٍ تحميل المستوى..."));
		T.Add(TEXT("Loading.World"), TEXT("جارٍ تحميل Backrooms... %d%% (أجزاء: %d)"));
		T.Add(TEXT("Loading.Ready"), TEXT("العالم جاهز! خلل في الواقع..."));
		T.Add(TEXT("Loading.Loaded"), TEXT("تم تحميل المستوى"));
		T.Add(TEXT("Loading.Noclip"), TEXT("Noclip: السقوط في Backrooms..."));
		T.Add(TEXT("Loading.Backrooms"), TEXT("جارٍ تحميل Backrooms..."));
		T.Add(TEXT("Loading.Chunks"), TEXT("أجزاء"));
		T.Add(TEXT("Loading.WorldReady"), TEXT("العالم جاهز! خلل في الواقع..."));
		T.Add(TEXT("Loading.LevelDone"), TEXT("تم تحميل المستوى"));

		T.Add(TEXT("Keys.SpaceBar"), TEXT("مسافة"));
		T.Add(TEXT("Keys.LeftShift"), TEXT("Shift"));
		T.Add(TEXT("Keys.RightShift"), TEXT("Shift (يمين)"));
		T.Add(TEXT("Keys.LeftCtrl"), TEXT("Ctrl"));
		T.Add(TEXT("Keys.RightCtrl"), TEXT("Ctrl (يمين)"));
		T.Add(TEXT("Keys.LeftAlt"), TEXT("Alt"));
		T.Add(TEXT("Keys.RightAlt"), TEXT("AltGr"));
		T.Add(TEXT("Keys.Escape"), TEXT("Esc"));
		T.Add(TEXT("Keys.LeftMouse"), TEXT("زر أيسر"));
		T.Add(TEXT("Keys.RightMouse"), TEXT("زر أيمن"));
		T.Add(TEXT("Keys.MiddleMouse"), TEXT("العجلة (نقر)"));
		T.Add(TEXT("Keys.ThumbMouse1"), TEXT("زر جانبي 1"));
		T.Add(TEXT("Keys.ThumbMouse2"), TEXT("زر جانبي 2"));
		T.Add(TEXT("Keys.MouseX"), TEXT("فأرة X"));
		T.Add(TEXT("Keys.MouseY"), TEXT("فأرة Y"));
		T.Add(TEXT("Keys.MouseWheel"), TEXT("عجلة الفأرة"));
		T.Add(TEXT("Keys.Backspace"), TEXT("مسح"));
		T.Add(TEXT("Keys.Tab"), TEXT("Tab"));
		T.Add(TEXT("Keys.Enter"), TEXT("إدخال"));
		T.Add(TEXT("Keys.On"), TEXT("تشغيل"));
		T.Add(TEXT("Keys.Off"), TEXT("إيقاف"));

		T.Add(TEXT("Set.DisplayMode.Fullscreen"), TEXT("ملء الشاشة"));
		T.Add(TEXT("Set.DisplayMode.Borderless"), TEXT("بدون إطار"));
		T.Add(TEXT("Set.DisplayMode.Windowed"), TEXT("نافذة"));
		T.Add(TEXT("Set.RecordingEffect.Off"), TEXT("معطّل"));
		T.Add(TEXT("Set.RecordingEffect.Light"), TEXT("تأثير فيلم خفيف"));
		T.Add(TEXT("Set.RecordingEffect.Medium"), TEXT("تأثير فيلم متوسط"));
		T.Add(TEXT("Set.RecordingEffect.Strong"), TEXT("تأثير فيلم قوي"));
		T.Add(TEXT("Set.Quality.Overall"), TEXT("الجودة العامة"));
		T.Add(TEXT("Set.PotatoMode"), TEXT("وضع البطاطس"));
		T.Add(TEXT("Set.PotatoMode.Hint"), TEXT("أقصى أداء على حساب الرسومات."));
		T.Add(TEXT("Set.Graphics"), TEXT("الرسومات"));
		T.Add(TEXT("Q.Low"), TEXT("منخفضة"));
		T.Add(TEXT("Q.Medium"), TEXT("متوسطة"));
		T.Add(TEXT("Q.High"), TEXT("عالية"));
		T.Add(TEXT("Q.Epic"), TEXT("ملحمية"));
		T.Add(TEXT("Q.Ultra"), TEXT("فائقة"));

		T.Add(TEXT("Pause.Resume"), TEXT("استئناف"));
		T.Add(TEXT("Pause.Restart"), TEXT("إعادة المحاولة"));
		T.Add(TEXT("Pause.Quit"), TEXT("الخروج من اللعبة"));

		T.Add(TEXT("Interact.TakeOff"), TEXT("إبعاد"));

		T.Add(TEXT("Menu.StressTest"), TEXT("اختبار الإجهاد"));
		T.Add(TEXT("HUD.Fps"), TEXT("FPS"));
		T.Add(TEXT("HUD.FpsAvg"), TEXT("المتوسط"));
		T.Add(TEXT("HUD.FpsOneLow"), TEXT("1% منخفض"));
		T.Add(TEXT("Loading.StressTest"), TEXT("اختبار الإجهاد - تحميل..."));
	}
}
