#include "BLine.h"

BLine::BLine(bool exist, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) :Shape("BLine"), is_exist(exist)
{
	point1 = D2D1::Point2F(x1, y1);
	point2 = D2D1::Point2F(x2, y2);
	point3 = D2D1::Point2F(x3, y3);
	point4 = D2D1::Point2F(x4, y4);
}

bool BLine::is_selected(float x, float y)
{
	bool res = false;

	//如果鼠标到point[0]或point[3]的距离小于5，就算选中
	if (sqrt((x - point1.x) * (x - point1.x) + (y - point1.y) * (y - point1.y)) < 5)
	{
		res = true;
	}
	else if (sqrt((x - point2.x) * (x - point2.x) + (y - point2.y) * (y - point2.y)) < 5)
	{
		res = true;
	}
	else if (sqrt((x - point3.x) * (x - point3.x) + (y - point3.y) * (y - point3.y)) < 5)
	{
		res = true;
	}
	else if (sqrt((x - point4.x) * (x - point4.x) + (y - point4.y) * (y - point4.y)) < 5)
	{
		res = true;
	}

	return res;
}

void BLine::translate(float dx, float dy)
{
	point1.x += dx;
	point1.y += dy;
	point2.x += dx;
	point2.y += dy;
	point3.x += dx;
	point3.y += dy;
	point4.x += dx;
	point4.y += dy;
}

void BLine::rotate(POINT old_point, POINT mouse)
{
	point1 = Shape::rotate(point1, old_point, mouse);
	point2 = Shape::rotate(point2, old_point, mouse);
	point3 = Shape::rotate(point3, old_point, mouse);
	point4 = Shape::rotate(point4, old_point, mouse);
}

void BLine::zoom(double factor)
{
	point1 = Shape::zoom(point1, point1, factor);
	point2 = Shape::zoom(point2, point1, factor);
	point3 = Shape::zoom(point3, point1, factor);
	point4 = Shape::zoom(point4, point1, factor);
}
