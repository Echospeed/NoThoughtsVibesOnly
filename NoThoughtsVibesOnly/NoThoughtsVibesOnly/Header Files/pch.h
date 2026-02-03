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
#include "StateManager.h"
#include "Util.h"        
#include "Input.h"       
#include "Transform.h"   
#include "Colour.h"      

// ---------------------------------------------------------------------------
// Graphics & UI Headers
// ---------------------------------------------------------------------------
#include "SpriteRenderer.h"
#include "TextRenderer.h"  
#include "Button.h"        

// ---------------------------------------------------------------------------
// Game Object System Headers
// ---------------------------------------------------------------------------
#include "GameObjectType.h"
#include "GameObject.h"    
#include "Collider.h"
// ---------------------------------------------------------------------------
// Game State Headers
// ---------------------------------------------------------------------------
#include "FinishPage.h"
#include "MenuPage.h"
#include "SplashPage.h"
#include "GamePage.h"
#include "WinPage.h"


#endif //PCH_H