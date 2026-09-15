#include "BackroomsLocRegistry.h"

// Türkçe.
namespace BackroomsLoc
{
	void Register_TR(FLanguageTable& T)
	{
		T.Add(TEXT("Menu.NewGame"), TEXT("Yeni Oyun"));
		T.Add(TEXT("Menu.Continue"), TEXT("Devam Et"));
		T.Add(TEXT("Menu.Levels"), TEXT("Bölümler"));
		T.Add(TEXT("Menu.Achievements"), TEXT("Başarımlar"));
		T.Add(TEXT("Menu.Settings"), TEXT("Ayarlar"));
		T.Add(TEXT("Menu.Controls"), TEXT("Kontroller"));
		T.Add(TEXT("Menu.Quit"), TEXT("Çıkış"));
		T.Add(TEXT("Menu.Subtitle"), TEXT("LİMİNAL İNİŞ"));
		T.Add(TEXT("Menu.Back"), TEXT("<   G E R İ"));

		T.Add(TEXT("Tab.Screen"), TEXT("EKRAN"));
		T.Add(TEXT("Tab.Quality"), TEXT("KALİTE"));
		T.Add(TEXT("Tab.Camera"), TEXT("KAMERA"));
		T.Add(TEXT("Tab.Sound"), TEXT("SES"));
		T.Add(TEXT("Tab.Game"), TEXT("OYUN"));
		T.Add(TEXT("Tab.Controls"), TEXT("KONTROLLER"));
		T.Add(TEXT("Settings.Title"), TEXT("A Y A R L A R"));
		T.Add(TEXT("Settings.Categories"), TEXT("K A T E G O R İ L E R"));

		T.Add(TEXT("Set.Resolution"), TEXT("Çözünürlük"));
		T.Add(TEXT("Set.DisplayMode"), TEXT("Ekran Modu"));
		T.Add(TEXT("Set.VSync"), TEXT("VSync"));
		T.Add(TEXT("Set.RenderScale"), TEXT("Render Ölçeği"));
		T.Add(TEXT("Set.Gamma"), TEXT("Gama"));
		T.Add(TEXT("Set.Language"), TEXT("Dil"));

		T.Add(TEXT("Set.Shadows"), TEXT("Gölgeler"));
		T.Add(TEXT("Set.Textures"), TEXT("Dokular"));
		T.Add(TEXT("Set.AntiAliasing"), TEXT("Kenar Yumuşatma"));
		T.Add(TEXT("Set.PostProcess"), TEXT("İşlem Sonrası"));
		T.Add(TEXT("Set.Effects"), TEXT("Efektler"));
		T.Add(TEXT("Set.ViewDistance"), TEXT("Görüş Mesafesi"));
		T.Add(TEXT("Set.DepthOfField"), TEXT("Alan Derinliği"));
		T.Add(TEXT("Set.Bloom"), TEXT("Parlama (Bloom)"));
		T.Add(TEXT("Set.Lumen"), TEXT("Lumen (aydınlatma)"));
		T.Add(TEXT("Set.Upscale"), TEXT("DLSS / Ölçekleme"));

		T.Add(TEXT("Set.FOV"), TEXT("Görüş Açısı"));
		T.Add(TEXT("Set.Sensitivity"), TEXT("Hassasiyet"));
		T.Add(TEXT("Set.RecordingEffect"), TEXT("Kayıt Kamerası Efekti"));

		T.Add(TEXT("Set.Volume.Master"), TEXT("Genel"));
		T.Add(TEXT("Set.Volume.Menu"), TEXT("Müzik / menü"));
		T.Add(TEXT("Set.Volume.Game"), TEXT("Oyun sesleri"));
		T.Add(TEXT("Set.Volume.Monster"), TEXT("Canavar sesleri"));
		T.Add(TEXT("Set.Volume.Other"), TEXT("Diğer sesler"));

		T.Add(TEXT("Set.Difficulty"), TEXT("Zorluk"));
		T.Add(TEXT("Set.Difficulty.Hint"),
			TEXT("Zorluk üretimi etkiler: duvar yoğunluğu, geçit sayısı, erzak ve ganimet bolluğu."));

		T.Add(TEXT("Levels.Title"), TEXT("B Ö L Ü M L E R"));
		T.Add(TEXT("Levels.Locked"), TEXT("KİLİTLİ"));
		T.Add(TEXT("Levels.UnlockedFmt"),
			TEXT("Açılan bölümler: 10 üzerinden %d. Yenileri indikçe açılır — önizlemelerini görmek için daha derine in."));

		T.Add(TEXT("Ach.Title"), TEXT("B A Ş A R I M L A R"));
		T.Add(TEXT("Ach.Unlocked"), TEXT("BAŞARIM"));
		T.Add(TEXT("Ach.ProgressFmt"), TEXT("Açılan: %d / %d     Puan: %d"));

		T.Add(TEXT("Diff.Peaceful.Name"), TEXT("Huzurlu"));
		T.Add(TEXT("Diff.Peaceful.Desc"), TEXT("Canavar ve baskı yok. Sadece keşif ve dünya."));
		T.Add(TEXT("Diff.Easy.Name"), TEXT("Kolay"));
		T.Add(TEXT("Diff.Easy.Desc"), TEXT("Açık, cömert mekanlar. Bir canavar geç gelir."));
		T.Add(TEXT("Diff.Normal.Name"), TEXT("Normal"));
		T.Add(TEXT("Diff.Normal.Desc"), TEXT("Amaçlanan denge: dar koridorlar, açlık, tek bir varlık."));
		T.Add(TEXT("Diff.Hard.Name"), TEXT("Zor"));
		T.Add(TEXT("Diff.Hard.Desc"), TEXT("Daha çok duvar, daha az kapı, kıt ganimet, iki varlık."));
		T.Add(TEXT("Diff.Nightmare.Name"), TEXT("Kabus"));
		T.Add(TEXT("Diff.Nightmare.Desc"), TEXT("Tuzak labirenti, minimum erzak, üç varlık, ölüm kalıcı."));

		T.Add(TEXT("HUD.Sanity"), TEXT("AKIL"));
		T.Add(TEXT("HUD.Flashlight"), TEXT("EL FENERİ"));
		T.Add(TEXT("HUD.Hunger"), TEXT("AÇLIK"));
		T.Add(TEXT("HUD.Thirst"), TEXT("SUSUZLUK"));
		T.Add(TEXT("HUD.Health"), TEXT("SAĞLIK"));
		T.Add(TEXT("HUD.Stamina"), TEXT("DAYANIKLILIK"));
		T.Add(TEXT("HUD.Inventory"), TEXT("ENVANTER"));
		T.Add(TEXT("HUD.Empty"), TEXT("boş"));
		T.Add(TEXT("Prog.Level"), TEXT("Seviye"));
		T.Add(TEXT("Prog.LevelUp"), TEXT("Seviye atlad?"));

		T.Add(TEXT("Interact.Inspect"), TEXT("İncele"));
		T.Add(TEXT("Interact.Throw"), TEXT("Fırlat"));
		T.Add(TEXT("Interact.Drop"), TEXT("Bırak"));
		T.Add(TEXT("Interact.Take"), TEXT("Al"));
		T.Add(TEXT("Interact.Push"), TEXT("İt"));
		T.Add(TEXT("Interact.Use"), TEXT("Kullan"));
		T.Add(TEXT("Interact.Pickup"), TEXT("Al"));

		T.Add(TEXT("Hint.Critical"), TEXT("Kritik durum — hemen ilaç bul"));
		T.Add(TEXT("Hint.Sanity"), TEXT("Akıl tükeniyor — saklan ve nefeslen"));
		T.Add(TEXT("Hint.Hunger"), TEXT("Çok açsın — yemek bul"));
		T.Add(TEXT("Hint.Thirst"), TEXT("Çok susadın — su iç"));
		T.Add(TEXT("Hint.Monster"), TEXT("Yakında — ses çıkarma"));
		T.Add(TEXT("Hint.Poison"), TEXT("Zehirlendin — badem suyu veya ilk yardım çantası bul"));
		T.Add(TEXT("Hint.Radiation"), TEXT("Radyasyon yüksek — buradan uzaklaş"));
		T.Add(TEXT("Hint.Sleep"), TEXT("Bitkinsin — dur ve dinlen"));
		T.Add(TEXT("Hint.Battery"), TEXT("El feneri neredeyse bitti"));

		T.Add(TEXT("Ach.FirstSteps.Name"), TEXT("İlk Adımlar"));
		T.Add(TEXT("Ach.FirstSteps.Desc"), TEXT("Backrooms'a inmek."));
		T.Add(TEXT("Ach.Tourist.Name"), TEXT("Turist"));
		T.Add(TEXT("Ach.Tourist.Desc"), TEXT("3 farklı mekanı ziyaret et."));
		T.Add(TEXT("Ach.Explorer.Name"), TEXT("Kaşif"));
		T.Add(TEXT("Ach.Explorer.Desc"), TEXT("5 farklı mekanı ziyaret et."));
		T.Add(TEXT("Ach.DeepDiver.Name"), TEXT("Derinlerde"));
		T.Add(TEXT("Ach.DeepDiver.Desc"), TEXT("7 farklı mekanı ziyaret et."));
		T.Add(TEXT("Ach.AllLevels.Name"), TEXT("Tüm Kat"));
		T.Add(TEXT("Ach.AllLevels.Desc"), TEXT("10 mekanın hepsini ziyaret et."));
		T.Add(TEXT("Ach.Survivor.Name"), TEXT("Hayatta Kalan"));
		T.Add(TEXT("Ach.Survivor.Desc"), TEXT("Backrooms'ta 15 dakika dayan."));
		T.Add(TEXT("Ach.Marathon.Name"), TEXT("Maratoncu"));
		T.Add(TEXT("Ach.Marathon.Desc"), TEXT("Yer altında 3 kilometre yürü."));
		T.Add(TEXT("Ach.Scavenger.Name"), TEXT("Toplayıcı"));
		T.Add(TEXT("Ach.Scavenger.Desc"), TEXT("10 eşya topla."));
		T.Add(TEXT("Ach.Pharmacist.Name"), TEXT("Eczacı"));
		T.Add(TEXT("Ach.Pharmacist.Desc"), TEXT("5 ilaç kullan."));
		T.Add(TEXT("Ach.Hydrated.Name"), TEXT("Badem Tadı"));
		T.Add(TEXT("Ach.Hydrated.Desc"), TEXT("5 porsiyon badem suyu iç."));
		T.Add(TEXT("Ach.MonsterAware.Name"), TEXT("Yakında"));
		T.Add(TEXT("Ach.MonsterAware.Desc"), TEXT("Bir varlıkla karşılaş ve hayatta kal."));
		T.Add(TEXT("Ach.EscapeArtist.Name"), TEXT("İlk Çıkış"));
		T.Add(TEXT("Ach.EscapeArtist.Desc"), TEXT("Bir çıkış bul."));
		T.Add(TEXT("Ach.FrequentFlyer.Name"), TEXT("Sık Gelen"));
		T.Add(TEXT("Ach.FrequentFlyer.Desc"), TEXT("5 çıkıştan geç."));
		T.Add(TEXT("Ach.DeepFear.Name"), TEXT("Korku Sınırı"));
		T.Add(TEXT("Ach.DeepFear.Desc"), TEXT("Ortam baskısını en üste çıkar."));
		T.Add(TEXT("Ach.Dead.Name"), TEXT("Onlardan Biri"));
		T.Add(TEXT("Ach.Dead.Desc"), TEXT("Backrooms'ta öl."));

		T.Add(TEXT("Bind.MoveForwardPlus"), TEXT("İleri"));
		T.Add(TEXT("Bind.MoveForwardMinus"), TEXT("Geri"));
		T.Add(TEXT("Bind.MoveRightPlus"), TEXT("Sağ"));
		T.Add(TEXT("Bind.MoveRightMinus"), TEXT("Sol"));
		T.Add(TEXT("Bind.Jump"), TEXT("Zıpla"));
		T.Add(TEXT("Bind.Sprint"), TEXT("Koş"));
		T.Add(TEXT("Bind.Flashlight"), TEXT("Fener"));
		T.Add(TEXT("Bind.View"), TEXT("1. / 3. kişi görünümü"));
		T.Add(TEXT("Bind.Attack"), TEXT("Yumruk at"));
		T.Add(TEXT("Bind.Grab"), TEXT("Eşya al / bırak"));
		T.Add(TEXT("Bind.Push"), TEXT("Eşyayı it"));
		T.Add(TEXT("Bind.Throw"), TEXT("Eşyayı fırlat"));
		T.Add(TEXT("Bind.Inspect"), TEXT("Eşyayı incele"));
		T.Add(TEXT("Bind.Use"), TEXT("Eşyayı kullan"));
		T.Add(TEXT("Bind.Slot1"), TEXT("Yuva 1"));
		T.Add(TEXT("Bind.Slot2"), TEXT("Yuva 2"));
		T.Add(TEXT("Bind.Slot3"), TEXT("Yuva 3"));
		T.Add(TEXT("Bind.Slot4"), TEXT("Yuva 4"));
		T.Add(TEXT("Bind.Inventory"), TEXT("Envanter"));
		T.Add(TEXT("Bind.PauseMenu"), TEXT("Duraklat / menü"));
		T.Add(TEXT("Bind.Mouse"), TEXT("Fare"));
		T.Add(TEXT("Bind.Look"), TEXT("Bak"));
		T.Add(TEXT("Bind.Change"), TEXT("değiştir"));
		T.Add(TEXT("Bind.PressKey"), TEXT("Atamak için bir tuşa bas... (İptal için Esc)"));
		T.Add(TEXT("Bind.Cancel"), TEXT("İPTAL  ×"));
		T.Add(TEXT("Bind.Reset"), TEXT("Tüm tuşları varsayılana sıfırla"));

		T.Add(TEXT("Ach.WellFed.Name"), TEXT("İyi Beslenmiş"));
		T.Add(TEXT("Ach.WellFed.Desc"), TEXT("10 porsiyon yemek ye."));
		T.Add(TEXT("Ach.Electrician.Name"), TEXT("Elektrikçi"));
		T.Add(TEXT("Ach.Electrician.Desc"), TEXT("Fenere 10 pil tak."));
		T.Add(TEXT("Ach.Sprinter.Name"), TEXT("Atlet"));
		T.Add(TEXT("Ach.Sprinter.Desc"), TEXT("50 kez koşmaya başla."));
		T.Add(TEXT("Ach.Cartographer.Name"), TEXT("Haritacı"));
		T.Add(TEXT("Ach.Cartographer.Desc"), TEXT("200 odayı geç."));
		T.Add(TEXT("Ach.Speedrun.Name"), TEXT("Speedrunner"));
		T.Add(TEXT("Ach.Speedrun.Desc"), TEXT("Yer altında 5 kilometre kat et."));
		T.Add(TEXT("Ach.Gourmand.Name"), TEXT("Gurme"));
		T.Add(TEXT("Ach.Gourmand.Desc"), TEXT("Backrooms'ta 30 dakika dayan."));

		T.Add(TEXT("GameOver.Title"), TEXT("ÖLDÜN"));
		T.Add(TEXT("GameOver.Subtitle"), TEXT("Backrooms bırakmaz. Tekrar dene."));
		T.Add(TEXT("GameOver.Restart"), TEXT("Yeniden Başlat"));
		T.Add(TEXT("GameOver.Quit"), TEXT("Çık"));

		T.Add(TEXT("Level.W0"), TEXT("L0 · Lobi"));
		T.Add(TEXT("Level.W1"), TEXT("L1 · Yerleşim Bölgesi"));
		T.Add(TEXT("Level.W2"), TEXT("L2 · Su Hattı"));
		T.Add(TEXT("Level.W3"), TEXT("L3 · Elektrik Santrali"));
		T.Add(TEXT("Level.W4"), TEXT("L4 · Ofisler"));
		T.Add(TEXT("Level.W5"), TEXT("L5 · Otel"));
		T.Add(TEXT("Level.W6"), TEXT("L6 · Karanlık"));
		T.Add(TEXT("Level.W7"), TEXT("L7 · Okyanus"));
		T.Add(TEXT("Level.W8"), TEXT("L8 · Mağaralar"));
		T.Add(TEXT("Level.W9"), TEXT("L9 · Hastane"));

		T.Add(TEXT("Item.AlmondWater.Name"), TEXT("Badem Suyu"));
		T.Add(TEXT("Item.AlmondWater.Desc"), TEXT("Susuzluğu giderir ve aklı biraz düzeltir."));
		T.Add(TEXT("Item.CanFood.Name"), TEXT("Konserve"));
		T.Add(TEXT("Item.CanFood.Desc"), TEXT("Konserve et. Açlığı giderir."));
		T.Add(TEXT("Item.MedKit.Name"), TEXT("İlk Yardım Çantası"));
		T.Add(TEXT("Item.MedKit.Desc"), TEXT("Kanamayı durdurur. Sağlığı düzeltir."));
		T.Add(TEXT("Item.Pill.Name"), TEXT("Sakinleştirici"));
		T.Add(TEXT("Item.Pill.Desc"), TEXT("Sinirleri yatıştırır. Aklı düzeltir."));
		T.Add(TEXT("Item.Energy.Name"), TEXT("Enerji İçeceği"));
		T.Add(TEXT("Item.Energy.Desc"), TEXT("Canlandırır ve kafayı biraz açar."));
		T.Add(TEXT("Item.Battery.Name"), TEXT("Pil"));
		T.Add(TEXT("Item.Battery.Desc"), TEXT("Fener için güç hücresi."));

		T.Add(TEXT("Event.LightFlicker"), TEXT("Titreyen ışık"));
		T.Add(TEXT("Event.LightOutage"), TEXT("Işıklar söndü!"));
		T.Add(TEXT("Event.BoxDisappear"), TEXT("Bir kutu kayboldu..."));
		T.Add(TEXT("Event.BoxAppear"), TEXT("Bir kutu belirdi!"));
		T.Add(TEXT("Event.EntityGrowl"), TEXT("Karanlıktan bir hırlama..."));
		T.Add(TEXT("Event.EntityFootsteps"), TEXT("Duvardan adım sesleri..."));
		T.Add(TEXT("Event.Whisper"), TEXT("Fısıltı..."));
		T.Add(TEXT("Event.DistantBang"), TEXT("Uzaktan bir gümbürtü!"));
		T.Add(TEXT("Event.WallDrawing"), TEXT("Duvarda bir çizim..."));
		T.Add(TEXT("Event.PipeCreak"), TEXT("Bir boru gıcırdıyor"));
		T.Add(TEXT("Event.DoorSlam"), TEXT("Bir kapı çarpıyor!"));
		T.Add(TEXT("Event.EmergencyLight"), TEXT("Acil durum ışığı"));
		T.Add(TEXT("Event.FogIncrease"), TEXT("Sis yoğunlaşıyor..."));
		T.Add(TEXT("Event.StaticNoise"), TEXT("Parazit..."));
		T.Add(TEXT("Event.FootprintAppear"), TEXT("Zeminde ayak izleri..."));
		T.Add(TEXT("Event.Unknown"), TEXT("Bilinmeyen olay"));

		T.Add(TEXT("Loading.Level"), TEXT("Seviye yükleniyor..."));
		T.Add(TEXT("Loading.World"), TEXT("Backrooms yükleniyor... %d%% (parça: %d)"));
		T.Add(TEXT("Loading.Ready"), TEXT("Dünya hazır! Gerçeklik hatası..."));
		T.Add(TEXT("Loading.Loaded"), TEXT("Seviye yüklendi"));
		T.Add(TEXT("Loading.Noclip"), TEXT("Noclip: Backrooms'a düşüş..."));
		T.Add(TEXT("Loading.Backrooms"), TEXT("Backrooms yükleniyor..."));
		T.Add(TEXT("Loading.Chunks"), TEXT("chunk"));
		T.Add(TEXT("Loading.WorldReady"), TEXT("Dünya hazır! Gerçeklik hatası..."));
		T.Add(TEXT("Loading.LevelDone"), TEXT("Bölüm yüklendi"));

		T.Add(TEXT("Keys.SpaceBar"), TEXT("Boşluk"));
		T.Add(TEXT("Keys.LeftShift"), TEXT("Shift"));
		T.Add(TEXT("Keys.RightShift"), TEXT("Shift (sağ)"));
		T.Add(TEXT("Keys.LeftCtrl"), TEXT("Ctrl"));
		T.Add(TEXT("Keys.RightCtrl"), TEXT("Ctrl (sağ)"));
		T.Add(TEXT("Keys.LeftAlt"), TEXT("Alt"));
		T.Add(TEXT("Keys.RightAlt"), TEXT("AltGr"));
		T.Add(TEXT("Keys.Escape"), TEXT("Esc"));
		T.Add(TEXT("Keys.LeftMouse"), TEXT("Sol tık"));
		T.Add(TEXT("Keys.RightMouse"), TEXT("Sağ tık"));
		T.Add(TEXT("Keys.MiddleMouse"), TEXT("Tekerlek (tık)"));
		T.Add(TEXT("Keys.ThumbMouse1"), TEXT("Yan tuş 1"));
		T.Add(TEXT("Keys.ThumbMouse2"), TEXT("Yan tuş 2"));
		T.Add(TEXT("Keys.MouseX"), TEXT("Fare X"));
		T.Add(TEXT("Keys.MouseY"), TEXT("Fare Y"));
		T.Add(TEXT("Keys.MouseWheel"), TEXT("Fare tekerleği"));
		T.Add(TEXT("Keys.Backspace"), TEXT("Geri Al"));
		T.Add(TEXT("Keys.Tab"), TEXT("Tab"));
		T.Add(TEXT("Keys.Enter"), TEXT("Enter"));
		T.Add(TEXT("Keys.On"), TEXT("AÇIK"));
		T.Add(TEXT("Keys.Off"), TEXT("KAPALI"));

		T.Add(TEXT("Set.DisplayMode.Fullscreen"), TEXT("Tam ekran"));
		T.Add(TEXT("Set.DisplayMode.Borderless"), TEXT("Kenarlıksız"));
		T.Add(TEXT("Set.DisplayMode.Windowed"), TEXT("Pencere"));
		T.Add(TEXT("Set.RecordingEffect.Off"), TEXT("Kapalı"));
		T.Add(TEXT("Set.RecordingEffect.Light"), TEXT("Hafif film efekti"));
		T.Add(TEXT("Set.RecordingEffect.Medium"), TEXT("Orta film efekti"));
		T.Add(TEXT("Set.RecordingEffect.Strong"), TEXT("Güçlü film efekti"));
		T.Add(TEXT("Set.Quality.Overall"), TEXT("Genel kalite"));
		T.Add(TEXT("Set.PotatoMode"), TEXT("Patates modu"));
		T.Add(TEXT("Set.PotatoMode.Hint"), TEXT("Görsellerden ödün vererek maksimum FPS."));
		T.Add(TEXT("Set.Graphics"), TEXT("Grafik"));
		T.Add(TEXT("Q.Low"), TEXT("Düşük"));
		T.Add(TEXT("Q.Medium"), TEXT("Orta"));
		T.Add(TEXT("Q.High"), TEXT("Yüksek"));
		T.Add(TEXT("Q.Epic"), TEXT("Epik"));
		T.Add(TEXT("Q.Ultra"), TEXT("Ultra"));

		T.Add(TEXT("Pause.Resume"), TEXT("Devam Et"));
		T.Add(TEXT("Pause.Restart"), TEXT("Yeniden Başlat"));
		T.Add(TEXT("Pause.Quit"), TEXT("Oyundan Çık"));

		T.Add(TEXT("Interact.TakeOff"), TEXT("Kaldır"));

		T.Add(TEXT("Menu.StressTest"), TEXT("STRES TESTI"));
		T.Add(TEXT("HUD.Fps"), TEXT("FPS"));
		T.Add(TEXT("HUD.FpsAvg"), TEXT("Ort"));
		T.Add(TEXT("HUD.FpsOneLow"), TEXT("1% Dusuk"));
		T.Add(TEXT("Loading.StressTest"), TEXT("Stres testi..."));
	}
}
