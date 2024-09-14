#include "Shape.h"

//Shape::Shape(std::string name) :name(name) {}

const double M_PI = 3.14159265358979323846;

POINT Shape::rotate(POINT raw_point, POINT old, POINT mouse)
{
	// mouse和old点构成的边与水平线构成的角为旋转角
	float theta = atan2(mouse.y - old.y, mouse.x - old.x);
	POINT new_point;

	new_point.x = old.x + (raw_point.x - old.x) * cos(theta) - (raw_point.y - old.y) * sin(theta);
	new_point.y = old.y + (raw_point.x - old.x) * sin(theta) + (raw_point.y - old.y) * cos(theta);

	return new_point;
}

D2D1_POINT_2F Shape::rotate(D2D1_POINT_2F raw_point, POINT old, POINT mouse)
{
	// mouse和old点构成的边与水平线构成的角为旋转角
	float theta = atan2(mouse.y - old.y, mouse.x - old.x);
	D2D1_POINT_2F new_point;
	
	new_point.x = old.x + (raw_point.x - old.x) * cos(theta) - (raw_point.y - old.y) * sin(theta);
	new_point.y = old.y + (raw_point.x - old.x) * sin(theta) + (raw_point.y - old.y) * cos(theta);

	return new_point;
}

POINT Shape::zoom(POINT raw_point, POINT center, double factor)
{
	POINT new_point;

	new_point.x = center.x + (raw_point.x - center.x) * factor;
	new_point.y = center.y + (raw_point.y - center.y) * factor;

	return new_point;
}

D2D1_POINT_2F Shape::zoom(D2D1_POINT_2F raw_point, D2D1_POINT_2F center, double factor)
{
	D2D1_POINT_2F new_point;

	new_point.x = center.x + (raw_point.x - center.x) * factor;
	new_point.y = center.y + (raw_point.y - center.y) * factor;

	return new_point;
}