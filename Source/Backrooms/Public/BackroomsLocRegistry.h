#pragma once

#include "CoreMinimal.h"
#include "BackroomsLocalization.h"

// Внутренний реестр локализации. Каждый язык живёт в своём файле
// BackroomsLoc_<Lang>.cpp и реализует одну функцию регистрации строк.
// Ядро (BackroomsLocalization.cpp) вызывает их все один раз, лениво.
//
// Добавить строку в существующий язык: добавить T.Add(...) в его файле.
// Добавить язык: создать BackroomsLoc_<Lang>.cpp, объявить функцию здесь и
// вызвать её в BackroomsLocalization.cpp — UI/меню трогать не нужно.

namespace BackroomsLoc
{
	BACKROOMS_API void Register_RU(FLanguageTable& T);
	BACKROOMS_API void Register_EN(FLanguageTable& T);
	BACKROOMS_API void Register_ES(FLanguageTable& T);
	BACKROOMS_API void Register_FR(FLanguageTable& T);
	BACKROOMS_API void Register_DE(FLanguageTable& T);
	BACKROOMS_API void Register_IT(FLanguageTable& T);
	BACKROOMS_API void Register_PT(FLanguageTable& T);
	BACKROOMS_API void Register_PL(FLanguageTable& T);
	BACKROOMS_API void Register_TR(FLanguageTable& T);
	BACKROOMS_API void Register_ZH(FLanguageTable& T);
	BACKROOMS_API void Register_JA(FLanguageTable& T);
	BACKROOMS_API void Register_KO(FLanguageTable& T);
	BACKROOMS_API void Register_AR(FLanguageTable& T);
}
