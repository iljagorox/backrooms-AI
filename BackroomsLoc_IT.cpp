#include "BackroomsLocRegistry.h"

// Italiano.
namespace BackroomsLoc
{
	void Register_IT(FLanguageTable& T)
	{
		T.Add(TEXT("Menu.NewGame"), TEXT("Nuova partita"));
		T.Add(TEXT("Menu.Continue"), TEXT("Continua"));
		T.Add(TEXT("Menu.Levels"), TEXT("Livelli"));
		T.Add(TEXT("Menu.Achievements"), TEXT("Obiettivi"));
		T.Add(TEXT("Menu.Settings"), TEXT("Impostazioni"));
		T.Add(TEXT("Menu.Controls"), TEXT("Controlli"));
		T.Add(TEXT("Menu.Quit"), TEXT("Esci"));
		T.Add(TEXT("Menu.Subtitle"), TEXT("DISCESA LIMINALE"));
		T.Add(TEXT("Menu.Back"), TEXT("<   I N D I E T R O"));

		T.Add(TEXT("Tab.Screen"), TEXT("SCHERMO"));
		T.Add(TEXT("Tab.Quality"), TEXT("QUALITÀ"));
		T.Add(TEXT("Tab.Camera"), TEXT("TELECAMERA"));
		T.Add(TEXT("Tab.Sound"), TEXT("AUDIO"));
		T.Add(TEXT("Tab.Game"), TEXT("GIOCO"));
		T.Add(TEXT("Tab.Controls"), TEXT("CONTROLLI"));
		T.Add(TEXT("Settings.Title"), TEXT("I M P O S T A Z I O N I"));
		T.Add(TEXT("Settings.Categories"), TEXT("C A T E G O R I E"));

		T.Add(TEXT("Set.Resolution"), TEXT("Risoluzione"));
		T.Add(TEXT("Set.DisplayMode"), TEXT("Modalità schermo"));
		T.Add(TEXT("Set.VSync"), TEXT("VSync"));
		T.Add(TEXT("Set.RenderScale"), TEXT("Scala di rendering"));
		T.Add(TEXT("Set.Gamma"), TEXT("Gamma"));
		T.Add(TEXT("Set.Language"), TEXT("Lingua"));

		T.Add(TEXT("Set.Shadows"), TEXT("Ombre"));
		T.Add(TEXT("Set.Textures"), TEXT("Texture"));
		T.Add(TEXT("Set.AntiAliasing"), TEXT("Antialiasing"));
		T.Add(TEXT("Set.PostProcess"), TEXT("Post-elaborazione"));
		T.Add(TEXT("Set.Effects"), TEXT("Effetti"));
		T.Add(TEXT("Set.ViewDistance"), TEXT("Distanza visiva"));
		T.Add(TEXT("Set.DepthOfField"), TEXT("Profondità di campo"));
		T.Add(TEXT("Set.Bloom"), TEXT("Bagliore (Bloom)"));
		T.Add(TEXT("Set.Lumen"), TEXT("Lumen (illuminazione)"));
		T.Add(TEXT("Set.Upscale"), TEXT("DLSS / Upscaling"));

		T.Add(TEXT("Set.FOV"), TEXT("Campo visivo"));
		T.Add(TEXT("Set.Sensitivity"), TEXT("Sensibilità"));
		T.Add(TEXT("Set.RecordingEffect"), TEXT("Effetto telecamera di registrazione"));

		T.Add(TEXT("Set.Volume.Master"), TEXT("Generale"));
		T.Add(TEXT("Set.Volume.Menu"), TEXT("Musica / menu"));
		T.Add(TEXT("Set.Volume.Game"), TEXT("Suoni di gioco"));
		T.Add(TEXT("Set.Volume.Monster"), TEXT("Suoni dei mostri"));
		T.Add(TEXT("Set.Volume.Other"), TEXT("Altri suoni"));

		T.Add(TEXT("Set.Difficulty"), TEXT("Difficoltà"));
		T.Add(TEXT("Set.Difficulty.Hint"),
			TEXT("La difficoltà influisce sulla generazione: densità dei muri, numero di passaggi, generosità di provviste e bottino."));

		T.Add(TEXT("Levels.Title"), TEXT("L I V E L L I"));
		T.Add(TEXT("Levels.Locked"), TEXT("BLOCCATO"));
		T.Add(TEXT("Levels.UnlockedFmt"),
			TEXT("Livelli sbloccati: %d di 10. I nuovi si aprono scendendo — vai più a fondo per vederne le anteprime."));

		T.Add(TEXT("Ach.Title"), TEXT("O B I E T T I V I"));
		T.Add(TEXT("Ach.Unlocked"), TEXT("OBIETTIVO"));
		T.Add(TEXT("Ach.ProgressFmt"), TEXT("Sbloccati: %d / %d     Punti: %d"));

		T.Add(TEXT("Diff.Peaceful.Name"), TEXT("Pacifico"));
		T.Add(TEXT("Diff.Peaceful.Desc"), TEXT("Niente mostri né pressione. Solo esplorazione e il mondo."));
		T.Add(TEXT("Diff.Easy.Name"), TEXT("Facile"));
		T.Add(TEXT("Diff.Easy.Desc"), TEXT("Luoghi aperti e generosi. Un mostro compare tardi."));
		T.Add(TEXT("Diff.Normal.Name"), TEXT("Normale"));
		T.Add(TEXT("Diff.Normal.Desc"), TEXT("L'equilibrio previsto: corridoi stretti, fame, una sola entità."));
		T.Add(TEXT("Diff.Hard.Name"), TEXT("Difficile"));
		T.Add(TEXT("Diff.Hard.Desc"), TEXT("Più muri e meno porte, bottino scarso, due entità."));
		T.Add(TEXT("Diff.Nightmare.Name"), TEXT("Incubo"));
		T.Add(TEXT("Diff.Nightmare.Desc"), TEXT("Un labirinto trappola, provviste minime, tre entità, la morte è definitiva."));

		T.Add(TEXT("HUD.Sanity"), TEXT("SANITÀ MENTALE"));
		T.Add(TEXT("HUD.Flashlight"), TEXT("TORCIA"));
		T.Add(TEXT("HUD.Hunger"), TEXT("FAME"));
		T.Add(TEXT("HUD.Thirst"), TEXT("SETE"));
		T.Add(TEXT("HUD.Health"), TEXT("SALUTE"));
		T.Add(TEXT("HUD.Stamina"), TEXT("RESISTENZA"));
		T.Add(TEXT("HUD.Inventory"), TEXT("INVENTARIO"));
		T.Add(TEXT("HUD.Empty"), TEXT("vuoto"));
		T.Add(TEXT("Prog.Level"), TEXT("Livello"));
		T.Add(TEXT("Prog.LevelUp"), TEXT("Nuovo livello"));

		T.Add(TEXT("Interact.Inspect"), TEXT("Esamina"));
		T.Add(TEXT("Interact.Throw"), TEXT("Lancia"));
		T.Add(TEXT("Interact.Drop"), TEXT("Posiziona"));
		T.Add(TEXT("Interact.Take"), TEXT("Prendi"));
		T.Add(TEXT("Interact.Push"), TEXT("Spingi"));
		T.Add(TEXT("Interact.Use"), TEXT("Usa"));
		T.Add(TEXT("Interact.Pickup"), TEXT("Raccogli"));

		T.Add(TEXT("Hint.Critical"), TEXT("Stato critico — trova subito una medicina"));
		T.Add(TEXT("Hint.Sanity"), TEXT("Sanità mentale bassa — nasconditi e respira"));
		T.Add(TEXT("Hint.Hunger"), TEXT("Grande fame — trova del cibo"));
		T.Add(TEXT("Hint.Thirst"), TEXT("Grande sete — bevi dell'acqua"));
		T.Add(TEXT("Hint.Monster"), TEXT("È vicino — non fare rumore"));
		T.Add(TEXT("Hint.Poison"), TEXT("Sei avvelenato — cerca acqua di mandorla o una cassetta di pronto soccorso"));
		T.Add(TEXT("Hint.Radiation"), TEXT("Radiazione alta — vattene da qui"));
		T.Add(TEXT("Hint.Sleep"), TEXT("Sei esausto — fermati e riposa"));
		T.Add(TEXT("Hint.Battery"), TEXT("La torcia è quasi scarica"));

		T.Add(TEXT("Ach.FirstSteps.Name"), TEXT("Primi passi"));
		T.Add(TEXT("Ach.FirstSteps.Desc"), TEXT("Scendere nei Backrooms."));
		T.Add(TEXT("Ach.Tourist.Name"), TEXT("Turista"));
		T.Add(TEXT("Ach.Tourist.Desc"), TEXT("Visitare 3 luoghi diversi."));
		T.Add(TEXT("Ach.Explorer.Name"), TEXT("Esploratore"));
		T.Add(TEXT("Ach.Explorer.Desc"), TEXT("Visitare 5 luoghi diversi."));
		T.Add(TEXT("Ach.DeepDiver.Name"), TEXT("In profondità"));
		T.Add(TEXT("Ach.DeepDiver.Desc"), TEXT("Visitare 7 luoghi diversi."));
		T.Add(TEXT("Ach.AllLevels.Name"), TEXT("Piano completo"));
		T.Add(TEXT("Ach.AllLevels.Desc"), TEXT("Visitare tutti i 10 luoghi."));
		T.Add(TEXT("Ach.Survivor.Name"), TEXT("Sopravvissuto"));
		T.Add(TEXT("Ach.Survivor.Desc"), TEXT("Resistere 15 minuti nei Backrooms."));
		T.Add(TEXT("Ach.Marathon.Name"), TEXT("Maratoneta"));
		T.Add(TEXT("Ach.Marathon.Desc"), TEXT("Percorrere 3 chilometri sottoterra."));
		T.Add(TEXT("Ach.Scavenger.Name"), TEXT("Raccoglitore"));
		T.Add(TEXT("Ach.Scavenger.Desc"), TEXT("Raccogliere 10 oggetti."));
		T.Add(TEXT("Ach.Pharmacist.Name"), TEXT("Farmacista"));
		T.Add(TEXT("Ach.Pharmacist.Desc"), TEXT("Usare 5 medicine."));
		T.Add(TEXT("Ach.Hydrated.Name"), TEXT("Sapore di mandorla"));
		T.Add(TEXT("Ach.Hydrated.Desc"), TEXT("Bere 5 porzioni di acqua di mandorla."));
		T.Add(TEXT("Ach.MonsterAware.Name"), TEXT("È vicino"));
		T.Add(TEXT("Ach.MonsterAware.Desc"), TEXT("Incontrare un'entità e sopravvivere."));
		T.Add(TEXT("Ach.EscapeArtist.Name"), TEXT("Prima uscita"));
		T.Add(TEXT("Ach.EscapeArtist.Desc"), TEXT("Trovare un'uscita."));
		T.Add(TEXT("Ach.FrequentFlyer.Name"), TEXT("Ospite frequente"));
		T.Add(TEXT("Ach.FrequentFlyer.Desc"), TEXT("Attraversare 5 uscite."));
		T.Add(TEXT("Ach.DeepFear.Name"), TEXT("Limite della paura"));
		T.Add(TEXT("Ach.DeepFear.Desc"), TEXT("Portare la pressione ambientale al massimo."));
		T.Add(TEXT("Ach.Dead.Name"), TEXT("Uno di loro"));
		T.Add(TEXT("Ach.Dead.Desc"), TEXT("Morire nei Backrooms."));

		T.Add(TEXT("Bind.MoveForwardPlus"), TEXT("Avanti"));
		T.Add(TEXT("Bind.MoveForwardMinus"), TEXT("Indietro"));
		T.Add(TEXT("Bind.MoveRightPlus"), TEXT("Destra"));
		T.Add(TEXT("Bind.MoveRightMinus"), TEXT("Sinistra"));
		T.Add(TEXT("Bind.Jump"), TEXT("Salta"));
		T.Add(TEXT("Bind.Sprint"), TEXT("Corri"));
		T.Add(TEXT("Bind.Flashlight"), TEXT("Torcia"));
		T.Add(TEXT("Bind.View"), TEXT("1ª / 3ª persona"));
		T.Add(TEXT("Bind.Attack"), TEXT("Pugno"));
		T.Add(TEXT("Bind.Grab"), TEXT("Raccogli / posa oggetto"));
		T.Add(TEXT("Bind.Push"), TEXT("Spingi oggetto"));
		T.Add(TEXT("Bind.Throw"), TEXT("Lancia oggetto"));
		T.Add(TEXT("Bind.Inspect"), TEXT("Ispeziona oggetto"));
		T.Add(TEXT("Bind.Use"), TEXT("Usa oggetto"));
		T.Add(TEXT("Bind.Slot1"), TEXT("Slot 1"));
		T.Add(TEXT("Bind.Slot2"), TEXT("Slot 2"));
		T.Add(TEXT("Bind.Slot3"), TEXT("Slot 3"));
		T.Add(TEXT("Bind.Slot4"), TEXT("Slot 4"));
		T.Add(TEXT("Bind.Inventory"), TEXT("Inventario"));
		T.Add(TEXT("Bind.PauseMenu"), TEXT("Pausa / menu"));
		T.Add(TEXT("Bind.Mouse"), TEXT("Mouse"));
		T.Add(TEXT("Bind.Look"), TEXT("Guardare"));
		T.Add(TEXT("Bind.Change"), TEXT("cambia"));
		T.Add(TEXT("Bind.PressKey"), TEXT("Premi un tasto per assegnare... (Esc per annullare)"));
		T.Add(TEXT("Bind.Cancel"), TEXT("ANNULLA  ×"));
		T.Add(TEXT("Bind.Reset"), TEXT("Ripristina tutti i tasti predefiniti"));

		T.Add(TEXT("Ach.WellFed.Name"), TEXT("Ben nutrito"));
		T.Add(TEXT("Ach.WellFed.Desc"), TEXT("Mangiare 10 porzioni di cibo."));
		T.Add(TEXT("Ach.Electrician.Name"), TEXT("Elettricista"));
		T.Add(TEXT("Ach.Electrician.Desc"), TEXT("Inserire 10 batterie nella torcia."));
		T.Add(TEXT("Ach.Sprinter.Name"), TEXT("Sprinter"));
		T.Add(TEXT("Ach.Sprinter.Desc"), TEXT("Iniziare a correre 50 volte."));
		T.Add(TEXT("Ach.Cartographer.Name"), TEXT("Cartografo"));
		T.Add(TEXT("Ach.Cartographer.Desc"), TEXT("Attraversare 200 stanze."));
		T.Add(TEXT("Ach.Speedrun.Name"), TEXT("Speedrunner"));
		T.Add(TEXT("Ach.Speedrun.Desc"), TEXT("Percorrere 5 chilometri sottoterra."));
		T.Add(TEXT("Ach.Gourmand.Name"), TEXT("Gourmet"));
		T.Add(TEXT("Ach.Gourmand.Desc"), TEXT("Resistere 30 minuti nei Backrooms."));

		T.Add(TEXT("GameOver.Title"), TEXT("SEI MORTO"));
		T.Add(TEXT("GameOver.Subtitle"), TEXT("I Backrooms non mollano mai. Riprova."));
		T.Add(TEXT("GameOver.Restart"), TEXT("Ricomincia"));
		T.Add(TEXT("GameOver.Quit"), TEXT("Esci"));

		T.Add(TEXT("Level.W0"), TEXT("L0 · Atrio"));
		T.Add(TEXT("Level.W1"), TEXT("L1 · Zona abitata"));
		T.Add(TEXT("Level.W2"), TEXT("L2 · Acquedotto"));
		T.Add(TEXT("Level.W3"), TEXT("L3 · Centrale elettrica"));
		T.Add(TEXT("Level.W4"), TEXT("L4 · Uffici"));
		T.Add(TEXT("Level.W5"), TEXT("L5 · Hotel"));
		T.Add(TEXT("Level.W6"), TEXT("L6 · Oscurità"));
		T.Add(TEXT("Level.W7"), TEXT("L7 · Oceano"));
		T.Add(TEXT("Level.W8"), TEXT("L8 · Grotte"));
		T.Add(TEXT("Level.W9"), TEXT("L9 · Ospedale"));

		T.Add(TEXT("Item.AlmondWater.Name"), TEXT("Acqua di mandorla"));
		T.Add(TEXT("Item.AlmondWater.Desc"), TEXT("Placa la sete e ripristina un po' di sanità mentale."));
		T.Add(TEXT("Item.CanFood.Name"), TEXT("Scatoletta"));
		T.Add(TEXT("Item.CanFood.Desc"), TEXT("Carne in scatola. Placa la fame."));
		T.Add(TEXT("Item.MedKit.Name"), TEXT("Cassetta di pronto soccorso"));
		T.Add(TEXT("Item.MedKit.Desc"), TEXT("Ferma l'emorragia. Ripristina la salute."));
		T.Add(TEXT("Item.Pill.Name"), TEXT("Sedativo"));
		T.Add(TEXT("Item.Pill.Desc"), TEXT("Calma i nervi. Ripristina la sanità mentale."));
		T.Add(TEXT("Item.Energy.Name"), TEXT("Bevanda energetica"));
		T.Add(TEXT("Item.Energy.Desc"), TEXT("Stimola e schiarisce un po' la mente."));
		T.Add(TEXT("Item.Battery.Name"), TEXT("Batteria"));
		T.Add(TEXT("Item.Battery.Desc"), TEXT("Una pila per la torcia."));

		T.Add(TEXT("Event.LightFlicker"), TEXT("Luce tremolante"));
		T.Add(TEXT("Event.LightOutage"), TEXT("La luce si è spenta!"));
		T.Add(TEXT("Event.BoxDisappear"), TEXT("Una cassa è sparita..."));
		T.Add(TEXT("Event.BoxAppear"), TEXT("È apparsa una cassa!"));
		T.Add(TEXT("Event.EntityGrowl"), TEXT("Un ringhio dall'oscurità..."));
		T.Add(TEXT("Event.EntityFootsteps"), TEXT("Passi dietro il muro..."));
		T.Add(TEXT("Event.Whisper"), TEXT("Sussurri..."));
		T.Add(TEXT("Event.DistantBang"), TEXT("Un boato in lontananza!"));
		T.Add(TEXT("Event.WallDrawing"), TEXT("Un disegno sul muro..."));
		T.Add(TEXT("Event.PipeCreak"), TEXT("Un tubo scricchiola"));
		T.Add(TEXT("Event.DoorSlam"), TEXT("Una porta sbatte!"));
		T.Add(TEXT("Event.EmergencyLight"), TEXT("Luce d'emergenza"));
		T.Add(TEXT("Event.FogIncrease"), TEXT("La nebbia si addensa..."));
		T.Add(TEXT("Event.StaticNoise"), TEXT("Disturbi..."));
		T.Add(TEXT("Event.FootprintAppear"), TEXT("Impronte sul pavimento..."));
		T.Add(TEXT("Event.Unknown"), TEXT("Evento sconosciuto"));

		T.Add(TEXT("Loading.Level"), TEXT("Caricamento livello..."));
		T.Add(TEXT("Loading.World"), TEXT("Caricamento dei Backrooms... %d%% (chunk: %d)"));
		T.Add(TEXT("Loading.Ready"), TEXT("Mondo pronto! Anomalia della realtà..."));
		T.Add(TEXT("Loading.Loaded"), TEXT("Livello caricato"));
		T.Add(TEXT("Loading.Noclip"), TEXT("Noclip: caduta nei Backrooms..."));
		T.Add(TEXT("Loading.Backrooms"), TEXT("Caricamento dei Backrooms..."));
		T.Add(TEXT("Loading.Chunks"), TEXT("chunk"));
		T.Add(TEXT("Loading.WorldReady"), TEXT("Mondo pronto! Glitch di realtà..."));
		T.Add(TEXT("Loading.LevelDone"), TEXT("Livello caricato"));

		T.Add(TEXT("Keys.SpaceBar"), TEXT("Spazio"));
		T.Add(TEXT("Keys.LeftShift"), TEXT("Shift"));
		T.Add(TEXT("Keys.RightShift"), TEXT("Shift (destro)"));
		T.Add(TEXT("Keys.LeftCtrl"), TEXT("Ctrl"));
		T.Add(TEXT("Keys.RightCtrl"), TEXT("Ctrl (destro)"));
		T.Add(TEXT("Keys.LeftAlt"), TEXT("Alt"));
		T.Add(TEXT("Keys.RightAlt"), TEXT("AltGr"));
		T.Add(TEXT("Keys.Escape"), TEXT("Esc"));
		T.Add(TEXT("Keys.LeftMouse"), TEXT("Tasto sx"));
		T.Add(TEXT("Keys.RightMouse"), TEXT("Tasto dx"));
		T.Add(TEXT("Keys.MiddleMouse"), TEXT("Rotella (clic)"));
		T.Add(TEXT("Keys.ThumbMouse1"), TEXT("Tasto laterale 1"));
		T.Add(TEXT("Keys.ThumbMouse2"), TEXT("Tasto laterale 2"));
		T.Add(TEXT("Keys.MouseX"), TEXT("Mouse X"));
		T.Add(TEXT("Keys.MouseY"), TEXT("Mouse Y"));
		T.Add(TEXT("Keys.MouseWheel"), TEXT("Rotella del mouse"));
		T.Add(TEXT("Keys.Backspace"), TEXT("Backspace"));
		T.Add(TEXT("Keys.Tab"), TEXT("Tab"));
		T.Add(TEXT("Keys.Enter"), TEXT("Invio"));
		T.Add(TEXT("Keys.On"), TEXT("SÌ"));
		T.Add(TEXT("Keys.Off"), TEXT("NO"));

		T.Add(TEXT("Set.DisplayMode.Fullscreen"), TEXT("Schermo intero"));
		T.Add(TEXT("Set.DisplayMode.Borderless"), TEXT("Senza bordi"));
		T.Add(TEXT("Set.DisplayMode.Windowed"), TEXT("Finestra"));
		T.Add(TEXT("Set.RecordingEffect.Off"), TEXT("Disattivato"));
		T.Add(TEXT("Set.RecordingEffect.Light"), TEXT("Effetto pellicola leggero"));
		T.Add(TEXT("Set.RecordingEffect.Medium"), TEXT("Effetto pellicola medio"));
		T.Add(TEXT("Set.RecordingEffect.Strong"), TEXT("Effetto pellicola forte"));
		T.Add(TEXT("Set.Quality.Overall"), TEXT("Qualità generale"));
		T.Add(TEXT("Set.PotatoMode"), TEXT("Modalità patata"));
		T.Add(TEXT("Set.PotatoMode.Hint"), TEXT("FPS massimi a scapito della grafica."));
		T.Add(TEXT("Set.Graphics"), TEXT("Grafica"));
		T.Add(TEXT("Q.Low"), TEXT("Bassa"));
		T.Add(TEXT("Q.Medium"), TEXT("Media"));
		T.Add(TEXT("Q.High"), TEXT("Alta"));
		T.Add(TEXT("Q.Epic"), TEXT("Epica"));
		T.Add(TEXT("Q.Ultra"), TEXT("Ultra"));

		T.Add(TEXT("Pause.Resume"), TEXT("Riprendi"));
		T.Add(TEXT("Pause.Restart"), TEXT("Ricomincia"));
		T.Add(TEXT("Pause.Quit"), TEXT("Esci dal gioco"));

		T.Add(TEXT("Interact.TakeOff"), TEXT("Riponi"));

		T.Add(TEXT("Menu.StressTest"), TEXT("STRESS TEST"));
		T.Add(TEXT("HUD.Fps"), TEXT("FPS"));
		T.Add(TEXT("HUD.FpsAvg"), TEXT("Media"));
		T.Add(TEXT("HUD.FpsOneLow"), TEXT("1% Low"));
		T.Add(TEXT("Loading.StressTest"), TEXT("Stress test..."));
	}
}
