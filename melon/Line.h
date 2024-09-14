#pragma once

#include "Shape.h"

class Line :public Shape
{
private:
	D2D1_POINT_2F start, end;	// 起点和终点
	bool is_exist;
	int type;

public:
	Line(bool exist, int type, float start_x, float start_y, float end_x, float end_y);
	~Line() {}
	void draw(ID2D1HwndRenderTarget* pRenderTarget, ID2D1SolidColorBrush* pSolidColorBrush, float strokeWidth);
	bool is_selected(float x, float y);

	bool get_is_exist() { return is_exist; }
	void set_is_exist(bool is_exist) { this->is_exist = is_exist; }

	POINT get_start() { return { (int)start.x, (int)start.y }; }
	POINT get_end() { return { (int)end.x, (int)end.y }; }

	int get_type() { return type; }

	void translate(float dx, float dy);
	void rotate(POINT old_point, POINT mouse);
	void zoom(double factor);
};