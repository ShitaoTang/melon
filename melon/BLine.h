#pragma once

#include "Shape.h"

class BLine :public Shape
{
private:
	D2D1_POINT_2F point1;	// 起点
	D2D1_POINT_2F point2;	// 第一控制点
	D2D1_POINT_2F point3;	// 第二控制点
	D2D1_POINT_2F point4;	// 终点
	bool is_exist;			// 是否存在

public:
	BLine(bool exist, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
	~BLine() {}

	void draw(ID2D1HwndRenderTarget* pRenderTarget, ID2D1SolidColorBrush* pSolidColorBrush, float strokeWidth) {};
	bool is_selected(float x, float y);

	bool get_is_exist() { return is_exist; }
	void set_is_exist(bool is_exist) { this->is_exist = is_exist; }

	D2D1_POINT_2F get_d2d_point1() { return point1; }
	D2D1_POINT_2F get_d2d_point2() { return point2; }
	D2D1_POINT_2F get_d2d_point3() { return point3; }
	D2D1_POINT_2F get_d2d_point4() { return point4; }

	void translate(float dx, float dy);
	void rotate(POINT old_point, POINT mouse);
	void zoom(double factor);
};