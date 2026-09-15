#include "BackroomsLocRegistry.h"

// English.
namespace BackroomsLoc
{
	void Register_EN(FLanguageTable& T)
	{
		T.Add(TEXT("Menu.NewGame"), TEXT("New Game"));
		T.Add(TEXT("Menu.Continue"), TEXT("Continue"));
		T.Add(TEXT("Menu.Levels"), TEXT("Levels"));
		T.Add(TEXT("Menu.Achievements"), TEXT("Achievements"));
		T.Add(TEXT("Menu.Settings"), TEXT("Settings"));
		T.Add(TEXT("Menu.Controls"), TEXT("Controls"));
		T.Add(TEXT("Menu.Quit"), TEXT("Quit"));
		T.Add(TEXT("Menu.Subtitle"), TEXT("LIMINAL DESCENT"));
		T.Add(TEXT("Menu.Back"), TEXT("<   B A C K"));

		T.Add(TEXT("Tab.Screen"), TEXT("SCREEN"));
		T.Add(TEXT("Tab.Quality"), TEXT("QUALITY"));
		T.Add(TEXT("Tab.Camera"), TEXT("CAMERA"));
		T.Add(TEXT("Tab.Sound"), TEXT("SOUND"));
		T.Add(TEXT("Tab.Game"), TEXT("GAME"));
		T.Add(TEXT("Tab.Controls"), TEXT("CONTROLS"));
		T.Add(TEXT("Settings.Title"), TEXT("S E T T I N G S"));
		T.Add(TEXT("Settings.Categories"), TEXT("C A T E G O R I E S"));

		T.Add(TEXT("Set.Resolution"), TEXT("Resolution"));
		T.Add(TEXT("Set.DisplayMode"), TEXT("Display Mode"));
		T.Add(TEXT("Set.VSync"), TEXT("VSync"));
		T.Add(TEXT("Set.RenderScale"), TEXT("Render Scale"));
		T.Add(TEXT("Set.Gamma"), TEXT("Gamma"));
		T.Add(TEXT("Set.Language"), TEXT("Language"));

		T.Add(TEXT("Set.Shadows"), TEXT("Shadows"));
		T.Add(TEXT("Set.Textures"), TEXT("Textures"));
		T.Add(TEXT("Set.AntiAliasing"), TEXT("Anti-Aliasing"));
		T.Add(TEXT("Set.PostProcess"), TEXT("Post-Processing"));
		T.Add(TEXT("Set.Effects"), TEXT("Effects"));
		T.Add(TEXT("Set.ViewDistance"), TEXT("View Distance"));
		T.Add(TEXT("Set.DepthOfField"), TEXT("Depth of Field"));
		T.Add(TEXT("Set.Bloom"), TEXT("Bloom"));
		T.Add(TEXT("Set.Lumen"), TEXT("Lumen (lighting)"));
		T.Add(TEXT("Set.Upscale"), TEXT("DLSS / Upscale"));

		T.Add(TEXT("Set.FOV"), TEXT("Field of View"));
		T.Add(TEXT("Set.Sensitivity"), TEXT("Sensitivity"));
		T.Add(TEXT("Set.RecordingEffect"), TEXT("Recording Camera Effect"));

		T.Add(TEXT("Set.Volume.Master"), TEXT("Master"));
		T.Add(TEXT("Set.Volume.Menu"), TEXT("Music / Menu"));
		T.Add(TEXT("Set.Volume.Game"), TEXT("In-Game Sounds"));
		T.Add(TEXT("Set.Volume.Monster"), TEXT("Monster Sounds"));
		T.Add(TEXT("Set.Volume.Other"), TEXT("Other Sounds"));

		T.Add(TEXT("Set.Difficulty"), TEXT("Difficulty"));
		T.Add(TEXT("Set.Difficulty.Hint"),
			TEXT("Difficulty affects generation: wall density, doorway count, supply generosity and drops."));

		T.Add(TEXT("Levels.Title"), TEXT("L E V E L S"));
		T.Add(TEXT("Levels.Locked"), TEXT("LOCKED"));
		T.Add(TEXT("Levels.UnlockedFmt"),
			TEXT("Levels unlocked: %d of 10. New ones open as you descend — go deeper to see their previews."));

		T.Add(TEXT("Ach.Title"), TEXT("A C H I E V E M E N T S"));
		T.Add(TEXT("Ach.Unlocked"), TEXT("ACHIEVEMENT"));
		T.Add(TEXT("Ach.ProgressFmt"), TEXT("Unlocked: %d / %d     Points: %d"));

		T.Add(TEXT("Diff.Peaceful.Name"), TEXT("Peaceful"));
		T.Add(TEXT("Diff.Peaceful.Desc"), TEXT("No monsters, no pressure. Exploration and the world itself."));
		T.Add(TEXT("Diff.Easy.Name"), TEXT("Easy"));
		T.Add(TEXT("Diff.Easy.Desc"), TEXT("Open, generous locations. One monster appears late."));
		T.Add(TEXT("Diff.Normal.Name"), TEXT("Normal"));
		T.Add(TEXT("Diff.Normal.Desc"), TEXT("The intended balance: tight corridors, hunger, a lone entity."));
		T.Add(TEXT("Diff.Hard.Name"), TEXT("Hard"));
		T.Add(TEXT("Diff.Hard.Desc"), TEXT("More walls and fewer doors, scarce loot, two entities."));
		T.Add(TEXT("Diff.Nightmare.Name"), TEXT("Nightmare"));
		T.Add(TEXT("Diff.Nightmare.Desc"), TEXT("A trap maze, minimal supplies, three entities, death is final."));

		T.Add(TEXT("HUD.Sanity"), TEXT("SANITY"));
		T.Add(TEXT("HUD.Flashlight"), TEXT("FLASHLIGHT"));
		T.Add(TEXT("HUD.Hunger"), TEXT("HUNGER"));
		T.Add(TEXT("HUD.Thirst"), TEXT("THIRST"));
		T.Add(TEXT("HUD.Health"), TEXT("HEALTH"));
		T.Add(TEXT("HUD.Stamina"), TEXT("STAMINA"));
		T.Add(TEXT("HUD.Inventory"), TEXT("INVENTORY"));
		T.Add(TEXT("HUD.Empty"), TEXT("empty"));
		T.Add(TEXT("Prog.Level"), TEXT("Level"));
		T.Add(TEXT("Prog.LevelUp"), TEXT("Level up"));

		T.Add(TEXT("Interact.Inspect"), TEXT("Inspect"));
		T.Add(TEXT("Interact.Throw"), TEXT("Throw"));
		T.Add(TEXT("Interact.Drop"), TEXT("Drop"));
		T.Add(TEXT("Interact.Take"), TEXT("Take"));
		T.Add(TEXT("Interact.Push"), TEXT("Push"));
		T.Add(TEXT("Interact.Use"), TEXT("Use"));
		T.Add(TEXT("Interact.Pickup"), TEXT("Pick up"));

		T.Add(TEXT("Hint.Critical"), TEXT("Critical condition — find medicine now"));
		T.Add(TEXT("Hint.Sanity"), TEXT("Sanity is low — hide and catch your breath"));
		T.Add(TEXT("Hint.Hunger"), TEXT("Starving — find food"));
		T.Add(TEXT("Hint.Thirst"), TEXT("Very thirsty — drink some water"));
		T.Add(TEXT("Hint.Monster"), TEXT("It is close — stay quiet"));
		T.Add(TEXT("Hint.Poison"), TEXT("You are poisoned — find almond water or a medkit"));
		T.Add(TEXT("Hint.Radiation"), TEXT("Radiation is high — get out of here"));
		T.Add(TEXT("Hint.Sleep"), TEXT("You are exhausted — stop and rest"));
		T.Add(TEXT("Hint.Battery"), TEXT("Flashlight is almost dead"));

		T.Add(TEXT("Ach.FirstSteps.Name"), TEXT("First Steps"));
		T.Add(TEXT("Ach.FirstSteps.Desc"), TEXT("Descend into the Backrooms."));
		T.Add(TEXT("Ach.Tourist.Name"), TEXT("Tourist"));
		T.Add(TEXT("Ach.Tourist.Desc"), TEXT("Visit 3 different locations."));
		T.Add(TEXT("Ach.Explorer.Name"), TEXT("Explorer"));
		T.Add(TEXT("Ach.Explorer.Desc"), TEXT("Visit 5 different locations."));
		T.Add(TEXT("Ach.DeepDiver.Name"), TEXT("Deep Diver"));
		T.Add(TEXT("Ach.DeepDiver.Desc"), TEXT("Visit 7 different locations."));
		T.Add(TEXT("Ach.AllLevels.Name"), TEXT("Whole Floor"));
		T.Add(TEXT("Ach.AllLevels.Desc"), TEXT("Visit all 10 locations."));
		T.Add(TEXT("Ach.Survivor.Name"), TEXT("Survivor"));
		T.Add(TEXT("Ach.Survivor.Desc"), TEXT("Last 15 minutes in the Backrooms."));
		T.Add(TEXT("Ach.Marathon.Name"), TEXT("Marathoner"));
		T.Add(TEXT("Ach.Marathon.Desc"), TEXT("Walk 3 kilometres underground."));
		T.Add(TEXT("Ach.Scavenger.Name"), TEXT("Scavenger"));
		T.Add(TEXT("Ach.Scavenger.Desc"), TEXT("Pick up 10 items."));
		T.Add(TEXT("Ach.Pharmacist.Name"), TEXT("Pharmacist"));
		T.Add(TEXT("Ach.Pharmacist.Desc"), TEXT("Use 5 medicines."));
		T.Add(TEXT("Ach.Hydrated.Name"), TEXT("Almond Taste"));
		T.Add(TEXT("Ach.Hydrated.Desc"), TEXT("Drink 5 servings of almond water."));
		T.Add(TEXT("Ach.MonsterAware.Name"), TEXT("It Is Near"));
		T.Add(TEXT("Ach.MonsterAware.Desc"), TEXT("Meet an entity and survive."));
		T.Add(TEXT("Ach.EscapeArtist.Name"), TEXT("First Exit"));
		T.Add(TEXT("Ach.EscapeArtist.Desc"), TEXT("Find an exit from a location."));
		T.Add(TEXT("Ach.FrequentFlyer.Name"), TEXT("Frequent Guest"));
		T.Add(TEXT("Ach.FrequentFlyer.Desc"), TEXT("Pass through 5 exits."));
		T.Add(TEXT("Ach.DeepFear.Name"), TEXT("Edge of Fear"));
		T.Add(TEXT("Ach.DeepFear.Desc"), TEXT("Drive environmental pressure to its maximum."));
		T.Add(TEXT("Ach.Dead.Name"), TEXT("One of Them"));
		T.Add(TEXT("Ach.Dead.Desc"), TEXT("Die in the Backrooms."));
		T.Add(TEXT("Bind.MoveForwardPlus"), TEXT("Forward"));
		T.Add(TEXT("Bind.MoveForwardMinus"), TEXT("Back"));
		T.Add(TEXT("Bind.MoveRightPlus"), TEXT("Right"));
		T.Add(TEXT("Bind.MoveRightMinus"), TEXT("Left"));
		T.Add(TEXT("Bind.Jump"), TEXT("Jump"));
		T.Add(TEXT("Bind.Sprint"), TEXT("Sprint"));
		T.Add(TEXT("Bind.Flashlight"), TEXT("Flashlight"));
		T.Add(TEXT("Bind.View"), TEXT("1st / 3rd person"));
		T.Add(TEXT("Bind.Attack"), TEXT("Punch"));
		T.Add(TEXT("Bind.Grab"), TEXT("Take / drop item"));
		T.Add(TEXT("Bind.Push"), TEXT("Push item"));
		T.Add(TEXT("Bind.Throw"), TEXT("Throw item"));
		T.Add(TEXT("Bind.Inspect"), TEXT("Inspect item"));
		T.Add(TEXT("Bind.Use"), TEXT("Use item"));
		T.Add(TEXT("Bind.Slot1"), TEXT("Slot 1"));
		T.Add(TEXT("Bind.Slot2"), TEXT("Slot 2"));
		T.Add(TEXT("Bind.Slot3"), TEXT("Slot 3"));
		T.Add(TEXT("Bind.Slot4"), TEXT("Slot 4"));
		T.Add(TEXT("Bind.Inventory"), TEXT("Inventory"));
		T.Add(TEXT("Bind.PauseMenu"), TEXT("Pause / menu"));
		T.Add(TEXT("Bind.Mouse"), TEXT("Mouse"));
		T.Add(TEXT("Bind.Look"), TEXT("Look"));
		T.Add(TEXT("Bind.Change"), TEXT("change"));
		T.Add(TEXT("Bind.PressKey"), TEXT("Press a key to assign... (Esc to cancel)"));
		T.Add(TEXT("Bind.Cancel"), TEXT("CANCEL  ×"));
		T.Add(TEXT("Bind.Reset"), TEXT("Reset all keys to defaults"));
		T.Add(TEXT("Ach.WellFed.Name"), TEXT("Well Fed"));
		T.Add(TEXT("Ach.WellFed.Desc"), TEXT("Eat 10 servings of food."));
		T.Add(TEXT("Ach.Electrician.Name"), TEXT("Electrician"));
		T.Add(TEXT("Ach.Electrician.Desc"), TEXT("Insert 10 batteries into the flashlight."));
		T.Add(TEXT("Ach.Sprinter.Name"), TEXT("Sprinter"));
		T.Add(TEXT("Ach.Sprinter.Desc"), TEXT("Start sprinting 50 times."));
		T.Add(TEXT("Ach.Cartographer.Name"), TEXT("Cartographer"));
		T.Add(TEXT("Ach.Cartographer.Desc"), TEXT("Pass through 200 rooms."));
		T.Add(TEXT("Ach.Speedrun.Name"), TEXT("Speedrunner"));
		T.Add(TEXT("Ach.Speedrun.Desc"), TEXT("Cover 5 kilometres underground."));
		T.Add(TEXT("Ach.Gourmand.Name"), TEXT("Gourmand"));
T.Add(TEXT("Ach.Gourmand.Desc"), TEXT("Last 30 minutes in the Backrooms."));

		// --- Death screen ---
		T.Add(TEXT("GameOver.Title"), TEXT("YOU DIED"));
		T.Add(TEXT("GameOver.Subtitle"), TEXT("The Backrooms never let go. Try again."));
		T.Add(TEXT("GameOver.Restart"), TEXT("Restart"));
		T.Add(TEXT("GameOver.Quit"), TEXT("Quit"));

		// --- Levels (level book) ---
		T.Add(TEXT("Level.W0"), TEXT("L0 · Lobby"));
		T.Add(TEXT("Level.W1"), TEXT("L1 · Inhabited Zone"));
		T.Add(TEXT("Level.W2"), TEXT("L2 · Waterworks"));
		T.Add(TEXT("Level.W3"), TEXT("L3 · Power Station"));
		T.Add(TEXT("Level.W4"), TEXT("L4 · Offices"));
		T.Add(TEXT("Level.W5"), TEXT("L5 · Hotel"));
		T.Add(TEXT("Level.W6"), TEXT("L6 · Darkness"));
		T.Add(TEXT("Level.W7"), TEXT("L7 · Ocean"));
		T.Add(TEXT("Level.W8"), TEXT("L8 · Caves"));
		T.Add(TEXT("Level.W9"), TEXT("L9 · Hospital"));

		// --- Items ---
		T.Add(TEXT("Item.AlmondWater.Name"), TEXT("Almond Water"));
		T.Add(TEXT("Item.AlmondWater.Desc"), TEXT("Quenches thirst and slightly restores sanity."));
		T.Add(TEXT("Item.CanFood.Name"), TEXT("Canned Food"));
		T.Add(TEXT("Item.CanFood.Desc"), TEXT("Canned meat. Satisfies hunger."));
		T.Add(TEXT("Item.MedKit.Name"), TEXT("Medkit"));
		T.Add(TEXT("Item.MedKit.Desc"), TEXT("Stops bleeding. Restores health."));
		T.Add(TEXT("Item.Pill.Name"), TEXT("Sedative"));
		T.Add(TEXT("Item.Pill.Desc"), TEXT("Calms the nerves. Restores sanity."));
		T.Add(TEXT("Item.Energy.Name"), TEXT("Energy Drink"));
		T.Add(TEXT("Item.Energy.Desc"), TEXT("Invigorates and slightly clears the mind."));
		T.Add(TEXT("Item.Battery.Name"), TEXT("Battery"));
		T.Add(TEXT("Item.Battery.Desc"), TEXT("A power cell for the flashlight."));

		// --- Backrooms events ---
		T.Add(TEXT("Event.LightFlicker"), TEXT("Flickering light"));
		T.Add(TEXT("Event.LightOutage"), TEXT("The lights went out!"));
		T.Add(TEXT("Event.BoxDisappear"), TEXT("A box vanished..."));
		T.Add(TEXT("Event.BoxAppear"), TEXT("A box appeared!"));
		T.Add(TEXT("Event.EntityGrowl"), TEXT("A growl from the darkness..."));
		T.Add(TEXT("Event.EntityFootsteps"), TEXT("Footsteps behind the wall..."));
		T.Add(TEXT("Event.Whisper"), TEXT("Whispers..."));
		T.Add(TEXT("Event.DistantBang"), TEXT("A bang from afar!"));
		T.Add(TEXT("Event.WallDrawing"), TEXT("A drawing on the wall..."));
		T.Add(TEXT("Event.PipeCreak"), TEXT("A pipe creaks"));
		T.Add(TEXT("Event.DoorSlam"), TEXT("A door slams!"));
		T.Add(TEXT("Event.EmergencyLight"), TEXT("Emergency light"));
		T.Add(TEXT("Event.FogIncrease"), TEXT("The fog thickens..."));
		T.Add(TEXT("Event.StaticNoise"), TEXT("Static..."));
		T.Add(TEXT("Event.FootprintAppear"), TEXT("Footprints on the floor..."));
		T.Add(TEXT("Event.Unknown"), TEXT("Unknown event"));

		// --- Loading ---
		T.Add(TEXT("Loading.Level"), TEXT("Loading level..."));
		T.Add(TEXT("Loading.World"), TEXT("Loading the Backrooms... %d%% (chunks: %d)"));
		T.Add(TEXT("Loading.Ready"), TEXT("World ready! Reality glitch..."));
		T.Add(TEXT("Loading.Loaded"), TEXT("Level loaded"));
		T.Add(TEXT("Loading.Noclip"), TEXT("Noclip: falling into the Backrooms..."));
		T.Add(TEXT("Loading.Backrooms"), TEXT("Loading the Backrooms..."));
		T.Add(TEXT("Loading.Chunks"), TEXT("chunks"));
		T.Add(TEXT("Loading.WorldReady"), TEXT("World ready! Reality glitch..."));
		T.Add(TEXT("Loading.LevelDone"), TEXT("Level loaded"));

		// --- Key names ---
		T.Add(TEXT("Keys.SpaceBar"), TEXT("Space"));
		T.Add(TEXT("Keys.LeftShift"), TEXT("Shift"));
		T.Add(TEXT("Keys.RightShift"), TEXT("Shift (right)"));
		T.Add(TEXT("Keys.LeftCtrl"), TEXT("Ctrl"));
		T.Add(TEXT("Keys.RightCtrl"), TEXT("Ctrl (right)"));
		T.Add(TEXT("Keys.LeftAlt"), TEXT("Alt"));
		T.Add(TEXT("Keys.RightAlt"), TEXT("AltGr"));
		T.Add(TEXT("Keys.Escape"), TEXT("Esc"));
		T.Add(TEXT("Keys.LeftMouse"), TEXT("LMB"));
		T.Add(TEXT("Keys.RightMouse"), TEXT("RMB"));
		T.Add(TEXT("Keys.MiddleMouse"), TEXT("Mouse wheel (click)"));
		T.Add(TEXT("Keys.ThumbMouse1"), TEXT("Side button 1"));
		T.Add(TEXT("Keys.ThumbMouse2"), TEXT("Side button 2"));
		T.Add(TEXT("Keys.MouseX"), TEXT("Mouse X"));
		T.Add(TEXT("Keys.MouseY"), TEXT("Mouse Y"));
		T.Add(TEXT("Keys.MouseWheel"), TEXT("Mouse wheel"));
		T.Add(TEXT("Keys.Backspace"), TEXT("Backspace"));
		T.Add(TEXT("Keys.Tab"), TEXT("Tab"));
		T.Add(TEXT("Keys.Enter"), TEXT("Enter"));
		T.Add(TEXT("Keys.On"), TEXT("ON"));
		T.Add(TEXT("Keys.Off"), TEXT("OFF"));

		// --- Settings values ---
		T.Add(TEXT("Set.DisplayMode.Fullscreen"), TEXT("Fullscreen"));
		T.Add(TEXT("Set.DisplayMode.Borderless"), TEXT("Borderless"));
		T.Add(TEXT("Set.DisplayMode.Windowed"), TEXT("Windowed"));
		T.Add(TEXT("Set.RecordingEffect.Off"), TEXT("Off"));
		T.Add(TEXT("Set.RecordingEffect.Light"), TEXT("Light film grain"));
		T.Add(TEXT("Set.RecordingEffect.Medium"), TEXT("Medium film grain"));
		T.Add(TEXT("Set.RecordingEffect.Strong"), TEXT("Strong film grain"));
		T.Add(TEXT("Set.Quality.Overall"), TEXT("Overall Quality"));
		T.Add(TEXT("Set.PotatoMode"), TEXT("Potato Mode"));
		T.Add(TEXT("Set.PotatoMode.Hint"), TEXT("Maximum FPS at the cost of visuals."));
		T.Add(TEXT("Set.Graphics"), TEXT("Graphics"));
		T.Add(TEXT("Q.Low"), TEXT("Low"));
		T.Add(TEXT("Q.Medium"), TEXT("Medium"));
		T.Add(TEXT("Q.High"), TEXT("High"));
		T.Add(TEXT("Q.Epic"), TEXT("Epic"));
		T.Add(TEXT("Q.Ultra"), TEXT("Ultra"));

		// --- Pause ---
		T.Add(TEXT("Pause.Resume"), TEXT("Resume"));
		T.Add(TEXT("Pause.Restart"), TEXT("Restart"));
		T.Add(TEXT("Pause.Quit"), TEXT("Quit to desktop"));

		// --- Player actions ---
		T.Add(TEXT("Interact.TakeOff"), TEXT("Put away"));

		T.Add(TEXT("Menu.StressTest"), TEXT("STRESS TEST"));
		T.Add(TEXT("HUD.Fps"), TEXT("FPS"));
		T.Add(TEXT("HUD.FpsAvg"), TEXT("Avg"));
		T.Add(TEXT("HUD.FpsOneLow"), TEXT("1% Low"));
		T.Add(TEXT("Loading.StressTest"), TEXT("Stress Test — cooking the Backrooms..."));
	}
}
