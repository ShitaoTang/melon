#pragma once

#include "Shape.h"

class Polylines : public Shape
{
private:
	std::vector<D2D1_POINT_2F> points;	// ¶¥µã
	bool is_exist;
	bool is_filled;

public:
	Polylines(bool exist, bool is_filled, std::vector<D2D1_POINT_2F> points);
	~Polylines() {}
	void draw(ID2D1HwndRenderTarget* pRenderTarget, ID2D1SolidColorBrush* pSolidColorBrush, float strokeWidth);
	bool is_selected(float x, float y);
	bool is_selected(D2D1_POINT_2F start, D2D1_POINT_2F end, float x, float y);

	bool get_is_exist() { return is_exist; }
	void set_is_exist(bool is_exist) { this->is_exist = is_exist; }

	bool get_is_filled() { return is_filled; }
	void set_is_filled(bool is_filled) { this->is_filled = is_filled; }

	bool get_is_closed() { return points[0].x == points[points.size() - 1].x && points[0].y == points[points.size() - 1].y; }

	bool is_triangle();

	bool is_in_rect(POINT left_top, POINT right_bottom);

	std::vector<D2D1_POINT_2F> get_points() { return points; }

	void translate(float dx, float dy);
	void rotate(POINT old_point, POINT mouse);
	void zoom(double factor);
};