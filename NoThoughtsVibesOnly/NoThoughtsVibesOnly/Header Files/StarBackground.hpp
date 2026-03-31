#pragma once
// Author: John Chiow
// ============================================================================
// StarBackground.hpp - Animated star field for menu/UI screens
// ============================================================================
// CORRECT CALL ORDER every frame:
//
//   _Load():
//     Meshes::CreateCircleMesh();
//     Meshes::CreateSquareCenterOriginMesh();
//     StarBackground::Init();
//
//   _Update():
//     StarBackground::Update(dt);
//     // button updates happen here via mainPageObj loop
//
//   _Draw():
//     StarBackground::DrawBackground();  // dark blue fill - MUST BE FIRST
//     StarBackground::Draw();            // stars on top of background
//     myText.Draw();                     // text on top of stars
//     // buttons already drew during Update() - they appear on top naturally
// ============================================================================
namespace StarBackground
{
    void Init();
    void Update(float dt);
    void DrawBackground(); // Draw ONLY the dark blue background quad
    void Draw();           // Draw ONLY the stars (no background quad)
}