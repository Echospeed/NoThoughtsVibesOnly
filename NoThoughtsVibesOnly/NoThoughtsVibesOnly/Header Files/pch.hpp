#pragma once

#ifndef PCH_H
#define PCH_H
//#ifndef _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
//#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
//#endif
// ---------------------------------------------------------------------------
// Windows Macro Guards - MUST come before any Windows or engine headers
// ---------------------------------------------------------------------------
// NOMINMAX    : Prevents Windows.h from defining min/max as macros,
//               which breaks RapidJSON's use of std::min / std::max.
// WIN32_LEAN_AND_MEAN : Strips rarely-used Windows headers for faster builds.
// _CRT_SECURE_NO_WARNINGS : Suppresses fopen/strcpy deprecation warnings.
// ---------------------------------------------------------------------------
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// ---------------------------------------------------------------------------
// Standard Library Headers
// ---------------------------------------------------------------------------
#include <vector>
#include <string>
#include <iostream>
#include <math.h>
#include <crtdbg.h>  // For memory leak detection

// ---------------------------------------------------------------------------
// Engine/External Headers
// ---------------------------------------------------------------------------
#include "AEEngine.h"

// ---------------------------------------------------------------------------
// Project Framework Headers
// ---------------------------------------------------------------------------
#include "StateManager.hpp"
#include "Util.hpp"        
#include "Input.hpp"       
#include "Transform.hpp"   
#include "Colour.hpp"      
//#include "Audio.hpp"
#include "AudioManager.hpp"

// ---------------------------------------------------------------------------
// Graphics & UI Headers
// ---------------------------------------------------------------------------
#include "SpriteRenderer.hpp"
#include "TextRenderer.hpp"  
#include "Button.hpp"        

// ---------------------------------------------------------------------------
// Game Object System Headers
// ---------------------------------------------------------------------------
#include "GameObjectType.hpp"
#include "GameObject.hpp"    
#include "Collider.hpp"

// ---------------------------------------------------------------------------
// Config System
// ---------------------------------------------------------------------------
#include "GameConfig.hpp"

// ---------------------------------------------------------------------------
// Game State Headers
// ---------------------------------------------------------------------------
#include "FinishPage.hpp"
#include "MenuPage.hpp"
#include "SplashPage.hpp"
#include "GamePage.hpp"
#include "WinPage.hpp"
#include "LevelSelectPage.hpp"

#endif //PCH_H