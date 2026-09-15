#include "BackroomsLocRegistry.h"

// Español.
namespace BackroomsLoc
{
	void Register_ES(FLanguageTable& T)
	{
		T.Add(TEXT("Menu.NewGame"), TEXT("Nueva partida"));
		T.Add(TEXT("Menu.Continue"), TEXT("Continuar"));
		T.Add(TEXT("Menu.Levels"), TEXT("Niveles"));
		T.Add(TEXT("Menu.Achievements"), TEXT("Logros"));
		T.Add(TEXT("Menu.Settings"), TEXT("Ajustes"));
		T.Add(TEXT("Menu.Controls"), TEXT("Controles"));
		T.Add(TEXT("Menu.Quit"), TEXT("Salir"));
		T.Add(TEXT("Menu.Subtitle"), TEXT("DESCENSO LIMINAL"));
		T.Add(TEXT("Menu.Back"), TEXT("<   A T R Á S"));

		T.Add(TEXT("Tab.Screen"), TEXT("PANTALLA"));
		T.Add(TEXT("Tab.Quality"), TEXT("CALIDAD"));
		T.Add(TEXT("Tab.Camera"), TEXT("CÁMARA"));
		T.Add(TEXT("Tab.Sound"), TEXT("SONIDO"));
		T.Add(TEXT("Tab.Game"), TEXT("JUEGO"));
		T.Add(TEXT("Tab.Controls"), TEXT("CONTROLES"));
		T.Add(TEXT("Settings.Title"), TEXT("A J U S T E S"));
		T.Add(TEXT("Settings.Categories"), TEXT("C A T E G O R Í A S"));

		T.Add(TEXT("Set.Resolution"), TEXT("Resolución"));
		T.Add(TEXT("Set.DisplayMode"), TEXT("Modo de pantalla"));
		T.Add(TEXT("Set.VSync"), TEXT("VSync"));
		T.Add(TEXT("Set.RenderScale"), TEXT("Escala de render"));
		T.Add(TEXT("Set.Gamma"), TEXT("Gamma"));
		T.Add(TEXT("Set.Language"), TEXT("Idioma"));

		T.Add(TEXT("Set.Shadows"), TEXT("Sombras"));
		T.Add(TEXT("Set.Textures"), TEXT("Texturas"));
		T.Add(TEXT("Set.AntiAliasing"), TEXT("Suavizado"));
		T.Add(TEXT("Set.PostProcess"), TEXT("Posprocesado"));
		T.Add(TEXT("Set.Effects"), TEXT("Efectos"));
		T.Add(TEXT("Set.ViewDistance"), TEXT("Distancia de visión"));
		T.Add(TEXT("Set.DepthOfField"), TEXT("Profundidad de campo"));
		T.Add(TEXT("Set.Bloom"), TEXT("Resplandor (Bloom)"));
		T.Add(TEXT("Set.Lumen"), TEXT("Lumen (iluminación)"));
		T.Add(TEXT("Set.Upscale"), TEXT("DLSS / Escalado"));

		T.Add(TEXT("Set.FOV"), TEXT("Campo de visión"));
		T.Add(TEXT("Set.Sensitivity"), TEXT("Sensibilidad"));
		T.Add(TEXT("Set.RecordingEffect"), TEXT("Efecto de cámara de grabación"));

		T.Add(TEXT("Set.Volume.Master"), TEXT("General"));
		T.Add(TEXT("Set.Volume.Menu"), TEXT("Música / menú"));
		T.Add(TEXT("Set.Volume.Game"), TEXT("Sonidos del juego"));
		T.Add(TEXT("Set.Volume.Monster"), TEXT("Sonidos de monstruos"));
		T.Add(TEXT("Set.Volume.Other"), TEXT("Otros sonidos"));

		T.Add(TEXT("Set.Difficulty"), TEXT("Dificultad"));
		T.Add(TEXT("Set.Difficulty.Hint"),
			TEXT("La dificultad afecta a la generación: densidad de muros, número de puertas, generosidad de suministros y botín."));

		T.Add(TEXT("Levels.Title"), TEXT("N I V E L E S"));
		T.Add(TEXT("Levels.Locked"), TEXT("BLOQUEADO"));
		T.Add(TEXT("Levels.UnlockedFmt"),
			TEXT("Niveles desbloqueados: %d de 10. Los nuevos se abren al descender — baja más para ver sus imágenes."));

		T.Add(TEXT("Ach.Title"), TEXT("L O G R O S"));
		T.Add(TEXT("Ach.Unlocked"), TEXT("LOGRO"));
		T.Add(TEXT("Ach.ProgressFmt"), TEXT("Desbloqueados: %d / %d     Puntos: %d"));

		T.Add(TEXT("Diff.Peaceful.Name"), TEXT("Pacífico"));
		T.Add(TEXT("Diff.Peaceful.Desc"), TEXT("Sin monstruos ni presión. Solo exploración y el mundo."));
		T.Add(TEXT("Diff.Easy.Name"), TEXT("Fácil"));
		T.Add(TEXT("Diff.Easy.Desc"), TEXT("Lugares abiertos y generosos. Un monstruo aparece tarde."));
		T.Add(TEXT("Diff.Normal.Name"), TEXT("Normal"));
		T.Add(TEXT("Diff.Normal.Desc"), TEXT("El equilibrio previsto: pasillos estrechos, hambre, una entidad solitaria."));
		T.Add(TEXT("Diff.Hard.Name"), TEXT("Difícil"));
		T.Add(TEXT("Diff.Hard.Desc"), TEXT("Más muros y menos puertas, botín escaso, dos entidades."));
		T.Add(TEXT("Diff.Nightmare.Name"), TEXT("Pesadilla"));
		T.Add(TEXT("Diff.Nightmare.Desc"), TEXT("Un laberinto trampa, suministros mínimos, tres entidades, la muerte es definitiva."));

		T.Add(TEXT("HUD.Sanity"), TEXT("CORDURA"));
		T.Add(TEXT("HUD.Flashlight"), TEXT("LINTERNA"));
		T.Add(TEXT("HUD.Hunger"), TEXT("HAMBRE"));
		T.Add(TEXT("HUD.Thirst"), TEXT("SED"));
		T.Add(TEXT("HUD.Health"), TEXT("SALUD"));
		T.Add(TEXT("HUD.Stamina"), TEXT("RESISTENCIA"));
		T.Add(TEXT("HUD.Inventory"), TEXT("INVENTARIO"));
		T.Add(TEXT("HUD.Empty"), TEXT("vacío"));
		T.Add(TEXT("Prog.Level"), TEXT("Nivel"));
		T.Add(TEXT("Prog.LevelUp"), TEXT("Subida de nivel"));

		T.Add(TEXT("Interact.Inspect"), TEXT("Examinar"));
		T.Add(TEXT("Interact.Throw"), TEXT("Lanzar"));
		T.Add(TEXT("Interact.Drop"), TEXT("Soltar"));
		T.Add(TEXT("Interact.Take"), TEXT("Tomar"));
		T.Add(TEXT("Interact.Push"), TEXT("Empujar"));
		T.Add(TEXT("Interact.Use"), TEXT("Usar"));
		T.Add(TEXT("Interact.Pickup"), TEXT("Recoger"));

		T.Add(TEXT("Hint.Critical"), TEXT("Estado crítico — busca medicina ya"));
		T.Add(TEXT("Hint.Sanity"), TEXT("Cordura baja — escóndete y respira"));
		T.Add(TEXT("Hint.Hunger"), TEXT("Mucha hambre — busca comida"));
		T.Add(TEXT("Hint.Thirst"), TEXT("Mucha sed — bebe agua"));
		T.Add(TEXT("Hint.Monster"), TEXT("Está cerca — no hagas ruido"));
		T.Add(TEXT("Hint.Poison"), TEXT("Estás envenenado — busca agua de almendra o un botiquín"));
		T.Add(TEXT("Hint.Radiation"), TEXT("Radiación alta — sal de aquí"));
		T.Add(TEXT("Hint.Sleep"), TEXT("Estás agotado — detente y descansa"));
		T.Add(TEXT("Hint.Battery"), TEXT("La linterna casi se apaga"));

		T.Add(TEXT("Ach.FirstSteps.Name"), TEXT("Primeros pasos"));
		T.Add(TEXT("Ach.FirstSteps.Desc"), TEXT("Descender a los Backrooms."));
		T.Add(TEXT("Ach.Tourist.Name"), TEXT("Turista"));
		T.Add(TEXT("Ach.Tourist.Desc"), TEXT("Visitar 3 lugares distintos."));
		T.Add(TEXT("Ach.Explorer.Name"), TEXT("Explorador"));
		T.Add(TEXT("Ach.Explorer.Desc"), TEXT("Visitar 5 lugares distintos."));
		T.Add(TEXT("Ach.DeepDiver.Name"), TEXT("En las profundidades"));
		T.Add(TEXT("Ach.DeepDiver.Desc"), TEXT("Visitar 7 lugares distintos."));
		T.Add(TEXT("Ach.AllLevels.Name"), TEXT("Piso completo"));
		T.Add(TEXT("Ach.AllLevels.Desc"), TEXT("Visitar los 10 lugares."));
		T.Add(TEXT("Ach.Survivor.Name"), TEXT("Superviviente"));
		T.Add(TEXT("Ach.Survivor.Desc"), TEXT("Aguantar 15 minutos en los Backrooms."));
		T.Add(TEXT("Ach.Marathon.Name"), TEXT("Maratonista"));
		T.Add(TEXT("Ach.Marathon.Desc"), TEXT("Recorrer 3 kilómetros bajo tierra."));
		T.Add(TEXT("Ach.Scavenger.Name"), TEXT("Carroñero"));
		T.Add(TEXT("Ach.Scavenger.Desc"), TEXT("Recoger 10 objetos."));
		T.Add(TEXT("Ach.Pharmacist.Name"), TEXT("Farmacéutico"));
		T.Add(TEXT("Ach.Pharmacist.Desc"), TEXT("Usar 5 medicinas."));
		T.Add(TEXT("Ach.Hydrated.Name"), TEXT("Sabor a almendra"));
		T.Add(TEXT("Ach.Hydrated.Desc"), TEXT("Beber 5 raciones de agua de almendra."));
		T.Add(TEXT("Ach.MonsterAware.Name"), TEXT("Está cerca"));
		T.Add(TEXT("Ach.MonsterAware.Desc"), TEXT("Encontrar una entidad y sobrevivir."));
		T.Add(TEXT("Ach.EscapeArtist.Name"), TEXT("Primera salida"));
		T.Add(TEXT("Ach.EscapeArtist.Desc"), TEXT("Encontrar una salida."));
		T.Add(TEXT("Ach.FrequentFlyer.Name"), TEXT("Visitante frecuente"));
		T.Add(TEXT("Ach.FrequentFlyer.Desc"), TEXT("Pasar por 5 salidas."));
		T.Add(TEXT("Ach.DeepFear.Name"), TEXT("Límite del miedo"));
		T.Add(TEXT("Ach.DeepFear.Desc"), TEXT("Llevar la presión del entorno al máximo."));
		T.Add(TEXT("Ach.Dead.Name"), TEXT("Uno de ellos"));
		T.Add(TEXT("Ach.Dead.Desc"), TEXT("Morir en los Backrooms."));

		T.Add(TEXT("Bind.MoveForwardPlus"), TEXT("Adelante"));
		T.Add(TEXT("Bind.MoveForwardMinus"), TEXT("Atrás"));
		T.Add(TEXT("Bind.MoveRightPlus"), TEXT("Derecha"));
		T.Add(TEXT("Bind.MoveRightMinus"), TEXT("Izquierda"));
		T.Add(TEXT("Bind.Jump"), TEXT("Saltar"));
		T.Add(TEXT("Bind.Sprint"), TEXT("Correr"));
		T.Add(TEXT("Bind.Flashlight"), TEXT("Linterna"));
		T.Add(TEXT("Bind.View"), TEXT("Vista 1ª / 3ª persona"));
		T.Add(TEXT("Bind.Attack"), TEXT("Golpe de puño"));
		T.Add(TEXT("Bind.Grab"), TEXT("Coger / soltar objeto"));
		T.Add(TEXT("Bind.Push"), TEXT("Empujar objeto"));
		T.Add(TEXT("Bind.Throw"), TEXT("Lanzar objeto"));
		T.Add(TEXT("Bind.Inspect"), TEXT("Examinar objeto"));
		T.Add(TEXT("Bind.Use"), TEXT("Usar objeto"));
		T.Add(TEXT("Bind.Slot1"), TEXT("Ranura 1"));
		T.Add(TEXT("Bind.Slot2"), TEXT("Ranura 2"));
		T.Add(TEXT("Bind.Slot3"), TEXT("Ranura 3"));
		T.Add(TEXT("Bind.Slot4"), TEXT("Ranura 4"));
		T.Add(TEXT("Bind.Inventory"), TEXT("Inventario"));
		T.Add(TEXT("Bind.PauseMenu"), TEXT("Pausa / menú"));
		T.Add(TEXT("Bind.Mouse"), TEXT("Ratón"));
		T.Add(TEXT("Bind.Look"), TEXT("Mirar"));
		T.Add(TEXT("Bind.Change"), TEXT("cambiar"));
		T.Add(TEXT("Bind.PressKey"), TEXT("Pulsa una tecla para asignar... (Esc para cancelar)"));
		T.Add(TEXT("Bind.Cancel"), TEXT("CANCELAR  ×"));
		T.Add(TEXT("Bind.Reset"), TEXT("Restablecer todas las teclas por defecto"));

		T.Add(TEXT("Ach.WellFed.Name"), TEXT("Bien alimentado"));
		T.Add(TEXT("Ach.WellFed.Desc"), TEXT("Comer 10 raciones de comida."));
		T.Add(TEXT("Ach.Electrician.Name"), TEXT("Electricista"));
		T.Add(TEXT("Ach.Electrician.Desc"), TEXT("Insertar 10 pilas en la linterna."));
		T.Add(TEXT("Ach.Sprinter.Name"), TEXT("Corredor"));
		T.Add(TEXT("Ach.Sprinter.Desc"), TEXT("Empezar a correr 50 veces."));
		T.Add(TEXT("Ach.Cartographer.Name"), TEXT("Cartógrafo"));
		T.Add(TEXT("Ach.Cartographer.Desc"), TEXT("Atravesar 200 habitaciones."));
		T.Add(TEXT("Ach.Speedrun.Name"), TEXT("Velocista"));
		T.Add(TEXT("Ach.Speedrun.Desc"), TEXT("Recorrer 5 kilómetros bajo tierra."));
		T.Add(TEXT("Ach.Gourmand.Name"), TEXT("Gourmet"));
		T.Add(TEXT("Ach.Gourmand.Desc"), TEXT("Aguantar 30 minutos en los Backrooms."));

		T.Add(TEXT("GameOver.Title"), TEXT("HAS MUERTO"));
		T.Add(TEXT("GameOver.Subtitle"), TEXT("Los Backrooms no te sueltan. Inténtalo de nuevo."));
		T.Add(TEXT("GameOver.Restart"), TEXT("Reintentar"));
		T.Add(TEXT("GameOver.Quit"), TEXT("Salir"));

		T.Add(TEXT("Level.W0"), TEXT("L0 · Vestíbulo"));
		T.Add(TEXT("Level.W1"), TEXT("L1 · Zona habitada"));
		T.Add(TEXT("Level.W2"), TEXT("L2 · Tuberías"));
		T.Add(TEXT("Level.W3"), TEXT("L3 · Central eléctrica"));
		T.Add(TEXT("Level.W4"), TEXT("L4 · Oficinas"));
		T.Add(TEXT("Level.W5"), TEXT("L5 · Hotel"));
		T.Add(TEXT("Level.W6"), TEXT("L6 · Oscuridad"));
		T.Add(TEXT("Level.W7"), TEXT("L7 · Océano"));
		T.Add(TEXT("Level.W8"), TEXT("L8 · Cuevas"));
		T.Add(TEXT("Level.W9"), TEXT("L9 · Hospital"));

		T.Add(TEXT("Item.AlmondWater.Name"), TEXT("Agua de almendra"));
		T.Add(TEXT("Item.AlmondWater.Desc"), TEXT("Calma la sed y restaura algo de cordura."));
		T.Add(TEXT("Item.CanFood.Name"), TEXT("Comida enlatada"));
		T.Add(TEXT("Item.CanFood.Desc"), TEXT("Carne enlatada. Calma el hambre."));
		T.Add(TEXT("Item.MedKit.Name"), TEXT("Botiquín"));
		T.Add(TEXT("Item.MedKit.Desc"), TEXT("Detiene la hemorragia. Restaura la salud."));
		T.Add(TEXT("Item.Pill.Name"), TEXT("Tranquilizante"));
		T.Add(TEXT("Item.Pill.Desc"), TEXT("Calma los nervios. Restaura la cordura."));
		T.Add(TEXT("Item.Energy.Name"), TEXT("Bebida energética"));
		T.Add(TEXT("Item.Energy.Desc"), TEXT("Revitaliza y despeja un poco la mente."));
		T.Add(TEXT("Item.Battery.Name"), TEXT("Pila"));
		T.Add(TEXT("Item.Battery.Desc"), TEXT("Pila de alimentación para la linterna."));

		T.Add(TEXT("Event.LightFlicker"), TEXT("Parpadeo de luz"));
		T.Add(TEXT("Event.LightOutage"), TEXT("¡Se apagó la luz!"));
		T.Add(TEXT("Event.BoxDisappear"), TEXT("Una caja desapareció..."));
		T.Add(TEXT("Event.BoxAppear"), TEXT("¡Apareció una caja!"));
		T.Add(TEXT("Event.EntityGrowl"), TEXT("Un gruñido desde la oscuridad..."));
		T.Add(TEXT("Event.EntityFootsteps"), TEXT("Pasos tras la pared..."));
		T.Add(TEXT("Event.Whisper"), TEXT("Susurros..."));
		T.Add(TEXT("Event.DistantBang"), TEXT("¡Un estruendo a lo lejos!"));
		T.Add(TEXT("Event.WallDrawing"), TEXT("Un dibujo en la pared..."));
		T.Add(TEXT("Event.PipeCreak"), TEXT("Una tubería cruje"));
		T.Add(TEXT("Event.DoorSlam"), TEXT("¡Una puerta se cierra de golpe!"));
		T.Add(TEXT("Event.EmergencyLight"), TEXT("Luz de emergencia"));
		T.Add(TEXT("Event.FogIncrease"), TEXT("La niebla se espesa..."));
		T.Add(TEXT("Event.StaticNoise"), TEXT("Interferencias..."));
		T.Add(TEXT("Event.FootprintAppear"), TEXT("Huellas en el suelo..."));
		T.Add(TEXT("Event.Unknown"), TEXT("Evento desconocido"));

		T.Add(TEXT("Loading.Level"), TEXT("Cargando nivel..."));
		T.Add(TEXT("Loading.World"), TEXT("Cargando los Backrooms... %d%% (fragmentos: %d)"));
		T.Add(TEXT("Loading.Ready"), TEXT("¡Mundo listo! Falla en la realidad..."));
		T.Add(TEXT("Loading.Loaded"), TEXT("Nivel cargado"));
		T.Add(TEXT("Loading.Noclip"), TEXT("Noclip: cayendo en los Backrooms..."));
		T.Add(TEXT("Loading.Backrooms"), TEXT("Cargando los Backrooms..."));
		T.Add(TEXT("Loading.Chunks"), TEXT("chunks"));
		T.Add(TEXT("Loading.WorldReady"), TEXT("¡Mundo listo! Glitch de realidad..."));
		T.Add(TEXT("Loading.LevelDone"), TEXT("Nivel cargado"));

		T.Add(TEXT("Keys.SpaceBar"), TEXT("Espacio"));
		T.Add(TEXT("Keys.LeftShift"), TEXT("Shift"));
		T.Add(TEXT("Keys.RightShift"), TEXT("Shift (dcha.)"));
		T.Add(TEXT("Keys.LeftCtrl"), TEXT("Ctrl"));
		T.Add(TEXT("Keys.RightCtrl"), TEXT("Ctrl (dcho.)"));
		T.Add(TEXT("Keys.LeftAlt"), TEXT("Alt"));
		T.Add(TEXT("Keys.RightAlt"), TEXT("AltGr"));
		T.Add(TEXT("Keys.Escape"), TEXT("Esc"));
		T.Add(TEXT("Keys.LeftMouse"), TEXT("Botón izq."));
		T.Add(TEXT("Keys.RightMouse"), TEXT("Botón der."));
		T.Add(TEXT("Keys.MiddleMouse"), TEXT("Rueda (clic)"));
		T.Add(TEXT("Keys.ThumbMouse1"), TEXT("Botón lateral 1"));
		T.Add(TEXT("Keys.ThumbMouse2"), TEXT("Botón lateral 2"));
		T.Add(TEXT("Keys.MouseX"), TEXT("Ratón X"));
		T.Add(TEXT("Keys.MouseY"), TEXT("Ratón Y"));
		T.Add(TEXT("Keys.MouseWheel"), TEXT("Rueda del ratón"));
		T.Add(TEXT("Keys.Backspace"), TEXT("Retroceso"));
		T.Add(TEXT("Keys.Tab"), TEXT("Tabulador"));
		T.Add(TEXT("Keys.Enter"), TEXT("Intro"));
		T.Add(TEXT("Keys.On"), TEXT("SÍ"));
		T.Add(TEXT("Keys.Off"), TEXT("NO"));

		T.Add(TEXT("Set.DisplayMode.Fullscreen"), TEXT("Pantalla completa"));
		T.Add(TEXT("Set.DisplayMode.Borderless"), TEXT("Sin bordes"));
		T.Add(TEXT("Set.DisplayMode.Windowed"), TEXT("Ventana"));
		T.Add(TEXT("Set.RecordingEffect.Off"), TEXT("Desactivado"));
		T.Add(TEXT("Set.RecordingEffect.Light"), TEXT("Película ligera"));
		T.Add(TEXT("Set.RecordingEffect.Medium"), TEXT("Película media"));
		T.Add(TEXT("Set.RecordingEffect.Strong"), TEXT("Película fuerte"));
		T.Add(TEXT("Set.Quality.Overall"), TEXT("Calidad general"));
		T.Add(TEXT("Set.PotatoMode"), TEXT("Modo patata"));
		T.Add(TEXT("Set.PotatoMode.Hint"), TEXT("FPS máximo sacrificando los gráficos."));
		T.Add(TEXT("Set.Graphics"), TEXT("Gráficos"));
		T.Add(TEXT("Q.Low"), TEXT("Baja"));
		T.Add(TEXT("Q.Medium"), TEXT("Media"));
		T.Add(TEXT("Q.High"), TEXT("Alta"));
		T.Add(TEXT("Q.Epic"), TEXT("Épica"));
		T.Add(TEXT("Q.Ultra"), TEXT("Ultra"));

		T.Add(TEXT("Pause.Resume"), TEXT("Continuar"));
		T.Add(TEXT("Pause.Restart"), TEXT("Reiniciar"));
		T.Add(TEXT("Pause.Quit"), TEXT("Salir del juego"));

		T.Add(TEXT("Interact.TakeOff"), TEXT("Guardar"));

		T.Add(TEXT("Menu.StressTest"), TEXT("PRUEBA DE ESTRES"));
		T.Add(TEXT("HUD.Fps"), TEXT("FPS"));
		T.Add(TEXT("HUD.FpsAvg"), TEXT("Prom"));
		T.Add(TEXT("HUD.FpsOneLow"), TEXT("1% Bajo"));
		T.Add(TEXT("Loading.StressTest"), TEXT("Prueba de estres..."));
	}
}
