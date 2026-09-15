#include "BackroomsLocRegistry.h"

// Deutsch.
namespace BackroomsLoc
{
	void Register_DE(FLanguageTable& T)
	{
		T.Add(TEXT("Menu.NewGame"), TEXT("Neues Spiel"));
		T.Add(TEXT("Menu.Continue"), TEXT("Fortsetzen"));
		T.Add(TEXT("Menu.Levels"), TEXT("Level"));
		T.Add(TEXT("Menu.Achievements"), TEXT("Erfolge"));
		T.Add(TEXT("Menu.Settings"), TEXT("Einstellungen"));
		T.Add(TEXT("Menu.Controls"), TEXT("Steuerung"));
		T.Add(TEXT("Menu.Quit"), TEXT("Beenden"));
		T.Add(TEXT("Menu.Subtitle"), TEXT("LIMINALER ABSTIEG"));
		T.Add(TEXT("Menu.Back"), TEXT("<   Z U R Ü C K"));

		T.Add(TEXT("Tab.Screen"), TEXT("BILDSCHIRM"));
		T.Add(TEXT("Tab.Quality"), TEXT("QUALITÄT"));
		T.Add(TEXT("Tab.Camera"), TEXT("KAMERA"));
		T.Add(TEXT("Tab.Sound"), TEXT("TON"));
		T.Add(TEXT("Tab.Game"), TEXT("SPIEL"));
		T.Add(TEXT("Tab.Controls"), TEXT("STEUERUNG"));
		T.Add(TEXT("Settings.Title"), TEXT("E I N S T E L L U N G E N"));
		T.Add(TEXT("Settings.Categories"), TEXT("K A T E G O R I E N"));

		T.Add(TEXT("Set.Resolution"), TEXT("Auflösung"));
		T.Add(TEXT("Set.DisplayMode"), TEXT("Anzeigemodus"));
		T.Add(TEXT("Set.VSync"), TEXT("VSync"));
		T.Add(TEXT("Set.RenderScale"), TEXT("Render-Skalierung"));
		T.Add(TEXT("Set.Gamma"), TEXT("Gamma"));
		T.Add(TEXT("Set.Language"), TEXT("Sprache"));

		T.Add(TEXT("Set.Shadows"), TEXT("Schatten"));
		T.Add(TEXT("Set.Textures"), TEXT("Texturen"));
		T.Add(TEXT("Set.AntiAliasing"), TEXT("Kantenglättung"));
		T.Add(TEXT("Set.PostProcess"), TEXT("Nachbearbeitung"));
		T.Add(TEXT("Set.Effects"), TEXT("Effekte"));
		T.Add(TEXT("Set.ViewDistance"), TEXT("Sichtweite"));
		T.Add(TEXT("Set.DepthOfField"), TEXT("Schärfentiefe"));
		T.Add(TEXT("Set.Bloom"), TEXT("Bloom"));
		T.Add(TEXT("Set.Lumen"), TEXT("Lumen (Beleuchtung)"));
		T.Add(TEXT("Set.Upscale"), TEXT("DLSS / Hochskalierung"));

		T.Add(TEXT("Set.FOV"), TEXT("Sichtfeld"));
		T.Add(TEXT("Set.Sensitivity"), TEXT("Empfindlichkeit"));
		T.Add(TEXT("Set.RecordingEffect"), TEXT("Aufnahme-Kamera-Effekt"));

		T.Add(TEXT("Set.Volume.Master"), TEXT("Gesamt"));
		T.Add(TEXT("Set.Volume.Menu"), TEXT("Musik / Menü"));
		T.Add(TEXT("Set.Volume.Game"), TEXT("Ingame-Sounds"));
		T.Add(TEXT("Set.Volume.Monster"), TEXT("Monster-Sounds"));
		T.Add(TEXT("Set.Volume.Other"), TEXT("Sonstige Sounds"));

		T.Add(TEXT("Set.Difficulty"), TEXT("Schwierigkeit"));
		T.Add(TEXT("Set.Difficulty.Hint"),
			TEXT("Die Schwierigkeit beeinflusst die Generierung: Wanddichte, Anzahl der Durchgänge, Großzügigkeit von Vorräten und Beute."));

		T.Add(TEXT("Levels.Title"), TEXT("L E V E L"));
		T.Add(TEXT("Levels.Locked"), TEXT("GESPERRT"));
		T.Add(TEXT("Levels.UnlockedFmt"),
			TEXT("Freigeschaltete Level: %d von 10. Neue öffnen sich beim Hinabsteigen — geh tiefer, um ihre Vorschauen zu sehen."));

		T.Add(TEXT("Ach.Title"), TEXT("E R F O L G E"));
		T.Add(TEXT("Ach.Unlocked"), TEXT("ERFOLG"));
		T.Add(TEXT("Ach.ProgressFmt"), TEXT("Freigeschaltet: %d / %d     Punkte: %d"));

		T.Add(TEXT("Diff.Peaceful.Name"), TEXT("Friedlich"));
		T.Add(TEXT("Diff.Peaceful.Desc"), TEXT("Keine Monster, kein Druck. Nur Erkundung und die Welt."));
		T.Add(TEXT("Diff.Easy.Name"), TEXT("Leicht"));
		T.Add(TEXT("Diff.Easy.Desc"), TEXT("Offene, großzügige Orte. Ein Monster erscheint spät."));
		T.Add(TEXT("Diff.Normal.Name"), TEXT("Normal"));
		T.Add(TEXT("Diff.Normal.Desc"), TEXT("Die beabsichtigte Balance: enge Gänge, Hunger, eine einzelne Wesenheit."));
		T.Add(TEXT("Diff.Hard.Name"), TEXT("Schwer"));
		T.Add(TEXT("Diff.Hard.Desc"), TEXT("Mehr Wände und weniger Türen, spärliche Beute, zwei Wesenheiten."));
		T.Add(TEXT("Diff.Nightmare.Name"), TEXT("Albtraum"));
		T.Add(TEXT("Diff.Nightmare.Desc"), TEXT("Ein Fallenlabyrinth, minimale Vorräte, drei Wesenheiten, der Tod ist endgültig."));

		T.Add(TEXT("HUD.Sanity"), TEXT("VERSTAND"));
		T.Add(TEXT("HUD.Flashlight"), TEXT("TASCHENLAMPE"));
		T.Add(TEXT("HUD.Hunger"), TEXT("HUNGER"));
		T.Add(TEXT("HUD.Thirst"), TEXT("DURST"));
		T.Add(TEXT("HUD.Health"), TEXT("GESUNDHEIT"));
		T.Add(TEXT("HUD.Stamina"), TEXT("AUSDAUER"));
		T.Add(TEXT("HUD.Inventory"), TEXT("INVENTAR"));
		T.Add(TEXT("HUD.Empty"), TEXT("leer"));
		T.Add(TEXT("Prog.Level"), TEXT("Stufe"));
		T.Add(TEXT("Prog.LevelUp"), TEXT("Aufstieg"));

		T.Add(TEXT("Interact.Inspect"), TEXT("Untersuchen"));
		T.Add(TEXT("Interact.Throw"), TEXT("Werfen"));
		T.Add(TEXT("Interact.Drop"), TEXT("Ablegen"));
		T.Add(TEXT("Interact.Take"), TEXT("Nehmen"));
		T.Add(TEXT("Interact.Push"), TEXT("Stoßen"));
		T.Add(TEXT("Interact.Use"), TEXT("Benutzen"));
		T.Add(TEXT("Interact.Pickup"), TEXT("Aufheben"));

		T.Add(TEXT("Hint.Critical"), TEXT("Kritischer Zustand — finde sofort Medizin"));
		T.Add(TEXT("Hint.Sanity"), TEXT("Verstand schwindet — versteck dich und atme durch"));
		T.Add(TEXT("Hint.Hunger"), TEXT("Großer Hunger — finde Essen"));
		T.Add(TEXT("Hint.Thirst"), TEXT("Großer Durst — trinke Wasser"));
		T.Add(TEXT("Hint.Monster"), TEXT("Es ist nah — sei leise"));
		T.Add(TEXT("Hint.Poison"), TEXT("Du bist vergiftet — such Mandelwasser oder einen Verbandskasten"));
		T.Add(TEXT("Hint.Radiation"), TEXT("Hohe Strahlung — verschwinde hier"));
		T.Add(TEXT("Hint.Sleep"), TEXT("Du bist erschöpft — halt an und ruh dich aus"));
		T.Add(TEXT("Hint.Battery"), TEXT("Die Taschenlampe ist fast leer"));

		T.Add(TEXT("Ach.FirstSteps.Name"), TEXT("Erste Schritte"));
		T.Add(TEXT("Ach.FirstSteps.Desc"), TEXT("In die Backrooms hinabsteigen."));
		T.Add(TEXT("Ach.Tourist.Name"), TEXT("Tourist"));
		T.Add(TEXT("Ach.Tourist.Desc"), TEXT("3 verschiedene Orte besuchen."));
		T.Add(TEXT("Ach.Explorer.Name"), TEXT("Entdecker"));
		T.Add(TEXT("Ach.Explorer.Desc"), TEXT("5 verschiedene Orte besuchen."));
		T.Add(TEXT("Ach.DeepDiver.Name"), TEXT("In der Tiefe"));
		T.Add(TEXT("Ach.DeepDiver.Desc"), TEXT("7 verschiedene Orte besuchen."));
		T.Add(TEXT("Ach.AllLevels.Name"), TEXT("Ganze Etage"));
		T.Add(TEXT("Ach.AllLevels.Desc"), TEXT("Alle 10 Orte besuchen."));
		T.Add(TEXT("Ach.Survivor.Name"), TEXT("Überlebender"));
		T.Add(TEXT("Ach.Survivor.Desc"), TEXT("15 Minuten in den Backrooms durchhalten."));
		T.Add(TEXT("Ach.Marathon.Name"), TEXT("Marathonläufer"));
		T.Add(TEXT("Ach.Marathon.Desc"), TEXT("3 Kilometer unter der Erde zurücklegen."));
		T.Add(TEXT("Ach.Scavenger.Name"), TEXT("Sammler"));
		T.Add(TEXT("Ach.Scavenger.Desc"), TEXT("10 Gegenstände aufnehmen."));
		T.Add(TEXT("Ach.Pharmacist.Name"), TEXT("Apotheker"));
		T.Add(TEXT("Ach.Pharmacist.Desc"), TEXT("5 Medikamente verwenden."));
		T.Add(TEXT("Ach.Hydrated.Name"), TEXT("Mandelgeschmack"));
		T.Add(TEXT("Ach.Hydrated.Desc"), TEXT("5 Portionen Mandelwasser trinken."));
		T.Add(TEXT("Ach.MonsterAware.Name"), TEXT("Es ist nah"));
		T.Add(TEXT("Ach.MonsterAware.Desc"), TEXT("Einer Wesenheit begegnen und überleben."));
		T.Add(TEXT("Ach.EscapeArtist.Name"), TEXT("Erster Ausgang"));
		T.Add(TEXT("Ach.EscapeArtist.Desc"), TEXT("Einen Ausgang finden."));
		T.Add(TEXT("Ach.FrequentFlyer.Name"), TEXT("Häufiger Gast"));
		T.Add(TEXT("Ach.FrequentFlyer.Desc"), TEXT("5 Ausgänge passieren."));
		T.Add(TEXT("Ach.DeepFear.Name"), TEXT("Grenze der Angst"));
		T.Add(TEXT("Ach.DeepFear.Desc"), TEXT("Den Umgebungsdruck auf das Maximum treiben."));
		T.Add(TEXT("Ach.Dead.Name"), TEXT("Einer von ihnen"));
		T.Add(TEXT("Ach.Dead.Desc"), TEXT("In den Backrooms sterben."));

		T.Add(TEXT("Bind.MoveForwardPlus"), TEXT("Vorwärts"));
		T.Add(TEXT("Bind.MoveForwardMinus"), TEXT("Zurück"));
		T.Add(TEXT("Bind.MoveRightPlus"), TEXT("Rechts"));
		T.Add(TEXT("Bind.MoveRightMinus"), TEXT("Links"));
		T.Add(TEXT("Bind.Jump"), TEXT("Springen"));
		T.Add(TEXT("Bind.Sprint"), TEXT("Rennen"));
		T.Add(TEXT("Bind.Flashlight"), TEXT("Taschenlampe"));
		T.Add(TEXT("Bind.View"), TEXT("1./3. Person"));
		T.Add(TEXT("Bind.Attack"), TEXT("Faustschlag"));
		T.Add(TEXT("Bind.Grab"), TEXT("Gegenstand nehmen / ablegen"));
		T.Add(TEXT("Bind.Push"), TEXT("Gegenstand schieben"));
		T.Add(TEXT("Bind.Throw"), TEXT("Gegenstand werfen"));
		T.Add(TEXT("Bind.Inspect"), TEXT("Gegenstand untersuchen"));
		T.Add(TEXT("Bind.Use"), TEXT("Gegenstand benutzen"));
		T.Add(TEXT("Bind.Slot1"), TEXT("Slot 1"));
		T.Add(TEXT("Bind.Slot2"), TEXT("Slot 2"));
		T.Add(TEXT("Bind.Slot3"), TEXT("Slot 3"));
		T.Add(TEXT("Bind.Slot4"), TEXT("Slot 4"));
		T.Add(TEXT("Bind.Inventory"), TEXT("Inventar"));
		T.Add(TEXT("Bind.PauseMenu"), TEXT("Pause / Menü"));
		T.Add(TEXT("Bind.Mouse"), TEXT("Maus"));
		T.Add(TEXT("Bind.Look"), TEXT("Umschauen"));
		T.Add(TEXT("Bind.Change"), TEXT("ändern"));
		T.Add(TEXT("Bind.PressKey"), TEXT("Taste zum Zuweisen drücken... (Esc zum Abbrechen)"));
		T.Add(TEXT("Bind.Cancel"), TEXT("ABBRECHEN  ×"));
		T.Add(TEXT("Bind.Reset"), TEXT("Alle Tasten auf Standardwerte zurücksetzen"));

		T.Add(TEXT("Ach.WellFed.Name"), TEXT("Gut genährt"));
		T.Add(TEXT("Ach.WellFed.Desc"), TEXT("10 Portionen Essen verzehren."));
		T.Add(TEXT("Ach.Electrician.Name"), TEXT("Elektriker"));
		T.Add(TEXT("Ach.Electrician.Desc"), TEXT("10 Batterien in die Taschenlampe einsetzen."));
		T.Add(TEXT("Ach.Sprinter.Name"), TEXT("Sprinter"));
		T.Add(TEXT("Ach.Sprinter.Desc"), TEXT("50-mal zu rennen beginnen."));
		T.Add(TEXT("Ach.Cartographer.Name"), TEXT("Kartograf"));
		T.Add(TEXT("Ach.Cartographer.Desc"), TEXT("200 Räume durchqueren."));
		T.Add(TEXT("Ach.Speedrun.Name"), TEXT("Speedrunner"));
		T.Add(TEXT("Ach.Speedrun.Desc"), TEXT("5 Kilometer unter der Erde zurücklegen."));
		T.Add(TEXT("Ach.Gourmand.Name"), TEXT("Feinschmecker"));
		T.Add(TEXT("Ach.Gourmand.Desc"), TEXT("30 Minuten in den Backrooms überleben."));

		T.Add(TEXT("GameOver.Title"), TEXT("DU BIST GESTORBEN"));
		T.Add(TEXT("GameOver.Subtitle"), TEXT("Die Backrooms lassen nicht los. Versuch es erneut."));
		T.Add(TEXT("GameOver.Restart"), TEXT("Neustart"));
		T.Add(TEXT("GameOver.Quit"), TEXT("Beenden"));

		T.Add(TEXT("Level.W0"), TEXT("L0 · Lobby"));
		T.Add(TEXT("Level.W1"), TEXT("L1 · Bewohnte Zone"));
		T.Add(TEXT("Level.W2"), TEXT("L2 · Wasserleitungen"));
		T.Add(TEXT("Level.W3"), TEXT("L3 · Kraftwerk"));
		T.Add(TEXT("Level.W4"), TEXT("L4 · Büros"));
		T.Add(TEXT("Level.W5"), TEXT("L5 · Hotel"));
		T.Add(TEXT("Level.W6"), TEXT("L6 · Dunkelheit"));
		T.Add(TEXT("Level.W7"), TEXT("L7 · Ozean"));
		T.Add(TEXT("Level.W8"), TEXT("L8 · Höhlen"));
		T.Add(TEXT("Level.W9"), TEXT("L9 · Krankenhaus"));

		T.Add(TEXT("Item.AlmondWater.Name"), TEXT("Mandelwasser"));
		T.Add(TEXT("Item.AlmondWater.Desc"), TEXT("Stillt den Durst und stellt etwas Verstand wieder her."));
		T.Add(TEXT("Item.CanFood.Name"), TEXT("Dosenfleisch"));
		T.Add(TEXT("Item.CanFood.Desc"), TEXT("Eingekochtes Fleisch. Stillt den Hunger."));
		T.Add(TEXT("Item.MedKit.Name"), TEXT("Verbandskasten"));
		T.Add(TEXT("Item.MedKit.Desc"), TEXT("Stoppt Blutungen. Stellt Gesundheit wieder her."));
		T.Add(TEXT("Item.Pill.Name"), TEXT("Beruhigungsmittel"));
		T.Add(TEXT("Item.Pill.Desc"), TEXT("Beruhigt die Nerven. Stellt Verstand wieder her."));
		T.Add(TEXT("Item.Energy.Name"), TEXT("Energy-Drink"));
		T.Add(TEXT("Item.Energy.Desc"), TEXT("Macht munter und klart den Kopf etwas."));
		T.Add(TEXT("Item.Battery.Name"), TEXT("Batterie"));
		T.Add(TEXT("Item.Battery.Desc"), TEXT("Ein Energiespeicher für die Taschenlampe."));

		T.Add(TEXT("Event.LightFlicker"), TEXT("Flackerndes Licht"));
		T.Add(TEXT("Event.LightOutage"), TEXT("Das Licht ist ausgefallen!"));
		T.Add(TEXT("Event.BoxDisappear"), TEXT("Eine Kiste verschwand..."));
		T.Add(TEXT("Event.BoxAppear"), TEXT("Eine Kiste erschien!"));
		T.Add(TEXT("Event.EntityGrowl"), TEXT("Ein Knurren aus dem Dunkel..."));
		T.Add(TEXT("Event.EntityFootsteps"), TEXT("Schritte hinter der Wand..."));
		T.Add(TEXT("Event.Whisper"), TEXT("Flüstern..."));
		T.Add(TEXT("Event.DistantBang"), TEXT("Ein Krach von weit her!"));
		T.Add(TEXT("Event.WallDrawing"), TEXT("Eine Zeichnung an der Wand..."));
		T.Add(TEXT("Event.PipeCreak"), TEXT("Ein Rohr knarrt"));
		T.Add(TEXT("Event.DoorSlam"), TEXT("Eine Tür knallt!"));
		T.Add(TEXT("Event.EmergencyLight"), TEXT("Notlicht"));
		T.Add(TEXT("Event.FogIncrease"), TEXT("Der Nebel wird dichter..."));
		T.Add(TEXT("Event.StaticNoise"), TEXT("Rauschen..."));
		T.Add(TEXT("Event.FootprintAppear"), TEXT("Fußspuren auf dem Boden..."));
		T.Add(TEXT("Event.Unknown"), TEXT("Unbekanntes Ereignis"));

		T.Add(TEXT("Loading.Level"), TEXT("Level wird geladen..."));
		T.Add(TEXT("Loading.World"), TEXT("Backrooms werden geladen... %d%% (Chunks: %d)"));
		T.Add(TEXT("Loading.Ready"), TEXT("Welt bereit! Realitätsfehler..."));
		T.Add(TEXT("Loading.Loaded"), TEXT("Level geladen"));
		T.Add(TEXT("Loading.Noclip"), TEXT("Noclip: Fall in die Backrooms..."));
		T.Add(TEXT("Loading.Backrooms"), TEXT("Lade Backrooms..."));
		T.Add(TEXT("Loading.Chunks"), TEXT("Chunks"));
		T.Add(TEXT("Loading.WorldReady"), TEXT("Welt bereit! Realität zerfällt..."));
		T.Add(TEXT("Loading.LevelDone"), TEXT("Level geladen"));

		T.Add(TEXT("Keys.SpaceBar"), TEXT("Leertaste"));
		T.Add(TEXT("Keys.LeftShift"), TEXT("Shift"));
		T.Add(TEXT("Keys.RightShift"), TEXT("Shift (rechts)"));
		T.Add(TEXT("Keys.LeftCtrl"), TEXT("Strg"));
		T.Add(TEXT("Keys.RightCtrl"), TEXT("Strg (rechts)"));
		T.Add(TEXT("Keys.LeftAlt"), TEXT("Alt"));
		T.Add(TEXT("Keys.RightAlt"), TEXT("AltGr"));
		T.Add(TEXT("Keys.Escape"), TEXT("Esc"));
		T.Add(TEXT("Keys.LeftMouse"), TEXT("LMT"));
		T.Add(TEXT("Keys.RightMouse"), TEXT("RMT"));
		T.Add(TEXT("Keys.MiddleMouse"), TEXT("Mausrad (Klick)"));
		T.Add(TEXT("Keys.ThumbMouse1"), TEXT("Seitentaste 1"));
		T.Add(TEXT("Keys.ThumbMouse2"), TEXT("Seitentaste 2"));
		T.Add(TEXT("Keys.MouseX"), TEXT("Maus X"));
		T.Add(TEXT("Keys.MouseY"), TEXT("Maus Y"));
		T.Add(TEXT("Keys.MouseWheel"), TEXT("Mausrad"));
		T.Add(TEXT("Keys.Backspace"), TEXT("Rücktaste"));
		T.Add(TEXT("Keys.Tab"), TEXT("Tab"));
		T.Add(TEXT("Keys.Enter"), TEXT("Enter"));
		T.Add(TEXT("Keys.On"), TEXT("AN"));
		T.Add(TEXT("Keys.Off"), TEXT("AUS"));

		T.Add(TEXT("Set.DisplayMode.Fullscreen"), TEXT("Vollbild"));
		T.Add(TEXT("Set.DisplayMode.Borderless"), TEXT("Randlos"));
		T.Add(TEXT("Set.DisplayMode.Windowed"), TEXT("Fenster"));
		T.Add(TEXT("Set.RecordingEffect.Off"), TEXT("Aus"));
		T.Add(TEXT("Set.RecordingEffect.Light"), TEXT("Leichtes Filmkorn"));
		T.Add(TEXT("Set.RecordingEffect.Medium"), TEXT("Mittleres Filmkorn"));
		T.Add(TEXT("Set.RecordingEffect.Strong"), TEXT("Starkes Filmkorn"));
		T.Add(TEXT("Set.Quality.Overall"), TEXT("Gesamtqualität"));
		T.Add(TEXT("Set.PotatoMode"), TEXT("Kartoffel-Modus"));
		T.Add(TEXT("Set.PotatoMode.Hint"), TEXT("Maximale Leistung auf Kosten der Grafik."));
		T.Add(TEXT("Set.Graphics"), TEXT("Grafik"));
		T.Add(TEXT("Q.Low"), TEXT("Niedrig"));
		T.Add(TEXT("Q.Medium"), TEXT("Mittel"));
		T.Add(TEXT("Q.High"), TEXT("Hoch"));
		T.Add(TEXT("Q.Epic"), TEXT("Episch"));
		T.Add(TEXT("Q.Ultra"), TEXT("Ultra"));

		T.Add(TEXT("Pause.Resume"), TEXT("Weiter"));
		T.Add(TEXT("Pause.Restart"), TEXT("Neustart"));
		T.Add(TEXT("Pause.Quit"), TEXT("Spiel beenden"));

		T.Add(TEXT("Interact.TakeOff"), TEXT("Wegstecken"));

		T.Add(TEXT("Menu.StressTest"), TEXT("STRESSTEST"));
		T.Add(TEXT("HUD.Fps"), TEXT("FPS"));
		T.Add(TEXT("HUD.FpsAvg"), TEXT("Avg"));
		T.Add(TEXT("HUD.FpsOneLow"), TEXT("1% Low"));
		T.Add(TEXT("Loading.StressTest"), TEXT("Stresstest..."));
	}
}
