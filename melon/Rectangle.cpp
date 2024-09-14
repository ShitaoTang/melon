#include "Rectangle.h"

Rectangles::Rectangles(bool exist, bool filled, float left, float top, float right, float bottom, float lbleft, float lbbottom, float rtright, float rttop) :Shape("Rectangle"), is_exist(exist), is_filled(filled)
{
	//rect.left = min(left, right);
	//rect.top = min(top, bottom);
	//rect.right = max(left, right);
	//rect.bottom = max(top, bottom);
	//rect.lbleft = rect.left;
	//rect.lbbottom = rect.bottom;
	//rect.rtright = rect.right;
	//rect.rttop = rect.top;

	rect.left = left;
	rect.top = top;
	rect.right = right;
	rect.bottom = bottom;
	rect.lbleft = lbleft;
	rect.lbbottom = lbbottom;
	rect.rtright = rtright;
	rect.rttop = rttop;

	selected = false;
}

void Rectangles::draw(ID2D1HwndRenderTarget* pRenderTarget, ID2D1SolidColorBrush* pSolidColorBrush, float strokeWidth)
{
	//pRenderTarget->DrawRectangle(
	//	rect,
	//	pSolidColorBrush,
	//	strokeWidth
	//);

	// 通过绘制直线的方式绘制矩形
	pRenderTarget->DrawLine(
		D2D1::Point2F(rect.left, rect.top),
		D2D1::Point2F(rect.rtright, rect.rttop),
		pSolidColorBrush,
		strokeWidth
	);

	pRenderTarget->DrawLine(
		D2D1::Point2F(rect.rtright, rect.rttop),
		D2D1::Point2F(rect.right, rect.bottom),
		pSolidColorBrush,
		strokeWidth
	);

	pRenderTarget->DrawLine(
		D2D1::Point2F(rect.right, rect.bottom),
		D2D1::Point2F(rect.lbleft, rect.lbbottom),
		pSolidColorBrush,
		strokeWidth
	);

	pRenderTarget->DrawLine(
		D2D1::Point2F(rect.lbleft, rect.lbbottom),
		D2D1::Point2F(rect.left, rect.top),
		pSolidColorBrush,
		strokeWidth
	);
}

bool Rectangles::is_selected(float x, float y)
{
	// 如果上边水平
	if (rect.top == rect.rttop)
	{
		if(y >= rect.top - 3 && y <= rect.top + 3 && x >= min(rect.left, rect.rtright) - 3 && x <= max(rect.left, rect.rtright) + 3)
			return true;
	}

	// 如果下边水平
	if (rect.bottom == rect.lbbottom)
	{
		if (y >= rect.bottom - 3 && y <= rect.bottom + 3 && x >= min(rect.lbleft, rect.right) - 3 && x <= max(rect.lbleft, rect.right) + 3)
			return true;
	}

	// 如果左边垂直
	if (rect.left == rect.lbleft)
	{
		if (x >= rect.left - 3 && x <= rect.left + 3 && y >= min(rect.top, rect.lbbottom) - 3 && y <= max(rect.top, rect.lbbottom) + 3)
			return true;
	}

	// 如果右边垂直
	if (rect.right == rect.rtright)
	{
		if (x >= rect.right - 3 && x <= rect.right + 3 && y >= min(rect.rttop, rect.bottom) - 3 && y <= max(rect.rttop, rect.bottom) + 3)
			return true;
	}

	// 上
	if(is_selected(D2D1::Point2F(rect.left, rect.top), D2D1::Point2F(rect.rtright, rect.rttop), x, y))
		return true;
	// 右
	if (is_selected(D2D1::Point2F(rect.rtright, rect.rttop), D2D1::Point2F(rect.right, rect.bottom), x, y))
		return true;
	// 下
	if (is_selected(D2D1::Point2F(rect.right, rect.bottom), D2D1::Point2F(rect.lbleft, rect.lbbottom), x, y))
		return true;
	// 左
	if (is_selected(D2D1::Point2F(rect.lbleft, rect.lbbottom), D2D1::Point2F(rect.left, rect.top), x, y))
		return true;

	return false;
}

bool Rectangles::is_selected(D2D1_POINT_2F start, D2D1_POINT_2F end, float x, float y)
{
	// 计算鼠标点是否在线段上，注意，在其延长线上不算在线段上
	float a = end.y - start.y;
	float b = start.x - end.x;
	float c = end.x * start.y - start.x * end.y;
	float d = abs(a * x + b * y + c) / sqrt(a * a + b * b);
	if (d < 10)
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

void Rectangles::translate(float dx, float dy)
{
	rect.left += dx;
	rect.top += dy;
	rect.right += dx;
	rect.bottom += dy;
	rect.lbleft += dx;
	rect.lbbottom += dy;
	rect.rtright += dx;
	rect.rttop += dy;
}

void Rectangles::rotate(POINT old_point, POINT mouse)
{
	D2D1_POINT_2F center = D2D1::Point2F((rect.left + rect.right + rect.lbleft + rect.rtright) / 4, (rect.top + rect.bottom + rect.rttop + rect.lbbottom) / 4);
	D2D1_POINT_2F new_center = Shape::rotate(center, old_point, mouse);
	D2D1_POINT_2F new_left_top = Shape::rotate(D2D1::Point2F(rect.left, rect.top), old_point, mouse);
	D2D1_POINT_2F new_right_top = Shape::rotate(D2D1::Point2F(rect.rtright, rect.rttop), old_point, mouse);
	D2D1_POINT_2F new_right_bottom = Shape::rotate(D2D1::Point2F(rect.right, rect.bottom), old_point, mouse);
	D2D1_POINT_2F new_left_bottom = Shape::rotate(D2D1::Point2F(rect.lbleft, rect.lbbottom), old_point, mouse);

	rect.left = new_left_top.x;
	rect.top = new_left_top.y;
	rect.right = new_right_bottom.x;
	rect.bottom = new_right_bottom.y;
	rect.lbleft = new_left_bottom.x;
	rect.lbbottom = new_left_bottom.y;
	rect.rtright = new_right_top.x;
	rect.rttop = new_right_top.y;
}

void Rectangles::zoom(double factor)
{
	D2D1_POINT_2F center = D2D1::Point2F((rect.left + rect.right + rect.lbleft + rect.rtright) / 4, (rect.top + rect.bottom + rect.rttop + rect.lbbottom) / 4);
	D2D1_POINT_2F new_left_top = Shape::zoom(D2D1::Point2F(rect.left, rect.top), center, factor);
	D2D1_POINT_2F new_right_top = Shape::zoom(D2D1::Point2F(rect.rtright, rect.rttop), center, factor);
	D2D1_POINT_2F new_right_bottom = Shape::zoom(D2D1::Point2F(rect.right, rect.bottom), center, factor);
	D2D1_POINT_2F new_left_bottom = Shape::zoom(D2D1::Point2F(rect.lbleft, rect.lbbottom), center, factor);

	rect.left = new_left_top.x;
	rect.top = new_left_top.y;
	rect.right = new_right_bottom.x;
	rect.bottom = new_right_bottom.y;
	rect.lbleft = new_left_bottom.x;
	rect.lbbottom = new_left_bottom.y;
	rect.rtright = new_right_top.x;
	rect.rttop = new_right_top.y;
}

bool Rectangles::is_in_rect(D2D_RECT_F r)
{
	// 如果8个点都在矩形rect内，则返回true
	if(r.left <= rect.left && r.top <= rect.top && r.right >= rect.right && r.bottom >= rect.bottom &&
				r.left <= rect.lbleft && r.top <= rect.lbbottom && r.right >= rect.rtright && r.bottom >= rect.rttop)
		return true;

	return false;
}
