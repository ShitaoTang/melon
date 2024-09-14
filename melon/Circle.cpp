#include "Circle.h"

Circle::Circle(bool exist, int type, float x, float y, float radius)
{
	ellipse.point = D2D1::Point2F(x, y);
	ellipse.radiusX = radius;
	ellipse.radiusY = radius;
	is_exist = exist;
	// is_filled = filled;
	this->type = type;
}

void Circle::draw(ID2D1HwndRenderTarget* pRenderTarget, ID2D1SolidColorBrush* pSolidColorBrush, float strokeWidth)
{
	if (is_exist)
	{
		pSolidColorBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
		pRenderTarget->DrawEllipse(ellipse, pSolidColorBrush, strokeWidth);
	}
}

// �ж�����Ƿ�ѡ��Բ
bool Circle::is_selected(float x, float y)
{
	bool res = false;

	// ����������Բ�ĺ���������˵㸽��5�ľ����ڣ���������Բ��
	POINT left = { (int)(ellipse.point.x - ellipse.radiusX), (int)ellipse.point.y };
	POINT right = { (int)(ellipse.point.x + ellipse.radiusX), (int)ellipse.point.y };
	if (!(x >= left.x && x <= right.x))	// �������ں��������˵�֮�⣬�����䵽�˵�ľ���
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

void Circle::translate(float dx, float dy)
{
	ellipse.point.x += dx;
	ellipse.point.y += dy;
}

void Circle::rotate(POINT old_point, POINT mouse)
{
	ellipse.point = Shape::rotate(ellipse.point, old_point, mouse);
}

void Circle::zoom(double factor)
{
	ellipse.radiusX *= factor;
	ellipse.radiusY *= factor;
}

bool Circle::is_in_rect(D2D_RECT_F r)
{
	if (ellipse.point.x - ellipse.radiusX >= r.left && ellipse.point.x + ellipse.radiusX <= r.right && ellipse.point.y - ellipse.radiusY >= r.top && ellipse.point.y + ellipse.radiusY <= r.bottom)
	{
		return true;
	}

	return false;
}
