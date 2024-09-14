#pragma once

#include "Shape.h"

class Ellipses :public Shape
{
private:
	D2D1_ELLIPSE ellipse;
	bool is_exist;

public:
	Ellipses(bool exist, float x, float y, float radiusX, float radiusY);
	~Ellipses() {}
	void draw(ID2D1HwndRenderTarget* pRenderTarget, ID2D1SolidColorBrush* pSolidColorBrush, float strokeWidth);
	bool is_selected(float x, float y);

	bool get_is_exist() { return is_exist; }
	void set_is_exist(bool is_exist) { this->is_exist = is_exist; }

	D2D1_ELLIPSE get_ellipse() { return ellipse; }
	POINT get_center() { return { (int)ellipse.point.x, (int)ellipse.point.y }; }
	float get_radiusX() { return ellipse.radiusX; }
	float get_radiusY() { return ellipse.radiusY; }

	void set_center(POINT center) { ellipse.point.x = center.x; ellipse.point.y = center.y; }
	void set_radiusX(float radiusX) { ellipse.radiusX = radiusX; }
	void set_radiusY(float radiusY) { ellipse.radiusY = radiusY; }

	void translate(float dx, float dy);
	void rotate(POINT old_point, POINT mouse);
	void zoom(double factor);

	bool is_in_rect(D2D_RECT_F r);
};