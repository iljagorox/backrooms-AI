#include "BackroomsLocRegistry.h"

// Français.
namespace BackroomsLoc
{
	void Register_FR(FLanguageTable& T)
	{
		T.Add(TEXT("Menu.NewGame"), TEXT("Nouvelle partie"));
		T.Add(TEXT("Menu.Continue"), TEXT("Continuer"));
		T.Add(TEXT("Menu.Levels"), TEXT("Niveaux"));
		T.Add(TEXT("Menu.Achievements"), TEXT("Succès"));
		T.Add(TEXT("Menu.Settings"), TEXT("Paramètres"));
		T.Add(TEXT("Menu.Controls"), TEXT("Commandes"));
		T.Add(TEXT("Menu.Quit"), TEXT("Quitter"));
		T.Add(TEXT("Menu.Subtitle"), TEXT("DESCENTE LIMINALE"));
		T.Add(TEXT("Menu.Back"), TEXT("<   R E T O U R"));

		T.Add(TEXT("Tab.Screen"), TEXT("ÉCRAN"));
		T.Add(TEXT("Tab.Quality"), TEXT("QUALITÉ"));
		T.Add(TEXT("Tab.Camera"), TEXT("CAMÉRA"));
		T.Add(TEXT("Tab.Sound"), TEXT("SON"));
		T.Add(TEXT("Tab.Game"), TEXT("JEU"));
		T.Add(TEXT("Tab.Controls"), TEXT("COMMANDES"));
		T.Add(TEXT("Settings.Title"), TEXT("P A R A M È T R E S"));
		T.Add(TEXT("Settings.Categories"), TEXT("C A T É G O R I E S"));

		T.Add(TEXT("Set.Resolution"), TEXT("Résolution"));
		T.Add(TEXT("Set.DisplayMode"), TEXT("Mode d'affichage"));
		T.Add(TEXT("Set.VSync"), TEXT("VSync"));
		T.Add(TEXT("Set.RenderScale"), TEXT("Échelle de rendu"));
		T.Add(TEXT("Set.Gamma"), TEXT("Gamma"));
		T.Add(TEXT("Set.Language"), TEXT("Langue"));

		T.Add(TEXT("Set.Shadows"), TEXT("Ombres"));
		T.Add(TEXT("Set.Textures"), TEXT("Textures"));
		T.Add(TEXT("Set.AntiAliasing"), TEXT("Anticrénelage"));
		T.Add(TEXT("Set.PostProcess"), TEXT("Post-traitement"));
		T.Add(TEXT("Set.Effects"), TEXT("Effets"));
		T.Add(TEXT("Set.ViewDistance"), TEXT("Distance de vue"));
		T.Add(TEXT("Set.DepthOfField"), TEXT("Profondeur de champ"));
		T.Add(TEXT("Set.Bloom"), TEXT("Halo (Bloom)"));
		T.Add(TEXT("Set.Lumen"), TEXT("Lumen (éclairage)"));
		T.Add(TEXT("Set.Upscale"), TEXT("DLSS / Mise à l'échelle"));

		T.Add(TEXT("Set.FOV"), TEXT("Champ de vision"));
		T.Add(TEXT("Set.Sensitivity"), TEXT("Sensibilité"));
		T.Add(TEXT("Set.RecordingEffect"), TEXT("Effet caméra d'enregistrement"));

		T.Add(TEXT("Set.Volume.Master"), TEXT("Général"));
		T.Add(TEXT("Set.Volume.Menu"), TEXT("Musique / menu"));
		T.Add(TEXT("Set.Volume.Game"), TEXT("Sons du jeu"));
		T.Add(TEXT("Set.Volume.Monster"), TEXT("Sons des monstres"));
		T.Add(TEXT("Set.Volume.Other"), TEXT("Autres sons"));

		T.Add(TEXT("Set.Difficulty"), TEXT("Difficulté"));
		T.Add(TEXT("Set.Difficulty.Hint"),
			TEXT("La difficulté influe sur la génération : densité des murs, nombre de portes, générosité des provisions et du butin."));

		T.Add(TEXT("Levels.Title"), TEXT("N I V E A U X"));
		T.Add(TEXT("Levels.Locked"), TEXT("VERROUILLÉ"));
		T.Add(TEXT("Levels.UnlockedFmt"),
			TEXT("Niveaux débloqués : %d sur 10. Les nouveaux s'ouvrent en descendant — descends plus bas pour voir leurs aperçus."));

		T.Add(TEXT("Ach.Title"), TEXT("S U C C È S"));
		T.Add(TEXT("Ach.Unlocked"), TEXT("SUCCÈS"));
		T.Add(TEXT("Ach.ProgressFmt"), TEXT("Débloqués : %d / %d     Points : %d"));

		T.Add(TEXT("Diff.Peaceful.Name"), TEXT("Paisible"));
		T.Add(TEXT("Diff.Peaceful.Desc"), TEXT("Sans monstres ni pression. Seulement l'exploration et le monde."));
		T.Add(TEXT("Diff.Easy.Name"), TEXT("Facile"));
		T.Add(TEXT("Diff.Easy.Desc"), TEXT("Lieux ouverts et généreux. Un monstre apparaît tard."));
		T.Add(TEXT("Diff.Normal.Name"), TEXT("Normal"));
		T.Add(TEXT("Diff.Normal.Desc"), TEXT("L'équilibre prévu : couloirs étroits, faim, une entité solitaire."));
		T.Add(TEXT("Diff.Hard.Name"), TEXT("Difficile"));
		T.Add(TEXT("Diff.Hard.Desc"), TEXT("Plus de murs et moins de portes, butin rare, deux entités."));
		T.Add(TEXT("Diff.Nightmare.Name"), TEXT("Cauchemar"));
		T.Add(TEXT("Diff.Nightmare.Desc"), TEXT("Un labyrinthe piège, provisions minimales, trois entités, la mort est définitive."));

		T.Add(TEXT("HUD.Sanity"), TEXT("SANTÉ MENTALE"));
		T.Add(TEXT("HUD.Flashlight"), TEXT("LAMPE TORCHE"));
		T.Add(TEXT("HUD.Hunger"), TEXT("FAIM"));
		T.Add(TEXT("HUD.Thirst"), TEXT("SOIF"));
		T.Add(TEXT("HUD.Health"), TEXT("SANTÉ"));
		T.Add(TEXT("HUD.Stamina"), TEXT("ENDURANCE"));
		T.Add(TEXT("HUD.Inventory"), TEXT("INVENTAIRE"));
		T.Add(TEXT("HUD.Empty"), TEXT("vide"));
		T.Add(TEXT("Prog.Level"), TEXT("Niveau"));
		T.Add(TEXT("Prog.LevelUp"), TEXT("Niveau superieur"));

		T.Add(TEXT("Interact.Inspect"), TEXT("Examiner"));
		T.Add(TEXT("Interact.Throw"), TEXT("Lancer"));
		T.Add(TEXT("Interact.Drop"), TEXT("Poser"));
		T.Add(TEXT("Interact.Take"), TEXT("Prendre"));
		T.Add(TEXT("Interact.Push"), TEXT("Pousser"));
		T.Add(TEXT("Interact.Use"), TEXT("Utiliser"));
		T.Add(TEXT("Interact.Pickup"), TEXT("Ramasser"));

		T.Add(TEXT("Hint.Critical"), TEXT("État critique — trouve un médicament immédiatement"));
		T.Add(TEXT("Hint.Sanity"), TEXT("Santé mentale faible — cache-toi et respire"));
		T.Add(TEXT("Hint.Hunger"), TEXT("Famine — trouve de la nourriture"));
		T.Add(TEXT("Hint.Thirst"), TEXT("Très assoiffé — bois de l'eau"));
		T.Add(TEXT("Hint.Monster"), TEXT("Il est proche — ne fais pas de bruit"));
		T.Add(TEXT("Hint.Poison"), TEXT("Tu es empoisonné — trouve de l'eau d'amande ou une trousse de secours"));
		T.Add(TEXT("Hint.Radiation"), TEXT("Radiations élevées — sors d'ici"));
		T.Add(TEXT("Hint.Sleep"), TEXT("Tu es épuisé — arrête-toi et repose-toi"));
		T.Add(TEXT("Hint.Battery"), TEXT("La lampe torche est presque à plat"));

		T.Add(TEXT("Ach.FirstSteps.Name"), TEXT("Premiers pas"));
		T.Add(TEXT("Ach.FirstSteps.Desc"), TEXT("Descendre dans les Backrooms."));
		T.Add(TEXT("Ach.Tourist.Name"), TEXT("Touriste"));
		T.Add(TEXT("Ach.Tourist.Desc"), TEXT("Visiter 3 lieux différents."));
		T.Add(TEXT("Ach.Explorer.Name"), TEXT("Explorateur"));
		T.Add(TEXT("Ach.Explorer.Desc"), TEXT("Visiter 5 lieux différents."));
		T.Add(TEXT("Ach.DeepDiver.Name"), TEXT("En profondeur"));
		T.Add(TEXT("Ach.DeepDiver.Desc"), TEXT("Visiter 7 lieux différents."));
		T.Add(TEXT("Ach.AllLevels.Name"), TEXT("Étage complet"));
		T.Add(TEXT("Ach.AllLevels.Desc"), TEXT("Visiter les 10 lieux."));
		T.Add(TEXT("Ach.Survivor.Name"), TEXT("Survivant"));
		T.Add(TEXT("Ach.Survivor.Desc"), TEXT("Tenir 15 minutes dans les Backrooms."));
		T.Add(TEXT("Ach.Marathon.Name"), TEXT("Marathonien"));
		T.Add(TEXT("Ach.Marathon.Desc"), TEXT("Parcourir 3 kilomètres sous terre."));
		T.Add(TEXT("Ach.Scavenger.Name"), TEXT("Fouilleur"));
		T.Add(TEXT("Ach.Scavenger.Desc"), TEXT("Ramasser 10 objets."));
		T.Add(TEXT("Ach.Pharmacist.Name"), TEXT("Pharmacien"));
		T.Add(TEXT("Ach.Pharmacist.Desc"), TEXT("Utiliser 5 médicaments."));
		T.Add(TEXT("Ach.Hydrated.Name"), TEXT("Goût d'amande"));
		T.Add(TEXT("Ach.Hydrated.Desc"), TEXT("Boire 5 rations d'eau d'amande."));
		T.Add(TEXT("Ach.MonsterAware.Name"), TEXT("Il est proche"));
		T.Add(TEXT("Ach.MonsterAware.Desc"), TEXT("Rencontrer une entité et survivre."));
		T.Add(TEXT("Ach.EscapeArtist.Name"), TEXT("Première sortie"));
		T.Add(TEXT("Ach.EscapeArtist.Desc"), TEXT("Trouver une sortie."));
		T.Add(TEXT("Ach.FrequentFlyer.Name"), TEXT("Visiteur fréquent"));
		T.Add(TEXT("Ach.FrequentFlyer.Desc"), TEXT("Passer par 5 sorties."));
		T.Add(TEXT("Ach.DeepFear.Name"), TEXT("Limite de la peur"));
		T.Add(TEXT("Ach.DeepFear.Desc"), TEXT("Pousser la pression de l'environnement au maximum."));
		T.Add(TEXT("Ach.Dead.Name"), TEXT("L'un d'eux"));
		T.Add(TEXT("Ach.Dead.Desc"), TEXT("Mourir dans les Backrooms."));

		T.Add(TEXT("Bind.MoveForwardPlus"), TEXT("Avancer"));
		T.Add(TEXT("Bind.MoveForwardMinus"), TEXT("Reculer"));
		T.Add(TEXT("Bind.MoveRightPlus"), TEXT("Droite"));
		T.Add(TEXT("Bind.MoveRightMinus"), TEXT("Gauche"));
		T.Add(TEXT("Bind.Jump"), TEXT("Sauter"));
		T.Add(TEXT("Bind.Sprint"), TEXT("Courir"));
		T.Add(TEXT("Bind.Flashlight"), TEXT("Lampe torche"));
		T.Add(TEXT("Bind.View"), TEXT("Vue 1re / 3e personne"));
		T.Add(TEXT("Bind.Attack"), TEXT("Coup de poing"));
		T.Add(TEXT("Bind.Grab"), TEXT("Prendre / poser un objet"));
		T.Add(TEXT("Bind.Push"), TEXT("Pousser un objet"));
		T.Add(TEXT("Bind.Throw"), TEXT("Lancer un objet"));
		T.Add(TEXT("Bind.Inspect"), TEXT("Inspecter un objet"));
		T.Add(TEXT("Bind.Use"), TEXT("Utiliser un objet"));
		T.Add(TEXT("Bind.Slot1"), TEXT("Emplacement 1"));
		T.Add(TEXT("Bind.Slot2"), TEXT("Emplacement 2"));
		T.Add(TEXT("Bind.Slot3"), TEXT("Emplacement 3"));
		T.Add(TEXT("Bind.Slot4"), TEXT("Emplacement 4"));
		T.Add(TEXT("Bind.Inventory"), TEXT("Inventaire"));
		T.Add(TEXT("Bind.PauseMenu"), TEXT("Pause / menu"));
		T.Add(TEXT("Bind.Mouse"), TEXT("Souris"));
		T.Add(TEXT("Bind.Look"), TEXT("Regarder"));
		T.Add(TEXT("Bind.Change"), TEXT("changer"));
		T.Add(TEXT("Bind.PressKey"), TEXT("Appuyez sur une touche pour assigner... (Échap pour annuler)"));
		T.Add(TEXT("Bind.Cancel"), TEXT("ANNULER  ×"));
		T.Add(TEXT("Bind.Reset"), TEXT("Réinitialiser toutes les touches par défaut"));

		T.Add(TEXT("Ach.WellFed.Name"), TEXT("Repus"));
		T.Add(TEXT("Ach.WellFed.Desc"), TEXT("Manger 10 rations de nourriture."));
		T.Add(TEXT("Ach.Electrician.Name"), TEXT("Électricien"));
		T.Add(TEXT("Ach.Electrician.Desc"), TEXT("Insérer 10 piles dans la lampe torche."));
		T.Add(TEXT("Ach.Sprinter.Name"), TEXT("Sprinteur"));
		T.Add(TEXT("Ach.Sprinter.Desc"), TEXT("Commencer à courir 50 fois."));
		T.Add(TEXT("Ach.Cartographer.Name"), TEXT("Cartographe"));
		T.Add(TEXT("Ach.Cartographer.Desc"), TEXT("Traverser 200 pièces."));
		T.Add(TEXT("Ach.Speedrun.Name"), TEXT("Speedrunner"));
		T.Add(TEXT("Ach.Speedrun.Desc"), TEXT("Parcourir 5 kilomètres sous terre."));
		T.Add(TEXT("Ach.Gourmand.Name"), TEXT("Gourmand"));
		T.Add(TEXT("Ach.Gourmand.Desc"), TEXT("Tenir 30 minutes dans les Backrooms."));

		T.Add(TEXT("GameOver.Title"), TEXT("VOUS ÊTES MORT"));
		T.Add(TEXT("GameOver.Subtitle"), TEXT("Les Backrooms ne vous lâchent jamais. Essayez encore."));
		T.Add(TEXT("GameOver.Restart"), TEXT("Recommencer"));
		T.Add(TEXT("GameOver.Quit"), TEXT("Quitter"));

		T.Add(TEXT("Level.W0"), TEXT("L0 · Hall d'entrée"));
		T.Add(TEXT("Level.W1"), TEXT("L1 · Zone habitée"));
		T.Add(TEXT("Level.W2"), TEXT("L2 · Conduites d'eau"));
		T.Add(TEXT("Level.W3"), TEXT("L3 · Centrale électrique"));
		T.Add(TEXT("Level.W4"), TEXT("L4 · Bureaux"));
		T.Add(TEXT("Level.W5"), TEXT("L5 · Hôtel"));
		T.Add(TEXT("Level.W6"), TEXT("L6 · Ténèbres"));
		T.Add(TEXT("Level.W7"), TEXT("L7 · Océan"));
		T.Add(TEXT("Level.W8"), TEXT("L8 · Grottes"));
		T.Add(TEXT("Level.W9"), TEXT("L9 · Hôpital"));

		T.Add(TEXT("Item.AlmondWater.Name"), TEXT("Eau d'amande"));
		T.Add(TEXT("Item.AlmondWater.Desc"), TEXT("Étanche la soif et restaure un peu la santé mentale."));
		T.Add(TEXT("Item.CanFood.Name"), TEXT("Conserve de viande"));
		T.Add(TEXT("Item.CanFood.Desc"), TEXT("Viande en conserve. Calme la faim."));
		T.Add(TEXT("Item.MedKit.Name"), TEXT("Trousse de soins"));
		T.Add(TEXT("Item.MedKit.Desc"), TEXT("Arrête l'hémorragie. Restaure la vie."));
		T.Add(TEXT("Item.Pill.Name"), TEXT("Tranquillisant"));
		T.Add(TEXT("Item.Pill.Desc"), TEXT("Calme les nerfs. Restaure la santé mentale."));
		T.Add(TEXT("Item.Energy.Name"), TEXT("Boisson énergisante"));
		T.Add(TEXT("Item.Energy.Desc"), TEXT("Revigore et éclaircit un peu l'esprit."));
		T.Add(TEXT("Item.Battery.Name"), TEXT("Pile"));
		T.Add(TEXT("Item.Battery.Desc"), TEXT("Une pile pour la lampe torche."));

		T.Add(TEXT("Event.LightFlicker"), TEXT("Lueur vacillante"));
		T.Add(TEXT("Event.LightOutage"), TEXT("La lumière s'est éteinte !"));
		T.Add(TEXT("Event.BoxDisappear"), TEXT("Une caisse a disparu..."));
		T.Add(TEXT("Event.BoxAppear"), TEXT("Une caisse est apparue !"));
		T.Add(TEXT("Event.EntityGrowl"), TEXT("Un grondement dans l'obscurité..."));
		T.Add(TEXT("Event.EntityFootsteps"), TEXT("Des pas derrière le mur..."));
		T.Add(TEXT("Event.Whisper"), TEXT("Des murmures..."));
		T.Add(TEXT("Event.DistantBang"), TEXT("Un fracas au loin !"));
		T.Add(TEXT("Event.WallDrawing"), TEXT("Un dessin sur le mur..."));
		T.Add(TEXT("Event.PipeCreak"), TEXT("Un tuyau grince"));
		T.Add(TEXT("Event.DoorSlam"), TEXT("Une porte claque !"));
		T.Add(TEXT("Event.EmergencyLight"), TEXT("Lumière de secours"));
		T.Add(TEXT("Event.FogIncrease"), TEXT("Le brouillard s'épaissit..."));
		T.Add(TEXT("Event.StaticNoise"), TEXT("Parasites..."));
		T.Add(TEXT("Event.FootprintAppear"), TEXT("Des traces sur le sol..."));
		T.Add(TEXT("Event.Unknown"), TEXT("Événement inconnu"));

		T.Add(TEXT("Loading.Level"), TEXT("Chargement du niveau..."));
		T.Add(TEXT("Loading.World"), TEXT("Chargement des Backrooms... %d%% (fragments : %d)"));
		T.Add(TEXT("Loading.Ready"), TEXT("Monde prêt ! Défaut de réalité..."));
		T.Add(TEXT("Loading.Loaded"), TEXT("Niveau chargé"));
		T.Add(TEXT("Loading.Noclip"), TEXT("Noclip : chute dans les Backrooms..."));
		T.Add(TEXT("Loading.Backrooms"), TEXT("Chargement des Backrooms..."));
		T.Add(TEXT("Loading.Chunks"), TEXT("morceaux"));
		T.Add(TEXT("Loading.WorldReady"), TEXT("Monde prêt ! Glitch de réalité..."));
		T.Add(TEXT("Loading.LevelDone"), TEXT("Niveau chargé"));

		T.Add(TEXT("Keys.SpaceBar"), TEXT("Espace"));
		T.Add(TEXT("Keys.LeftShift"), TEXT("Shift"));
		T.Add(TEXT("Keys.RightShift"), TEXT("Shift (droite)"));
		T.Add(TEXT("Keys.LeftCtrl"), TEXT("Ctrl"));
		T.Add(TEXT("Keys.RightCtrl"), TEXT("Ctrl (droit)"));
		T.Add(TEXT("Keys.LeftAlt"), TEXT("Alt"));
		T.Add(TEXT("Keys.RightAlt"), TEXT("AltGr"));
		T.Add(TEXT("Keys.Escape"), TEXT("Échap"));
		T.Add(TEXT("Keys.LeftMouse"), TEXT("Clic gauche"));
		T.Add(TEXT("Keys.RightMouse"), TEXT("Clic droit"));
		T.Add(TEXT("Keys.MiddleMouse"), TEXT("Molette (clic)"));
		T.Add(TEXT("Keys.ThumbMouse1"), TEXT("Bouton latéral 1"));
		T.Add(TEXT("Keys.ThumbMouse2"), TEXT("Bouton latéral 2"));
		T.Add(TEXT("Keys.MouseX"), TEXT("Souris X"));
		T.Add(TEXT("Keys.MouseY"), TEXT("Souris Y"));
		T.Add(TEXT("Keys.MouseWheel"), TEXT("Molette de la souris"));
		T.Add(TEXT("Keys.Backspace"), TEXT("Retour arrière"));
		T.Add(TEXT("Keys.Tab"), TEXT("Tab"));
		T.Add(TEXT("Keys.Enter"), TEXT("Entrée"));
		T.Add(TEXT("Keys.On"), TEXT("OUI"));
		T.Add(TEXT("Keys.Off"), TEXT("NON"));

		T.Add(TEXT("Set.DisplayMode.Fullscreen"), TEXT("Plein écran"));
		T.Add(TEXT("Set.DisplayMode.Borderless"), TEXT("Sans bordure"));
		T.Add(TEXT("Set.DisplayMode.Windowed"), TEXT("Fenêtré"));
		T.Add(TEXT("Set.RecordingEffect.Off"), TEXT("Désactivé"));
		T.Add(TEXT("Set.RecordingEffect.Light"), TEXT("Pellicule légère"));
		T.Add(TEXT("Set.RecordingEffect.Medium"), TEXT("Pellicule moyenne"));
		T.Add(TEXT("Set.RecordingEffect.Strong"), TEXT("Pellicule forte"));
		T.Add(TEXT("Set.Quality.Overall"), TEXT("Qualité générale"));
		T.Add(TEXT("Set.PotatoMode"), TEXT("Mode patate"));
		T.Add(TEXT("Set.PotatoMode.Hint"), TEXT("FPS maximum au détriment des visuels."));
		T.Add(TEXT("Set.Graphics"), TEXT("Graphismes"));
		T.Add(TEXT("Q.Low"), TEXT("Basse"));
		T.Add(TEXT("Q.Medium"), TEXT("Moyenne"));
		T.Add(TEXT("Q.High"), TEXT("Élevée"));
		T.Add(TEXT("Q.Epic"), TEXT("Épique"));
		T.Add(TEXT("Q.Ultra"), TEXT("Ultra"));

		T.Add(TEXT("Pause.Resume"), TEXT("Reprendre"));
		T.Add(TEXT("Pause.Restart"), TEXT("Recommencer"));
		T.Add(TEXT("Pause.Quit"), TEXT("Quitter le jeu"));

		T.Add(TEXT("Interact.TakeOff"), TEXT("Ranger"));

		T.Add(TEXT("Menu.StressTest"), TEXT("TEST DE STRESS"));
		T.Add(TEXT("HUD.Fps"), TEXT("FPS"));
		T.Add(TEXT("HUD.FpsAvg"), TEXT("Moy"));
		T.Add(TEXT("HUD.FpsOneLow"), TEXT("1% Bas"));
		T.Add(TEXT("Loading.StressTest"), TEXT("Test de stress..."));
	}
}
