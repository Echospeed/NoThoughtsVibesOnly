#pragma once

#ifndef PCH_H
#define PCH_H

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
#include "Audio.hpp"

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
// Game State Headers
// ---------------------------------------------------------------------------
#include "FinishPage.hpp"
#include "MenuPage.hpp"
#include "SplashPage.hpp"
#include "GamePage.hpp"
#include "WinPage.hpp"
#include "LevelSelectPage.hpp"


#endif //PCH_H