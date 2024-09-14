#include "Line.h"

Line::Line(bool exist, int type, float start_x, float start_y, float end_x, float end_y)
	:Shape("Line"), is_exist(exist), type(type), start(D2D1::Point2F(start_x, start_y)), end(D2D1::Point2F(end_x, end_y))
{
}

//Line::~Line() {}

void Line::draw(ID2D1HwndRenderTarget* pRenderTarget, ID2D1SolidColorBrush* pSolidColorBrush, float width)
{
	pRenderTarget->DrawLine(
		start,
		end,
		pSolidColorBrush,
		width
	);
}

bool Line::is_selected(float x, float y)
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

void Line::translate(float dx, float dy)
{
	start.x += dx;
	start.y += dy;
	end.x += dx;
	end.y += dy;
}

void Line::rotate(POINT old_point, POINT mouse)
{
	start = Shape::rotate(start, old_point, mouse);
	end = Shape::rotate(end, old_point, mouse);
}

void Line::zoom(double factor)
{
	// 以两点中点为中心缩放
	D2D1_POINT_2F center = D2D1::Point2F((start.x + end.x) / 2, (start.y + end.y) / 2);

	start = Shape::zoom(start, center, factor);
	end = Shape::zoom(end, center, factor);
}
