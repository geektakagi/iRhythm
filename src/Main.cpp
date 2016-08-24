# include <Siv3D.hpp>
# include <HamFramework.hpp>
# include <iostream>
# include <array>
# include <vector>
#include "gameGraphics.hpp"

struct GameData {
	int score = 0;
	int combo = 0;
	int great = 0;
	int poor = 0;
	int miss = 0;
};

enum Sheet {
	 NodeTime
	,Node
	,Position
};

using MyApp = SceneManager<String, GameData>;

struct Title : MyApp::Scene
{
	const Font menuFont{ 16, Typeface::Medium };
	const Rect messageBox = Rect(400, 320).setCenter(Window::Center());
	const Array<String> texts
	{
		L"はじめる",
		L"譜面作り",
		L"オプション",
		L"ゲームを終了"
	};

	Stopwatch stopwatch1, stopwatch2;
	uint32 selectIndex = 0;
	double start = 0, end = 0;

	void init() override {
		stopwatch1.start();
	}

	void update() override {
		// if (Input::MouseL.clicked)
			// changeScene(L"Game");

		Graphics2D::SetTransform(Mat3x2::Identity());

		if (Input::KeyEnter.clicked) {
			switch ((int)end) {
			case 0:
				changeScene(L"Game");
				break;
			case 1:
				changeScene(L"Create");
				break;
			case 2:
				changeScene(L"Setting");
				break;
			case 3:
				System::Exit();
				break;
			default:
				break;
			}
		}

		if (Input::KeyUp.clicked) {
			start = selectIndex;
			end = selectIndex = static_cast<uint32>((selectIndex + texts.size() - 1) % texts.size());
			stopwatch2.restart();
		}

		if (Input::KeyDown.clicked) {
			start = selectIndex;
			end = (++selectIndex %= texts.size());
			stopwatch2.restart();
		}

		RoundRect(messageBox, 20).draw(Color(20, 30, 120));

		const double t0 = Min(stopwatch1.ms() / 500.0, 1.0);
		const double e0 = EaseOut(Easing::Quart, t0);

		if (e0 > 0.0) {
			{
				const double t = Min(stopwatch2.ms() / 300.0, 1.0);
				const double y = Window::Center().y - 50 + EaseOut(start, end, Easing::Quart, t) * 50;

				RectF(16 * e0, 16 * e0).setCenter(Window::Center().x - 150, y).rotated(stopwatch1.ms() / 500.0).draw(AlphaF(0.9 * e0));
				RectF(180 * e0, 50).setCenter(Window::Center().x, y).drawShadow({ 0, 0 }, 10, 0, { 1.0, 0.2 * e0 });
			}

			for (auto i : step(texts.size())) {
				const double t = Saturate((static_cast<double>(stopwatch1.ms()) - i * 50) / 500.0);
				const double e = EaseOut(Easing::Quart, t);
				const Vec2 center(Window::Center().x, Window::Center().y - 50 + i * 50);

				Graphics2D::SetTransform(Mat3x2::Translate(-center).scale(e).translate(center));
				menuFont(texts[i]).drawCenter(center);
			}
		}
	}

	void draw() const override {
		FontAsset(L"Title")(L"Eyethm").drawCenter(100);
	}
};

struct Game : MyApp::Scene {
	const Sound music		{ L"Asset/Hope.mp3" };
	const Sound hitSound	{ L"Asset/clap.mp3" };
	const Texture backImage	{ L"Asset/backimage.jpg" };
	const CSVReader sheet	{ L"Asset/test.csv" };
		  CSVWriter logOut	{ L"Asset/log.csv" };
	const double circleSpeed{ 1.0 };
	const int circleSize	{ 100 };
	const Font font			{ 18 };
	const int judgeLine	 = 1000000;
	const int circleRate = judgeLine / 100;

	const Point center = Window::Center();
	const Rect messageBox = Rect(300, 150).setCenter(center);
	const Rect button1 = Rect(120, 40).setCenter(center.movedBy(-70, 30));
	const Rect button2 = Rect(120, 40).setCenter(center.movedBy(70, 30));

	EyeXState state;
	gameGraphics *graphics = new gameGraphics();

	unsigned int frameCount = 0;
	unsigned short int nodeCount = 0;
	int64 startTime, nowTime , nextNodeTime, frameTime = 0, passedTime = 0;
	Vec2 eyePosition{ 0, 0 };
	std::vector<Vec3> circles;
	bool m_gameOver = false;
	bool tobiiEnabled = false;

	EasingController<double> easing{ 0.0, 1.0, Easing::Quart, 500.0 };

	Vec2 getEyePosition() {
		Vec2 pos{ 0, 0 };
		if (!tobiiEnabled) {
			if (TobiiEyeX::HasNewState()) {
				TobiiEyeX::GetState(state);
				pos = state.clientGazePos;
			}
		} 
		else {
			pos = { Mouse::Pos().x, Mouse::Pos().y };
		}

		if (TobiiEyeX::HasNewState()) {
			TobiiEyeX::GetState(state);
			pos = state.clientGazePos;
		}

		return pos;		
	}

	void init() override {
		if (TobiiEyeX::Start()) {
			tobiiEnabled = true;
		}

		if (!sheet) {
			return;
		}
		nextNodeTime = sheet.get<int>(0, Sheet::NodeTime);
		
	}

	void updateFadeIn(double) override {
		music.play();
		startTime = Time::GetMicrosec();
		music.setVolume(0.5);
	}

	void update() override {
		nowTime = Time::GetMicrosec();
		eyePosition = getEyePosition();
		passedTime += frameTime;
		frameTime = (nowTime - startTime) - passedTime;
		++frameCount;

		Graphics2D::SetTransform(Mat3x2::Identity());
		const double e = easing.easeOut();

		if ( Input::KeyEnter.clicked ) {
			changeScene(L"Title");
		}

		if ( music.isPlaying() == false ) {
			changeScene(L"Title");
		}

		// before 2 sec
		if (nowTime - startTime >= (nextNodeTime - judgeLine) && nodeCount < sheet.rows) {
			Point circlePos = sheet.get<Point>(nodeCount, Sheet::Position);
			circles.push_back({ circlePos.x, circlePos.y, judgeLine });			
			nextNodeTime = sheet.get<int64>(++nodeCount,Sheet::NodeTime);
		}

		for (auto it = circles.begin(); it != circles.end(); ++it) {
			if (Input::KeySpace.clicked || Input::MouseL.clicked) {
				if (circleSize >= std::abs(std::sqrt(std::pow(eyePosition.x - it->x, 2) + std::pow(eyePosition.y - it->y, 2)))) {
					graphics->DrawJudge(graphics->GOOD, it->xy());

					circles.erase(it);
					hitSound.playMulti(0.1);
					m_data->score += 1 + (2 * m_data->combo);
					m_data->combo++;

					if (circles.size() <= 0) {
						break;
					}
					it = circles.begin();
				}
			}

			if (it->z <= 0 && circleSize >= std::abs(std::sqrt(std::pow(eyePosition.x - it->x, 2) + std::pow(eyePosition.y - it->y, 2)))) {
				graphics->DrawJudge(graphics->GOOD, it->xy());

				circles.erase(it);
				hitSound.playMulti(0.1);
				m_data->score += 1 + (2 * m_data->combo);
				m_data->combo++;

				if (circles.size() <= 0) {
					break;
				}
				it = circles.begin();
			}

			// Miss
			if (it->z <= -(circleSize / 2)) {
				graphics->DrawJudge(graphics->MISS, it->xy());
				m_data->combo = 0;
				m_data->miss++;
				circles.erase(it);
				if (circles.size() <= 0) {
					break;
				}
				it = circles.begin();
			}
		}

		for (auto& circle : circles) { 
			circle.z -= frameTime;
		}

		if (e != 0.0) {
			Graphics2D::SetTransform(Mat3x2::Translate(-center).scale(e).translate(center));
			const ColorF uiColor = AlphaF(e);

			RoundRect(messageBox, 20).draw(ColorF(0.2, 0.6, 0.4, e));
			font(L"終了しますか？").drawCenter(center.movedBy(0, -30), uiColor);

			button1.drawFrame(2, 0, uiColor);
			font(L"はい").drawCenter(button1.center, uiColor);

			button2.drawFrame(2, 0, uiColor);
			font(L"いいえ").drawCenter(button2.center, uiColor);
		}

		if (!easing.isActive()) {
			if (e == 1.0) {
				if (button1.mouseOver)
					button1.draw(AlphaF(0.3));

				if (button2.mouseOver)
					button2.draw(AlphaF(0.3));

				if (button1.leftClicked)
					System::Exit();

				if (button2.leftClicked){}
					// easing.start();
			}
			else if (Input::MouseL.clicked) {
				// easing.start();
			}
		}
		logOut.writeRow(frameTime);
	}

	void draw() const override {
		backImage.draw(Window::Center().x - backImage.width/2, Window::Center().y - backImage.height/2);
		for (auto&& circle : circles) {
			Circle(circle.xy(), circleSize).drawShadow({ 10,10 }, 40);
			Circle(circle.xy(), circleSize).draw();
			Circle(circle.xy(), (circleSize + (circle.z / circleRate))).drawFrame(3, 3, Palette::Brown);
		}

		graphics->drawString("Score : " + std::to_string(m_data->score));
		graphics->drawString("Combo : " + std::to_string(m_data->combo));
		graphics->drawString("Miss : "  + std::to_string(m_data->miss));
		
		Circle(eyePosition, 30).draw(Palette::Red);

		double theta;
		const auto fft = FFT::Analyze(music);
		for (auto i : step(320)) {
			theta = i * 0.01;
			RectF(i * Window::Width() / 320, Window::Height(), Window::Width()/320, -Pow(fft.buffer[i], 0.6) * 200).draw(HSV(240 - i));
			Line(sin((double)i / 52) * circleSize * 3
				, cos((double)i / 52) * circleSize * 3
				, sin((double)i / 52) * (circleSize*3 + (Pow(fft.buffer[i], 0.6) * 1000))
				, cos((double)i / 52) * (circleSize*3 + (Pow(fft.buffer[i], 0.6) * 1000))
			).movedBy(Window::Center()).draw(HSV(240 - i));
		}

#if defined(_DEBUG)
		int c = 0;
		for (auto&& circle : circles) {
			font(circle.z / circleRate).draw(circle.xy(), Palette::Orange);
			++c;
		}

		graphics->drawString("");
		graphics->drawString("[Debug Info]", Palette::Red);
		graphics->drawString("draw Count : " + std::to_string(frameCount));
		for (auto&& circle : circles) {
			graphics->drawString("x:" + std::to_string(circle.x) + " y:" + std::to_string(circle.y) + " z:" + std::to_string(circle.z));
		}
#endif
		graphics->endFrameDraw();
	}
};

struct Create : MyApp::Scene {
	const Sound music{ L"Asset/Hope.mp3" };
	CSVWriter sheet{ L"Asset/test.csv" };

	int64 offset ,startTime ,nowTime;
	std::vector<Vec3> nodes;
	int64 nt;

	void init() {
		// music.play();
		// startTime = Time::GetMicrosec();
	}

	void update() override {
		if (Input::MouseR.clicked) {
			music.play();
			startTime = Time::GetMicrosec();
		}

		if (Input::MouseL.clicked) {
			nowTime = Time::GetMicrosec();
			if (music.isPlaying()) {
				nodes.push_back({ Mouse::Pos().x ,Mouse::Pos().y ,nowTime - startTime });
			}
			
		}

		if (Input::KeyEnter.clicked) {
			for (auto& node : nodes ) {
				sheet.writeRow(node.z ,L"circle" ,node.xy());
			}			
			changeScene(L"Title");
		}

	}

	void draw() const override {
		FontAsset(L"Title")(L"右クリックで曲再生、左クリックでノード生成").drawCenter(100);
	}

};

struct Result : MyApp::Scene
{
	void init() override
	{
		// m_data->highScore = Max(m_data->highScore, m_data->currentScore);
	}

	void update() override
	{
		if (Input::MouseL.clicked)
			changeScene(L"Title");
	}

	void draw() const override
	{
		// FontAsset(L"Score")(L"Score: ", m_data->currentScore).draw(100, 100);
		// FontAsset(L"Score")(L"High score: ", m_data->highScore).draw(100, 160);
	}
};

struct Test : MyApp::Scene
{
	const Sound music{ L"Asset/Hope.mp3" };
	const int circleSize = 450;
	

	void init() override
	{
		music.play();
	}

	void update() override
	{
		if (Input::MouseR.clicked)
			changeScene(L"Title");
	}

	void draw() const override
	{
		double theta;
		const auto fft = FFT::Analyze(music);
		for (auto i : step(320)) {
			theta = i * 0.01;
			RectF(i * 2, Window::Height(), 2, -Pow(fft.buffer[i], 0.6) * 1000).draw(HSV(240 - i));
			Line(sin((double)i/50) * circleSize
				, cos((double)i / 50) * circleSize
				, sin((double)i / 50) * (circleSize + (Pow(fft.buffer[i], 0.6) * 1000))
				, cos((double)i / 50) * (circleSize + (Pow(fft.buffer[i], 0.6) * 1000))
				).movedBy(Window::Center()).draw(HSV(240 - i));
		}		
	}
};


void Main()
{

	Window::Resize(1920	, 1080);

	FontAsset::Register(L"Title", 60);
	FontAsset::Register(L"Score", 40);

	MyApp manager(SceneManagerOption::ShowSceneName);
	manager.add<Title>(L"Title");
	manager.add<Game>(L"Game");
	manager.add<Create>(L"Create");
	manager.add<Result>(L"Result");
	manager.add<Test>(L"Test");

	while (System::Update())
	{
		if (!manager.updateAndDraw())
			break;
	}
}