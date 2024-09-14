#pragma once

#include "resource.h"
#include "Line.h"
#include "Ellipse.h"
#include "Rectangle.h"
#include "Bezier.h"
#include "Polyline.h"
#include "Circle.h"
#include "BLine.h"

#include "headers.h"

template <class Interface>
inline void SafeRelease(Interface** ppInterfaceToRelease)
{
	if (*ppInterfaceToRelease != nullptr)
	{
		(*ppInterfaceToRelease)->Release();

		(*ppInterfaceToRelease) = nullptr;
	}
}

#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

class Melon
{
public:
	Melon();
	~Melon();

	// 注册窗口类并创建窗口
	HRESULT Initialize();

	// 消息循环
	void RunMessageLoop();

private:
	// 初始化设备无关资源
	HRESULT CreateDeviceIndependentResources();

	// 初始化设备相关资源
	HRESULT CreateDeviceResources();

	//释放设备相关资源
	void DiscardDeviceResources();

	// 绘制
	HRESULT OnRender();

	// 调整窗口大小
	void OnResize(
		UINT width,
		UINT height
	);

	// 处理窗口消息
	static LRESULT CALLBACK WndProc(
		HWND hWnd,
		UINT message,
		WPARAM wParam,
		LPARAM lParam
	);

	void DrawBLine(D2D1_POINT_2F startPoint, D2D1_POINT_2F controlPoint1, D2D1_POINT_2F controlPoint2, D2D1_POINT_2F endPoint, ID2D1SolidColorBrush* m_pSolidColorBrush);

	POINT get_dbclk_mouse_pos()
	{
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(m_hwnd, &p);
		return p;
	}

	void DrawBezier(D2D1_POINT_2F point1, D2D1_POINT_2F point2, D2D1_POINT_2F point3, D2D1_POINT_2F point4, ID2D1SolidColorBrush* m_pSolidColorBrush, float strokeWidth);

	// 判断两线段是否相交
	bool is_intersect_ll(POINT p1, POINT p2, POINT p3, POINT p4);

	// 获取两线段的交点
	POINT get_intersect_ll(POINT p1, POINT p2, POINT p3, POINT p4);

	bool is_intersect_ll(D2D1_POINT_2F p1, D2D1_POINT_2F p2, D2D1_POINT_2F p3, D2D1_POINT_2F p4);

	D2D1_POINT_2F get_intersect_ll(D2D1_POINT_2F p1, D2D1_POINT_2F p2, D2D1_POINT_2F p3, D2D1_POINT_2F p4);

	// 获取直线与椭圆的交点
	void get_intersect_le(POINT p1, POINT p2, D2D1_ELLIPSE e, std::vector<POINT>& intersections);

	// 获取椭圆与椭圆的交点
	void get_intersect_ee(D2D1_ELLIPSE e1, D2D1_ELLIPSE e2, std::vector<POINT>& intersections);

	// 获取点到所有直线的垂足
	void get_footpoint(POINT p, std::vector<POINT>& foot_points);

	// 设置像素点，以填充小矩形代替像素点
	void put_pixel(int x, int y, ID2D1SolidColorBrush* brush)
	{
		D2D1_RECT_F rect;
		rect.left = static_cast<FLOAT>(x);
		rect.top = static_cast<FLOAT>(y);
		rect.right = static_cast<FLOAT>(x + 1);
		rect.bottom = static_cast<FLOAT>(y + 1);
		m_pRenderTarget->FillRectangle(&rect, brush);
	}

	void Bresenham_line(int x1, int y1, int x2, int y2);

	void Bresenham_circle(int xc, int yc, int r, ID2D1SolidColorBrush* m_pBrush);

	void fill_rect(int left, int top, int right, int bottom)
	{
		for (int i = left; i <= right; ++i)
			for (int j = top; j <= bottom; ++j)
				put_pixel(i, j, m_pFillBrush);
	}

	// 填充三角形
	void fill_triangle(int x1, int y1, int x2, int y2, int x3, int y3);

	// 种子填充算法，递归函数
	void seed_fill(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3, ID2D1SolidColorBrush* brush);

	// 判断点(x, y)是否在三角形内
	bool is_in_triangle(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3);

	void fill_triangle_scanline(int x1, int y1, int x2, int y2, int x3, int y3, ID2D1SolidColorBrush* brush);

	void scanline(std::vector<POINT> vertices);

	// void scanline_seed(int x, int y);

	// 绘制四阶Bezier曲线
	void draw_4d_bezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int x5, int y5);

	POINT rotate(POINT raw_point, POINT old, POINT mouse)
	{
		// mouse和old点构成的边与水平线构成的角为旋转角
		float theta = atan2(mouse.y - old.y, mouse.x - old.x);
		POINT new_point;

		new_point.x = old.x + (raw_point.x - old.x) * cos(theta) - (raw_point.y - old.y) * sin(theta);
		new_point.y = old.y + (raw_point.x - old.x) * sin(theta) + (raw_point.y - old.y) * cos(theta);

		return new_point;
	}

	D2D1_POINT_2F rotate(D2D1_POINT_2F raw_point, POINT old, POINT mouse)
	{
		// mouse和old点构成的边与水平线构成的角为旋转角
		float theta = atan2(mouse.y - old.y, mouse.x - old.x);
		D2D1_POINT_2F new_point;

		new_point.x = old.x + (raw_point.x - old.x) * cos(theta) - (raw_point.y - old.y) * sin(theta);
		new_point.y = old.y + (raw_point.x - old.x) * sin(theta) + (raw_point.y - old.y) * cos(theta);

		return new_point;
	}

	POINT zoom(POINT raw_point, POINT center, double factor);
	D2D1_POINT_2F zoom(D2D1_POINT_2F raw_point, D2D1_POINT_2F center, double factor);

	unsigned char get_code(POINT left_top, POINT right_bottom, POINT p);

	std::vector<POINT> code_clip(POINT left_top, POINT right_bottom, POINT p1, POINT p2);

	std::vector<POINT> LiangBarsky(POINT left_top, POINT right_bottom, POINT p1, POINT p2);

	std::vector<D2D1_POINT_2F> WeilerAtherton(std::vector<D2D1_POINT_2F> sp, std::vector<D2D1_POINT_2F> cp);

	// double get_distance(D2D1_POINT_2F p1, D2D1_POINT_2F p2);

private:
	HWND m_hwnd;
	ID2D1Factory* m_pDirect2dFactory;
	ID2D1HwndRenderTarget* m_pRenderTarget;
	ID2D1SolidColorBrush* m_pBlackBrush;
	ID2D1SolidColorBrush* m_pWhiteBrush;
	ID2D1SolidColorBrush* m_pRedBrush;
	ID2D1SolidColorBrush* m_pBLineBrush;
	ID2D1SolidColorBrush* m_pLightSlateGrayBrush;
	ID2D1SolidColorBrush* m_pFillBrush;

	// 虚线线型
	ID2D1StrokeStyle* m_pStrokeStyle;

	IDWriteFactory* m_pDWriteFactory;
	IDWriteTextFormat* m_pTextFormat;

	// 直线族
	std::vector<Line*> lines;
	// 椭圆族
	std::vector<Ellipses*> ellipses;
	// 矩形族
	std::vector<Rectangles*> rectangles;
	// 贝塞尔曲线族
	std::vector<Bezier*> bezier;
	// 多义线族
	std::vector<Polylines*> polylines;
	// 圆形
	std::vector<Circle*> circles;
	// BLine
	std::vector<BLine*> blines;
};