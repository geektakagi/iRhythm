#pragma once
#include <iostream>
#include <Siv3D.hpp>
#include <iostream>
#include <locale>
#include <codecvt>

class gameGraphics {
public:
	gameGraphics();
	~gameGraphics();

	enum Judge : long {
		GOOD = 300,
		MISS = 0
	};

	// draw a text (message, (Color)Palette::Color, (Vec2)Position )
	void drawString(const wchar, Color color = Palette::Red, Vec2 position = { 0,0 });
	void drawString(const std::string, Color color = Palette::Red, Vec2 position = {0,0});
	void drawString(const int, Color color = Palette::Red, Vec2 position = { 0,0 });
	void drawString(const double, Color color = Palette::Red, Vec2 position = { 0,0 });

	void DrawJudge(const Judge, const Vec2);

	// End frame draw
	void endFrameDraw(void);

private:
	const unsigned short int fontSize = 35;
	const unsigned short int printMargin = 30;
	const Font font{ this->fontSize };

	const Texture texJudgeGood{ L"Asset/GOOD.png" };
	const Texture texJudgeMiss{ L"Asset/MISS.png" };

	std::vector<Vec3> JudgeGoodTex;
	std::vector<Vec3> JudgeMissTex;
	int printCount = 0;

};
