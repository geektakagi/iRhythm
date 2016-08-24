#include "gameGraphics.hpp"

gameGraphics::gameGraphics() {
	if (!texJudgeMiss || !texJudgeGood) {
		// テクスチャのロード失敗
	}
}


gameGraphics::~gameGraphics() {

}

//  (message, (Color)Palette::Color = Blue, (Vec2)Position = {0,0} )
void gameGraphics::drawString(const wchar str, const Color color, Vec2 position) {
	++printCount;
	this->font(str).draw({ position.x, (this->printMargin * (this->printCount - 1)) }, color);
}

void gameGraphics::drawString(const std::string str, const Color color, Vec2 position) {
	++printCount;

	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cv;
	std::wstring wsmessage = cv.from_bytes(str);

	this->font(wsmessage).draw({ position.x,(this->printMargin * (this->printCount - 1)) }, color);
}

void gameGraphics::drawString(const int num, const Color color, Vec2 position) {
	++printCount;
	this->font(num).draw({ position.x,(this->printMargin * (this->printCount - 1)) }, color);
}

void gameGraphics::drawString(const double num, const Color color, Vec2 position) {
	++printCount;
	this->font(num).draw({ position.x, (this->printMargin * (this->printCount - 1)) }, color);
}

void gameGraphics::endFrameDraw(void) {
	for (auto missItr = JudgeMissTex.begin(); missItr != JudgeMissTex.end(); ++missItr) {
		missItr->z -= 4;
		texJudgeMiss.draw(missItr->xy(), Alpha((int32)missItr->z));

		if (missItr->z <= 0) {
			JudgeMissTex.erase(missItr);
			if (JudgeMissTex.size() <= 0) {
				break;
			}
			missItr = JudgeMissTex.begin();
		}
	}

	for (auto goodItr = JudgeGoodTex.begin(); goodItr != JudgeGoodTex.end(); ++goodItr) {
		goodItr->z -= 4;
		texJudgeGood.draw(goodItr->xy(), Alpha((int32)goodItr->z));

		if (goodItr->z <= 0) {
			JudgeGoodTex.erase(goodItr);
			if (JudgeGoodTex.size() <= 0) {
				break;
			}
			goodItr = JudgeGoodTex.begin();
		}		
	}

	this->printCount = 0;
}

void gameGraphics::DrawJudge(gameGraphics::Judge judge, Vec2 position) {
	switch (judge) {
	case gameGraphics::GOOD:
		// texJudgeGood.draw(position);
		JudgeGoodTex.push_back({ position.x ,position.y ,256 });
		break;

	case gameGraphics::MISS:
		// texJudgeMiss.draw(position);
		JudgeMissTex.push_back({ position.x ,position.y ,256 });
		break;
		
	default:
		break;
	}
}