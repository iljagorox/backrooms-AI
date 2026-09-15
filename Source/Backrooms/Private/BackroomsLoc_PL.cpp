#include "BackroomsLocRegistry.h"

// Polski.
namespace BackroomsLoc
{
	void Register_PL(FLanguageTable& T)
	{
		T.Add(TEXT("Menu.NewGame"), TEXT("Nowa gra"));
		T.Add(TEXT("Menu.Continue"), TEXT("Kontynuuj"));
		T.Add(TEXT("Menu.Levels"), TEXT("Poziomy"));
		T.Add(TEXT("Menu.Achievements"), TEXT("Osiągnięcia"));
		T.Add(TEXT("Menu.Settings"), TEXT("Ustawienia"));
		T.Add(TEXT("Menu.Controls"), TEXT("Sterowanie"));
		T.Add(TEXT("Menu.Quit"), TEXT("Wyjście"));
		T.Add(TEXT("Menu.Subtitle"), TEXT("LIMINALNE ZANURZENIE"));
		T.Add(TEXT("Menu.Back"), TEXT("<   W S T E C Z"));

		T.Add(TEXT("Tab.Screen"), TEXT("EKRAN"));
		T.Add(TEXT("Tab.Quality"), TEXT("JAKOŚĆ"));
		T.Add(TEXT("Tab.Camera"), TEXT("KAMERA"));
		T.Add(TEXT("Tab.Sound"), TEXT("DŹWIĘK"));
		T.Add(TEXT("Tab.Game"), TEXT("GRA"));
		T.Add(TEXT("Tab.Controls"), TEXT("STEROWANIE"));
		T.Add(TEXT("Settings.Title"), TEXT("U S T A W I E N I A"));
		T.Add(TEXT("Settings.Categories"), TEXT("K A T E G O R I E"));

		T.Add(TEXT("Set.Resolution"), TEXT("Rozdzielczość"));
		T.Add(TEXT("Set.DisplayMode"), TEXT("Tryb ekranu"));
		T.Add(TEXT("Set.VSync"), TEXT("VSync"));
		T.Add(TEXT("Set.RenderScale"), TEXT("Skala renderowania"));
		T.Add(TEXT("Set.Gamma"), TEXT("Gamma"));
		T.Add(TEXT("Set.Language"), TEXT("Język"));

		T.Add(TEXT("Set.Shadows"), TEXT("Cienie"));
		T.Add(TEXT("Set.Textures"), TEXT("Tekstury"));
		T.Add(TEXT("Set.AntiAliasing"), TEXT("Wygładzanie"));
		T.Add(TEXT("Set.PostProcess"), TEXT("Postprocessing"));
		T.Add(TEXT("Set.Effects"), TEXT("Efekty"));
		T.Add(TEXT("Set.ViewDistance"), TEXT("Zasięg widzenia"));
		T.Add(TEXT("Set.DepthOfField"), TEXT("Głębia ostrości"));
		T.Add(TEXT("Set.Bloom"), TEXT("Poświata (Bloom)"));
		T.Add(TEXT("Set.Lumen"), TEXT("Lumen (oświetlenie)"));
		T.Add(TEXT("Set.Upscale"), TEXT("DLSS / Skalowanie"));

		T.Add(TEXT("Set.FOV"), TEXT("Pole widzenia"));
		T.Add(TEXT("Set.Sensitivity"), TEXT("Czułość"));
		T.Add(TEXT("Set.RecordingEffect"), TEXT("Efekt kamery nagrywającej"));

		T.Add(TEXT("Set.Volume.Master"), TEXT("Ogólna"));
		T.Add(TEXT("Set.Volume.Menu"), TEXT("Muzyka / Menu"));
		T.Add(TEXT("Set.Volume.Game"), TEXT("Dźwięki w grze"));
		T.Add(TEXT("Set.Volume.Monster"), TEXT("Dźwięki potworów"));
		T.Add(TEXT("Set.Volume.Other"), TEXT("Inne dźwięki"));

		T.Add(TEXT("Set.Difficulty"), TEXT("Poziom trudności"));
		T.Add(TEXT("Set.Difficulty.Hint"),
			TEXT("Poziom trudności wpływa na generację: gęstość ścian, liczbę przejść, hojność zapasów i łupów."));

		T.Add(TEXT("Levels.Title"), TEXT("P O Z I O M Y"));
		T.Add(TEXT("Levels.Locked"), TEXT("ZABLOKOWANE"));
		T.Add(TEXT("Levels.UnlockedFmt"),
			TEXT("Odblokowane poziomy: %d z 10. Nowe otwierają się w miarę schodzenia — idź głębiej, by zobaczyć ich podglądy."));

		T.Add(TEXT("Ach.Title"), TEXT("O S I Ą G N I Ę C I A"));
		T.Add(TEXT("Ach.Unlocked"), TEXT("OSIĄGNIĘCIE"));
		T.Add(TEXT("Ach.ProgressFmt"), TEXT("Odblokowano: %d / %d     Punkty: %d"));

		T.Add(TEXT("Diff.Peaceful.Name"), TEXT("Spokojny"));
		T.Add(TEXT("Diff.Peaceful.Desc"), TEXT("Bez potworów i presji. Tylko eksploracja i świat."));
		T.Add(TEXT("Diff.Easy.Name"), TEXT("Łatwy"));
		T.Add(TEXT("Diff.Easy.Desc"), TEXT("Otwarte, hojne lokacje. Potwór pojawia się późno."));
		T.Add(TEXT("Diff.Normal.Name"), TEXT("Normalny"));
		T.Add(TEXT("Diff.Normal.Desc"), TEXT("Zamierzony balans: ciasne korytarze, głód, samotna istota."));
		T.Add(TEXT("Diff.Hard.Name"), TEXT("Trudny"));
		T.Add(TEXT("Diff.Hard.Desc"), TEXT("Więcej ścian i mniej drzwi, skąpe łupy, dwie istoty."));
		T.Add(TEXT("Diff.Nightmare.Name"), TEXT("Koszmar"));
		T.Add(TEXT("Diff.Nightmare.Desc"), TEXT("Labirynt-pułapka, minimum zapasów, trzy istoty, śmierć jest ostateczna."));

		T.Add(TEXT("HUD.Sanity"), TEXT("POCZYTALNOŚĆ"));
		T.Add(TEXT("HUD.Flashlight"), TEXT("LATARKA"));
		T.Add(TEXT("HUD.Hunger"), TEXT("GŁÓD"));
		T.Add(TEXT("HUD.Thirst"), TEXT("PRAGNIENIE"));
		T.Add(TEXT("HUD.Health"), TEXT("ZDROWIE"));
		T.Add(TEXT("HUD.Stamina"), TEXT("WYTRZYMAŁOŚĆ"));
		T.Add(TEXT("HUD.Inventory"), TEXT("EKWIPUNEK"));
		T.Add(TEXT("HUD.Empty"), TEXT("puste"));
		T.Add(TEXT("Prog.Level"), TEXT("Poziom"));
		T.Add(TEXT("Prog.LevelUp"), TEXT("Nowy poziom"));

		T.Add(TEXT("Interact.Inspect"), TEXT("Obejrzyj"));
		T.Add(TEXT("Interact.Throw"), TEXT("Rzuć"));
		T.Add(TEXT("Interact.Drop"), TEXT("Upuść"));
		T.Add(TEXT("Interact.Take"), TEXT("Weź"));
		T.Add(TEXT("Interact.Push"), TEXT("Popchnij"));
		T.Add(TEXT("Interact.Use"), TEXT("Użyj"));
		T.Add(TEXT("Interact.Pickup"), TEXT("Podnieś"));

		T.Add(TEXT("Hint.Critical"), TEXT("Stan krytyczny — natychmiast znajdź lekarstwo"));
		T.Add(TEXT("Hint.Sanity"), TEXT("Poczytalność na wyczerpaniu — ukryj się i odpocznij"));
		T.Add(TEXT("Hint.Hunger"), TEXT("Wielki głód — znajdź jedzenie"));
		T.Add(TEXT("Hint.Thirst"), TEXT("Wielkie pragnienie — napij się wody"));
		T.Add(TEXT("Hint.Monster"), TEXT("Jest blisko — nie rób hałasu"));
		T.Add(TEXT("Hint.Poison"), TEXT("Jesteś zatruty — potrzebujesz wody migdałowej lub apteczki"));
		T.Add(TEXT("Hint.Radiation"), TEXT("Wysokie promieniowanie — uciekaj stąd"));
		T.Add(TEXT("Hint.Sleep"), TEXT("Jesteś wyczerpany — zatrzymaj się i odpocznij"));
		T.Add(TEXT("Hint.Battery"), TEXT("Latarka prawie rozładowana"));

		T.Add(TEXT("Ach.FirstSteps.Name"), TEXT("Pierwsze kroki"));
		T.Add(TEXT("Ach.FirstSteps.Desc"), TEXT("Zejść do Backrooms."));
		T.Add(TEXT("Ach.Tourist.Name"), TEXT("Turysta"));
		T.Add(TEXT("Ach.Tourist.Desc"), TEXT("Odwiedzić 3 różne lokacje."));
		T.Add(TEXT("Ach.Explorer.Name"), TEXT("Odkrywca"));
		T.Add(TEXT("Ach.Explorer.Desc"), TEXT("Odwiedzić 5 różnych lokacji."));
		T.Add(TEXT("Ach.DeepDiver.Name"), TEXT("W głębinach"));
		T.Add(TEXT("Ach.DeepDiver.Desc"), TEXT("Odwiedzić 7 różnych lokacji."));
		T.Add(TEXT("Ach.AllLevels.Name"), TEXT("Całe piętro"));
		T.Add(TEXT("Ach.AllLevels.Desc"), TEXT("Odwiedzić wszystkie 10 lokacji."));
		T.Add(TEXT("Ach.Survivor.Name"), TEXT("Ocalony"));
		T.Add(TEXT("Ach.Survivor.Desc"), TEXT("Przetrwać 15 minut w Backrooms."));
		T.Add(TEXT("Ach.Marathon.Name"), TEXT("Maratończyk"));
		T.Add(TEXT("Ach.Marathon.Desc"), TEXT("Przejść 3 kilometry pod ziemią."));
		T.Add(TEXT("Ach.Scavenger.Name"), TEXT("Zbieracz"));
		T.Add(TEXT("Ach.Scavenger.Desc"), TEXT("Podnieść 10 przedmiotów."));
		T.Add(TEXT("Ach.Pharmacist.Name"), TEXT("Farmaceuta"));
		T.Add(TEXT("Ach.Pharmacist.Desc"), TEXT("Użyć 5 leków."));
		T.Add(TEXT("Ach.Hydrated.Name"), TEXT("Migdałowy smak"));
		T.Add(TEXT("Ach.Hydrated.Desc"), TEXT("Wypić 5 porcji wody migdałowej."));
		T.Add(TEXT("Ach.MonsterAware.Name"), TEXT("Jest blisko"));
		T.Add(TEXT("Ach.MonsterAware.Desc"), TEXT("Spotkać istotę i przetrwać."));
		T.Add(TEXT("Ach.EscapeArtist.Name"), TEXT("Pierwsze wyjście"));
		T.Add(TEXT("Ach.EscapeArtist.Desc"), TEXT("Znaleźć wyjście."));
		T.Add(TEXT("Ach.FrequentFlyer.Name"), TEXT("Częsty gość"));
		T.Add(TEXT("Ach.FrequentFlyer.Desc"), TEXT("Przejść przez 5 wyjść."));
		T.Add(TEXT("Ach.DeepFear.Name"), TEXT("Granica strachu"));
		T.Add(TEXT("Ach.DeepFear.Desc"), TEXT("Doprowadzić presję otoczenia do maksimum."));
		T.Add(TEXT("Ach.Dead.Name"), TEXT("Jeden z nich"));
		T.Add(TEXT("Ach.Dead.Desc"), TEXT("Zginąć w Backrooms."));

		T.Add(TEXT("Bind.MoveForwardPlus"), TEXT("Naprzód"));
		T.Add(TEXT("Bind.MoveForwardMinus"), TEXT("Wstecz"));
		T.Add(TEXT("Bind.MoveRightPlus"), TEXT("Prawo"));
		T.Add(TEXT("Bind.MoveRightMinus"), TEXT("Lewo"));
		T.Add(TEXT("Bind.Jump"), TEXT("Skok"));
		T.Add(TEXT("Bind.Sprint"), TEXT("Bieg"));
		T.Add(TEXT("Bind.Flashlight"), TEXT("Latarka"));
		T.Add(TEXT("Bind.View"), TEXT("Widok 1. / 3. osoba"));
		T.Add(TEXT("Bind.Attack"), TEXT("Cios pięścią"));
		T.Add(TEXT("Bind.Grab"), TEXT("Podnieś / połóż przedmiot"));
		T.Add(TEXT("Bind.Push"), TEXT("Pchnij przedmiot"));
		T.Add(TEXT("Bind.Throw"), TEXT("Rzuć przedmiot"));
		T.Add(TEXT("Bind.Inspect"), TEXT("Obejrzyj przedmiot"));
		T.Add(TEXT("Bind.Use"), TEXT("Użyj przedmiotu"));
		T.Add(TEXT("Bind.Slot1"), TEXT("Slot 1"));
		T.Add(TEXT("Bind.Slot2"), TEXT("Slot 2"));
		T.Add(TEXT("Bind.Slot3"), TEXT("Slot 3"));
		T.Add(TEXT("Bind.Slot4"), TEXT("Slot 4"));
		T.Add(TEXT("Bind.Inventory"), TEXT("Ekwipunek"));
		T.Add(TEXT("Bind.PauseMenu"), TEXT("Pauza / menu"));
		T.Add(TEXT("Bind.Mouse"), TEXT("Mysz"));
		T.Add(TEXT("Bind.Look"), TEXT("Rozglądanie"));
		T.Add(TEXT("Bind.Change"), TEXT("zmień"));
		T.Add(TEXT("Bind.PressKey"), TEXT("Naciśnij klawisz, aby przypisać... (Esc — anuluj)"));
		T.Add(TEXT("Bind.Cancel"), TEXT("ANULUJ  ×"));
		T.Add(TEXT("Bind.Reset"), TEXT("Przywróć wszystkie klawisze domyślne"));

		T.Add(TEXT("Ach.WellFed.Name"), TEXT("Najedzony"));
		T.Add(TEXT("Ach.WellFed.Desc"), TEXT("Zjeść 10 porcji jedzenia."));
		T.Add(TEXT("Ach.Electrician.Name"), TEXT("Elektryk"));
		T.Add(TEXT("Ach.Electrician.Desc"), TEXT("Włożyć 10 baterii do latarki."));
		T.Add(TEXT("Ach.Sprinter.Name"), TEXT("Sprinter"));
		T.Add(TEXT("Ach.Sprinter.Desc"), TEXT("Ruszyć biegiem 50 razy."));
		T.Add(TEXT("Ach.Cartographer.Name"), TEXT("Kartograf"));
		T.Add(TEXT("Ach.Cartographer.Desc"), TEXT("Przejść przez 200 pomieszczeń."));
		T.Add(TEXT("Ach.Speedrun.Name"), TEXT("Speedrunner"));
		T.Add(TEXT("Ach.Speedrun.Desc"), TEXT("Pokonać 5 kilometrów pod ziemią."));
		T.Add(TEXT("Ach.Gourmand.Name"), TEXT("Smakosz"));
		T.Add(TEXT("Ach.Gourmand.Desc"), TEXT("Przetrwać 30 minut w Backrooms."));

		T.Add(TEXT("GameOver.Title"), TEXT("ZGINĄŁEŚ"));
		T.Add(TEXT("GameOver.Subtitle"), TEXT("Backrooms nie wypuszcza. Spróbuj ponownie."));
		T.Add(TEXT("GameOver.Restart"), TEXT("Od nowa"));
		T.Add(TEXT("GameOver.Quit"), TEXT("Wyjdź"));

		T.Add(TEXT("Level.W0"), TEXT("L0 · Hol"));
		T.Add(TEXT("Level.W1"), TEXT("L1 · Strefa zamieszkana"));
		T.Add(TEXT("Level.W2"), TEXT("L2 · Wodociągi"));
		T.Add(TEXT("Level.W3"), TEXT("L3 · Elektrownia"));
		T.Add(TEXT("Level.W4"), TEXT("L4 · Biura"));
		T.Add(TEXT("Level.W5"), TEXT("L5 · Hotel"));
		T.Add(TEXT("Level.W6"), TEXT("L6 · Ciemność"));
		T.Add(TEXT("Level.W7"), TEXT("L7 · Ocean"));
		T.Add(TEXT("Level.W8"), TEXT("L8 · Jaskinie"));
		T.Add(TEXT("Level.W9"), TEXT("L9 · Szpital"));

		T.Add(TEXT("Item.AlmondWater.Name"), TEXT("Woda migdałowa"));
		T.Add(TEXT("Item.AlmondWater.Desc"), TEXT("Gaszi pragnienie i lekko przywraca zdrowie psychiczne."));
		T.Add(TEXT("Item.CanFood.Name"), TEXT("Konserwa"));
		T.Add(TEXT("Item.CanFood.Desc"), TEXT("Mięso w puszce. Gaszi głód."));
		T.Add(TEXT("Item.MedKit.Name"), TEXT("Apteczka"));
		T.Add(TEXT("Item.MedKit.Desc"), TEXT("Tamuje krwawienie. Przywraca zdrowie."));
		T.Add(TEXT("Item.Pill.Name"), TEXT("Środek uspokajający"));
		T.Add(TEXT("Item.Pill.Desc"), TEXT("Koi nerwy. Przywraca zdrowie psychiczne."));
		T.Add(TEXT("Item.Energy.Name"), TEXT("Energetyk"));
		T.Add(TEXT("Item.Energy.Desc"), TEXT("Orzeźwia i lekko rozjaśnia umysł."));
		T.Add(TEXT("Item.Battery.Name"), TEXT("Bateria"));
		T.Add(TEXT("Item.Battery.Desc"), TEXT("Zasilanie do latarki."));

		T.Add(TEXT("Event.LightFlicker"), TEXT("Migające światło"));
		T.Add(TEXT("Event.LightOutage"), TEXT("Zgasło światło!"));
		T.Add(TEXT("Event.BoxDisappear"), TEXT("Skrzynia zniknęła..."));
		T.Add(TEXT("Event.BoxAppear"), TEXT("Pojawiła się skrzynia!"));
		T.Add(TEXT("Event.EntityGrowl"), TEXT("Warkot z ciemności..."));
		T.Add(TEXT("Event.EntityFootsteps"), TEXT("Kroki za ścianą..."));
		T.Add(TEXT("Event.Whisper"), TEXT("Szept..."));
		T.Add(TEXT("Event.DistantBang"), TEXT("Huk z daleka!"));
		T.Add(TEXT("Event.WallDrawing"), TEXT("Rysunek na ścianie..."));
		T.Add(TEXT("Event.PipeCreak"), TEXT("Skrzypi rura"));
		T.Add(TEXT("Event.DoorSlam"), TEXT("Trzaśnięcie drzwi!"));
		T.Add(TEXT("Event.EmergencyLight"), TEXT("Awaryjne światło"));
		T.Add(TEXT("Event.FogIncrease"), TEXT("Mgła gęstnieje..."));
		T.Add(TEXT("Event.StaticNoise"), TEXT("Zakłócenia..."));
		T.Add(TEXT("Event.FootprintAppear"), TEXT("Ślady na podłodze..."));
		T.Add(TEXT("Event.Unknown"), TEXT("Nieznane zdarzenie"));

		T.Add(TEXT("Loading.Level"), TEXT("Ładowanie poziomu..."));
		T.Add(TEXT("Loading.World"), TEXT("Ładowanie Backrooms... %d%% (kawałków: %d)"));
		T.Add(TEXT("Loading.Ready"), TEXT("Świat gotowy! Usterka rzeczywistości..."));
		T.Add(TEXT("Loading.Loaded"), TEXT("Poziom załadowany"));
		T.Add(TEXT("Loading.Noclip"), TEXT("Noclip: wpadanie do Backrooms..."));
		T.Add(TEXT("Loading.Backrooms"), TEXT("Ładowanie Backrooms..."));
		T.Add(TEXT("Loading.Chunks"), TEXT("chunków"));
		T.Add(TEXT("Loading.WorldReady"), TEXT("Świat gotowy! Glitch rzeczywistości..."));
		T.Add(TEXT("Loading.LevelDone"), TEXT("Poziom załadowany"));

		T.Add(TEXT("Keys.SpaceBar"), TEXT("Spacja"));
		T.Add(TEXT("Keys.LeftShift"), TEXT("Shift"));
		T.Add(TEXT("Keys.RightShift"), TEXT("Shift (prawy)"));
		T.Add(TEXT("Keys.LeftCtrl"), TEXT("Ctrl"));
		T.Add(TEXT("Keys.RightCtrl"), TEXT("Ctrl (prawy)"));
		T.Add(TEXT("Keys.LeftAlt"), TEXT("Alt"));
		T.Add(TEXT("Keys.RightAlt"), TEXT("AltGr"));
		T.Add(TEXT("Keys.Escape"), TEXT("Esc"));
		T.Add(TEXT("Keys.LeftMouse"), TEXT("LPM"));
		T.Add(TEXT("Keys.RightMouse"), TEXT("PPM"));
		T.Add(TEXT("Keys.MiddleMouse"), TEXT("Kółko (klik)"));
		T.Add(TEXT("Keys.ThumbMouse1"), TEXT("Boczny przycisk 1"));
		T.Add(TEXT("Keys.ThumbMouse2"), TEXT("Boczny przycisk 2"));
		T.Add(TEXT("Keys.MouseX"), TEXT("Mysz X"));
		T.Add(TEXT("Keys.MouseY"), TEXT("Mysz Y"));
		T.Add(TEXT("Keys.MouseWheel"), TEXT("Kółko myszy"));
		T.Add(TEXT("Keys.Backspace"), TEXT("Backspace"));
		T.Add(TEXT("Keys.Tab"), TEXT("Tab"));
		T.Add(TEXT("Keys.Enter"), TEXT("Enter"));
		T.Add(TEXT("Keys.On"), TEXT("TAK"));
		T.Add(TEXT("Keys.Off"), TEXT("NIE"));

		T.Add(TEXT("Set.DisplayMode.Fullscreen"), TEXT("Pełny ekran"));
		T.Add(TEXT("Set.DisplayMode.Borderless"), TEXT("Bez ramki"));
		T.Add(TEXT("Set.DisplayMode.Windowed"), TEXT("Okno"));
		T.Add(TEXT("Set.RecordingEffect.Off"), TEXT("Wyłączony"));
		T.Add(TEXT("Set.RecordingEffect.Light"), TEXT("Lekkie zniekształcenie taśmy"));
		T.Add(TEXT("Set.RecordingEffect.Medium"), TEXT("Średnie zniekształcenie taśmy"));
		T.Add(TEXT("Set.RecordingEffect.Strong"), TEXT("Mocne zniekształcenie taśmy"));
		T.Add(TEXT("Set.Quality.Overall"), TEXT("Ogólna jakość"));
		T.Add(TEXT("Set.PotatoMode"), TEXT("Tryb ziemniaczany"));
		T.Add(TEXT("Set.PotatoMode.Hint"), TEXT("Maksymalne FPS kosztem grafiki."));
		T.Add(TEXT("Set.Graphics"), TEXT("Grafika"));
		T.Add(TEXT("Q.Low"), TEXT("Niska"));
		T.Add(TEXT("Q.Medium"), TEXT("Średnia"));
		T.Add(TEXT("Q.High"), TEXT("Wysoka"));
		T.Add(TEXT("Q.Epic"), TEXT("Epicka"));
		T.Add(TEXT("Q.Ultra"), TEXT("Ultra"));

		T.Add(TEXT("Pause.Resume"), TEXT("Wznów"));
		T.Add(TEXT("Pause.Restart"), TEXT("Od nowa"));
		T.Add(TEXT("Pause.Quit"), TEXT("Wyjdź z gry"));

		T.Add(TEXT("Interact.TakeOff"), TEXT("Schowaj"));

		T.Add(TEXT("Menu.StressTest"), TEXT("TEST OBCIĄZENIOWY"));
		T.Add(TEXT("HUD.Fps"), TEXT("FPS"));
		T.Add(TEXT("HUD.FpsAvg"), TEXT("Sr"));
		T.Add(TEXT("HUD.FpsOneLow"), TEXT("1% Niski"));
		T.Add(TEXT("Loading.StressTest"), TEXT("Test obciazeniowy..."));
	}
}
