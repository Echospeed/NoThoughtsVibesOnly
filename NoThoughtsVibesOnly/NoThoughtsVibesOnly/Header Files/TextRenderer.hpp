#pragma once
// Author: John Chiow
#include "AEEngine.h"
#include "GameObject.hpp"
#include <string>
#include <sstream>

enum TextAlignment
{
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT
};

class TextRenderer
{
private:
	s8 font{};
	f32 scale{};
	f32 maxPixelWidth{ 0.0f }; // Optional max width for auto-scaling  
	AEVec2 position{};
	Colour colour{};
	std::string text{}; // Shift to private once done
	TextAlignment alignment{ ALIGN_CENTER };

public:
	TextRenderer();

	TextRenderer(s8 font, f32 scale, AEVec2 position, Colour colour, std::string = "", TextAlignment alignment = ALIGN_CENTER);

	template <typename T>
	TextRenderer& operator<<(const T& value)
	{
		std::ostringstream stream;	

		stream << value;

		// Update the struct's text variable directly
		this->text += stream.str();

		return *this;
	}

	AEVec2 SetPosition(AEVec2 newPosition);

	AEVec2 GetPosition() const;

	f32 GetScale() const;

	void SetScale(f32 newScale);

	void SetColour(Colour newColour);

	void SetAlignment(TextAlignment newAlignment);

	void SetMaxPixelWidth(f32 maxWidth);

	~TextRenderer();

	void Draw();
};


/*==============================================================================*/
// USAGE:
// ----------------------------------------------------------------------------
//   TextRenderer Operator Overloading Example:

//   TextRenderer tr;
//   tr << "Score: " << playerScore; // Concatenates "Score: " and playerScore into tr.text
//   If playerscore uses the special types such as u8, s8, f32, etc., 
//	 the overloaded operator<< will not be able to handle the print. 
//   Use static_change to convert to a compatible type (e.g., int or float) before streaming:

/*==============================================================================*/