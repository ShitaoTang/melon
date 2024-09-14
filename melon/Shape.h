#pragma once

#include "headers.h"

class Shape
{
private:
	std::string name;

public:
	Shape() :name("default") {}
	Shape(std::string name) :name(name){}
	virtual ~Shape() {}
	std::string getName() { return name; }
	virtual void draw(ID2D1HwndRenderTarget* pRenderTarget, ID2D1SolidColorBrush* pSolidColorBrush, float strokeWidth) = 0;
	virtual bool is_selected(float x, float y) = 0;		// 指示鼠标是否停靠在图形上

	POINT rotate(POINT raw_point, POINT old_point, POINT mouse);
	D2D1_POINT_2F rotate(D2D1_POINT_2F raw_point, POINT old_point, POINT mouse);
	POINT zoom(POINT raw_point, POINT center, double factor);
	D2D1_POINT_2F zoom(D2D1_POINT_2F raw_point, D2D1_POINT_2F center, double factor);
};