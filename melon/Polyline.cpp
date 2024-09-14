#include "Polyline.h"

Polylines::Polylines(bool exist, bool filled, std::vector<D2D1_POINT_2F> points)
{
	this->is_exist = exist;
	this->is_filled = filled;
	this->points = points;
}

void Polylines::draw(ID2D1HwndRenderTarget* pRenderTarget, ID2D1SolidColorBrush* pSolidColorBrush, float strokeWidth)
{
	if (is_exist)
	{
		pSolidColorBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
		pRenderTarget->DrawLine(points[0], points[1], pSolidColorBrush, strokeWidth);
		for (int i = 1; i < points.size() - 1; i++)
		{
			pRenderTarget->DrawLine(points[i], points[i + 1], pSolidColorBrush, strokeWidth);
		}
	}
}

bool Polylines::is_selected(D2D1_POINT_2F start, D2D1_POINT_2F end, float x, float y)
{
	// 计算鼠标点是否在线段上，注意，在其延长线上不算在线段上
	float a = end.y - start.y;
	float b = start.x - end.x;
	float c = end.x * start.y - start.x * end.y;
	float d = abs(a * x + b * y + c) / sqrt(a * a + b * b);
	if (d < 3)
	{
		float min_x = start.x < end.x ? start.x : end.x;
		float max_x = start.x > end.x ? start.x : end.x;
		float min_y = start.y < end.y ? start.y : end.y;
		float max_y = start.y > end.y ? start.y : end.y;
		if (x >= min_x && x <= max_x && y >= min_y && y <= max_y)
		{
			return true;
		}
	}
	
	return false;
}

bool Polylines::is_triangle()
{
	if (points.size() == 4 && points[0].x == points[3].x && points[0].y == points[3].y)
		return true;

	// 前两个重合的情况
	if (points.size() == 5 && points[0].x == points[4].x && points[0].y == points[0].y && points[0].x == points[1].x && points[0].y == points[1].y)
		return true;

	return false;
}

bool Polylines::is_in_rect(POINT left_top, POINT right_bottom)
{
	for (auto p : points)
		if (!(p.x <= right_bottom.x && p.x >= left_top.x && p.y >= left_top.y && p.y <= right_bottom.y))
			return false;

	return true;
}


void Polylines::translate(float dx, float dy)
{
	for (int i = 0; i < points.size(); i++)
	{
		points[i].x += dx;
		points[i].y += dy;
	}
}

void Polylines::rotate(POINT old_point, POINT mouse)
{
	for (int i = 0; i < points.size(); i++)
	{
		points[i] = Shape::rotate(points[i], old_point, mouse);
	}
}

void Polylines::zoom(double factor)
{
	for (int i = 0; i < points.size(); i++)
	{
		points[i] = Shape::zoom(points[i], points[0], factor);
	}
}

bool Polylines::is_selected(float x, float y)
{
	for (int i = 0; i < points.size() - 1; i++)
	{
		if (is_selected(points[i], points[i + 1], x, y))
		{
			return true;
		}
	}
	return false;
}
