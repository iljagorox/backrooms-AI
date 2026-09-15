#include "BackroomsLocRegistry.h"

// Português (Brasil).
namespace BackroomsLoc
{
	void Register_PT(FLanguageTable& T)
	{
		T.Add(TEXT("Menu.NewGame"), TEXT("Novo jogo"));
		T.Add(TEXT("Menu.Continue"), TEXT("Continuar"));
		T.Add(TEXT("Menu.Levels"), TEXT("Níveis"));
		T.Add(TEXT("Menu.Achievements"), TEXT("Conquistas"));
		T.Add(TEXT("Menu.Settings"), TEXT("Configurações"));
		T.Add(TEXT("Menu.Controls"), TEXT("Controles"));
		T.Add(TEXT("Menu.Quit"), TEXT("Sair"));
		T.Add(TEXT("Menu.Subtitle"), TEXT("DESCIDA LIMINAL"));
		T.Add(TEXT("Menu.Back"), TEXT("<   V O L T A R"));

		T.Add(TEXT("Tab.Screen"), TEXT("TELA"));
		T.Add(TEXT("Tab.Quality"), TEXT("QUALIDADE"));
		T.Add(TEXT("Tab.Camera"), TEXT("CÂMERA"));
		T.Add(TEXT("Tab.Sound"), TEXT("SOM"));
		T.Add(TEXT("Tab.Game"), TEXT("JOGO"));
		T.Add(TEXT("Tab.Controls"), TEXT("CONTROLES"));
		T.Add(TEXT("Settings.Title"), TEXT("C O N F I G U R A Ç Õ E S"));
		T.Add(TEXT("Settings.Categories"), TEXT("C A T E G O R I A S"));

		T.Add(TEXT("Set.Resolution"), TEXT("Resolução"));
		T.Add(TEXT("Set.DisplayMode"), TEXT("Modo de tela"));
		T.Add(TEXT("Set.VSync"), TEXT("VSync"));
		T.Add(TEXT("Set.RenderScale"), TEXT("Escala de renderização"));
		T.Add(TEXT("Set.Gamma"), TEXT("Gama"));
		T.Add(TEXT("Set.Language"), TEXT("Idioma"));

		T.Add(TEXT("Set.Shadows"), TEXT("Sombras"));
		T.Add(TEXT("Set.Textures"), TEXT("Texturas"));
		T.Add(TEXT("Set.AntiAliasing"), TEXT("Suavização"));
		T.Add(TEXT("Set.PostProcess"), TEXT("Pós-processamento"));
		T.Add(TEXT("Set.Effects"), TEXT("Efeitos"));
		T.Add(TEXT("Set.ViewDistance"), TEXT("Distância de visão"));
		T.Add(TEXT("Set.DepthOfField"), TEXT("Profundidade de campo"));
		T.Add(TEXT("Set.Bloom"), TEXT("Brilho (Bloom)"));
		T.Add(TEXT("Set.Lumen"), TEXT("Lumen (iluminação)"));
		T.Add(TEXT("Set.Upscale"), TEXT("DLSS / Escala"));

		T.Add(TEXT("Set.FOV"), TEXT("Campo de visão"));
		T.Add(TEXT("Set.Sensitivity"), TEXT("Sensibilidade"));
		T.Add(TEXT("Set.RecordingEffect"), TEXT("Efeito de câmera de gravação"));

		T.Add(TEXT("Set.Volume.Master"), TEXT("Geral"));
		T.Add(TEXT("Set.Volume.Menu"), TEXT("Música / menu"));
		T.Add(TEXT("Set.Volume.Game"), TEXT("Sons do jogo"));
		T.Add(TEXT("Set.Volume.Monster"), TEXT("Sons de monstros"));
		T.Add(TEXT("Set.Volume.Other"), TEXT("Outros sons"));

		T.Add(TEXT("Set.Difficulty"), TEXT("Dificuldade"));
		T.Add(TEXT("Set.Difficulty.Hint"),
			TEXT("A dificuldade afeta a geração: densidade de paredes, número de passagens, generosidade de suprimentos e itens."));

		T.Add(TEXT("Levels.Title"), TEXT("N Í V E I S"));
		T.Add(TEXT("Levels.Locked"), TEXT("BLOQUEADO"));
		T.Add(TEXT("Levels.UnlockedFmt"),
			TEXT("Níveis liberados: %d de 10. Os novos abrem conforme você desce — vá mais fundo para ver as prévias."));

		T.Add(TEXT("Ach.Title"), TEXT("C O N Q U I S T A S"));
		T.Add(TEXT("Ach.Unlocked"), TEXT("CONQUISTA"));
		T.Add(TEXT("Ach.ProgressFmt"), TEXT("Liberadas: %d / %d     Pontos: %d"));

		T.Add(TEXT("Diff.Peaceful.Name"), TEXT("Pacífico"));
		T.Add(TEXT("Diff.Peaceful.Desc"), TEXT("Sem monstros nem pressão. Só exploração e o mundo."));
		T.Add(TEXT("Diff.Easy.Name"), TEXT("Fácil"));
		T.Add(TEXT("Diff.Easy.Desc"), TEXT("Locais abertos e generosos. Um monstro aparece tarde."));
		T.Add(TEXT("Diff.Normal.Name"), TEXT("Normal"));
		T.Add(TEXT("Diff.Normal.Desc"), TEXT("O equilíbrio pretendido: corredores estreitos, fome, uma entidade solitária."));
		T.Add(TEXT("Diff.Hard.Name"), TEXT("Difícil"));
		T.Add(TEXT("Diff.Hard.Desc"), TEXT("Mais paredes e menos portas, itens escassos, duas entidades."));
		T.Add(TEXT("Diff.Nightmare.Name"), TEXT("Pesadelo"));
		T.Add(TEXT("Diff.Nightmare.Desc"), TEXT("Um labirinto armadilha, suprimentos mínimos, três entidades, a morte é definitiva."));

		T.Add(TEXT("HUD.Sanity"), TEXT("SANIDADE"));
		T.Add(TEXT("HUD.Flashlight"), TEXT("LANTERNA"));
		T.Add(TEXT("HUD.Hunger"), TEXT("FOME"));
		T.Add(TEXT("HUD.Thirst"), TEXT("SEDE"));
		T.Add(TEXT("HUD.Health"), TEXT("SAÚDE"));
		T.Add(TEXT("HUD.Stamina"), TEXT("VIGOR"));
		T.Add(TEXT("HUD.Inventory"), TEXT("INVENTÁRIO"));
		T.Add(TEXT("HUD.Empty"), TEXT("vazio"));
		T.Add(TEXT("Prog.Level"), TEXT("Nivel"));
		T.Add(TEXT("Prog.LevelUp"), TEXT("Subiu de nivel"));

		T.Add(TEXT("Interact.Inspect"), TEXT("Examinar"));
		T.Add(TEXT("Interact.Throw"), TEXT("Arremessar"));
		T.Add(TEXT("Interact.Drop"), TEXT("Largar"));
		T.Add(TEXT("Interact.Take"), TEXT("Pegar"));
		T.Add(TEXT("Interact.Push"), TEXT("Empurrar"));
		T.Add(TEXT("Interact.Use"), TEXT("Usar"));
		T.Add(TEXT("Interact.Pickup"), TEXT("Apanhar"));

		T.Add(TEXT("Hint.Critical"), TEXT("Estado crítico — encontre remédio agora"));
		T.Add(TEXT("Hint.Sanity"), TEXT("Sanidade baixa — esconda-se e respire"));
		T.Add(TEXT("Hint.Hunger"), TEXT("Muita fome — encontre comida"));
		T.Add(TEXT("Hint.Thirst"), TEXT("Muita sede — beba água"));
		T.Add(TEXT("Hint.Monster"), TEXT("Está perto — fique quieto"));
		T.Add(TEXT("Hint.Poison"), TEXT("Você está envenenado — busque água de amêndoa ou um kit de primeiros socorros"));
		T.Add(TEXT("Hint.Radiation"), TEXT("Radiação alta — saia daqui"));
		T.Add(TEXT("Hint.Sleep"), TEXT("Você está exausto — pare e descanse"));
		T.Add(TEXT("Hint.Battery"), TEXT("A lanterna está quase acabando"));

		T.Add(TEXT("Ach.FirstSteps.Name"), TEXT("Primeiros passos"));
		T.Add(TEXT("Ach.FirstSteps.Desc"), TEXT("Descer para os Backrooms."));
		T.Add(TEXT("Ach.Tourist.Name"), TEXT("Turista"));
		T.Add(TEXT("Ach.Tourist.Desc"), TEXT("Visitar 3 locais diferentes."));
		T.Add(TEXT("Ach.Explorer.Name"), TEXT("Explorador"));
		T.Add(TEXT("Ach.Explorer.Desc"), TEXT("Visitar 5 locais diferentes."));
		T.Add(TEXT("Ach.DeepDiver.Name"), TEXT("Nas profundezas"));
		T.Add(TEXT("Ach.DeepDiver.Desc"), TEXT("Visitar 7 locais diferentes."));
		T.Add(TEXT("Ach.AllLevels.Name"), TEXT("Andar inteiro"));
		T.Add(TEXT("Ach.AllLevels.Desc"), TEXT("Visitar todos os 10 locais."));
		T.Add(TEXT("Ach.Survivor.Name"), TEXT("Sobrevivente"));
		T.Add(TEXT("Ach.Survivor.Desc"), TEXT("Aguentar 15 minutos nos Backrooms."));
		T.Add(TEXT("Ach.Marathon.Name"), TEXT("Maratonista"));
		T.Add(TEXT("Ach.Marathon.Desc"), TEXT("Percorrer 3 quilômetros no subsolo."));
		T.Add(TEXT("Ach.Scavenger.Name"), TEXT("Catador"));
		T.Add(TEXT("Ach.Scavenger.Desc"), TEXT("Pegar 10 itens."));
		T.Add(TEXT("Ach.Pharmacist.Name"), TEXT("Farmacêutico"));
		T.Add(TEXT("Ach.Pharmacist.Desc"), TEXT("Usar 5 medicamentos."));
		T.Add(TEXT("Ach.Hydrated.Name"), TEXT("Gosto de amêndoa"));
		T.Add(TEXT("Ach.Hydrated.Desc"), TEXT("Beber 5 porções de água de amêndoa."));
		T.Add(TEXT("Ach.MonsterAware.Name"), TEXT("Está perto"));
		T.Add(TEXT("Ach.MonsterAware.Desc"), TEXT("Encontrar uma entidade e sobreviver."));
		T.Add(TEXT("Ach.EscapeArtist.Name"), TEXT("Primeira saída"));
		T.Add(TEXT("Ach.EscapeArtist.Desc"), TEXT("Encontrar uma saída."));
		T.Add(TEXT("Ach.FrequentFlyer.Name"), TEXT("Visitante frequente"));
		T.Add(TEXT("Ach.FrequentFlyer.Desc"), TEXT("Passar por 5 saídas."));
		T.Add(TEXT("Ach.DeepFear.Name"), TEXT("Limite do medo"));
		T.Add(TEXT("Ach.DeepFear.Desc"), TEXT("Levar a pressão do ambiente ao máximo."));
		T.Add(TEXT("Ach.Dead.Name"), TEXT("Um deles"));
		T.Add(TEXT("Ach.Dead.Desc"), TEXT("Morrer nos Backrooms."));

		T.Add(TEXT("Bind.MoveForwardPlus"), TEXT("Frente"));
		T.Add(TEXT("Bind.MoveForwardMinus"), TEXT("Trás"));
		T.Add(TEXT("Bind.MoveRightPlus"), TEXT("Direita"));
		T.Add(TEXT("Bind.MoveRightMinus"), TEXT("Esquerda"));
		T.Add(TEXT("Bind.Jump"), TEXT("Pular"));
		T.Add(TEXT("Bind.Sprint"), TEXT("Correr"));
		T.Add(TEXT("Bind.Flashlight"), TEXT("Lanterna"));
		T.Add(TEXT("Bind.View"), TEXT("Visão 1ª / 3ª pessoa"));
		T.Add(TEXT("Bind.Attack"), TEXT("Soco"));
		T.Add(TEXT("Bind.Grab"), TEXT("Pegar / soltar item"));
		T.Add(TEXT("Bind.Push"), TEXT("Empurrar item"));
		T.Add(TEXT("Bind.Throw"), TEXT("Arremessar item"));
		T.Add(TEXT("Bind.Inspect"), TEXT("Inspecionar item"));
		T.Add(TEXT("Bind.Use"), TEXT("Usar item"));
		T.Add(TEXT("Bind.Slot1"), TEXT("Slot 1"));
		T.Add(TEXT("Bind.Slot2"), TEXT("Slot 2"));
		T.Add(TEXT("Bind.Slot3"), TEXT("Slot 3"));
		T.Add(TEXT("Bind.Slot4"), TEXT("Slot 4"));
		T.Add(TEXT("Bind.Inventory"), TEXT("Inventário"));
		T.Add(TEXT("Bind.PauseMenu"), TEXT("Pausa / menu"));
		T.Add(TEXT("Bind.Mouse"), TEXT("Mouse"));
		T.Add(TEXT("Bind.Look"), TEXT("Olhar"));
		T.Add(TEXT("Bind.Change"), TEXT("alterar"));
		T.Add(TEXT("Bind.PressKey"), TEXT("Pressione uma tecla para atribuir... (Esc para cancelar)"));
		T.Add(TEXT("Bind.Cancel"), TEXT("CANCELAR  ×"));
		T.Add(TEXT("Bind.Reset"), TEXT("Restaurar todas as teclas padrão"));

		T.Add(TEXT("Ach.WellFed.Name"), TEXT("Bem alimentado"));
		T.Add(TEXT("Ach.WellFed.Desc"), TEXT("Comer 10 porções de comida."));
		T.Add(TEXT("Ach.Electrician.Name"), TEXT("Eletricista"));
		T.Add(TEXT("Ach.Electrician.Desc"), TEXT("Inserir 10 pilhas na lanterna."));
		T.Add(TEXT("Ach.Sprinter.Name"), TEXT("Velocista"));
		T.Add(TEXT("Ach.Sprinter.Desc"), TEXT("Começar a correr 50 vezes."));
		T.Add(TEXT("Ach.Cartographer.Name"), TEXT("Cartógrafo"));
		T.Add(TEXT("Ach.Cartographer.Desc"), TEXT("Atravessar 200 salas."));
		T.Add(TEXT("Ach.Speedrun.Name"), TEXT("Speedrunner"));
		T.Add(TEXT("Ach.Speedrun.Desc"), TEXT("Percorrer 5 quilômetros no subsolo."));
		T.Add(TEXT("Ach.Gourmand.Name"), TEXT("Gourmet"));
		T.Add(TEXT("Ach.Gourmand.Desc"), TEXT("Aguentar 30 minutos nos Backrooms."));

		T.Add(TEXT("GameOver.Title"), TEXT("VOCÊ MORREU"));
		T.Add(TEXT("GameOver.Subtitle"), TEXT("Os Backrooms nunca soltam. Tente novamente."));
		T.Add(TEXT("GameOver.Restart"), TEXT("Recomeçar"));
		T.Add(TEXT("GameOver.Quit"), TEXT("Sair"));

		T.Add(TEXT("Level.W0"), TEXT("L0 · Saguão"));
		T.Add(TEXT("Level.W1"), TEXT("L1 · Zona habitada"));
		T.Add(TEXT("Level.W2"), TEXT("L2 · Encanamento"));
		T.Add(TEXT("Level.W3"), TEXT("L3 · Usina elétrica"));
		T.Add(TEXT("Level.W4"), TEXT("L4 · Escritórios"));
		T.Add(TEXT("Level.W5"), TEXT("L5 · Hotel"));
		T.Add(TEXT("Level.W6"), TEXT("L6 · Escuridão"));
		T.Add(TEXT("Level.W7"), TEXT("L7 · Oceano"));
		T.Add(TEXT("Level.W8"), TEXT("L8 · Cavernas"));
		T.Add(TEXT("Level.W9"), TEXT("L9 · Hospital"));

		T.Add(TEXT("Item.AlmondWater.Name"), TEXT("Água de amêndoa"));
		T.Add(TEXT("Item.AlmondWater.Desc"), TEXT("Mata a sede e restaura um pouco de sanidade."));
		T.Add(TEXT("Item.CanFood.Name"), TEXT("Comida enlatada"));
		T.Add(TEXT("Item.CanFood.Desc"), TEXT("Carne enlatada. Mata a fome."));
		T.Add(TEXT("Item.MedKit.Name"), TEXT("Kit médico"));
		T.Add(TEXT("Item.MedKit.Desc"), TEXT("Estanca o sangramento. Restaura a saúde."));
		T.Add(TEXT("Item.Pill.Name"), TEXT("Tranquilizante"));
		T.Add(TEXT("Item.Pill.Desc"), TEXT("Acalma os nervos. Restaura a sanidade."));
		T.Add(TEXT("Item.Energy.Name"), TEXT("Energético"));
		T.Add(TEXT("Item.Energy.Desc"), TEXT("Revigora e clareia um pouco a mente."));
		T.Add(TEXT("Item.Battery.Name"), TEXT("Pilha"));
		T.Add(TEXT("Item.Battery.Desc"), TEXT("Uma pilha de energia para a lanterna."));

		T.Add(TEXT("Event.LightFlicker"), TEXT("Luz tremeluzindo"));
		T.Add(TEXT("Event.LightOutage"), TEXT("A luz apagou!"));
		T.Add(TEXT("Event.BoxDisappear"), TEXT("Uma caixa desapareceu..."));
		T.Add(TEXT("Event.BoxAppear"), TEXT("Uma caixa apareceu!"));
		T.Add(TEXT("Event.EntityGrowl"), TEXT("Um rosnado na escuridão..."));
		T.Add(TEXT("Event.EntityFootsteps"), TEXT("Passos atrás da parede..."));
		T.Add(TEXT("Event.Whisper"), TEXT("Sussurros..."));
		T.Add(TEXT("Event.DistantBang"), TEXT("Um estrondo ao longe!"));
		T.Add(TEXT("Event.WallDrawing"), TEXT("Um desenho na parede..."));
		T.Add(TEXT("Event.PipeCreak"), TEXT("Um cano range"));
		T.Add(TEXT("Event.DoorSlam"), TEXT("Uma porta bate!"));
		T.Add(TEXT("Event.EmergencyLight"), TEXT("Luz de emergência"));
		T.Add(TEXT("Event.FogIncrease"), TEXT("A névoa engrossa..."));
		T.Add(TEXT("Event.StaticNoise"), TEXT("Interferência..."));
		T.Add(TEXT("Event.FootprintAppear"), TEXT("Pegadas no chão..."));
		T.Add(TEXT("Event.Unknown"), TEXT("Evento desconhecido"));

		T.Add(TEXT("Loading.Level"), TEXT("Carregando nível..."));
		T.Add(TEXT("Loading.World"), TEXT("Carregando os Backrooms... %d%% (chunks: %d)"));
		T.Add(TEXT("Loading.Ready"), TEXT("Mundo pronto! Falha na realidade..."));
		T.Add(TEXT("Loading.Loaded"), TEXT("Nível carregado"));
		T.Add(TEXT("Loading.Noclip"), TEXT("Noclip: caindo nos Backrooms..."));
		T.Add(TEXT("Loading.Backrooms"), TEXT("Carregando os Backrooms..."));
		T.Add(TEXT("Loading.Chunks"), TEXT("pedaços"));
		T.Add(TEXT("Loading.WorldReady"), TEXT("Mundo pronto! Glitch de realidade..."));
		T.Add(TEXT("Loading.LevelDone"), TEXT("Nível carregado"));

		T.Add(TEXT("Keys.SpaceBar"), TEXT("Espaço"));
		T.Add(TEXT("Keys.LeftShift"), TEXT("Shift"));
		T.Add(TEXT("Keys.RightShift"), TEXT("Shift (dir.)"));
		T.Add(TEXT("Keys.LeftCtrl"), TEXT("Ctrl"));
		T.Add(TEXT("Keys.RightCtrl"), TEXT("Ctrl (dir.)"));
		T.Add(TEXT("Keys.LeftAlt"), TEXT("Alt"));
		T.Add(TEXT("Keys.RightAlt"), TEXT("AltGr"));
		T.Add(TEXT("Keys.Escape"), TEXT("Esc"));
		T.Add(TEXT("Keys.LeftMouse"), TEXT("Botão esq."));
		T.Add(TEXT("Keys.RightMouse"), TEXT("Botão dir."));
		T.Add(TEXT("Keys.MiddleMouse"), TEXT("Roda (clique)"));
		T.Add(TEXT("Keys.ThumbMouse1"), TEXT("Botão lateral 1"));
		T.Add(TEXT("Keys.ThumbMouse2"), TEXT("Botão lateral 2"));
		T.Add(TEXT("Keys.MouseX"), TEXT("Mouse X"));
		T.Add(TEXT("Keys.MouseY"), TEXT("Mouse Y"));
		T.Add(TEXT("Keys.MouseWheel"), TEXT("Roda do mouse"));
		T.Add(TEXT("Keys.Backspace"), TEXT("Backspace"));
		T.Add(TEXT("Keys.Tab"), TEXT("Tab"));
		T.Add(TEXT("Keys.Enter"), TEXT("Enter"));
		T.Add(TEXT("Keys.On"), TEXT("SIM"));
		T.Add(TEXT("Keys.Off"), TEXT("NÃO"));

		T.Add(TEXT("Set.DisplayMode.Fullscreen"), TEXT("Tela cheia"));
		T.Add(TEXT("Set.DisplayMode.Borderless"), TEXT("Sem bordas"));
		T.Add(TEXT("Set.DisplayMode.Windowed"), TEXT("Janela"));
		T.Add(TEXT("Set.RecordingEffect.Off"), TEXT("Desativado"));
		T.Add(TEXT("Set.RecordingEffect.Light"), TEXT("Efeito filme leve"));
		T.Add(TEXT("Set.RecordingEffect.Medium"), TEXT("Efeito filme médio"));
		T.Add(TEXT("Set.RecordingEffect.Strong"), TEXT("Efeito filme forte"));
		T.Add(TEXT("Set.Quality.Overall"), TEXT("Qualidade geral"));
		T.Add(TEXT("Set.PotatoMode"), TEXT("Modo batata"));
		T.Add(TEXT("Set.PotatoMode.Hint"), TEXT("FPS máximo às custas dos gráficos."));
		T.Add(TEXT("Set.Graphics"), TEXT("Gráficos"));
		T.Add(TEXT("Q.Low"), TEXT("Baixa"));
		T.Add(TEXT("Q.Medium"), TEXT("Média"));
		T.Add(TEXT("Q.High"), TEXT("Alta"));
		T.Add(TEXT("Q.Epic"), TEXT("Épica"));
		T.Add(TEXT("Q.Ultra"), TEXT("Ultra"));

		T.Add(TEXT("Pause.Resume"), TEXT("Continuar"));
		T.Add(TEXT("Pause.Restart"), TEXT("Reiniciar"));
		T.Add(TEXT("Pause.Quit"), TEXT("Sair do jogo"));

		T.Add(TEXT("Interact.TakeOff"), TEXT("Guardar"));

		T.Add(TEXT("Menu.StressTest"), TEXT("TESTE DE ESTRESSE"));
		T.Add(TEXT("HUD.Fps"), TEXT("FPS"));
		T.Add(TEXT("HUD.FpsAvg"), TEXT("Media"));
		T.Add(TEXT("HUD.FpsOneLow"), TEXT("1% Baixo"));
		T.Add(TEXT("Loading.StressTest"), TEXT("Teste de estresse..."));
	}
}
