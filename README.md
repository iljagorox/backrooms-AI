# Backrooms-AI — UE 5.7 Module Structure

Проект переразложен в стандартную структуру модуля Unreal Engine 5.7.

## Структура

```
Source/
  Backrooms/
    Backrooms.Build.cs          // Правила сборки модуля
    Public/                     // 74 заголовочных файла (.h) — 2-я папка
      Backrooms.h               // Модуль
      BackroomsAuthoredChunk.h
      BackroomsChunkActor.h
      BackroomsWorldGenerator.h
      ... (все .h)
    Private/                    // 68 cpp файлов — 1-я папка
      Backrooms.cpp
      BackroomsChunkActor.cpp
      BackroomsWorldGenerator.cpp
      ... (все .cpp)
```

### Принцип разделения (как в UE 5.7)

- **Public/** — все `.h` файлы. Автоматически добавляются в include path модуля, доступны другим модулям и Blueprint.
  - Core: `Backrooms.h`, `BackroomsGameMode.h`, `BackroomsPlayerCharacter.h`, `BackroomsHandles.h`, `BackroomsTopology.h`
  - World Gen: `BackroomsWorldGenerator.h`, `BackroomsFloorPlan.h`, `BackroomsChunkActor.h`, `BackroomsCityChunkActor.h`, `BackroomsRoomGraph.h`, `BackroomsSpatialGraph.h`, `BackroomsGenerationData.h`, `RoomData.h`, `SocketData.h` и т.д.
  - AI: `BackroomsMonsterAIController.h`, `BackroomsRiggedMonster.h`, `BackroomsSenseComponent.h`
  - UI: `BackroomsHUDWidget.h`, `BackroomsMainMenuWidget.h`, `BackroomsPauseMenuWidget.h`, `BackroomsInventoryWidget.h`, `LoadingBarWidget.h`
  - Items/Inventory: `BackroomsItemPickup.h`, `BackroomsInventoryComponent.h`, `BackroomsEquipmentComponent.h`, `BackroomsItemSystem.h`
  - Audio/Settings: `BackroomsAudioSettings.h`, `BackroomsQualitySettings.h`, `BackroomsInputSettings.h`, `BackroomsLevelAudioActor.h`
  - Localization: `BackroomsLocalization.h`, `BackroomsLocRegistry.h`, `BackroomsLoc_*.h` (в Public для доступа)

- **Private/** — все `.cpp` файлы. Реализация, не видна вне модуля.
  - Логика генерации, AI, UI, локализации и т.д.

### Как использовать в UE 5.7

1. Скопируй папку `Source/Backrooms` в свой UE 5.7 проект в `Source/`
2. Добавь `"Backrooms"` в `PublicDependencyModuleNames` своего основного модуля если нужно
3. Перегенерируй `.uproject` файлы (ПКМ по .uproject → Generate Visual Studio project files)
4. Скомпилируй

Файл `Backrooms.Build.cs` уже настроен с зависимостями:
`Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `AIModule`, `NavigationSystem`, `UMG`, `Slate`, `GameplayTags`, `Niagara` и др.

### История изменений
Все файлы перемещены через `git mv`, история сохранена (R 100% в git status).
