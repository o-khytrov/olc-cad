#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
using namespace std;

struct sShape;

struct sNode {
	sShape* parent;
	olc::vf2d pos;
};

struct sShape
{

	std::vector<sNode> vecNodes;
	uint32_t nMaxNodes = 0;
	olc::Pixel col = olc::GREEN;

	static float fWorldScale;
	static olc::vf2d vWorldOffset;
	void WorldToScreen(const olc::vf2d& v, int& nScreenX, int& nScreenY)
	{
		nScreenX = (int)((v.x - vWorldOffset.x) * fWorldScale);
		nScreenY = (int)((v.y - vWorldOffset.y) * fWorldScale);
	}
	virtual void DrawYourself(olc::PixelGameEngine* page) = 0;
	sNode* GetNextNode(const olc::vf2d& p)
	{
		if (vecNodes.size() == nMaxNodes)
			return nullptr;

		sNode n;
		n.parent = this;
		n.pos = p;
		vecNodes.push_back(n);
		return &vecNodes[vecNodes.size() - 1];
	}

	void DrawNodes(olc::PixelGameEngine* pge)
	{
		for (auto& n : vecNodes)
		{
			int sx, sy;
			WorldToScreen(n.pos, sx, sy);
			pge->FillCircle(sx, sy, 2, olc::RED);

		}
	}
};

float sShape::fWorldScale = 1.0f;
olc::vf2d sShape::vWorldOffset = { 0,0 };

struct sLine :public sShape
{
	sLine()
	{
		nMaxNodes = 2;
		vecNodes.reserve(nMaxNodes);
	}

	void DrawYourself(olc::PixelGameEngine* pge) override
	{
		int sx, sy, ex, ey;
		WorldToScreen(vecNodes[0].pos, sx, sy);
		WorldToScreen(vecNodes[1].pos, ex, ey);
		pge->DrawLine(sx, sy, ex, ey, col);
	}
};
class Editor : public olc::PixelGameEngine
{
public:
	Editor()
	{
		sAppName = "Editor";
	}
private:
	olc::vf2d vOffset = { 0.0f,0.0f };
	olc::vf2d vStartPan = { 0.0f,0.0f };
	float fScale = 10.f;
	float fGrid = 1.0f;
	sLine* tempShape = nullptr;
	std::list<sLine*> listShapes;
	sNode* selectedNode = nullptr;
	olc::vf2d vCursor{ 0,0 };
	void WorldToScreen(const olc::vf2d& v, int& nScreenX, int& nScreenY)
	{
		nScreenX = (int)((v.x - vOffset.x) * fScale);
		nScreenY = (int)((v.y - vOffset.y) * fScale);
	}
	void ScreenToWorld(int nScreenX, int nScreenY, olc::vf2d& v)
	{
		v.x = (float)(nScreenX) / fScale + vOffset.x;
		v.y = (float)(nScreenY) / fScale + vOffset.y;
	}

public:
	bool OnUserCreate() override
	{
		vOffset = { (float)(-ScreenWidth() / 2) / fScale, (float)(-ScreenHeight() / 2) / fScale };
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (GetKey(olc::Key::E).bPressed)
		{
			//this->exit
		}
		olc::vf2d vMouse = { (float)GetMouseX(), (float)GetMouseY() };
		if (GetMouse(2).bPressed)
		{
			vStartPan = vMouse;
		}
		if (GetMouse(2).bHeld)
		{
			vOffset -= (vMouse - vStartPan) / fScale;
			vStartPan = vMouse;
		}
		olc::vf2d vMouseBeforeZoom;
		ScreenToWorld((int)vMouse.x, (int)vMouse.y, vMouseBeforeZoom);

		if (GetKey(olc::Key::Q).bHeld || GetMouseWheel() > 0)
		{
			fScale *= 1.1f;
		}
		if (GetKey(olc::Key::A).bHeld || GetMouseWheel() < 0)
		{
			fScale *= 0.9f;
		}
		olc::vf2d vMouseAfterZoom;
		ScreenToWorld((int)vMouse.x, (int)vMouse.y, vMouseAfterZoom);
		vOffset += (vMouseBeforeZoom - vMouseAfterZoom);
		//Clear 
		Clear(olc::BLACK);
		int sx, sy;
		int ex, ey;
		olc::vf2d vWorldTopLeft, vWorldBottomRight;
		ScreenToWorld(0, 0, vWorldTopLeft);
		ScreenToWorld(ScreenWidth(), ScreenHeight(), vWorldBottomRight);
		vWorldTopLeft.x = floor(vWorldTopLeft.x);
		vWorldTopLeft.y = floor(vWorldTopLeft.y);

		vWorldBottomRight.x = ceil(vWorldBottomRight.x);
		vWorldBottomRight.y = ceil(vWorldBottomRight.y);

		for (float x = vWorldTopLeft.x; x < vWorldBottomRight.x; x += fGrid)
		{
			for (float y = vWorldTopLeft.y; y < vWorldBottomRight.y; y += fGrid)
			{
				WorldToScreen({ x,y }, sx, sy);
				Draw(sx, sy, olc::BLUE);
			}
		}

		//Snap mouse cursor to nearest grid interval
		vCursor.x = floorf((vMouseAfterZoom.x + 0.5f) * fGrid);
		vCursor.y = floorf((vMouseAfterZoom.y + 0.5f) * fGrid);

		if (GetKey(olc::Key::L).bPressed)
		{
			tempShape = new sLine();
			selectedNode = tempShape->GetNextNode(vCursor);
			selectedNode = tempShape->GetNextNode(vCursor);
		}

		if (selectedNode != nullptr)
		{
			selectedNode->pos = vCursor;
		}

		if (GetMouse(0).bReleased)
		{
			if (tempShape != nullptr)
			{
				selectedNode = tempShape->GetNextNode(vCursor);

				if (selectedNode == nullptr)
				{
					tempShape->col = olc::WHITE;
					listShapes.push_back(tempShape);
				}
			}



		}

		WorldToScreen({ 0, vWorldTopLeft.y }, sx, sy);
		WorldToScreen({ 0, vWorldBottomRight.y }, ex, ey);
		DrawLine(sx, sy, ex, ey, olc::GREY, 0xF0F0F0F0);
		sx = 0;
		sy = 0;
		WorldToScreen({ vWorldTopLeft.x ,0 }, sx, sy);
		WorldToScreen({ vWorldBottomRight.x,0 }, ex, ey);
		DrawLine(sx, sy, ex, ey, olc::GREY, 0xF0F0F0F0);
		sShape::fWorldScale = fScale;
		sShape::vWorldOffset = vOffset;

		for (auto &shape: listShapes )
		{
			shape->DrawYourself(this);
			shape->DrawNodes(this);
		}
		if (tempShape != nullptr)
		{
			tempShape->DrawYourself(this);
			tempShape->DrawNodes(this);
		}


		WorldToScreen(vCursor, sx, sy);
		DrawCircle(sx, sy, 3, olc::YELLOW);
		DrawString(10, 10, "X=" + std::to_string(vCursor.x) + ",Y=" + std::to_string(vCursor.y));
		return true;
	}


};

int main()
{
	Editor editor;
	if (editor.Construct(1600, 960, 1, 1))
	{
		editor.Start();
	}
	return 0;
}