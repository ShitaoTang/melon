#include "Ellipse.h"

Ellipses::Ellipses(bool exist, float x, float y, float radiusX, float radiusY) :Shape("Ellipse"), is_exist(exist)
{
	ellipse.point = D2D1::Point2F(x, y);
	ellipse.radiusX = radiusX;
	ellipse.radiusY = radiusY;
}

void Ellipses::draw(ID2D1HwndRenderTarget* pRenderTarget, ID2D1SolidColorBrush* pSolidColorBrush, float strokeWidth)
{
	pRenderTarget->DrawEllipse(
		ellipse,
		pSolidColorBrush,
		strokeWidth
	);
}

bool Ellipses::is_selected(float x, float y)
{

	bool res = false;

	// ����������Բ�ĺ���������˵㸽��5�ľ����ڣ���������Բ��
	POINT left = { (int)(ellipse.point.x - ellipse.radiusX), (int)ellipse.point.y };
	POINT right = { (int)(ellipse.point.x + ellipse.radiusX), (int)ellipse.point.y };
	if(!(x >= left.x && x <= right.x))	// �������ں��������˵�֮�⣬�����䵽�˵�ľ���
	{
		if (sqrt((x - left.x) * (x - left.x) + (y - left.y) * (y - left.y)) < 5)
		{
			res = true;
		}
		else if (sqrt((x - right.x) * (x - right.x) + (y - right.y) * (y - right.y)) < 5)
		{
			res = true;
		}
	}
	else
	{
		// ������Բ�Ϻ�����Ϊx�ĵ��������
		float y1 = sqrt(ellipse.radiusY * ellipse.radiusY * (1 - (x - ellipse.point.x) * (x - ellipse.point.x) / (ellipse.radiusX * ellipse.radiusX))) + ellipse.point.y;
		float y2 = -sqrt(ellipse.radiusY * ellipse.radiusY * (1 - (x - ellipse.point.x) * (x - ellipse.point.x) / (ellipse.radiusX * ellipse.radiusX))) + ellipse.point.y;
		if (abs(y - y1) < 5 || abs(y - y2) < 5)
		{
			res = true;
		}
	}

	return res;
}

void Ellipses::translate(float dx, float dy)
{
	ellipse.point.x += dx;
	ellipse.point.y += dy;
}

void Ellipses::rotate(POINT old_point, POINT mouse)
{

}

void Ellipses::zoom(double factor)
{
	ellipse.radiusX *= factor;
	ellipse.radiusY *= factor;
}

bool Ellipses::is_in_rect(D2D_RECT_F r)
{
	if (ellipse.point.x - ellipse.radiusX >= r.left && ellipse.point.x + ellipse.radiusX <= r.right && ellipse.point.y - ellipse.radiusY >= r.top && ellipse.point.y + ellipse.radiusY <= r.bottom)
	{
		return true;
	}

	return false;
}
