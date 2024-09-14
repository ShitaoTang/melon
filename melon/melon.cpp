#include "melon.h"

const int NUM = 6;
const int LINE = 40001;
const int ELLIPSE = 40002;
const int RECTANGLE = 40003;
const int BEZIER = 40004;
const int POLYLINE = 40005;
const int POLYLINE_OK = 40007;
const int CIRCLE = 40014;
const int CLEAN = 40015;
const int PERPENDICULAR = 40016;
const int SHOWGRID = 40017;
const int BRESENHAM_LINE = 40018;
const int BRESENHAM_CIRCLE = 40019;
const int FILL = 40020;
const int BLINE = 40021;
const int TRANSLATE = 40022;
const int ROTATE = 40023;
const int ZOOM = 40024;
const int CLIP = 40025;

bool is_clean = false;
bool is_perpendicular = false;
bool is_show_grid = false;
bool signal_lclk_mouse = false;	// 用于标记鼠标左键点击信号
bool signal_rclk_mouse = false;	// 用于标记鼠标右键点击信号
bool signal_dblclk = false;
bool signal_mclk_mouse = false;

/* 变换所需变量 */
bool is_translating_r = false;
int index_r;
bool is_translating_b = false;
int index_b;
bool is_translating_bl = false;
int index_bl;
bool is_translating_c = false;
int index_c;
bool is_translating_e = false;
int index_e;
bool is_translating_l = false;
int index_l;
bool is_translating_p = false;
int index_p;

bool is_rotating_r = false;
bool is_rotating_b = false;
bool is_rotating_bl = false;
bool is_rotating_c = false;
bool is_rotating_e = false;
bool is_rotating_l = false;
bool is_rotating_p = false;

bool is_zooming_r = false;
bool is_zooming_b = false;
bool is_zooming_bl = false;
bool is_zooming_c = false;
bool is_zooming_e = false;
bool is_zooming_l = false;
bool is_zooming_p = false;

char translating = 0;
char rotating = 0;
char zooming = 0;
char clipping = 0;

std::vector<POINT> clip_points;
/* 变换所需变量 */

bool is_translate_finished = false;
bool is_rotate_finished = false;
bool is_zoom_finished = false;
bool is_clip_finished = false;

bool is_closed = false;	// 是否正在绘制闭合点

bool is_drawing_polyline = false;	// 用于标记是否正在绘制多段线
bool signal_rclk_polyline = false;

int flag = -1;	// 用于标记鼠标左键点击次数, -1表示第一次点击，1表示第二次点击
int flag_bezier = 0;	// 记录当前绘制的是第几个点，依次为起点，控制点1，控制点2，终点

int bflag = -1;
int flag_bline = 0;

int type_line = 0;	// 绘制直线时使用的方法，0为调用D2D1的DrawLine方法，1为调用自定义的Bresenham_line方法
int type_circle = 0;

POINT start, end;	// 用于记录鼠标左键点击时的起点和终点
POINT footpoint;
POINT control1, control2;	// 用于记录绘制贝塞尔曲线时的控制点1和控制点2
POINT bcontrol1, bcontrol2;
POINT old_point = { 0, 0 };	// 基点
std::vector<D2D1_POINT_2F> poly_points;

std::vector<bool> row_has_seed;

std::vector<POINT> vertices;

int state = LINE;	// 默认为画直线状态

typedef struct Edge
{
	int ymax, ymin;
	double x, dx;
}Edge;

bool operator==(const D2D1_POINT_2F& point1, const D2D1_POINT_2F& point2)
{
	return (point1.x == point2.x) && (point1.y == point2.y);
}

bool operator!=(const D2D1_POINT_2F& point1, const D2D1_POINT_2F& point2)
{
	return (point1.x != point2.x) || (point1.y != point2.y);
}

void Melon::Bresenham_line(int x1, int y1, int x2, int y2)
{
	int xl = x1; int yl = y1;
	int xr = x2; int yr = y2;
	if (x1 > x2) //确保左、右点对应(xl,yl)、(xr,yr)
	{
		xl = x2; yl = y2;
		xr = x1; yr = y1;
	}

	bool flag = true;//左点位于右点左下方（斜率大于等于0的情况）时为true，否则为假
	if (yl > yr)
		flag = false;


	//垂直
	if (xl == xr)
	{
		int di = -1;
		if (flag)	//左点位于右点下方时
			di = 1;
		for (int i = yl; i != yr; i += di)
		{
			put_pixel(xl, i, m_pBlackBrush);
		}
		put_pixel(xr, yr, m_pBlackBrush);
		return;
	}

	//水平线
	if (yl == yr)
	{
		for (int i = xl; i <= xr; ++i)
		{
			put_pixel(i, yl, m_pBlackBrush);
		}
		return;
	}

	//对角线1
	if ((xr - xl) == (yr - yl))
	{
		for (int i = xl, j = yl; i <= xr; ++i, ++j)
		{
			put_pixel(i, j, m_pBlackBrush);
		}
		return;
	}

	//对角线2
	if ((xr - xl) == -(yr - yl))
	{
		for (int i = xl, j = yl; i <= xr; ++i, --j)
		{
			put_pixel(i, j, m_pBlackBrush);
		}
		return;
	}

	int xk, yk;
	int dx, dy;
	int d, d1, d2;

	xk = xl; yk = yl;
	dx = xr - xl;
	dy = yr - yl;

	if (flag)	
	{
		if (dy < dx)	
		{
			d = 2 * (-dy) + dx;
			d1 = 2 * (-dy);
			d2 = 2 * (-dy + dx);

			while (xk <= xr)
			{
				put_pixel(xk, yk, m_pBlackBrush);
				if (d < 0)
				{
					++yk;
					d += d2;
				}
				else
				{
					d += d1;
				}
				++xk;
			}
		}
		else	
		{
			d = 2 * dx - dy;
			d1 = 2 * dx;
			d2 = 2 * (dx - dy);

			while (yk <= yr)
			{
				put_pixel(xk, yk, m_pBlackBrush);
				if (d >= 0)
				{
					++xk;
					d += d2;
				}
				else
				{
					d += d1;
				}
				++yk;
			}
		}
	}
	else	
	{
		if (-dy < dx)	
		{
			d = 2 * dy + dx;
			d1 = 2 * dy;
			d2 = 2 * (dy + dx);

			while (xk <= xr)
			{
				put_pixel(xk, yk, m_pBlackBrush);
				if (d <= 0)
				{
					--yk;
					d += d2;
				}
				else
				{
					d += d1;
				}
				++xk;
			}
		}
		else	
		{
			d = 2 * (-dy - dx);
			d1 = 2 * (-dx);
			d2 = 2 * (-dx - dy);

			while (yk >= yr)
			{
				put_pixel(xk, yk, m_pBlackBrush);
				if (d <= 0)
				{
					++xk;
					d += d2;
				}
				else
				{
					d += d1;
				}
				--yk;
			}
		}
	}
}

void Melon::Bresenham_circle(int xc, int yc, int r, ID2D1SolidColorBrush* m_pBrush)
{
	int x, y, d, d1, d2, direction;

	x = 0;
	y = r;
	d = 2 * (1 - r);

	while (x <= y)
	{
		put_pixel(x + xc, y + yc, m_pBrush);
		put_pixel(x + xc, -y + yc, m_pBrush);
		put_pixel(y + xc, x + yc, m_pBrush);
		put_pixel(y + xc, -x + yc, m_pBrush);
		put_pixel(-x + xc, y + yc, m_pBrush);
		put_pixel(-x + xc, -y + yc, m_pBrush);
		put_pixel(-y + xc, x + yc, m_pBrush);
		put_pixel(-y + xc, -x + yc, m_pBrush);

		if (d < 0)
		{
			d1 = 2 * d + 2 * y - 1;
			if (d1 <= 0)
				direction = 1;
			else
				direction = 2;
		}
		else if (d > 0)
		{
			d2 = 2 * d - 2 * x - 1;
			if (d2 <= 0)
				direction = 2;
			else
				direction = 3;
		}
		else
			direction = 3;

		switch (direction)
		{
		case 1:
			++x;
			d += 2 * x + 1;
			break;
		case 2:
			++x;
			--y;
			d += 2 * (x - y + 1);
			break;
		case 3:
			--y;
			d += (-2 * y + 1);
			break;
		}
	}
}

double get_distance(D2D1_POINT_2F p1, D2D1_POINT_2F p2)
{
	return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
	return 0.0;
}

// 自定义sort函数对直线排序
void sort_line(std::vector<Line*>* line)
{
	int len = line->size();
	for (int i = 0; i < len; ++i)
	{
		for (int j = i + 1; j < len; ++j)
		{
			if (line->at(i)->get_start().x > line->at(j)->get_start().x)
			{
				Line* temp = line->at(i);
				line->at(i) = line->at(j);
				line->at(j) = temp;
			}
			else if (line->at(i)->get_start().x == line->at(j)->get_start().x)
			{
				if (line->at(i)->get_start().y > line->at(j)->get_start().y)
				{
					Line* temp = line->at(i);
					line->at(i) = line->at(j);
					line->at(j) = temp;
				}
			}
		}
	}
}

// 用归并排序直线
void merge(std::vector<Line*>* line, int left, int mid, int right)
{
	int i = left;
	int j = mid + 1;
	std::vector<Line*> temp;
	while (i <= mid && j <= right)
	{
		if (line->at(i)->get_start().x < line->at(j)->get_start().x)
		{
			temp.push_back(line->at(i));
			i++;
		}
		else if (line->at(i)->get_start().x > line->at(j)->get_start().x)
		{
			temp.push_back(line->at(j));
			j++;
		}
		else
		{
			if (line->at(i)->get_start().y < line->at(j)->get_start().y)
			{
				temp.push_back(line->at(i));
				i++;
			}
			else if (line->at(i)->get_start().y > line->at(j)->get_start().y)
			{
				temp.push_back(line->at(j));
				j++;
			}
			else
			{
				if (line->at(i)->get_end().x < line->at(j)->get_end().x)
				{
					temp.push_back(line->at(i));
					i++;
				}
				else if (line->at(i)->get_end().x > line->at(j)->get_end().x)
				{
					temp.push_back(line->at(j));
					j++;
				}
				else
				{
					if (line->at(i)->get_end().y <= line->at(j)->get_end().y)
					{
						temp.push_back(line->at(i));
						i++;
					}
					else
					{
						temp.push_back(line->at(j));
						j++;
					}
				}
			}
		}
	}
	while (i <= mid)
	{
		temp.push_back(line->at(i));
		i++;
	}
	while (j <= right)
	{
		temp.push_back(line->at(j));
		j++;
	}
	for (i = left; i <= right; i++)
	{
		line->at(i) = temp[i - left];
	}
}

// 归并排序函数
void merge_sort(std::vector<Line*>* line, int left, int right)
{
	if (left < right)
	{
		int mid = left + (right - left) / 2;
		merge_sort(line, left, mid);
		merge_sort(line, mid + 1, right);
		merge(line, left, mid, right);
	}
}

// 归一化排序后的直线容器
void sort_lines(std::vector<Line*>* line)
{
	merge_sort(line, 0, line->size() - 1);
}

// 自定义unique函数对直线去重
void unique_line(std::vector<Line*>& lines)
{
	for (auto it = lines.begin(); it != lines.end();)
	{
		auto next_it = std::next(it);

		if (next_it != lines.end() &&
			(*it)->get_start().x == (*next_it)->get_start().x &&
			(*it)->get_start().y == (*next_it)->get_start().y &&
			(*it)->get_end().x == (*next_it)->get_end().x &&
			(*it)->get_end().y == (*next_it)->get_end().y)
		{
			if ((*it)->get_is_exist() && !(*next_it)->get_is_exist())	// 保留不存在的直线的数据
			{
				delete* it; // 删除指针指向的对象
				it = lines.erase(it);
			}
			else if ((*it)->get_is_exist() && (*next_it)->get_is_exist())	// 都存在，保留最新记录
			{
				delete* it;
				it = lines.erase(it);
			}
			else if (!(*it)->get_is_exist() && (*next_it)->get_is_exist())	// 保留不存在的直线的数据
			{
				delete* it; // 删除指针指向的对象
				it = lines.erase(it);
			}
			else
			{
				delete* next_it; // 删除指针指向的对象
				next_it = lines.erase(next_it);
			}
		}
		else
		{
			++it;
		}
	}
}

// 自定义sort函数对椭圆排序
void sort_ellipse(std::vector<Ellipses*>* ellipses)
{
	int len = ellipses->size();
	for (int i = 0; i < len; ++i)
	{
		for (int j = i + 1; j < len; ++j)
		{
			if (ellipses->at(i)->get_center().x > ellipses->at(j)->get_center().x)
			{
				Ellipses* temp = ellipses->at(i);
				ellipses->at(i) = ellipses->at(j);
				ellipses->at(j) = temp;
			}
			else if (ellipses->at(i)->get_center().x == ellipses->at(j)->get_center().x)
			{
				if (ellipses->at(i)->get_center().y > ellipses->at(j)->get_center().y)
				{
					Ellipses* temp = ellipses->at(i);
					ellipses->at(i) = ellipses->at(j);
					ellipses->at(j) = temp;
				}
			}
		}
	}
}

// 用归并排序椭圆
void merge(std::vector<Ellipses*>* ellipses, int left, int mid, int right)
{
	int i = left;
	int j = mid + 1;
	std::vector<Ellipses*> temp;
	while (i <= mid && j <= right)
	{
		/*if (ellipses->at(i)->get_center().x < ellipses->at(j)->get_center().x)
		{
			temp.push_back(ellipses->at(i));
			i++;
		}
		else if(ellipses->at(i)->get_center().x == ellipses->at(j)->get_center().x)
		{
			if (ellipses->at(i)->get_center().y < ellipses->at(j)->get_center().y)
			{
				temp.push_back(ellipses->at(i));
				i++;
			}
			else if(ellipses->at(i)->get_center().y == ellipses->at(j)->get_center().y)
			{
				if (ellipses->at(i)->get_radiusX() < ellipses->at(j)->get_radiusX())
				{
					temp.push_back(ellipses->at(i));
					i++;
				}
				else if (ellipses->at(i)->get_radiusX() == ellipses->at(j)->get_radiusX())
				{
					if (ellipses->at(i)->get_radiusY() < ellipses->at(j)->get_radiusY())
					{
						temp.push_back(ellipses->at(i));
						i++;
					}
					else
					{
						temp.push_back(ellipses->at(j));
						j++;
					}
				}
				else
				{
					temp.push_back(ellipses->at(j));
					j++;
				}
			}
			else
			{
				temp.push_back(ellipses->at(j));
				j++;
			}
		}
		else
		{
			temp.push_back(ellipses->at(j));
			j++;
		}*/

		// 如果椭圆i的圆心x坐标小于椭圆j的圆心x坐标
		if (ellipses->at(i)->get_center().x < ellipses->at(j)->get_center().x)
		{
			temp.push_back(ellipses->at(i));
			++i;
		}
		else if (ellipses->at(i)->get_center().x > ellipses->at(j)->get_center().x)
		{
			temp.push_back(ellipses->at(j));
			++j;
		}
		else // 如果两椭圆圆心横坐标相等
		{
			if (ellipses->at(i)->get_center().y < ellipses->at(j)->get_center().y)
			{
				temp.push_back(ellipses->at(i));
				++i;
			}
			else if (ellipses->at(i)->get_center().y > ellipses->at(j)->get_center().y)
			{
				temp.push_back(ellipses->at(j));
				++j;
			}
			else // 如果两椭圆y坐标相等
			{
				if (ellipses->at(i)->get_radiusX() < ellipses->at(j)->get_radiusX())
				{
					temp.push_back(ellipses->at(i));
					++i;
				}
				else if (ellipses->at(i)->get_radiusX() > ellipses->at(j)->get_radiusX())
				{
					temp.push_back(ellipses->at(j));
					++j;
				}
				else // 如果两椭圆x半径相等
				{
					if (ellipses->at(i)->get_radiusY() <= ellipses->at(j)->get_radiusY())
					{
						temp.push_back(ellipses->at(i));
						++i;
					}
					else
					{
						temp.push_back(ellipses->at(j));
						++j;
					}
				}
			}
		}
	}
	while (i <= mid)
	{
		temp.push_back(ellipses->at(i));
		i++;
	}
	while (j <= right)
	{
		temp.push_back(ellipses->at(j));
		j++;
	}
	for (i = left; i <= right; i++)
	{
		ellipses->at(i) = temp[i - left];
	}
}

// 归并排序函数
void merge_sort(std::vector<Ellipses*>* ellipses, int left, int right)
{
	if (left < right)
	{
		int mid = left + (right - left) / 2;
		merge_sort(ellipses, left, mid);
		merge_sort(ellipses, mid + 1, right);
		merge(ellipses, left, mid, right);
	}
}

// 归一化排序后的椭圆容器
void sort_ellipses(std::vector<Ellipses*>* ellipses)
{
	merge_sort(ellipses, 0, ellipses->size() - 1);
}

// 自定义unique函数对椭圆去重
void unique_ellipse(std::vector<Ellipses*>* ellipses)
{
	for (auto it = ellipses->begin(); it != ellipses->end();)
	{
		auto next_it = std::next(it);

		if (next_it != ellipses->end() &&
			(*it)->get_center().x == (*next_it)->get_center().x &&
			(*it)->get_center().y == (*next_it)->get_center().y &&
			(*it)->get_radiusX() == (*next_it)->get_radiusX() &&
			(*it)->get_radiusY() == (*next_it)->get_radiusY())
		{
			if ((*it)->get_is_exist() && !(*next_it)->get_is_exist())
			{
				delete* it; // 删除指针指向的对象
				it = ellipses->erase(it);
			}
			else if ((*it)->get_is_exist() && (*next_it)->get_is_exist())
			{
				delete* it;
				it = ellipses->erase(it);
			}
			else if (!(*it)->get_is_exist() && (*next_it)->get_is_exist())
			{
				delete* it; // 删除指针指向的对象
				it = ellipses->erase(it);
			}
			else
			{
				delete* next_it; // 删除指针指向的对象
				next_it = ellipses->erase(next_it);
			}

		}
		else
		{
			++it;
		}
	}
}

// 自定义unique函数对矩形去重
void unique_rectangles(std::vector<Rectangles*>* rectangles)
{
	for (auto it = rectangles->begin(); it != rectangles->end();)
	{
		auto next_it = std::next(it);

		if (next_it != rectangles->end() &&
			(*it)->get_center().x == (*next_it)->get_center().x &&
			(*it)->get_center().y == (*next_it)->get_center().y &&
			(*it)->get_width() == (*next_it)->get_width() &&
			(*it)->get_height() == (*next_it)->get_height())
		{
			if ((*it)->get_is_exist() && !(*next_it)->get_is_exist())
			{
				delete* it; // 删除指针指向的对象
				it = rectangles->erase(it);
			}
			else if ((*it)->get_is_exist() && (*next_it)->get_is_exist())	// 都存在，保留最新记录
			{
				delete* it;
				it = rectangles->erase(it);
			}
			else
			{
				delete* next_it; // 删除指针指向的对象
				next_it = rectangles->erase(next_it);
			}
		}
		else
		{
			++it;
		}
	}
}

// 用归并排序矩形
void merge(std::vector<Rectangles*>* rectangles, int left, int mid, int right)
{
	int i = left;
	int j = mid + 1;
	std::vector<Rectangles*> temp;
	while (i <= mid && j <= right)
	{
		// 依次比较left, top, right, bottom, lbleft, lbbottom, rtright, rttop这几个值的大小，从小到大排序，如果某项相同，比较下一项
		if (rectangles->at(i)->get_left() < rectangles->at(j)->get_left())
		{
			temp.push_back(rectangles->at(i));
			++i;
		}
		else if (rectangles->at(i)->get_left() > rectangles->at(j)->get_left())
		{
			temp.push_back(rectangles->at(j));
			++j;
		}
		else
		{
			if (rectangles->at(i)->get_top() < rectangles->at(j)->get_top())
			{
				temp.push_back(rectangles->at(i));
				++i;
			}
			else if (rectangles->at(i)->get_top() > rectangles->at(j)->get_top())
			{
				temp.push_back(rectangles->at(j));
				++j;
			}
			else
			{
				if (rectangles->at(i)->get_right() < rectangles->at(j)->get_right())
				{
					temp.push_back(rectangles->at(i));
					++i;
				}
				else if (rectangles->at(i)->get_right() > rectangles->at(j)->get_right())
				{
					temp.push_back(rectangles->at(j));
					++j;
				}
				else
				{
					if (rectangles->at(i)->get_bottom() < rectangles->at(j)->get_bottom())
					{
						temp.push_back(rectangles->at(i));
						++i;
					}
					else if (rectangles->at(i)->get_bottom() > rectangles->at(j)->get_bottom())
					{
						temp.push_back(rectangles->at(j));
						++j;
					}
					else
					{
						if (rectangles->at(i)->get_lbleft() < rectangles->at(j)->get_lbleft())
						{
							temp.push_back(rectangles->at(i));
							++i;
						}
						else if (rectangles->at(i)->get_lbleft() > rectangles->at(j)->get_lbleft())
						{
							temp.push_back(rectangles->at(j));
							++j;
						}
						else
						{
							if (rectangles->at(i)->get_lbbottom() < rectangles->at(j)->get_lbbottom())
							{
								temp.push_back(rectangles->at(i));
								++i;
							}
							else if (rectangles->at(i)->get_lbbottom() > rectangles->at(j)->get_lbbottom())
							{
								temp.push_back(rectangles->at(j));
								++j;
							}
							else
							{
								if (rectangles->at(i)->get_rtright() < rectangles->at(j)->get_rtright())
								{
									temp.push_back(rectangles->at(i));
									++i;
								}
								else if (rectangles->at(i)->get_rtright() > rectangles->at(j)->get_rtright())
								{
									temp.push_back(rectangles->at(j));
									++j;
								}
								else
								{
									if (rectangles->at(i)->get_rttop() <= rectangles->at(j)->get_rttop())
									{
										temp.push_back(rectangles->at(i));
										++i;
									}
									else
									{
										temp.push_back(rectangles->at(j));
										++j;
									}

								}
							}
						}
					}
				}
			}
		}
	}
	while (i <= mid)
	{
		temp.push_back(rectangles->at(i));
		i++;
	}
	while (j <= right)
	{
		temp.push_back(rectangles->at(j));
		j++;
	}
	for (i = left; i <= right; i++)
	{
		rectangles->at(i) = temp[i - left];
	}
}

// 归并排序函数
void merge_sort(std::vector<Rectangles*>* rectangles, int left, int right)
{
	if (left < right)
	{
		int mid = left + (right - left) / 2;
		merge_sort(rectangles, left, mid);
		merge_sort(rectangles, mid + 1, right);
		merge(rectangles, left, mid, right);
	}
}

// 归一化排序后的矩形容器
void sort_rectangles(std::vector<Rectangles*>* rectangles)
{
	merge_sort(rectangles, 0, rectangles->size() - 1);
}

// 自定义sort函数对贝塞尔曲线排序
void sort_bezier(std::vector<Bezier*>* bezier)
{
	int len = bezier->size();
	for (int i = 0; i < len; ++i)
	{
		for (int j = i + 1; j < len; ++j)
		{
			if (bezier->at(i)->get_d2d_point1().x > bezier->at(j)->get_d2d_point1().x)
			{
				Bezier* temp = bezier->at(i);
				bezier->at(i) = bezier->at(j);
				bezier->at(j) = temp;
			}
			else if (bezier->at(i)->get_d2d_point1().x == bezier->at(j)->get_d2d_point1().x)
			{
				if (bezier->at(i)->get_d2d_point1().y > bezier->at(j)->get_d2d_point1().y)
				{
					Bezier* temp = bezier->at(i);
					bezier->at(i) = bezier->at(j);
					bezier->at(j) = temp;
				}
				else if ((bezier->at(i)->get_d2d_point1().y == bezier->at(j)->get_d2d_point1().y))
				{
					if (bezier->at(i)->get_d2d_point2().x > bezier->at(j)->get_d2d_point2().x)
					{
						Bezier* temp = bezier->at(i);
						bezier->at(i) = bezier->at(j);
						bezier->at(j) = temp;
					}
					else if (bezier->at(i)->get_d2d_point2().x == bezier->at(j)->get_d2d_point2().x)
					{
						if (bezier->at(i)->get_d2d_point2().y > bezier->at(j)->get_d2d_point2().y)
						{
							Bezier* temp = bezier->at(i);
							bezier->at(i) = bezier->at(j);
							bezier->at(j) = temp;
						}
						else if (bezier->at(i)->get_d2d_point2().y == bezier->at(j)->get_d2d_point2().y)
						{
							if (bezier->at(i)->get_d2d_point3().x > bezier->at(j)->get_d2d_point3().x)
							{
								Bezier* temp = bezier->at(i);
								bezier->at(i) = bezier->at(j);
								bezier->at(j) = temp;
							}
							else if (bezier->at(i)->get_d2d_point3().x == bezier->at(j)->get_d2d_point3().x)
							{
								if (bezier->at(i)->get_d2d_point3().y > bezier->at(j)->get_d2d_point3().y)
								{
									Bezier* temp = bezier->at(i);
									bezier->at(i) = bezier->at(j);
									bezier->at(j) = temp;
								}
								else if (bezier->at(i)->get_d2d_point3().y == bezier->at(j)->get_d2d_point3().y)
								{
									if (bezier->at(i)->get_d2d_point4().x > bezier->at(j)->get_d2d_point4().x)
									{
										Bezier* temp = bezier->at(i);
										bezier->at(i) = bezier->at(j);
										bezier->at(j) = temp;
									}
									else if (bezier->at(i)->get_d2d_point4().x == bezier->at(j)->get_d2d_point4().x)
									{
										if (bezier->at(i)->get_d2d_point4().y > bezier->at(j)->get_d2d_point4().y)
										{
											Bezier* temp = bezier->at(i);
											bezier->at(i) = bezier->at(j);
											bezier->at(j) = temp;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

// 用归并排序贝塞尔曲线
void merge(std::vector<Bezier*>* bezier, int left, int mid, int right)
{
	int i = left;
	int j = mid + 1;
	std::vector<Bezier*> temp;

	while (i <= mid && j <= right)
	{
		Bezier* bezier_i = bezier->at(i);
		Bezier* bezier_j = bezier->at(j);

		for (int k = 1; k <= 4; ++k)
		{
			D2D1_POINT_2F point_i;
			D2D1_POINT_2F point_j;

			switch (k)
			{
				case 1:
				{
					point_i = bezier_i->get_d2d_point1();
					point_j = bezier_j->get_d2d_point1();
					break;
				}
				case 2:
				{
					point_i = bezier_i->get_d2d_point2();
					point_j = bezier_j->get_d2d_point2();
					break;
				}
				case 3:
				{
					point_i = bezier_i->get_d2d_point3();
					point_j = bezier_j->get_d2d_point3();
					break;
				}
				case 4:
				{
					point_i = bezier_i->get_d2d_point4();
					point_j = bezier_j->get_d2d_point4();
					break;
				}

			}

			if (point_i.x < point_j.x || (point_i.x == point_j.x && point_i.y < point_j.y))
			{
				temp.push_back(bezier_i);
				i++;
				break;
			}
			else if (point_i.x > point_j.x || (point_i.x == point_j.x && point_i.y > point_j.y))
			{
				temp.push_back(bezier_j);
				j++;
				break;
			}
		}
	}

	while (i <= mid)
	{
		temp.push_back(bezier->at(i));
		i++;
	}

	while (j <= right)
	{
		temp.push_back(bezier->at(j));
		j++;
	}

	for (i = left; i <= right; i++)
	{
		bezier->at(i) = temp[i - left];
	}
}

// 归并排序函数
void merge_sort(std::vector<Bezier*>* bezier, int left, int right)
{
	if (left < right)
	{
		int mid = left + (right - left) / 2;
		merge_sort(bezier, left, mid);
		merge_sort(bezier, mid + 1, right);
		merge(bezier, left, mid, right);
	}
}

// 
void sort_beziers(std::vector<Bezier*>* bezier)
{
	merge_sort(bezier, 0, bezier->size() - 1);
}	

// 自定义unique函数对贝塞尔曲线去重
void unique_bezier(std::vector<Bezier*>* bezier)
{
	for (auto it = bezier->begin(); it != bezier->end();)
	{
		auto next_it = std::next(it);

		if (next_it != bezier->end() &&
			(*it)->get_d2d_point1().x == (*next_it)->get_d2d_point1().x &&
			(*it)->get_d2d_point1().y == (*next_it)->get_d2d_point1().y &&
			(*it)->get_d2d_point2().x == (*next_it)->get_d2d_point2().x &&
			(*it)->get_d2d_point2().y == (*next_it)->get_d2d_point2().y &&
			(*it)->get_d2d_point3().x == (*next_it)->get_d2d_point3().x &&
			(*it)->get_d2d_point3().y == (*next_it)->get_d2d_point3().y &&
			(*it)->get_d2d_point4().x == (*next_it)->get_d2d_point4().x &&
			(*it)->get_d2d_point4().y == (*next_it)->get_d2d_point4().y)
		{
			if ((*it)->get_is_exist() && !(*next_it)->get_is_exist())
			{
				delete* it; // 删除指针指向的对象
				it = bezier->erase(it);
			}
			else
			{
				delete* next_it; // 删除指针指向的对象
				next_it = bezier->erase(next_it);
			}
		}
		else
		{
			++it;
		}
	}
}

// 自定义sort函数对BLine排序
void sort_bline(std::vector<BLine*>* bline)
{
	int len = bline->size();
	for (int i = 0; i < len; ++i)
	{
		for (int j = i + 1; j < len; ++j)
		{
			if (bline->at(i)->get_d2d_point1().x > bline->at(j)->get_d2d_point1().x)
			{
				BLine* temp = bline->at(i);
				bline->at(i) = bline->at(j);
				bline->at(j) = temp;
			}
			else if (bline->at(i)->get_d2d_point1().x == bline->at(j)->get_d2d_point1().x)
			{
				if (bline->at(i)->get_d2d_point1().y > bline->at(j)->get_d2d_point1().y)
				{
					BLine* temp = bline->at(i);
					bline->at(i) = bline->at(j);
					bline->at(j) = temp;
				}
				else if ((bline->at(i)->get_d2d_point1().y == bline->at(j)->get_d2d_point1().y))
				{
					if (bline->at(i)->get_d2d_point2().x > bline->at(j)->get_d2d_point2().x)
					{
						BLine* temp = bline->at(i);
						bline->at(i) = bline->at(j);
						bline->at(j) = temp;
					}
					else if (bline->at(i)->get_d2d_point2().x == bline->at(j)->get_d2d_point2().x)
					{
						if (bline->at(i)->get_d2d_point2().y > bline->at(j)->get_d2d_point2().y)
						{
							BLine* temp = bline->at(i);
							bline->at(i) = bline->at(j);
							bline->at(j) = temp;
						}
						else if (bline->at(i)->get_d2d_point2().y == bline->at(j)->get_d2d_point2().y)
						{
							if (bline->at(i)->get_d2d_point3().x > bline->at(j)->get_d2d_point3().x)
							{
								BLine* temp = bline->at(i);
								bline->at(i) = bline->at(j);
								bline->at(j) = temp;
							}
							else if (bline->at(i)->get_d2d_point3().x == bline->at(j)->get_d2d_point3().x)
							{
								if (bline->at(i)->get_d2d_point3().y > bline->at(j)->get_d2d_point3().y)
								{
									BLine* temp = bline->at(i);
									bline->at(i) = bline->at(j);
									bline->at(j) = temp;
								}
								else if (bline->at(i)->get_d2d_point3().y == bline->at(j)->get_d2d_point3().y)
								{
									if (bline->at(i)->get_d2d_point4().x > bline->at(j)->get_d2d_point4().x)
									{
										BLine* temp = bline->at(i);
										bline->at(i) = bline->at(j);
										bline->at(j) = temp;
									}
									else if (bline->at(i)->get_d2d_point4().x == bline->at(j)->get_d2d_point4().x)
									{
										if (bline->at(i)->get_d2d_point4().y > bline->at(j)->get_d2d_point4().y)
										{
											BLine* temp = bline->at(i);
											bline->at(i) = bline->at(j);
											bline->at(j) = temp;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}


// 用归并排序BLine
void merge(std::vector<BLine*>* bline, int left, int mid, int right)
{
	int i = left;
	int j = mid + 1;
	std::vector<BLine*> temp;
	while (i <= mid && j <= right)
	{
		if (bline->at(i)->get_d2d_point1().x <= bline->at(j)->get_d2d_point1().x)
		{
			temp.push_back(bline->at(i));
			i++;
		}
		else
		{
			temp.push_back(bline->at(j));
			j++;
		}
	}
	while (i <= mid)
	{
		temp.push_back(bline->at(i));
		i++;
	}
	while (j <= right)
	{
		temp.push_back(bline->at(j));
		j++;
	}
	for (i = left; i <= right; i++)
	{
		bline->at(i) = temp[i - left];
	}
}

// 归并排序函数
void merge_sort(std::vector<BLine*>* bline, int left, int right)
{
	if (left < right)
	{
		int mid = left + (right - left) / 2;
		merge_sort(bline, left, mid);
		merge_sort(bline, mid + 1, right);
		merge(bline, left, mid, right);
	}
}

// 归一化排序后的BLine容器
void sort_blines(std::vector<BLine*>* bline)
{
	merge_sort(bline, 0, bline->size() - 1);
}

// 自定义unique函数对贝塞尔曲线去重
void unique_bline(std::vector<BLine*>* bline)
{
	for (auto it = bline->begin(); it != bline->end();)
	{
		auto next_it = std::next(it);

		if (next_it != bline->end() &&
			(*it)->get_d2d_point1().x == (*next_it)->get_d2d_point1().x &&
			(*it)->get_d2d_point1().y == (*next_it)->get_d2d_point1().y &&
			(*it)->get_d2d_point2().x == (*next_it)->get_d2d_point2().x &&
			(*it)->get_d2d_point2().y == (*next_it)->get_d2d_point2().y &&
			(*it)->get_d2d_point3().x == (*next_it)->get_d2d_point3().x &&
			(*it)->get_d2d_point3().y == (*next_it)->get_d2d_point3().y &&
			(*it)->get_d2d_point4().x == (*next_it)->get_d2d_point4().x &&
			(*it)->get_d2d_point4().y == (*next_it)->get_d2d_point4().y)
		{
			if ((*it)->get_is_exist() && !(*next_it)->get_is_exist())
			{
				delete* it; // 删除指针指向的对象
				it = bline->erase(it);
			}
			else if ((*it)->get_is_exist() && (*next_it)->get_is_exist())
			{
				delete* it;
				it = bline->erase(it);
			}
			else if (!(*it)->get_is_exist() && (*next_it)->get_is_exist())
			{
				delete* it; // 删除指针指向的对象
				it = bline->erase(it);
			}
			else
			{
				delete* next_it; // 删除指针指向的对象
				next_it = bline->erase(next_it);
			}
		}
		else
		{
			++it;
		}
	}
}

// 自定义sort函数对多段线排序
void merge(std::vector<Polylines*>* polylines, int left, int mid, int right)
{
	int i = left;
	int j = mid + 1;
	std::vector<Polylines*> temp;
	while (i <= mid && j <= right)
	{
		if (polylines->at(i)->get_points()[0].x <= polylines->at(j)->get_points()[0].x)
		{
			temp.push_back(polylines->at(i));
			i++;
		}
		else
		{
			temp.push_back(polylines->at(j));
			j++;
		}
	}
	while (i <= mid)
	{
		temp.push_back(polylines->at(i));
		i++;
	}
	while (j <= right)
	{
		temp.push_back(polylines->at(j));
		j++;
	}
	for (i = left; i <= right; i++)
	{
		polylines->at(i) = temp[i - left];
	}
}

// 归并排序函数  
void merge_sort(std::vector<Polylines*>* polylines, int left, int right)
{
	if (left < right)
	{
		int mid = left + (right - left) / 2;
		merge_sort(polylines, left, mid);
		merge_sort(polylines, mid + 1, right);
		merge(polylines, left, mid, right);
	}
}

// 归一化排序后的多段线容器  
void sort_polyline(std::vector<Polylines*>* polylines)
{
	merge_sort(polylines, 0, polylines->size() - 1);
}

// 将每个polylines对象的points数组中的点去重，如果当前点与上一个点相同，则删除当前点
void unique_polypoints(std::vector<Polylines*>* polylines)
{
	for (auto it = polylines->begin(); it != polylines->end(); ++it)
	{
		auto points = (*it)->get_points();
		for (auto it2 = points.begin(); it2 != points.end();)
		{
			auto next_it = std::next(it2);
			if (next_it != points.end() &&
				(*it2).x == (*next_it).x &&
				(*it2).y == (*next_it).y)
			{
				it2 = points.erase(it2);
			}
			else
			{
				++it2;
			}
		}
	}
}

// 自定义unique函数对多段线去重
void unique_polyline(std::vector<Polylines*>* polylines)
{
	for (auto it = polylines->begin(); it != polylines->end();)
	{
		auto next_it = std::next(it);

		if (next_it != polylines->end() &&
			(*it)->get_points()[0].x == (*next_it)->get_points()[0].x &&
			(*it)->get_points()[0].y == (*next_it)->get_points()[0].y &&
			(*it)->get_points()[1].x == (*next_it)->get_points()[1].x &&
			(*it)->get_points()[1].y == (*next_it)->get_points()[1].y)
		{
			if ((*it)->get_is_exist() && !(*next_it)->get_is_exist())
			{
				delete* it; // 删除指针指向的对象
				it = polylines->erase(it);
			}
			else if ((*it)->get_is_exist() && (*next_it)->get_is_exist())	// 都存在，保留最新记录
			{
				delete* it;
				it = polylines->erase(it);
			}
			else if (!(*it)->get_is_exist() && (*next_it)->get_is_exist())
			{
				delete* it; // 删除指针指向的对象
				it = polylines->erase(it);
			}
			else
			{
				delete* next_it; // 删除指针指向的对象
				next_it = polylines->erase(next_it);
			}
		}
		else
		{
			++it;
		}
	}
}

// 自定义sort函数对圆排序
void sort_circle(std::vector<Circle*>* circle)
{
	int len = circle->size();
	for (int i = 0; i < len; ++i)
	{
		for (int j = i + 1; j < len; ++j)
		{
			if (circle->at(i)->get_center().x > circle->at(j)->get_center().x)
			{
				Circle* temp = circle->at(i);
				circle->at(i) = circle->at(j);
				circle->at(j) = temp;
			}
			else if (circle->at(i)->get_center().x == circle->at(j)->get_center().x)
			{
				if (circle->at(i)->get_center().y > circle->at(j)->get_center().y)
				{
					Circle* temp = circle->at(i);
					circle->at(i) = circle->at(j);
					circle->at(j) = temp;
				}
			}
		}
	}
}

// 用归并排序圆
void merge(std::vector<Circle*>* circle, int left, int mid, int right)
{
	int i = left;
	int j = mid + 1;
	std::vector<Circle*> temp;
	while (i <= mid && j <= right)
	{
		if (circle->at(i)->get_center().x <= circle->at(j)->get_center().x)
		{
			temp.push_back(circle->at(i));
			i++;
		}
		else
		{
			temp.push_back(circle->at(j));
			j++;
		}
	}
	while (i <= mid)
	{
		temp.push_back(circle->at(i));
		i++;
	}
	while (j <= right)
	{
		temp.push_back(circle->at(j));
		j++;
	}
	for (i = left; i <= right; i++)
	{
		circle->at(i) = temp[i - left];
	}
}

// 归并排序函数
void merge_sort(std::vector<Circle*>* circle, int left, int right)
{
	if (left < right)
	{
		int mid = left + (right - left) / 2;
		merge_sort(circle, left, mid);
		merge_sort(circle, mid + 1, right);
		merge(circle, left, mid, right);
	}
}

// 归一化排序后的圆容器
void sort_circles(std::vector<Circle*>* circle)
{
	merge_sort(circle, 0, circle->size() - 1);
}

// 自定义unique函数对圆去重
void unique_circle(std::vector<Circle*>* circle)
{
	for (auto it = circle->begin(); it != circle->end();)
	{
		auto next_it = std::next(it);

		if (next_it != circle->end() &&
			(*it)->get_center().x == (*next_it)->get_center().x &&
			(*it)->get_center().y == (*next_it)->get_center().y &&
			(*it)->get_radius() == (*next_it)->get_radius())
		{
			if ((*it)->get_is_exist() && !(*next_it)->get_is_exist())
			{
				delete* it; // 删除指针指向的对象
				it = circle->erase(it);
			}
			else if ((*it)->get_is_exist() && (*next_it)->get_is_exist())	// 都存在，保留最新记录
			{
				delete* it;
				it = circle->erase(it);
			}
			else if (!(*it)->get_is_exist() && (*next_it)->get_is_exist())
			{
				delete* it; // 删除指针指向的对象
				it = circle->erase(it);
			}
			else
			{
				delete* next_it; // 删除指针指向的对象
				next_it = circle->erase(next_it);
			}
		}
		else
		{
			++it;
		}
	}
}

Melon::Melon() :
	m_hwnd(nullptr),
	m_pDirect2dFactory(nullptr),
	m_pRenderTarget(nullptr),
	m_pBlackBrush(nullptr),
	m_pWhiteBrush(nullptr),
	m_pRedBrush(nullptr),
	m_pLightSlateGrayBrush(nullptr),
	m_pBLineBrush(nullptr),
	m_pFillBrush(nullptr),
	m_pStrokeStyle(nullptr),
	m_pDWriteFactory(nullptr),
	m_pTextFormat(nullptr),
	lines(0)
{}

Melon::~Melon()
{
	SafeRelease(&m_pDirect2dFactory);
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pBlackBrush);
	SafeRelease(&m_pWhiteBrush);
	SafeRelease(&m_pRedBrush);
	SafeRelease(&m_pLightSlateGrayBrush);
	SafeRelease(&m_pBLineBrush);
	SafeRelease(&m_pFillBrush);
	SafeRelease(&m_pStrokeStyle);
	SafeRelease(&m_pDWriteFactory);
	SafeRelease(&m_pTextFormat);

	for (auto l : lines)
	{
		delete l;
	}

	for (auto e : ellipses)
	{
		delete e;
	}

	for (auto r : rectangles)
	{
		delete r;
	}

	for (auto b : bezier)
	{
		delete b;
	}

	for (auto p : polylines)
	{
		delete p;
	}

	for (auto c : circles)
	{
		delete c;
	}

	for (auto b : blines)
	{
		delete b;
	}
}

HRESULT Melon::Initialize()
{
	HRESULT res = CreateDeviceIndependentResources();

	if (SUCCEEDED(res))
	{
		HMENU hmenu = LoadMenu(HINST_THISCOMPONENT, MAKEINTRESOURCE(IDR_MENU1));	// 加载菜单资源

		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };	// 初始化结构体
		wcex.style = CS_HREDRAW | CS_VREDRAW;		// 窗口样式为水平和垂直都重绘
		wcex.lpfnWndProc = Melon::WndProc;			// 消息处理函数
		wcex.cbClsExtra = 0;						// 类扩展
		wcex.cbWndExtra = sizeof(LONG_PTR);			// 窗口扩展
		wcex.hInstance = HINST_THISCOMPONENT;		// 窗口实例句柄
		wcex.hbrBackground = nullptr;				// 背景画刷
		wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);			// 菜单资源
		wcex.hIcon = LoadIcon(HINST_THISCOMPONENT, MAKEINTRESOURCE(IDI_ICON1));	// 图标
		wcex.hCursor = LoadCursor(nullptr, IDI_APPLICATION);	// 加载光标
		wcex.lpszClassName = L"Melon";				// 窗口类名

		RegisterClassEx(&wcex);	// 注册窗口类

		// 创建窗口
		m_hwnd = CreateWindow(
			L"Melon",					// 窗口类名
			L"Melon",					// 窗口标题
			WS_OVERLAPPEDWINDOW,		// 窗口样式
			CW_USEDEFAULT,				// 窗口左上角x坐标
			CW_USEDEFAULT,				// 窗口左上角y坐标
			0,							// 窗口宽度
			0,							// 窗口高度
			nullptr,					// 父窗口句柄
			nullptr,					// 菜单句柄
			HINST_THISCOMPONENT,		// 窗口实例句柄
			this						// 窗口参数
		);

		if (m_hwnd)
		{
			float dpi = GetDpiForWindow(m_hwnd);	// 获取窗口dpi

			// 调整窗口大小
			SetWindowPos(
				m_hwnd,
				nullptr,
				0,
				0,
				static_cast<int>(ceil(1200.f * dpi / 96.f)),	// 16:10的屏幕显示
				static_cast<int>(ceil(750.f * dpi / 96.f)),
				SWP_NOMOVE
			);

			ShowWindow(m_hwnd, SW_SHOWNORMAL);	// 显示窗口
			UpdateWindow(m_hwnd);				// 更新窗口
		}
	}

	return res;
}

void Melon::RunMessageLoop()
{
	MSG message;

	while (GetMessage(&message, nullptr, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

HRESULT Melon::CreateDeviceIndependentResources()
{
	// 单线程模式
	HRESULT res = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

	return res;
}

HRESULT Melon::CreateDeviceResources()
{
	HRESULT res = S_OK;

	if (!m_pRenderTarget)
	{
		RECT rc;	//	客户区矩形
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);	// 客户区大小

		// 创建渲染目标
		res = m_pDirect2dFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hwnd, size),
			&m_pRenderTarget
		);

		//// 创建虚线线型
		//if (SUCCEEDED(res))
		//{
		//	res = m_pDirect2dFactory->CreateStrokeStyle(
		//		D2D1::StrokeStyleProperties(
		//			D2D1_CAP_STYLE_ROUND,	// 线帽样式
		//			D2D1_CAP_STYLE_ROUND,	// 线帽样式
		//			D2D1_CAP_STYLE_ROUND,	// 线帽样式
		//			D2D1_LINE_JOIN_MITER,	// 线段连接样式
		//			10.0f,					// 断点样式
		//			D2D1_DASH_STYLE_CUSTOM,	// 线型样式
		//			0.0f					// 断点偏移量
		//		),
		//		nullptr,
		//		0,
		//		&m_pStrokeStyle
		//	);
		//}

		D2D1_STROKE_STYLE_PROPERTIES strokeStyleProperties = D2D1::StrokeStyleProperties(
			D2D1_CAP_STYLE_SQUARE,  // The start cap.
			D2D1_CAP_STYLE_SQUARE,  // The end cap.
			D2D1_CAP_STYLE_TRIANGLE, // The dash cap.
			D2D1_LINE_JOIN_MITER, // The line join.
			10.0f, // The miter limit.
			D2D1_DASH_STYLE_DASH, // The dash style.
			100.0f // The dash offset.
		);

		res = m_pDirect2dFactory->CreateStrokeStyle(strokeStyleProperties, NULL, 0, &m_pStrokeStyle);

		// 创建黑色画刷.
		if (SUCCEEDED(res))
		{
			res = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Black),
				&m_pBlackBrush
			);
		}

		// 创建白色画刷
		if (SUCCEEDED(res))
		{
			res = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::White),
				&m_pWhiteBrush
			);
		}

		// 创建红色画刷
		if (SUCCEEDED(res))
		{
			res = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::Red),
				&m_pRedBrush
			);
		}

		// 创建浅灰色画刷
		if (SUCCEEDED(res))
		{
			res = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::LightSlateGray),
				&m_pLightSlateGrayBrush
			);
		}

		// 创建蓝色画刷
		if (SUCCEEDED(res))
		{
			res = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
				&m_pBLineBrush
			);
		}

		// 创建填充画刷
		if (SUCCEEDED(res))
		{
			res = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(0x717BCE),
				&m_pFillBrush
			);
		}

		// create dwrite factory
		DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&m_pDWriteFactory));

		// create text format
		if (SUCCEEDED(res))
		{
			res = m_pDWriteFactory->CreateTextFormat(
				L"Consolas",
				NULL,
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				15.0f,
				L"en-US",
				&m_pTextFormat
			);
		}
	}

	return res;
}

void Melon::DiscardDeviceResources()
{
	SafeRelease(&m_pRenderTarget);
}

HRESULT Melon::OnRender()
{
	HRESULT res = CreateDeviceResources();

	if (SUCCEEDED(res))
	{
		m_pRenderTarget->BeginDraw();	// 开始绘制

		m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));	// 清屏

		D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();	// 获取渲染目标大小
		// 绘制图形

		POINT pt_mouse;
		GetCursorPos(&pt_mouse);
		ScreenToClient(m_hwnd, (LPPOINT)&pt_mouse);

		COLORREF color = GetPixel(GetDC(m_hwnd), 150, 150);

		// 在绘图区底部绘制当前图形对象及鼠标坐标
		{
			std::string cur_objects[12] = { "Line", "Ellipse", "Rectangle", "Bezier", "Polyline", "Circle", "Fill", "BLine", "Translate", "Rotate", "Zoom", "Clip"};
			std::string s;
			switch (state)
			{
			case LINE:
				s = cur_objects[0];
				break;
			case ELLIPSE:
				s = cur_objects[1];
				break;
			case RECTANGLE:
				s = cur_objects[2];
				break;
			case BEZIER:
				s = cur_objects[3];
				break;
			case POLYLINE:
				s = cur_objects[4];
				break;
			case CIRCLE:
				s = cur_objects[5];
				break;
			case FILL:
				s = cur_objects[6];
				break;
			case BLINE:
				s = cur_objects[7];
				break;
			case TRANSLATE:
				s = cur_objects[8];
				break;
			case ROTATE:
				s = cur_objects[9];
				break;
			case ZOOM:
				s = cur_objects[10];
				break;
			case CLIP:
				s = cur_objects[11];
				break;
			default:
				break;
			}

			std::string color_str = std::to_string(color);

			std::string str = "Mouse: (x:" + std::to_string(pt_mouse.x) + ", y:" + std::to_string(pt_mouse.y) + ")" + "\t\tStatus: " + s;
			std::wstring widestr = std::wstring(str.begin(), str.end());
			const wchar_t* szText = widestr.c_str();

			m_pRenderTarget->DrawText(
				szText,
				wcslen(szText),
				m_pTextFormat,
				D2D1::RectF(5, rtSize.height - 20, rtSize.width / 2, rtSize.height),
				m_pBlackBrush
			);
		}

		// draw_4d_bezier(100, 100, 300, 400, 600, 500, 900, 200, 1000, 500);

		//是否显示栅格
		if (is_show_grid)
		{
			// Draw a grid background.
			int width = static_cast<int>(rtSize.width);
			int height = static_cast<int>(rtSize.height);

			for (int x = 0; x < width; x += 10)
			{
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
					D2D1::Point2F(static_cast<FLOAT>(x), rtSize.height - 40),
					m_pLightSlateGrayBrush,
					0.3f
				);
			}

			for (int y = 0; y < height - 40; y += 10)
			{
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(0.0f, static_cast<FLOAT>(y)),
					D2D1::Point2F(rtSize.width, static_cast<FLOAT>(y)),
					m_pLightSlateGrayBrush,
					0.3f
				);
			}
		}

		if (is_clean)
		{
			// 清空所有.txt文件
			std::fstream line_file("line.txt", std::ios::ate | std::ios::out);
			line_file.close();

			std::fstream ellipse_file("ellipse.txt", std::ios::ate | std::ios::out);
			ellipse_file.close();

			std::fstream rectangle_file("rectangle.txt", std::ios::ate | std::ios::out);
			rectangle_file.close();

			std::fstream bezier_file("bezier.txt", std::ios::ate | std::ios::out);
			bezier_file.close();

			std::fstream polyline_file("polyline.txt", std::ios::ate | std::ios::out);
			polyline_file.close();

			std::fstream circle_file("circle.txt", std::ios::ate | std::ios::out);
			circle_file.close();

			std::fstream bline_file("bline.txt", std::ios::ate | std::ios::out);
			bline_file.close();

			m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

			is_clean = false;
		}

		int dx = pt_mouse.x - old_point.x;
		int dy = pt_mouse.y - old_point.y;
		// factor为鼠标到old_point的距离与200的比值
		float factor = sqrt(pow(dx, 2) + pow(dy, 2)) / 200.0f;

		float x1, y1, x2, y2;
		int _type;
		bool exist;
		{
			// 定义文件流对象打开"line.txt"
			std::fstream line_file("line.txt", std::ios::ate | std::ios::in);
			if (!line_file.is_open())
			{
				exit(0);
			}

			line_file.clear();
			line_file.seekg(0, std::ios::beg);	// 定位到文件开头

			lines.clear();

			while (line_file >> exist >> _type >> x1 >> y1 >> x2 >> y2)
			{
				Line* l = new Line(exist, _type, x1, y1, x2, y2);

				lines.push_back(l);
			}

			// 对lines进行排序
			sort_lines(&lines);

			// 对lines进行去重
			unique_line(lines);

			for (auto l : lines)
			{
				if (l->get_is_exist())
				{
					if (l->get_type() == 0)
						l->draw(m_pRenderTarget, m_pBlackBrush, 0.5f);
					else if (l->get_type() == 1)
					{
						Bresenham_line(l->get_start().x, l->get_start().y, l->get_end().x, l->get_end().y);
					}
				}
			}

			for (auto l : lines)
			{
				if (l->get_is_exist() && l->is_selected(pt_mouse.x, pt_mouse.y))
				{
					// 使得鼠标移动到直线上时鼠标变成十字形状
					SetCursor(LoadCursor(NULL, IDC_CROSS));
					if (signal_rclk_mouse && l->get_is_exist())
					{
						// 鼠标右键点击时更改鼠标样式，直线消失
						SetCursor(LoadCursor(NULL, IDC_WAIT));

						if (state == TRANSLATE)
						{

						}
						else if (state == ROTATE)
						{

						}
						else if (state == ZOOM)
						{

						}
						else if (state == CLIP)
						{

						}
						else
							l->set_is_exist(false);
					}
				}
			}

			for (auto l : lines)
			{
				if(l->get_is_exist() && l->is_selected(pt_mouse.x, pt_mouse.y) && flag == 1)
				{
					if (state == TRANSLATE && translating != 16)
					{
						if (translating == 0)
						{
							translating = 16;
							index_l = std::distance(lines.begin(), std::find(lines.begin(), lines.end(), l));

							break;
						}
					}

					if (state == ROTATE && rotating != 16)
					{
						if (rotating == 0)
						{
							rotating = 16;
							index_l = std::distance(lines.begin(), std::find(lines.begin(), lines.end(), l));

							break;
						}
					}

					if (state == ZOOM && zooming != 16)
					{
						if (zooming == 0)
						{
							zooming = 16;
							index_l = std::distance(lines.begin(), std::find(lines.begin(), lines.end(), l));

							break;
						}
					}
				}
			}

			if (state == TRANSLATE && translating == 16 && !lines.empty())
			{
				if (flag == 1)
				{
					for(int i = 0; i < 40; ++i)
						lines[index_l]->draw(m_pRenderTarget, m_pWhiteBrush, 1.5f);

					lines[index_l]->set_is_exist(false);
					lines[index_l]->translate(dx, dy);

					lines[index_l]->draw(m_pRenderTarget, m_pBlackBrush, 0.5f);
				}
				if (is_translate_finished)
				{
					for (int i = 0; i < 40; ++i)
						lines[index_l]->draw(m_pRenderTarget, m_pWhiteBrush, 1.5f);

					std::ofstream out_line_file("line.txt", std::ios::app | std::ios::out);

					out_line_file << "0" << " " << lines[index_l]->get_type() << " " << lines[index_l]->get_start().x << " " << lines[index_l]->get_start().y << " " << lines[index_l]->get_end().x << " " << lines[index_l]->get_end().y << std::endl;

					out_line_file << "1" << " " << lines[index_l]->get_type() << " " << lines[index_l]->get_start().x + dx << " " << lines[index_l]->get_start().y + dy << " " << lines[index_l]->get_end().x + dx << " " << lines[index_l]->get_end().y + dy << std::endl;

					out_line_file.close();

					is_translate_finished = false;
					translating = 0;
					index_l = 0;
				}
			}

			if (state == ROTATE && rotating == 16 && !lines.empty())
			{
				if (flag == 1)
				{
					for (int i = 0; i < 40; ++i)
						lines[index_l]->draw(m_pRenderTarget, m_pWhiteBrush, 1.5f);

					lines[index_l]->set_is_exist(false);
					lines[index_l]->rotate(old_point, pt_mouse);

					// 用红心着重绘制
					D2D1_ELLIPSE tmp_e = D2D1::Ellipse(D2D1::Point2F(old_point.x, old_point.y), 2, 2);
					m_pRenderTarget->DrawEllipse(
						tmp_e,
						m_pRedBrush,
						1.5f
					);
					m_pRenderTarget->FillEllipse(tmp_e, m_pRedBrush);

					lines[index_l]->draw(m_pRenderTarget, m_pBlackBrush, 0.5f);
				}

				if (is_rotate_finished)
				{
					for (int i = 0; i < 40; ++i)
						lines[index_l]->draw(m_pRenderTarget, m_pWhiteBrush, 1.5f);

					std::ofstream out_line_file("line.txt", std::ios::app | std::ios::out);

					out_line_file << "0" << " " << lines[index_l]->get_type() << " " << lines[index_l]->get_start().x << " " << lines[index_l]->get_start().y << " " << lines[index_l]->get_end().x << " " << lines[index_l]->get_end().y << std::endl;

					POINT tmp_start, tmp_end;
					tmp_start = this->rotate(lines[index_l]->get_start(), old_point, pt_mouse);
					tmp_end = this->rotate(lines[index_l]->get_end(), old_point, pt_mouse);

					out_line_file << "1" << " " << lines[index_l]->get_type() << " " << tmp_start.x << " " << tmp_start.y << " " << tmp_end.x << " " << tmp_end.y << std::endl;

					out_line_file.close();

					is_rotate_finished = false;
					rotating = 0;
					index_l = 0;
				}
			}

			if(state == ZOOM && zooming == 16 && !lines.empty())
			{
				if (flag == 1)
				{
					for (int i = 0; i < 40; ++i)
						lines[index_l]->draw(m_pRenderTarget, m_pWhiteBrush, 1.5f);

					lines[index_l]->set_is_exist(false);
					lines[index_l]->zoom(factor);

					// 用红色线绘制oldpoint到pt_mouse的线段
					m_pRenderTarget->DrawLine(
						D2D1::Point2F(old_point.x, old_point.y),
						D2D1::Point2F(pt_mouse.x, pt_mouse.y),
						m_pRedBrush,
						1.5f,
						m_pStrokeStyle
					);

					lines[index_l]->draw(m_pRenderTarget, m_pBlackBrush, 0.5f);
				}

				if (is_zoom_finished)
				{
					for (int i = 0; i < 40; ++i)
						lines[index_l]->draw(m_pRenderTarget, m_pWhiteBrush, 1.5f);

					std::ofstream out_line_file("line.txt", std::ios::app | std::ios::out);

					out_line_file << "0" << " " << lines[index_l]->get_type() << " " << lines[index_l]->get_start().x << " " << lines[index_l]->get_start().y << " " << lines[index_l]->get_end().x << " " << lines[index_l]->get_end().y << std::endl;

					POINT tmp_start, tmp_end, tmp_center;
					tmp_start = lines[index_l]->get_start();
					tmp_end = lines[index_l]->get_end();
					tmp_center = { (tmp_start.x + tmp_end.x) / 2, (tmp_start.y + tmp_end.y) / 2};
					tmp_start = this->zoom(tmp_start, tmp_center, factor);
					tmp_end = this->zoom(tmp_end, tmp_center, factor);

					out_line_file << "1" << " " << lines[index_l]->get_type() << " " << tmp_start.x << " " << tmp_start.y << " " << tmp_end.x << " " << tmp_end.y << std::endl;

					out_line_file.close();

					is_zoom_finished = false;
					zooming = 0;
					index_l = 0;
				}
			}

			

			line_file.close();

			std::fstream ellipse_file("ellipse.txt", std::ios::ate | std::ios::in);
			if (!ellipse_file.is_open())
			{
				exit(0);
			}

			ellipse_file.clear();
			ellipse_file.seekg(0, std::ios::beg);	// 定位到文件开头

			ellipses.clear();

			float x, y, radiusX, radiusY;

			while (ellipse_file >> exist >> x >> y >> radiusX >> radiusY)
			{
				Ellipses* e = new Ellipses(exist, x, y, radiusX, radiusY);

				ellipses.push_back(e);
			}

			// 对ellipses进行排序
			sort_ellipses(&ellipses);

			// 对ellipses进行去重
			unique_ellipse(&ellipses);

			for (auto e : ellipses)
			{
				if (e->get_is_exist())
					e->draw(m_pRenderTarget, m_pBlackBrush, 0.5f);
			}

			for (auto e : ellipses)
			{
				if (e->get_is_exist() && e->is_selected(pt_mouse.x, pt_mouse.y))
				{
					// 使得鼠标移动到椭圆上时鼠标变成十字形状
					SetCursor(LoadCursor(NULL, IDC_CROSS));
					if (signal_rclk_mouse && e->get_is_exist())
					{
						// 鼠标右键点击时更改鼠标样式，椭圆消失
						SetCursor(LoadCursor(NULL, IDC_WAIT));

						if (state == TRANSLATE)
						{

						}
						else if (state == ROTATE)
						{

						}
						else if (state == ZOOM)
						{

						}
						else if (state == CLIP)
						{

						}
						else
							e->set_is_exist(false);
					}
				}
			}

			for (auto e : ellipses)
			{
				if (e->get_is_exist() && e->is_selected(pt_mouse.x, pt_mouse.y) && flag == 1)
				{
					if (state == TRANSLATE && translating != 8)
					{
						if (translating == 0)
						{
							translating = 8;
							index_e = std::distance(ellipses.begin(), std::find(ellipses.begin(), ellipses.end(), e));

							break;
						}
					}

					if (state == ROTATE && rotating != 8)
					{
						if (rotating == 0)
						{
							rotating = 8;
							index_e = std::distance(ellipses.begin(), std::find(ellipses.begin(), ellipses.end(), e));

							break;
						}
					}

					if (state == ZOOM && zooming != 8)
					{
						if (zooming == 0)
						{
							zooming = 8;
							index_e = std::distance(ellipses.begin(), std::find(ellipses.begin(), ellipses.end(), e));

							break;
						}
					}
				}
			}

			if (state == TRANSLATE && translating == 8 && !ellipses.empty())
			{
				if (flag == 1)
				{
					for (int i = 0; i < 20; ++i)
						ellipses[index_e]->draw(m_pRenderTarget, m_pWhiteBrush, 0.5f);

					ellipses[index_e]->set_is_exist(false);
					ellipses[index_e]->translate(dx, dy);

					ellipses[index_e]->draw(m_pRenderTarget, m_pBlackBrush, 0.5f);
				}
				if (is_translate_finished)
				{
					for (int i = 0; i < 20; ++i)
						ellipses[index_e]->draw(m_pRenderTarget, m_pWhiteBrush, 0.5f);

					std::ofstream out_ellipse_file("ellipse.txt", std::ios::app | std::ios::out);

					out_ellipse_file << "0" << " " << ellipses[index_e]->get_center().x << " " << ellipses[index_e]->get_center().y << " " << ellipses[index_e]->get_radiusX() << " " << ellipses[index_e]->get_radiusY() << std::endl;

					out_ellipse_file << "1" << " " << ellipses[index_e]->get_center().x + dx << " " << ellipses[index_e]->get_center().y + dy << " " << ellipses[index_e]->get_radiusX() << " " << ellipses[index_e]->get_radiusY() << std::endl;

					out_ellipse_file.close();

					is_translate_finished = false;
					translating = 0;
					index_e = 0;
				}
			}

			if (state == ZOOM && zooming == 8 && !ellipses.empty())
			{
				if (flag == 1)
				{
					for (int i = 0; i < 20; ++i)
						ellipses[index_e]->draw(m_pRenderTarget, m_pWhiteBrush, 0.5f);

					ellipses[index_e]->set_is_exist(false);
					ellipses[index_e]->zoom(factor);

					// 用红色线绘制oldpoint到pt_mouse的线段
					m_pRenderTarget->DrawLine(
						D2D1::Point2F(old_point.x, old_point.y),
						D2D1::Point2F(pt_mouse.x, pt_mouse.y),
						m_pRedBrush,
						1.5f,
						m_pStrokeStyle
					);

					ellipses[index_e]->draw(m_pRenderTarget, m_pBlackBrush, 0.5f);
				}

				if (is_zoom_finished)
				{
					for (int i = 0; i < 20; ++i)
						ellipses[index_e]->draw(m_pRenderTarget, m_pWhiteBrush, 0.5f);

					std::ofstream out_ellipse_file("ellipse.txt", std::ios::app | std::ios::out);

					out_ellipse_file << "0" << " " << ellipses[index_e]->get_center().x << " " << ellipses[index_e]->get_center().y << " " << ellipses[index_e]->get_radiusX() << " " << ellipses[index_e]->get_radiusY() << std::endl;

					out_ellipse_file << "1" << " " << ellipses[index_e]->get_center().x << " " << ellipses[index_e]->get_center().y << " " << ellipses[index_e]->get_radiusX() * factor << " " << ellipses[index_e]->get_radiusY() * factor << std::endl;

					out_ellipse_file.close();

					is_zoom_finished = false;
					zooming = 0;
					index_e = 0;
				}
			}

			ellipse_file.close();

			std::fstream rectangle_file("rectangle.txt", std::ios::ate | std::ios::in);

			if (!rectangle_file.is_open())
			{
				exit(0);
			}

			rectangle_file.clear();

			rectangle_file.seekg(0, std::ios::beg);	// 定位到文件开头

			rectangles.clear();

			float left, top, right, bottom;
			float lbleft, lbbottom, rtright, rttop;
			bool is_filled;

			while (rectangle_file >> exist >> is_filled >> left >> top >> right >> bottom >> lbleft >> lbbottom >> rtright >> rttop)
			{
				Rectangles* r = new Rectangles(exist, is_filled, left, top, right, bottom, lbleft, lbbottom, rtright, rttop);

				rectangles.push_back(r);
			}

			sort_rectangles(&rectangles);

			unique_rectangles(&rectangles);

			for (auto r : rectangles)
			{
				if (r->get_is_exist())
				{
					r->draw(m_pRenderTarget, m_pBlackBrush, 0.5f);

					if (r->get_is_filled())
					{

						// fill_rect(_left + 1, _top + 1, _right - 1, _bottom - 1);
						vertices.clear();
						vertices.push_back({ (int)r->get_left(), (int)r->get_top()});
						vertices.push_back({ (int)r->get_rtright() - 1,(int)r->get_rttop()});
						vertices.push_back({ (int)r->get_right() - 1, (int)r->get_bottom() - 1});
						vertices.push_back({ (int)r->get_lbleft(), (int)r->get_lbbottom() - 1});

						scanline(vertices);
					}
				}
			}

			for (auto r : rectangles)
			{
				if (r->get_is_exist() && r->is_selected(pt_mouse.x, pt_mouse.y))
				{
					// 使得鼠标移动到矩形上时鼠标变成十字形状
					SetCursor(LoadCursor(NULL, IDC_CROSS));
					if (signal_rclk_mouse && r->get_is_exist())
					{
						// 鼠标右键点击时更改鼠标样式，矩形消失
						SetCursor(LoadCursor(NULL, IDC_WAIT));

						if (state == FILL)
							r->set_is_filled(!r->get_is_filled());
						else if (state == TRANSLATE)
						{

						}
						else if (state == ROTATE)
						{

						}
						else if (state == ZOOM)
						{

						}
						else if (state == CLIP)
						{

						}
						else
							r->set_is_exist(false);

					}
				}
			}


			for (auto r : rectangles)
			{
				if(r->get_is_exist() && r->is_selected(pt_mouse.x, pt_mouse.y) && flag == 1)
				{
					if (state == TRANSLATE && translating != 64)
					{
						if (translating == 0)
						{
							//is_translating_r = true;
							translating = 64;
							// 将r在rectangles中的索引传递给index_r
							index_r = std::distance(rectangles.begin(), std::find(rectangles.begin(), rectangles.end(), r));

							break;
						}
					}

					if (state == ROTATE && rotating != 64)
					{
						if (rotating == 0)
						{
							rotating = 64;
							index_r = std::distance(rectangles.begin(), std::find(rectangles.begin(), rectangles.end(), r));

							break;
						}
					}

					if (state == ZOOM && zooming != 64)
					{
						if (zooming == 0)
						{
							zooming = 64;
							index_r = std::distance(rectangles.begin(), std::find(rectangles.begin(), rectangles.end(), r));

							break;
						}
					}
				}
			}

			if (state == TRANSLATE && translating == 64 && !rectangles.empty())
			{
				if (flag == 1)
				{
					for (int i = 0; i < 20; ++i)
						rectangles[index_r]->draw(m_pRenderTarget, m_pWhiteBrush, 0.5f);

					rectangles[index_r]->set_is_exist(false);
					rectangles[index_r]->translate(dx, dy);

					rectangles[index_r]->draw(m_pRenderTarget, m_pBlackBrush, 0.5f);
				}
				if (is_translate_finished)
				{
					for (int i = 0; i < 20; ++i)
						rectangles[index_r]->draw(m_pRenderTarget, m_pWhiteBrush, 0.5f);

					std::ofstream out_rectangle_file("rectangle.txt", std::ios::app | std::ios::out);

					out_rectangle_file << "0" << " " << rectangles[index_r]->get_is_filled() << " " << rectangles[index_r]->get_left() << " " << rectangles[index_r]->get_top() << " " << rectangles[index_r]->get_right() << " " << rectangles[index_r]->get_bottom() << " " << rectangles[index_r]->get_lbleft() << " " << rectangles[index_r]->get_lbbottom() << " " << rectangles[index_r]->get_rtright() << " " << rectangles[index_r]->get_rttop() << std::endl;

					out_rectangle_file << "1" << " " << rectangles[index_r]->get_is_filled() << " " << rectangles[index_r]->get_left() + dx << " " << rectangles[index_r]->get_top() + dy << " " << rectangles[index_r]->get_right() + dx << " " << rectangles[index_r]->get_bottom() + dy << " " << rectangles[index_r]->get_lbleft() + dx << " " << rectangles[index_r]->get_lbbottom() + dy << " " << rectangles[index_r]->get_rtright() + dx << " " << rectangles[index_r]->get_rttop() + dy<< std::endl;

					out_rectangle_file.close();

					is_translate_finished = false;
					//is_translating_r = false;
					translating = 0;
					index_r = 0;
				}
			}

			if (state == ROTATE && rotating == 64 && !rectangles.empty())
			{
				if (flag == 1)
				{
					for (int i = 0; i < 20; ++i)
						rectangles[index_r]->draw(m_pRenderTarget, m_pWhiteBrush, 0.5f);

					rectangles[index_r]->set_is_exist(false);
					rectangles[index_r]->rotate(old_point, pt_mouse);

					// 用红心着重绘制
					D2D1_ELLIPSE tmp_e = D2D1::Ellipse(D2D1::Point2F(old_point.x, old_point.y), 2, 2);
					m_pRenderTarget->DrawEllipse(
						tmp_e,
						m_pRedBrush,
						1.5f
					);
					m_pRenderTarget->FillEllipse(tmp_e, m_pRedBrush);

					rectangles[index_r]->draw(m_pRenderTarget, m_pBlackBrush, 0.5f);
				}

				if (is_rotate_finished)
				{
					for (int i = 0; i < 20; ++i)
						rectangles[index_r]->draw(m_pRenderTarget, m_pWhiteBrush, 0.5f);

					std::ofstream out_rectangle_file("rectangle.txt", std::ios::app | std::ios::out);

					out_rectangle_file << "0" << " " << rectangles[index_r]->get_is_filled() << " " << rectangles[index_r]->get_left() << " " << rectangles[index_r]->get_top() << " " << rectangles[index_r]->get_right() << " " << rectangles[index_r]->get_bottom() << " " << rectangles[index_r]->get_lbleft() << " " << rectangles[index_r]->get_lbbottom() << " " << rectangles[index_r]->get_rtright() << " " << rectangles[index_r]->get_rttop() << std::endl;

					POINT tmp_left, tmp_top, tmp_right, tmp_bottom;
					tmp_left = this->rotate(rectangles[index_r]->get_left_top(), old_point, pt_mouse);
					tmp_top = this->rotate(rectangles[index_r]->get_right_top(), old_point, pt_mouse);
					tmp_right = this->rotate(rectangles[index_r]->get_right_bottom(), old_point, pt_mouse);
					tmp_bottom = this->rotate(rectangles[index_r]->get_left_bottom(), old_point, pt_mouse);

					out_rectangle_file << "1" << " " << rectangles[index_r]->get_is_filled() << " " << tmp_left.x << " " << tmp_left.y << " " << tmp_right.x << " " << tmp_right.y << " " << tmp_bottom.x << " " << tmp_bottom.y << " " << tmp_top.x << " " << tmp_top.y << std::endl;

					out_rectangle_file.close();

					is_rotate_finished = false;
					rotating = 0;
					index_r = 0;
				}
			}

			if (state == ZOOM && zooming == 64 && !rectangles.empty())
			{
				if (flag == 1)
				{
					for (int i = 0; i < 20; ++i)
						rectangles[index_r]->draw(m_pRenderTarget, m_pWhiteBrush, 0.5f);

					rectangles[index_r]->set_is_exist(false);
					rectangles[index_r]->zoom(factor);

					// 用红色线绘制oldpoint到pt_mouse的线段
					m_pRenderTarget->DrawLine(
						D2D1::Point2F(old_point.x, old_point.y),
						D2D1::Point2F(pt_mouse.x, pt_mouse.y),
						m_pRedBrush,
						1.5f,
						m_pStrokeStyle
					);

					rectangles[index_r]->draw(m_pRenderTarget, m_pBlackBrush, 0.5f);
				}

				if (is_zoom_finished)
				{
					for (int i = 0; i < 20; ++i)
						rectangles[index_r]->draw(m_pRenderTarget, m_pWhiteBrush, 0.5f);

					std::ofstream out_rectangle_file("rectangle.txt", std::ios::app | std::ios::out);

					out_rectangle_file << "0" << " " << rectangles[index_r]->get_is_filled() << " " << rectangles[index_r]->get_left() << " " << rectangles[index_r]->get_top() << " " << rectangles[index_r]->get_right() << " " << rectangles[index_r]->get_bottom() << " " << rectangles[index_r]->get_lbleft() << " " << rectangles[index_r]->get_lbbottom() << " " << rectangles[index_r]->get_rtright() << " " << rectangles[index_r]->get_rttop() << std::endl;

					POINT tmp_left, tmp_top, tmp_right, tmp_bottom, tmp_center;
					tmp_left = rectangles[index_r]->get_left_top();
					tmp_top = rectangles[index_r]->get_right_top();
					tmp_right = rectangles[index_r]->get_right_bottom();
					tmp_bottom = rectangles[index_r]->get_left_bottom();
					tmp_center = { (tmp_left.x + tmp_right.x) / 2, (tmp_left.y + tmp_right.y) / 2 };
					tmp_left = this->zoom(tmp_left, tmp_center, factor);
					tmp_top = this->zoom(tmp_top, tmp_center, factor);
					tmp_right = this->zoom(tmp_right, tmp_center, factor);
					tmp_bottom = this->zoom(tmp_bottom, tmp_center, factor);

					out_rectangle_file << "1" << " " << rectangles[index_r]->get_is_filled() << " " << tmp_left.x << " " << tmp_left.y << " " << tmp_right.x << " " << tmp_right.y << " " << tmp_bottom.x << " " << tmp_bottom.y << " " << tmp_top.x << " " << tmp_top.y << std::endl;

					out_rectangle_file.close();

					is_zoom_finished = false;
					zooming = 0;
					index_r = 0;
				}
			}

			rectangle_file.close();

			std::fstream bezier_file("bezier.txt", std::ios::ate | std::ios::in);

			if (!bezier_file.is_open())
			{
				exit(0);
			}

			bezier_file.clear();

			bezier_file.seekg(0, std::ios::beg);	// 定位到文件开头

			bezier.clear();

			float x1_bezier, y1_bezier, x2_bezier, y2_bezier, x3_bezier, y3_bezier, x4_bezier, y4_bezier;

			while (bezier_file >> exist >> x1_bezier >> y1_bezier >> x2_bezier >> y2_bezier >> x3_bezier >> y3_bezier >> x4_bezier >> y4_bezier)
			{
				Bezier* b = new Bezier(exist, x1_bezier, y1_bezier, x2_bezier, y2_bezier, x3_bezier, y3_bezier, x4_bezier, y4_bezier);

				bezier.push_back(b);
			}

			sort_bezier(&bezier);

			unique_bezier(&bezier);

			for (auto b : bezier)
			{
				if (b->get_is_exist())
				{
					this->DrawBezier(b->get_d2d_point1(), b->get_d2d_point2(), b->get_d2d_point3(), b->get_d2d_point4(), m_pBlackBrush, 0.5f);
				}
			}

			for (auto b : bezier)
			{
				if (b->get_is_exist() && b->is_selected(pt_mouse.x, pt_mouse.y))
				{
					// 使得鼠标移动到贝塞尔曲线上时鼠标变成十字形状
					SetCursor(LoadCursor(NULL, IDC_CROSS));
					if (signal_rclk_mouse && b->get_is_exist())
					{
						// 鼠标右键点击时更改鼠标样式，贝塞尔曲线消失
						SetCursor(LoadCursor(NULL, IDC_WAIT));
						if (state == TRANSLATE)
						{

						}
						else if (state == ROTATE)
						{

						}
						else if (state == ZOOM)
						{

						}
						else if (state == CLIP)
						{

						}
						else
							b->set_is_exist(false);
					}
				}
			}

			for (auto b : bezier)
			{
				if (b->get_is_exist() && b->is_selected(pt_mouse.x, pt_mouse.y) && flag)
				{
					if (state == TRANSLATE &&translating != 1)
					{
						if (translating == 0)
						{
							//is_translating_b = true;
							translating = 1;

							index_b = std::distance(bezier.begin(), std::find(bezier.begin(), bezier.end(), b));

							break;
						}
					}

					if (state == ROTATE && rotating != 1)
					{
						if (rotating == 0)
						{
							rotating = 1;
							index_b = std::distance(bezier.begin(), std::find(bezier.begin(), bezier.end(), b));

							break;
						}
					}

					if (state == ZOOM && zooming != 1)
					{
						if (zooming == 0)
						{
							zooming = 1;
							index_b = std::distance(bezier.begin(), std::find(bezier.begin(), bezier.end(), b));

							break;
						}
					}
				}

			}

			if (state == TRANSLATE && translating == 1 && !bezier.empty())
			{
				if (flag == 1)
				{
					for(int i = 0; i < 20; ++i)
						this->DrawBezier(bezier[index_b]->get_d2d_point1(), bezier[index_b]->get_d2d_point2(), bezier[index_b]->get_d2d_point3(), bezier[index_b]->get_d2d_point4(), m_pWhiteBrush, 0.5f);

					bezier[index_b]->set_is_exist(false);
					bezier[index_b]->translate(dx, dy);

					this->DrawBezier(bezier[index_b]->get_d2d_point1(), bezier[index_b]->get_d2d_point2(), bezier[index_b]->get_d2d_point3(), bezier[index_b]->get_d2d_point4(), m_pBlackBrush, 0.5f);
				}

				if (is_translate_finished)
				{
					for (int i = 0; i < 20; ++i)
						this->DrawBezier(bezier[index_b]->get_d2d_point1(), bezier[index_b]->get_d2d_point2(), bezier[index_b]->get_d2d_point3(), bezier[index_b]->get_d2d_point4(), m_pWhiteBrush, 0.5f);

					std::ofstream out_bezier_file("bezier.txt", std::ios::app | std::ios::out);

					out_bezier_file << "0" << " " << bezier[index_b]->get_d2d_point1().x << " " << bezier[index_b]->get_d2d_point1().y << " " << bezier[index_b]->get_d2d_point2().x << " " << bezier[index_b]->get_d2d_point2().y << " " << bezier[index_b]->get_d2d_point3().x << " " << bezier[index_b]->get_d2d_point3().y << " " << bezier[index_b]->get_d2d_point4().x << " " << bezier[index_b]->get_d2d_point4().y << std::endl;

					out_bezier_file << "1" << " " << bezier[index_b]->get_d2d_point1().x + dx << " " << bezier[index_b]->get_d2d_point1().y + dy << " " << bezier[index_b]->get_d2d_point2().x + dx << " " << bezier[index_b]->get_d2d_point2().y + dy << " " << bezier[index_b]->get_d2d_point3().x + dx << " " << bezier[index_b]->get_d2d_point3().y + dy << " " << bezier[index_b]->get_d2d_point4().x + dx << " " << bezier[index_b]->get_d2d_point4().y + dy << std::endl;

					out_bezier_file.close();

					is_translate_finished = false;
					//is_translating_b = false;
					translating = 0;
					index_b = 0;
				}
			}

			if (state == ROTATE && rotating == 1 && !bezier.empty())
			{
				if (flag == 1)
				{
					for (int i = 0; i < 20; ++i)
						this->DrawBezier(bezier[index_b]->get_d2d_point1(), bezier[index_b]->get_d2d_point2(), bezier[index_b]->get_d2d_point3(), bezier[index_b]->get_d2d_point4(), m_pWhiteBrush, 0.5f);

					bezier[index_b]->set_is_exist(false);
					bezier[index_b]->rotate(old_point, pt_mouse);

					// 用红心着重绘制
					D2D1_ELLIPSE tmp_e = D2D1::Ellipse(D2D1::Point2F(old_point.x, old_point.y), 2, 2);
					m_pRenderTarget->DrawEllipse(
						tmp_e,
						m_pRedBrush,
						1.5f
					);
					m_pRenderTarget->FillEllipse(tmp_e, m_pRedBrush);

					this->DrawBezier(bezier[index_b]->get_d2d_point1(), bezier[index_b]->get_d2d_point2(), bezier[index_b]->get_d2d_point3(), bezier[index_b]->get_d2d_point4(), m_pBlackBrush, 0.5f);
				}

				if (is_rotate_finished)
				{
					for (int i = 0; i < 20; ++i)
						this->DrawBezier(bezier[index_b]->get_d2d_point1(), bezier[index_b]->get_d2d_point2(), bezier[index_b]->get_d2d_point3(), bezier[index_b]->get_d2d_point4(), m_pWhiteBrush, 0.5f);

					std::ofstream out_bezier_file("bezier.txt", std::ios::app | std::ios::out);

					out_bezier_file << "0" << " " << bezier[index_b]->get_d2d_point1().x << " " << bezier[index_b]->get_d2d_point1().y << " " << bezier[index_b]->get_d2d_point2().x << " " << bezier[index_b]->get_d2d_point2().y << " " << bezier[index_b]->get_d2d_point3().x << " " << bezier[index_b]->get_d2d_point3().y << " " << bezier[index_b]->get_d2d_point4().x << " " << bezier[index_b]->get_d2d_point4().y << std::endl;

					D2D_POINT_2F tmp1, tmp2, tmp3, tmp4;
					tmp1 = this->rotate(bezier[index_b]->get_d2d_point1(), old_point, pt_mouse);
					tmp2 = this->rotate(bezier[index_b]->get_d2d_point2(), old_point, pt_mouse);
					tmp3 = this->rotate(bezier[index_b]->get_d2d_point3(), old_point, pt_mouse);
					tmp4 = this->rotate(bezier[index_b]->get_d2d_point4(), old_point, pt_mouse);

					out_bezier_file << "1" << " " << tmp1.x << " " << tmp1.y << " " << tmp2.x << " " << tmp2.y << " " << tmp3.x << " " << tmp3.y << " " << tmp4.x << " " << tmp4.y << std::endl;

					out_bezier_file.close();

					is_rotate_finished = false;
					rotating = 0;
					index_b = 0;
				}
			}

			if(state == ZOOM && zooming == 1 && !bezier.empty())
			{ 
				if (flag == 1)
				{
					for (int i = 0; i < 20; ++i)
						this->DrawBezier(bezier[index_b]->get_d2d_point1(), bezier[index_b]->get_d2d_point2(), bezier[index_b]->get_d2d_point3(), bezier[index_b]->get_d2d_point4(), m_pWhiteBrush, 0.5f);

					bezier[index_b]->set_is_exist(false);
					bezier[index_b]->zoom(factor);

					// 用红色线绘制oldpoint到pt_mouse的线段
					m_pRenderTarget->DrawLine(
						D2D1::Point2F(old_point.x, old_point.y),
						D2D1::Point2F(pt_mouse.x, pt_mouse.y),
						m_pRedBrush,
						1.5f,
						m_pStrokeStyle
					);

					this->DrawBezier(bezier[index_b]->get_d2d_point1(), bezier[index_b]->get_d2d_point2(), bezier[index_b]->get_d2d_point3(), bezier[index_b]->get_d2d_point4(), m_pBlackBrush, 0.5f);
				}

				if (is_zoom_finished)
				{
					for (int i = 0; i < 20; ++i)
						this->DrawBezier(bezier[index_b]->get_d2d_point1(), bezier[index_b]->get_d2d_point2(), bezier[index_b]->get_d2d_point3(), bezier[index_b]->get_d2d_point4(), m_pWhiteBrush, 0.5f);

					std::ofstream out_bezier_file("bezier.txt", std::ios::app | std::ios::out);

					out_bezier_file << "0" << " " << bezier[index_b]->get_d2d_point1().x << " " << bezier[index_b]->get_d2d_point1().y << " " << bezier[index_b]->get_d2d_point2().x << " " << bezier[index_b]->get_d2d_point2().y << " " << bezier[index_b]->get_d2d_point3().x << " " << bezier[index_b]->get_d2d_point3().y << " " << bezier[index_b]->get_d2d_point4().x << " " << bezier[index_b]->get_d2d_point4().y << std::endl;

					D2D_POINT_2F tmp1, tmp2, tmp3, tmp4;
					tmp1 = bezier[index_b]->get_d2d_point1();
					tmp2 = bezier[index_b]->get_d2d_point2();
					tmp3 = bezier[index_b]->get_d2d_point3();
					tmp4 = bezier[index_b]->get_d2d_point4();
					
					tmp1 = this->zoom(tmp1, tmp1, factor);
					tmp2 = this->zoom(tmp2, tmp1, factor);
					tmp3 = this->zoom(tmp3, tmp1, factor);
					tmp4 = this->zoom(tmp4, tmp1, factor);

					out_bezier_file << "1" << " " << tmp1.x << " " << tmp1.y << " " << tmp2.x << " " << tmp2.y << " " << tmp3.x << " " << tmp3.y << " " << tmp4.x << " " << tmp4.y << std::endl;

					out_bezier_file.close();

					is_zoom_finished = false;
					zooming = 0;
					index_b = 0;
				}
			}

			bezier_file.close();

			std::fstream bline_file("bline.txt", std::ios::ate | std::ios::in);

			if (!bline_file.is_open())
			{
				exit(0);
			}

			bline_file.clear();

			bline_file.seekg(0, std::ios::beg);	// 定位到文件开头

			blines.clear();

			float x1_bline, y1_bline, x2_bline, y2_bline, x3_bline, y3_bline, x4_bline, y4_bline;

			while (bline_file >> exist >> x1_bline >> y1_bline >> x2_bline >> y2_bline >> x3_bline >> y3_bline >> x4_bline >> y4_bline)
			{
				BLine* b = new BLine(exist, x1_bline, y1_bline, x2_bline, y2_bline, x3_bline, y3_bline, x4_bline, y4_bline);

				blines.push_back(b);
			}

			sort_blines(&blines);

			unique_bline(&blines);

			for (auto b : blines)
			{
				if (b->get_is_exist())
				{
					this->DrawBLine(b->get_d2d_point1(), b->get_d2d_point2(), b->get_d2d_point3(), b->get_d2d_point4(), m_pBlackBrush);
					
					// 用虚线绘制控制多边形
					m_pRenderTarget->DrawLine(
						b->get_d2d_point1(),
						b->get_d2d_point2(),
						m_pBLineBrush,
						2.0f,
						m_pStrokeStyle
					);
					m_pRenderTarget->DrawLine(
						b->get_d2d_point2(),
						b->get_d2d_point3(),
						m_pBLineBrush,
						2.0f,
						m_pStrokeStyle
					);
					m_pRenderTarget->DrawLine(
						b->get_d2d_point3(),
						b->get_d2d_point4(),
						m_pBLineBrush,
						2.0f,
						m_pStrokeStyle
					);

				}
			}

			for (auto b : blines)
			{
				if (b->get_is_exist() && b->is_selected(pt_mouse.x, pt_mouse.y))
				{
					// 使得鼠标移动到BLine上时鼠标变成十字形状
					SetCursor(LoadCursor(NULL, IDC_CROSS));
					if (signal_rclk_mouse && b->get_is_exist())
					{
						// 鼠标右键点击时更改鼠标样式，BLine消失
						SetCursor(LoadCursor(NULL, IDC_WAIT));

						if (state == TRANSLATE)
						{

						}
						else if (state == ROTATE)
						{

						}
						else if (state == ZOOM)
						{

						}
						else if (state == CLIP)
						{

						}
						else
							b->set_is_exist(false);
					}
				}
			}

			for (auto b : blines)
			{
				if(b->get_is_exist() && b->is_selected(pt_mouse.x, pt_mouse.y) && flag == 1)
				{
					if (state == TRANSLATE && translating != 2)
					{
						if (translating == 0)
						{
							//is_translating_b = true;
							translating = 2;

							index_bl = std::distance(blines.begin(), std::find(blines.begin(), blines.end(), b));

							break;
						}
					}

					if (state == ROTATE && rotating != 2)
					{
						if (rotating == 0)
						{
							rotating = 2;
							index_bl = std::distance(blines.begin(), std::find(blines.begin(), blines.end(), b));

							break;
						}
					}

					if (state == ZOOM && zooming != 2)
					{
						if (zooming == 0)
						{
							zooming = 2;
							index_bl = std::distance(blines.begin(), std::find(blines.begin(), blines.end(), b));

							break;
						}
					}
				}
			}

			if (state == TRANSLATE && translating == 2 && !blines.empty())
			{
				if (flag == 1)
				{
					for (int i = 0; i < 20; ++i)
						this->DrawBLine(blines[index_bl]->get_d2d_point1(), blines[index_bl]->get_d2d_point2(), blines[index_bl]->get_d2d_point3(), blines[index_bl]->get_d2d_point4(), m_pWhiteBrush);

					// 用虚线绘制控制多边形
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point1(),
						blines[index_bl]->get_d2d_point2(),
						m_pWhiteBrush,
						3.0f
					);
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point2(),
						blines[index_bl]->get_d2d_point3(),
						m_pWhiteBrush,
						3.0f
					);
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point3(),
						blines[index_bl]->get_d2d_point4(),
						m_pWhiteBrush,
						3.0f
					);

					blines[index_bl]->set_is_exist(false);
					blines[index_bl]->translate(dx, dy);

					this->DrawBLine(blines[index_bl]->get_d2d_point1(), blines[index_bl]->get_d2d_point2(), blines[index_bl]->get_d2d_point3(), blines[index_bl]->get_d2d_point4(), m_pBlackBrush);

					// 用虚线绘制控制多边形
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point1(),
						blines[index_bl]->get_d2d_point2(),
						m_pBLineBrush,
						2.0f,
						m_pStrokeStyle
					);
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point2(),
						blines[index_bl]->get_d2d_point3(),
						m_pBLineBrush,
						2.0f,
						m_pStrokeStyle
					);
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point3(),
						blines[index_bl]->get_d2d_point4(),
						m_pBLineBrush,
						2.0f,
						m_pStrokeStyle
					);
				}

				if (is_translate_finished)
				{
					for (int i = 0; i < 20; ++i)
						this->DrawBLine(blines[index_bl]->get_d2d_point1(), blines[index_bl]->get_d2d_point2(), blines[index_bl]->get_d2d_point3(), blines[index_bl]->get_d2d_point4(), m_pWhiteBrush);

					// 用虚线绘制控制多边形
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point1(),
						blines[index_bl]->get_d2d_point2(),
						m_pWhiteBrush,
						3.0f
					);
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point2(),
						blines[index_bl]->get_d2d_point3(),
						m_pWhiteBrush,
						3.0f
					);
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point3(),
						blines[index_bl]->get_d2d_point4(),
						m_pWhiteBrush,
						3.0f
					);

					std::ofstream out_bline_file("bline.txt", std::ios::app | std::ios::out);

					out_bline_file << "0" << " " << blines[index_bl]->get_d2d_point1().x << " " << blines[index_bl]->get_d2d_point1().y << " " << blines[index_bl]->get_d2d_point2().x << " " << blines[index_bl]->get_d2d_point2().y << " " << blines[index_bl]->get_d2d_point3().x << " " << blines[index_bl]->get_d2d_point3().y << " " << blines[index_bl]->get_d2d_point4().x << " " << blines[index_bl]->get_d2d_point4().y << std::endl;

					out_bline_file << "1" << " " << blines[index_bl]->get_d2d_point1().x + dx << " " << blines[index_bl]->get_d2d_point1().y + dy << " " << blines[index_bl]->get_d2d_point2().x + dx << " " << blines[index_bl]->get_d2d_point2().y + dy << " " << blines[index_bl]->get_d2d_point3().x + dx << " " << blines[index_bl]->get_d2d_point3().y + dy << " " << blines[index_bl]->get_d2d_point4().x + dx << " " << blines[index_bl]->get_d2d_point4().y + dy << std::endl;

					out_bline_file.close();

					is_translate_finished = false;
					translating = 0;
					index_bl = 0;
				}
			}

			if (state == ROTATE && rotating == 2 && !blines.empty())
			{
				if (flag == 1)
				{
					for (int i = 0; i < 20; ++i)
						this->DrawBLine(blines[index_bl]->get_d2d_point1(), blines[index_bl]->get_d2d_point2(), blines[index_bl]->get_d2d_point3(), blines[index_bl]->get_d2d_point4(), m_pWhiteBrush);

					// 用虚线绘制控制多边形
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point1(),
						blines[index_bl]->get_d2d_point2(),
						m_pWhiteBrush,
						3.0f
					);
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point2(),
						blines[index_bl]->get_d2d_point3(),
						m_pWhiteBrush,
						3.0f
					);
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point3(),
						blines[index_bl]->get_d2d_point4(),
						m_pWhiteBrush,
						3.0f
					);

					blines[index_bl]->set_is_exist(false);
					blines[index_bl]->rotate(old_point, pt_mouse);

					// 用红心着重绘制
					D2D1_ELLIPSE tmp_e = D2D1::Ellipse(D2D1::Point2F(old_point.x, old_point.y), 2, 2);
					m_pRenderTarget->DrawEllipse(
						tmp_e,
						m_pRedBrush,
						1.5f
					);
					m_pRenderTarget->FillEllipse(tmp_e, m_pRedBrush);

					this->DrawBLine(blines[index_bl]->get_d2d_point1(), blines[index_bl]->get_d2d_point2(), blines[index_bl]->get_d2d_point3(), blines[index_bl]->get_d2d_point4(), m_pBlackBrush);

					// 用虚线绘制控制多边形
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point1(),
						blines[index_bl]->get_d2d_point2(),
						m_pBLineBrush,
						2.0f,
						m_pStrokeStyle
					);
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point2(),
						blines[index_bl]->get_d2d_point3(),
						m_pBLineBrush,
						2.0f,
						m_pStrokeStyle
					);
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point3(),
						blines[index_bl]->get_d2d_point4(),
						m_pBLineBrush,
						2.0f,
						m_pStrokeStyle
					);
				}
				
				if (is_rotate_finished)
				{
					for (int i = 0; i < 20; ++i)
						this->DrawBLine(blines[index_bl]->get_d2d_point1(), blines[index_bl]->get_d2d_point2(), blines[index_bl]->get_d2d_point3(), blines[index_bl]->get_d2d_point4(), m_pWhiteBrush);

					// 用虚线绘制控制多边形
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point1(),
						blines[index_bl]->get_d2d_point2(),
						m_pWhiteBrush,
						3.0f
					);
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point2(),
						blines[index_bl]->get_d2d_point3(),
						m_pWhiteBrush,
						3.0f
					);
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point3(),
						blines[index_bl]->get_d2d_point4(),
						m_pWhiteBrush,
						3.0f
					);

					std::ofstream out_bline_file("bline.txt", std::ios::app | std::ios::out);

					out_bline_file << "0" << " " << blines[index_bl]->get_d2d_point1().x << " " << blines[index_bl]->get_d2d_point1().y << " " << blines[index_bl]->get_d2d_point2().x << " " << blines[index_bl]->get_d2d_point2().y << " " << blines[index_bl]->get_d2d_point3().x << " " << blines[index_bl]->get_d2d_point3().y << " " << blines[index_bl]->get_d2d_point4().x << " " << blines[index_bl]->get_d2d_point4().y << std::endl;

					D2D_POINT_2F tmp1, tmp2, tmp3, tmp4;
					tmp1 = this->rotate(blines[index_bl]->get_d2d_point1(), old_point, pt_mouse);
					tmp2 = this->rotate(blines[index_bl]->get_d2d_point2(), old_point, pt_mouse);
					tmp3 = this->rotate(blines[index_bl]->get_d2d_point3(), old_point, pt_mouse);
					tmp4 = this->rotate(blines[index_bl]->get_d2d_point4(), old_point, pt_mouse);

					out_bline_file << "1" << " " << tmp1.x << " " << tmp1.y << " " << tmp2.x << " " << tmp2.y << " " << tmp3.x << " " << tmp3.y << " " << tmp4.x << " " << tmp4.y << std::endl;

					out_bline_file.close();

					is_rotate_finished = false;
					rotating = 0;
					index_bl = 0;
				}
			}

			if (state == ZOOM && zooming == 2 && !blines.empty())
			{
				if (flag == 1)
				{
					for (int i = 0; i < 20; ++i)
						this->DrawBLine(blines[index_bl]->get_d2d_point1(), blines[index_bl]->get_d2d_point2(), blines[index_bl]->get_d2d_point3(), blines[index_bl]->get_d2d_point4(), m_pWhiteBrush);

					// 用虚线绘制控制多边形
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point1(),
						blines[index_bl]->get_d2d_point2(),
						m_pWhiteBrush,
						3.0f
					);
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point2(),
						blines[index_bl]->get_d2d_point3(),
						m_pWhiteBrush,
						3.0f
					);
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point3(),
						blines[index_bl]->get_d2d_point4(),
						m_pWhiteBrush,
						3.0f
					);

					blines[index_bl]->set_is_exist(false);
					blines[index_bl]->zoom(factor);

					// 用红色线绘制oldpoint到pt_mouse的线段
					m_pRenderTarget->DrawLine(
						D2D1::Point2F(old_point.x, old_point.y),
						D2D1::Point2F(pt_mouse.x, pt_mouse.y),
						m_pRedBrush,
						1.5f,
						m_pStrokeStyle
					);

					this->DrawBLine(blines[index_bl]->get_d2d_point1(), blines[index_bl]->get_d2d_point2(), blines[index_bl]->get_d2d_point3(), blines[index_bl]->get_d2d_point4(), m_pBlackBrush);

					// 用虚线绘制控制多边形
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point1(),
						blines[index_bl]->get_d2d_point2(),
						m_pBLineBrush,
						2.0f,
						m_pStrokeStyle
					);
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point2(),
						blines[index_bl]->get_d2d_point3(),
						m_pBLineBrush,
						2.0f,
						m_pStrokeStyle
					);
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point3(),
						blines[index_bl]->get_d2d_point4(),
						m_pBLineBrush,
						2.0f,
						m_pStrokeStyle
					);

				}

				if (is_zoom_finished)
				{
					for (int i = 0; i < 20; ++i)
						this->DrawBLine(blines[index_bl]->get_d2d_point1(), blines[index_bl]->get_d2d_point2(), blines[index_bl]->get_d2d_point3(), blines[index_bl]->get_d2d_point4(), m_pWhiteBrush);

					// 用虚线绘制控制多边形
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point1(),
						blines[index_bl]->get_d2d_point2(),
						m_pWhiteBrush,
						3.0f
					);
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point2(),
						blines[index_bl]->get_d2d_point3(),
						m_pWhiteBrush,
						3.0f
					);
					m_pRenderTarget->DrawLine(
						blines[index_bl]->get_d2d_point3(),
						blines[index_bl]->get_d2d_point4(),
						m_pWhiteBrush,
						3.0f
					);

					std::ofstream out_bline_file("bline.txt", std::ios::app | std::ios::out);

					out_bline_file << "0" << " " << blines[index_bl]->get_d2d_point1().x << " " << blines[index_bl]->get_d2d_point1().y << " " << blines[index_bl]->get_d2d_point2().x << " " << blines[index_bl]->get_d2d_point2().y << " " << blines[index_bl]->get_d2d_point3().x << " " << blines[index_bl]->get_d2d_point3().y << " " << blines[index_bl]->get_d2d_point4().x << " " << blines[index_bl]->get_d2d_point4().y << std::endl;

					D2D_POINT_2F tmp1, tmp2, tmp3, tmp4;
					tmp1 = blines[index_bl]->get_d2d_point1();
					tmp2 = blines[index_bl]->get_d2d_point2();
					tmp3 = blines[index_bl]->get_d2d_point3();
					tmp4 = blines[index_bl]->get_d2d_point4();

					tmp1 = this->zoom(tmp1, tmp1, factor);
					tmp2 = this->zoom(tmp2, tmp1, factor);
					tmp3 = this->zoom(tmp3, tmp1, factor);
					tmp4 = this->zoom(tmp4, tmp1, factor);

					out_bline_file << "1" << " " << tmp1.x << " " << tmp1.y << " " << tmp2.x << " " << tmp2.y << " " << tmp3.x << " " << tmp3.y << " " << tmp4.x << " " << tmp4.y << std::endl;

					out_bline_file.close();

					is_zoom_finished = false;
					zooming = 0;
					index_bl = 0;
				}
			}

			bline_file.close();
		}

		std::fstream polyline_file("polyline.txt", std::ios::ate | std::ios::in);

		if (!polyline_file.is_open())
		{
			exit(0);
		}

		polyline_file.clear();

		polyline_file.seekg(0, std::ios::beg);	// 定位到文件开头

		polylines.clear();

		std::vector<D2D1_POINT_2F> points;
		std::string linetext;
		bool _exist = false;
		bool _filled = false;

		while (std::getline(polyline_file, linetext)) // 按行读取文件,将每行存入line中
		{
			if (linetext == "0" || linetext == "1")	// 读取到0或1时，将之前的点存入points中
			{
				if (!points.empty())	// 上一组数据读取完成后，将points中的点存入polylines中
				{
					Polylines* p = new Polylines(_exist, _filled, points);
					polylines.push_back(p);
					points.clear();		// 清空points，为读取下一组数据做准备
				}
				_exist = (linetext[0] == '1');

				std::getline(polyline_file, linetext);
				_filled = (linetext[0] == '1');
			}
			else
			{
				std::istringstream iss(linetext);	// istringstream是从string读取数据的流
				float x, y;
				iss >> x >> y;
				D2D1_POINT_2F _point = D2D1::Point2F(x, y);
				if (points.empty())
					points.push_back(D2D1::Point2F(x, y));
				else if (!points.empty() && _point.x != points.back().x && _point.y != points.back().y)
					points.push_back(D2D1::Point2F(x, y));
			}
		}
		if (!points.empty())	// 读取最后一组数据
		{
			Polylines* p = new Polylines(_exist, _filled, points);
			polylines.push_back(p);
		}

		sort_polyline(&polylines);

		unique_polypoints(&polylines);

		unique_polyline(&polylines);

		// 从polylines中读取数据，绘制多段线
		for (auto p : polylines)
		{
			if (p->get_is_exist())
			{
				for (int i = 0; i < p->get_points().size() - 1; ++i)
				{
					m_pRenderTarget->DrawLine(
						p->get_points()[i],
						p->get_points()[i + 1],
						m_pBlackBrush,
						0.5f
					);
				}

				// 如果多段线闭合成三角形，填充三角形
				if (p->is_triangle() && p->get_is_filled())
				{
					int len = p->get_points().size();
					if (len == 4)
						fill_triangle(p->get_points()[0].x, p->get_points()[0].y, p->get_points()[1].x, p->get_points()[1].y, p->get_points()[2].x, p->get_points()[2].y);
					else if (len == 5)
						fill_triangle(p->get_points()[0].x, p->get_points()[0].y, p->get_points()[2].x, p->get_points()[2].y, p->get_points()[3].x, p->get_points()[3].y);
				}
			}
		}

		for (auto p : polylines)
		{
			if (p->get_is_exist() && p->is_selected(pt_mouse.x, pt_mouse.y))
			{
				// 使得鼠标移动到多段线上时鼠标变成十字形状
				SetCursor(LoadCursor(NULL, IDC_CROSS));
				if (p->get_is_exist() && signal_rclk_mouse)
				{
					// 鼠标右键点击时更改鼠标样式，多段线消失
					SetCursor(LoadCursor(NULL, IDC_WAIT));

					if(state == FILL)
						p->set_is_filled(!p->get_is_filled());
					else if (state == TRANSLATE)
					{

					}
					else if (state == ROTATE)
					{

					}
					else if (state == ZOOM)
					{

					}
					else if (state == CLIP)
					{

					}
					else
						p->set_is_exist(false);
				}
			}
		}

		for (auto p : polylines)
		{
			if (p->get_is_exist() && p->is_selected(pt_mouse.x, pt_mouse.y) && flag == 1)
			{
				if (state == TRANSLATE && translating != 32)
				{
					if (translating == 0)
					{
						//is_translating_p = true;
						translating = 32;

						index_p = std::distance(polylines.begin(), std::find(polylines.begin(), polylines.end(), p));

						break;
					}
				}

				if (state == ROTATE && rotating != 32)
				{
					if (rotating == 0)
					{
						rotating = 32;
						index_p = std::distance(polylines.begin(), std::find(polylines.begin(), polylines.end(), p));

						break;
					}
				}

				if (state == ZOOM && zooming != 32)
				{
					if (zooming == 0)
					{
						zooming = 32;
						index_p = std::distance(polylines.begin(), std::find(polylines.begin(), polylines.end(), p));

						break;
					}
				}
			}

		}

		if (state == TRANSLATE && translating == 32 && !polylines.empty())
		{
			if (flag == 1)
			{
				for (int i = 0; i < 20; ++i)
				{
					for (int j = 0; j < polylines[index_p]->get_points().size() - 1; ++j)
					{
						m_pRenderTarget->DrawLine(
							polylines[index_p]->get_points()[j],
							polylines[index_p]->get_points()[j + 1],
							m_pWhiteBrush,
							0.5f
						);
					}
				}

				for (int i = 0; i < polylines[index_p]->get_points().size() - 1; ++i)
				{
					m_pRenderTarget->DrawLine(
						{
							polylines[index_p]->get_points()[i].x + dx,
							polylines[index_p]->get_points()[i].y + dy
						},
						{
							polylines[index_p]->get_points()[i + 1].x + dx,
							polylines[index_p]->get_points()[i + 1].y + dy
						},
						m_pBlackBrush,
						0.5f
					);
				}
			}

			if (is_translate_finished)
			{
				for (int i = 0; i < 20; ++i)
				{
					for (int j = 0; j < polylines[index_p]->get_points().size() - 1; ++j)
					{
						m_pRenderTarget->DrawLine(
							polylines[index_p]->get_points()[j],
							polylines[index_p]->get_points()[j + 1],
							m_pWhiteBrush,
							0.5f
						);
					}
				}

				std::ofstream out_polyline_file("polyline.txt", std::ios::app | std::ios::out);

				out_polyline_file << "0" << std::endl;
				out_polyline_file << polylines[index_p]->get_is_filled() << std::endl;

				for (auto point : polylines[index_p]->get_points())
				{
					out_polyline_file << point.x << " " << point.y << std::endl;
				}

				out_polyline_file << "1" << std::endl;
				out_polyline_file << polylines[index_p]->get_is_filled() << std::endl;

				for (auto point : polylines[index_p]->get_points())
				{
					out_polyline_file << point.x + dx << " " << point.y + dy << std::endl;
				}

				out_polyline_file.close();

				is_translate_finished = false;
				translating = 0;
				index_p = 0;
			}
		}

		if (state == ROTATE && rotating == 32 && !polylines.empty())
		{
			if (flag == 1)
			{
				for (int i = 0; i < 20; ++i)
				{
					for (int j = 0; j < polylines[index_p]->get_points().size() - 1; ++j)
					{
						m_pRenderTarget->DrawLine(
							polylines[index_p]->get_points()[j],
							polylines[index_p]->get_points()[j + 1],
							m_pWhiteBrush,
							0.5f
						);
					}
				}

				// 用红心着重绘制
				D2D1_ELLIPSE tmp_e = D2D1::Ellipse(D2D1::Point2F(old_point.x, old_point.y), 2, 2);
				m_pRenderTarget->DrawEllipse(
					tmp_e,
					m_pRedBrush,
					1.5f
				);
				m_pRenderTarget->FillEllipse(tmp_e, m_pRedBrush);

				for (int i = 0; i < polylines[index_p]->get_points().size() - 1; ++i)
				{
					m_pRenderTarget->DrawLine(
						this->rotate(polylines[index_p]->get_points()[i], old_point, pt_mouse),
						this->rotate(polylines[index_p]->get_points()[i + 1], old_point, pt_mouse),
						m_pBlackBrush,
						0.5f
					);
				}
			}

			if (is_rotate_finished)
			{
				for (int i = 0; i < 20; ++i)
				{
					for (int j = 0; j < polylines[index_p]->get_points().size() - 1; ++j)
					{
						m_pRenderTarget->DrawLine(
							polylines[index_p]->get_points()[j],
							polylines[index_p]->get_points()[j + 1],
							m_pWhiteBrush,
							0.5f
						);
					}
				}

				std::ofstream out_polyline_file("polyline.txt", std::ios::app | std::ios::out);

				out_polyline_file << "0" << std::endl;
				out_polyline_file << polylines[index_p]->get_is_filled() << std::endl;

				for (auto point : polylines[index_p]->get_points())
				{
					out_polyline_file << point.x << " " << point.y << std::endl;
				}

				out_polyline_file << "1" << std::endl;
				out_polyline_file << polylines[index_p]->get_is_filled() << std::endl;

				for (auto point : polylines[index_p]->get_points())
				{
					out_polyline_file << this->rotate(point, old_point, pt_mouse).x << " " << this->rotate(point, old_point, pt_mouse).y << std::endl;
				}

				out_polyline_file.close();

				is_rotate_finished = false;
				rotating = 0;
				index_p = 0;

			}
		}

		if (state == ZOOM && zooming == 32 && !polylines.empty())
		{
			if (flag == 1)
			{
				for (int i = 0; i < 20; ++i)
				{
					for (int j = 0; j < polylines[index_p]->get_points().size() - 1; ++j)
					{
						m_pRenderTarget->DrawLine(
							polylines[index_p]->get_points()[j],
							polylines[index_p]->get_points()[j + 1],
							m_pWhiteBrush,
							0.5f
						);
					}
				}

				for (int i = 0; i < polylines[index_p]->get_points().size() - 1; ++i)
				{
					m_pRenderTarget->DrawLine(
						this->zoom(polylines[index_p]->get_points()[i], polylines[index_p]->get_points()[0], factor),
						this->zoom(polylines[index_p]->get_points()[i + 1], polylines[index_p]->get_points()[0], factor),
						m_pBlackBrush,
						0.5f
					);
				}

				// 用红色线绘制oldpoint到pt_mouse的线段
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(old_point.x, old_point.y),
					D2D1::Point2F(pt_mouse.x, pt_mouse.y),
					m_pRedBrush,
					1.5f,
					m_pStrokeStyle
				);
			}

			if (is_zoom_finished)
			{
				for (int i = 0; i < 20; ++i)
				{
					for (int j = 0; j < polylines[index_p]->get_points().size() - 1; ++j)
					{
						m_pRenderTarget->DrawLine(
							polylines[index_p]->get_points()[j],
							polylines[index_p]->get_points()[j + 1],
							m_pWhiteBrush,
							0.5f
						);
					}
				}

				std::ofstream out_polyline_file("polyline.txt", std::ios::app | std::ios::out);

				out_polyline_file << "0" << std::endl;
				out_polyline_file << polylines[index_p]->get_is_filled() << std::endl;

				for (auto point : polylines[index_p]->get_points())
				{
					out_polyline_file << point.x << " " << point.y << std::endl;
				}

				out_polyline_file << "1" << std::endl;
				out_polyline_file << polylines[index_p]->get_is_filled() << std::endl;

				for (auto point : polylines[index_p]->get_points())
				{
					out_polyline_file << this->zoom(point, polylines[index_p]->get_points()[0], factor).x << " " << this->zoom(point, polylines[index_p]->get_points()[0], factor).y << std::endl;
				}

				out_polyline_file.close();

				is_zoom_finished = false;
				zooming = 0;
				index_p = 0;
			}
		}

		polyline_file.close();

		
		std::fstream circle_file("circle.txt", std::ios::ate | std::ios::in);

		if (!circle_file.is_open())
		{
			exit(0);
		}

		circle_file.clear();

		circle_file.seekg(0, std::ios::beg);	// 定位到文件开头

		circles.clear();

		float cx, cy, radius;
		int type;

		while (circle_file >> exist >> type >> cx >> cy >> radius)
		{
			Circle* c = new Circle(exist, type, cx, cy, radius);

			circles.push_back(c);
		}

		sort_circles(&circles);

		unique_circle(&circles);

		for (auto c : circles)
		{
			if (c->get_is_exist())
			{
				if (c->get_type() == 0)
				{
					m_pRenderTarget->DrawEllipse(
						D2D1::Ellipse(D2D1::Point2F(c->get_center().x, c->get_center().y), c->get_radius(), c->get_radius()),
						m_pBlackBrush,
						0.5f
					);
				}
				else if (c->get_type() == 1)
				{
					Bresenham_circle(c->get_center().x, c->get_center().y, c->get_radius(), m_pBlackBrush);
				}
			}
		}

		for (auto c : circles)
		{
			if (c->get_is_exist() && c->is_selected(pt_mouse.x, pt_mouse.y))
			{
				// 使得鼠标移动到圆上时鼠标变成十字形状
				SetCursor(LoadCursor(NULL, IDC_CROSS));
				if (signal_rclk_mouse && c->get_is_exist())
				{
					// 鼠标右键点击时更改鼠标样式，圆消失
					SetCursor(LoadCursor(NULL, IDC_WAIT));
					if (state == TRANSLATE)
					{

					}
					else if (state == ROTATE)
					{

					}
					else if (state == ZOOM)
					{

					}
					else if (state == CLIP)
					{

					}
					else
						c->set_is_exist(false);
				}
			}
		}
				
		for (auto c : circles)
		{
			if (c->get_is_exist() && c->is_selected(pt_mouse.x, pt_mouse.y) && flag == 1)
			{
				if (state == TRANSLATE && translating != 4)
				{
					if (translating == 0)
					{
						//is_translating_c = true;
						translating = 4;

						index_c = std::distance(circles.begin(), std::find(circles.begin(), circles.end(), c));

						break;
					}
				}

				if (state == ROTATE && rotating != 4)
				{
					if (rotating == 0)
					{
						rotating = 4;
						index_c = std::distance(circles.begin(), std::find(circles.begin(), circles.end(), c));

						break;
					}
				}

				if (state == ZOOM && zooming != 4)
				{
					if (zooming == 0)
					{
						zooming = 4;
						index_c = std::distance(circles.begin(), std::find(circles.begin(), circles.end(), c));

						break;
					}
				}
			}

		}

		if (state == TRANSLATE && translating == 4 && !circles.empty())
		{
			if (flag == 1)
			{
				for (int i = 0; i < 20; ++i)
				{
					if (circles[index_c]->get_type() == 0)
					{
						m_pRenderTarget->DrawEllipse(
							D2D1::Ellipse(D2D1::Point2F(circles[index_c]->get_center().x, circles[index_c]->get_center().y), circles[index_c]->get_radius(), circles[index_c]->get_radius()),
							m_pWhiteBrush,
							0.5f
						);
					}
					else if (circles[index_c]->get_type() == 1)
					{
						Bresenham_circle(circles[index_c]->get_center().x, circles[index_c]->get_center().y, circles[index_c]->get_radius(), m_pWhiteBrush);
					}
				}

				circles[index_c]->set_is_exist(false);
				circles[index_c]->translate(dx, dy);

				if (circles[index_c]->get_type() == 0)
				{
					m_pRenderTarget->DrawEllipse(
						D2D1::Ellipse(D2D1::Point2F(circles[index_c]->get_center().x, circles[index_c]->get_center().y), circles[index_c]->get_radius(), circles[index_c]->get_radius()),
						m_pBlackBrush,
						0.5f
					);
				}
				else if (circles[index_c]->get_type() == 1)
				{
					Bresenham_circle(circles[index_c]->get_center().x, circles[index_c]->get_center().y, circles[index_c]->get_radius(), m_pBlackBrush);
				}

			}

			if (is_translate_finished)
			{
				for (int i = 0; i < 20; ++i)
				{
					if (circles[index_c]->get_type() == 0)
					{
						m_pRenderTarget->DrawEllipse(
							D2D1::Ellipse(D2D1::Point2F(circles[index_c]->get_center().x, circles[index_c]->get_center().y), circles[index_c]->get_radius(), circles[index_c]->get_radius()),
							m_pWhiteBrush,
							0.5f
						);
					}
					else if (circles[index_c]->get_type() == 1)
					{
						Bresenham_circle(circles[index_c]->get_center().x, circles[index_c]->get_center().y, circles[index_c]->get_radius(), m_pWhiteBrush);
					}
				}

				std::ofstream out_circle_file("circle.txt", std::ios::app | std::ios::out);

				out_circle_file << "0" << " " << circles[index_c]->get_type() << " " << circles[index_c]->get_center().x << " " << circles[index_c]->get_center().y << " " << circles[index_c]->get_radius() << std::endl;

				out_circle_file << "1" << " " << circles[index_c]->get_type() << " " << circles[index_c]->get_center().x + dx << " " << circles[index_c]->get_center().y + dy << " " << circles[index_c]->get_radius() << std::endl;

				out_circle_file.close();

				is_translate_finished = false;
				translating = 0;
				index_c = 0;
			}
		}

		if (state == ROTATE && rotating == 4 && !circles.empty())
		{
			if (flag == 1)
			{
				for (int i = 0; i < 20; ++i)
				{
					if (circles[index_c]->get_type() == 0)
					{
						m_pRenderTarget->DrawEllipse(
							D2D1::Ellipse(D2D1::Point2F(circles[index_c]->get_center().x, circles[index_c]->get_center().y), circles[index_c]->get_radius(), circles[index_c]->get_radius()),
							m_pWhiteBrush,
							0.5f
						);
					}
					else if (circles[index_c]->get_type() == 1)
					{
						Bresenham_circle(circles[index_c]->get_center().x, circles[index_c]->get_center().y, circles[index_c]->get_radius(), m_pWhiteBrush);
					}
				}

				// 用红心着重绘制
				D2D1_ELLIPSE tmp_e = D2D1::Ellipse(D2D1::Point2F(old_point.x, old_point.y), 2, 2);
				m_pRenderTarget->DrawEllipse(
					tmp_e,
					m_pRedBrush,
					1.5f
				);
				m_pRenderTarget->FillEllipse(tmp_e, m_pRedBrush);

				circles[index_c]->set_is_exist(false);
				circles[index_c]->rotate(old_point, pt_mouse);

				if (circles[index_c]->get_type() == 0)
				{
					m_pRenderTarget->DrawEllipse(
						D2D1::Ellipse(D2D1::Point2F(circles[index_c]->get_center().x, circles[index_c]->get_center().y), circles[index_c]->get_radius(), circles[index_c]->get_radius()),
						m_pBlackBrush,
						0.5f
					);
				}
				else if (circles[index_c]->get_type() == 1)
				{
					Bresenham_circle(circles[index_c]->get_center().x, circles[index_c]->get_center().y, circles[index_c]->get_radius(), m_pBlackBrush);
				}
			}

			if (is_rotate_finished)
			{
				for (int i = 0; i < 20; ++i)
				{
					if (circles[index_c]->get_type() == 0)
					{
						m_pRenderTarget->DrawEllipse(
							D2D1::Ellipse(D2D1::Point2F(circles[index_c]->get_center().x, circles[index_c]->get_center().y), circles[index_c]->get_radius(), circles[index_c]->get_radius()),
							m_pWhiteBrush,
							0.5f
						);
					}
					else if (circles[index_c]->get_type() == 1)
					{
						Bresenham_circle(circles[index_c]->get_center().x, circles[index_c]->get_center().y, circles[index_c]->get_radius(), m_pWhiteBrush);
					}
				}

				std::ofstream out_circle_file("circle.txt", std::ios::app | std::ios::out);

				out_circle_file << "0" << " " << circles[index_c]->get_type() << " " << circles[index_c]->get_center().x << " " << circles[index_c]->get_center().y << " " << circles[index_c]->get_radius() << std::endl;

				POINT center = circles[index_c]->get_center();
				center = this->rotate(center, old_point, pt_mouse);

				out_circle_file << "1" << " " << circles[index_c]->get_type() << " " << center.x << " " <<center.y << " " << circles[index_c]->get_radius() << std::endl;

				out_circle_file.close();

				is_rotate_finished = false;
				rotating = 0;
				index_c = 0;
			}
		}

		if (state == ZOOM && zooming == 4 && !circles.empty())
		{
			if (flag == 1)
			{
				for (int i = 0; i < 20; ++i)
				{
					if (circles[index_c]->get_type() == 0)
					{
						m_pRenderTarget->DrawEllipse(
							D2D1::Ellipse(D2D1::Point2F(circles[index_c]->get_center().x, circles[index_c]->get_center().y), circles[index_c]->get_radius(), circles[index_c]->get_radius()),
							m_pWhiteBrush,
							0.5f
						);
					}
					else if (circles[index_c]->get_type() == 1)
					{
						Bresenham_circle(circles[index_c]->get_center().x, circles[index_c]->get_center().y, circles[index_c]->get_radius(), m_pWhiteBrush);
					}
				}

				circles[index_c]->set_is_exist(false);
				circles[index_c]->zoom(factor);

				if (circles[index_c]->get_type() == 0)
				{
					m_pRenderTarget->DrawEllipse(
						D2D1::Ellipse(D2D1::Point2F(circles[index_c]->get_center().x, circles[index_c]->get_center().y), circles[index_c]->get_radius(), circles[index_c]->get_radius()),
						m_pBlackBrush,
						0.5f
					);
				}
				else if (circles[index_c]->get_type() == 1)
				{
					Bresenham_circle(circles[index_c]->get_center().x, circles[index_c]->get_center().y, circles[index_c]->get_radius(), m_pBlackBrush);
				}

				// 用红色线绘制oldpoint到pt_mouse的线段
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(old_point.x, old_point.y),
					D2D1::Point2F(pt_mouse.x, pt_mouse.y),
					m_pRedBrush,
					1.5f,
					m_pStrokeStyle
				);
			}

			if (is_zoom_finished)
			{
				for (int i = 0; i < 20; ++i)
				{
					if (circles[index_c]->get_type() == 0)
					{
						m_pRenderTarget->DrawEllipse(
							D2D1::Ellipse(D2D1::Point2F(circles[index_c]->get_center().x, circles[index_c]->get_center().y), circles[index_c]->get_radius(), circles[index_c]->get_radius()),
							m_pWhiteBrush,
							0.5f
						);
					}
					else if (circles[index_c]->get_type() == 1)
					{
						Bresenham_circle(circles[index_c]->get_center().x, circles[index_c]->get_center().y, circles[index_c]->get_radius(), m_pWhiteBrush);
					}
				}

				std::ofstream out_circle_file("circle.txt", std::ios::app | std::ios::out);

				out_circle_file << "0" << " " << circles[index_c]->get_type() << " " << circles[index_c]->get_center().x << " " << circles[index_c]->get_center().y << " " << circles[index_c]->get_radius() << std::endl;

				out_circle_file << "1" << " " << circles[index_c]->get_type() << " " << circles[index_c]->get_center().x << " " << circles[index_c]->get_center().y << " " << circles[index_c]->get_radius() * factor << std::endl;

				out_circle_file.close();

				is_zoom_finished = false;
				zooming = 0;
				index_c = 0;
			}
		}

		circle_file.close();

		if (state == CLIP)
		{
			D2D_RECT_F clip_rect = D2D1::RectF(old_point.x, old_point.y, pt_mouse.x, pt_mouse.y);

			if (flag == 1)
			{
				// 画一个虚线矩形框，左上角是old_point，右下角是pt_mouse
				m_pRenderTarget->DrawRectangle(
					clip_rect,
					m_pRedBrush,
					1.5f,
					m_pStrokeStyle
				);

				clip_points.clear();
			}

			if (is_clip_finished)
			{
				// 遍历所有直线，使用code_clip函数进行裁剪
				std::vector<POINT> tmp_points;
				std::vector<Line*> clipped_lines;
				clipped_lines.clear();
				for (auto l : lines)
				{
					if (l->get_is_exist())
					{
						tmp_points.clear();

						tmp_points = LiangBarsky(old_point, pt_mouse, l->get_start(), l->get_end());
						//tmp_points = code_clip(old_point, pt_mouse, l->get_start(), l->get_end());
						//tmp_points = code_clip({ 350, 20 }, { 660, 140 },l->get_start(), l->get_end());
						for (auto p : tmp_points)
							clip_points.push_back(p);

						if (!tmp_points.empty())
							clipped_lines.push_back(l);
					}
				}

				// 将clipped_lines中的直线数据写入line.txt
				std::ofstream out_line_file("line.txt", std::ios::app | std::ios::out);

				for (auto l : clipped_lines)
				{
					out_line_file << "0" << " " << l->get_type() << " " << l->get_start().x << " " << l->get_start().y << " " << l->get_end().x << " " << l->get_end().y << std::endl;
				}
				clipped_lines.clear();

				int size = clip_points.size();
				double k = min(rtSize.width / (pt_mouse.x - old_point.x), rtSize.height / (pt_mouse.y - old_point.y));
				for (int i = 0; i < size; ++i)
				{
					clip_points[i].x = (clip_points[i].x - old_point.x) * k;
					clip_points[i].y = (clip_points[i].y - old_point.y) * k;
				}

				for (int i = 0; i < clip_points.size(); i += 2)
				{
					out_line_file << "1" << " " << "0" << " " << clip_points[i].x << " " << clip_points[i].y << " " << clip_points[i + 1].x << " " << clip_points[i + 1].y << std::endl;
				}

				out_line_file.close();


				std::vector<Rectangles*> clipped_rect;
				clipped_rect.clear();

				for (auto r : rectangles)
				{
					if (r->get_is_exist())
					{
						if (r->is_in_rect(clip_rect))
							clipped_rect.push_back(r);
					}
				}

				std::ofstream out_rect_file("rectangle.txt", std::ios::app | std::ios::out);

				for (auto r : clipped_rect)
				{
					out_rect_file << "0" << " " << r->get_is_filled() << " " << r->get_left() << " " << r->get_top() << " " << r->get_right() << " " << r->get_bottom() << " " << r->get_lbleft() << " " << r->get_lbbottom() << " " << r->get_rtright() << " " << r->get_rttop() << std::endl;
				}

				size = clipped_rect.size();
				for (int i = 0; i < size; ++i)
				{
					// 把8个点放大到整个窗口
					clipped_rect[i]->set_left((clipped_rect[i]->get_left() - old_point.x) * k);
					clipped_rect[i]->set_top((clipped_rect[i]->get_top() - old_point.y) * k);
					clipped_rect[i]->set_right((clipped_rect[i]->get_right() - old_point.x) * k);
					clipped_rect[i]->set_bottom((clipped_rect[i]->get_bottom() - old_point.y) * k);
					clipped_rect[i]->set_lbleft((clipped_rect[i]->get_lbleft() - old_point.x) * k);
					clipped_rect[i]->set_lbbottom((clipped_rect[i]->get_lbbottom() - old_point.y) * k);
					clipped_rect[i]->set_rtright((clipped_rect[i]->get_rtright() - old_point.x) * k);
					clipped_rect[i]->set_rttop((clipped_rect[i]->get_rttop() - old_point.y) * k);
				}

				for (int i = 0; i < size; ++i)
				{
					out_rect_file << "1" << " " << clipped_rect[i]->get_is_filled() << " " << clipped_rect[i]->get_left() << " " << clipped_rect[i]->get_top() << " " << clipped_rect[i]->get_right() << " " << clipped_rect[i]->get_bottom() << " " << clipped_rect[i]->get_lbleft() << " " << clipped_rect[i]->get_lbbottom() << " " << clipped_rect[i]->get_rtright() << " " << clipped_rect[i]->get_rttop() << std::endl;
				}

				out_rect_file.close();

				std::vector<Circle*> clipped_circle;
				clipped_circle.clear();

				for (auto c : circles)
				{
					if (c->get_is_exist())
					{
						if (c->is_in_rect(clip_rect))
							clipped_circle.push_back(c);
					}
				}

				std::ofstream out_circle_file("circle.txt", std::ios::app | std::ios::out);

				for (auto c : clipped_circle)
				{
					out_circle_file << "0" << " " << c->get_type() << " " << c->get_center().x << " " << c->get_center().y << " " << c->get_radius() << std::endl;
				}

				size = clipped_circle.size();
				for (int i = 0; i < size; ++i)
				{
					// 把圆心放大到整个窗口
					POINT new_center = { (clipped_circle[i]->get_center().x - old_point.x) * k, (clipped_circle[i]->get_center().y - old_point.y) * k };
					clipped_circle[i]->set_center(new_center);
					clipped_circle[i]->set_radius(clipped_circle[i]->get_radius() * k);
				}

				for (int i = 0; i < size; ++i)
				{
					out_circle_file << "1" << " " << clipped_circle[i]->get_type() << " " << clipped_circle[i]->get_center().x << " " << clipped_circle[i]->get_center().y << " " << clipped_circle[i]->get_radius() << std::endl;
				}

				out_circle_file.close();


				std::vector<Ellipses*> clipped_ellipse;
				clipped_ellipse.clear();

				for (auto e : ellipses)
				{
					if (e->get_is_exist())
					{
						if (e->is_in_rect(clip_rect))
							clipped_ellipse.push_back(e);
					}
				}

				std::ofstream out_ellipse_file("ellipse.txt", std::ios::app | std::ios::out);

				for (auto e : clipped_ellipse)
				{
					out_ellipse_file << "0" << " " << e->get_center().x << " " << e->get_center().y << " " << e->get_radiusX() << " " << e->get_radiusY() << std::endl;
				}

				size = clipped_ellipse.size();

				for (int i = 0; i < size; ++i)
				{
					// 把椭圆中心放大到整个窗口
					POINT new_center = { (clipped_ellipse[i]->get_center().x - old_point.x) * k, (clipped_ellipse[i]->get_center().y - old_point.y) * k };
					clipped_ellipse[i]->set_center(new_center);
					clipped_ellipse[i]->set_radiusX(clipped_ellipse[i]->get_radiusX() * k);
					clipped_ellipse[i]->set_radiusY(clipped_ellipse[i]->get_radiusY() * k);
				}

				for (int i = 0; i < size; ++i)
				{
					out_ellipse_file << "1" << " " << clipped_ellipse[i]->get_center().x << " " << clipped_ellipse[i]->get_center().y << " " << clipped_ellipse[i]->get_radiusX() << " " << clipped_ellipse[i]->get_radiusY() << std::endl;
				}

				out_ellipse_file.close();


				std::vector<Polylines*> clipped_polylines;
				clipped_lines.clear();


				std::vector<D2D1_POINT_2F> tmp_poly_points;
				std::vector<std::vector<D2D1_POINT_2F>> polypoints_vv;
				std::vector<D2D1_POINT_2F> tmp_v;

				std::ofstream out_polyline_file("polyline.txt", std::ios::app | std::ios::out);

				for (auto p : polylines)
				{
					if (p->get_is_exist() && p->get_is_closed())
					{
						if(!p->is_in_rect(old_point, pt_mouse))
						{
							tmp_poly_points.clear();
							D2D1_POINT_2F left_top = { old_point.x, old_point.y }, right_bottom = { pt_mouse.x, pt_mouse.y };
							D2D1_POINT_2F right_top = { pt_mouse.x, old_point.y }, left_bottom = { old_point.x, pt_mouse.y };

							std::vector<D2D1_POINT_2F> sp, cp;
							cp.push_back(left_top);
							cp.push_back(right_top);
							cp.push_back(right_bottom);
							cp.push_back(left_bottom);
							cp.push_back(left_top);

							for (auto pp : p->get_points())
							{
								sp.push_back(pp);
							}

							tmp_poly_points = WeilerAtherton(sp, cp);

							for (auto tcp : tmp_poly_points)
							{
								tmp_v.push_back(tcp);

								if (tmp_v[0] == tcp && tmp_v.size() != 1)
								{
									polypoints_vv.push_back(tmp_v);
									tmp_v.clear();
								}
							}

							out_polyline_file << "0\n0\n";
							for (auto pp : p->get_points())
							{
								out_polyline_file << static_cast<int>(pp.x) << " " << static_cast<int>(pp.y) << std::endl;
							}

							// 目前裁剪得到的几组点都被保存在二维数组polypoints_vv中
							for (auto pvv : polypoints_vv)
							{
								// 写入pl的数据到polyline.txt
								out_polyline_file << "1\n0\n";
								for (auto pp : pvv)
								{
									out_polyline_file << static_cast<int>(pp.x - old_point.x) * k  << " " << static_cast<int>(pp.y - old_point.y)  << std::endl;
								}
							}

							if (!tmp_poly_points.empty())
								clipped_polylines.push_back(p);
						}
						else
						{
							out_polyline_file << "0\n0\n";
							for (auto pp : p->get_points())
							{
								out_polyline_file << static_cast<int>(pp.x) << " " << static_cast<int>(pp.y) << std::endl;
							}

							out_polyline_file << "1\n";
							out_polyline_file << p->get_is_filled() << std::endl;
							for (auto pp : p->get_points())
							{
								out_polyline_file << static_cast<int>(pp.x - old_point.x) * k << " " << static_cast<int>(pp.y - old_point.y) << std::endl;
							}
						}
					}
				}

				out_polyline_file.close();


				clip_points.clear();
				is_clip_finished = false;
			}
		}

		// 实时绘制部分
		switch (state)
		{
		case LINE:
		{
			std::vector<POINT> footpoints;
			get_footpoint(start, footpoints);
			// 实时绘制鼠标移动时的直线
			if (flag == 1)
			{
				if (!is_perpendicular)
				{
					if (type_line == 0)
					{
						m_pRenderTarget->DrawLine(
							D2D1::Point2F(start.x, start.y),
							D2D1::Point2F(pt_mouse.x, pt_mouse.y),
							m_pBlackBrush,
							0.5f
						);
					}
					else if (type_line == 1)
						Bresenham_line(start.x, start.y, pt_mouse.x, pt_mouse.y);
				}
				else
				{
					// 对每个直线的垂足点与鼠标点的距离进行比较，找到距离最短的垂足，如果鼠标到垂足的距离小于10，则绘制垂足到鼠标的直线，并突出绘制垂足
					int min_d = 100000;
					int index = 0;
					for (int i = 0; i < footpoints.size(); ++i)
					{
						int distance = (int)sqrt((double)pow((footpoints[i].x - pt_mouse.x), 2) + (double)pow((footpoints[i].y - pt_mouse.y), 2));
						if (distance < min_d)
						{
							min_d = distance;
							index = i;
						}
					}

					if (min_d < 10)
					{
						footpoint = { footpoints[index].x, footpoints[index].y };

						m_pRenderTarget->DrawLine(
							D2D1::Point2F(footpoints[index].x, footpoints[index].y),
							D2D1::Point2F(start.x, start.y),
							m_pBlackBrush,
							0.5f
						);

						// 绘制一个×
						m_pRenderTarget->DrawLine(
							D2D1::Point2F(footpoints[index].x - 5, footpoints[index].y - 5),
							D2D1::Point2F(footpoints[index].x + 5, footpoints[index].y + 5),
							m_pBlackBrush,
							0.5f
						);

						m_pRenderTarget->DrawLine(
							D2D1::Point2F(footpoints[index].x - 5, footpoints[index].y + 5),
							D2D1::Point2F(footpoints[index].x + 5, footpoints[index].y - 5),
							m_pBlackBrush,
							0.5f
						);

						// 用红心着重绘制
						m_pRenderTarget->DrawEllipse(
							D2D1::Ellipse(D2D1::Point2F(footpoints[index].x, footpoints[index].y), 1, 1),
							m_pRedBrush,
							0.5f
						);
					}
					else
					{
						m_pRenderTarget->DrawLine(
							D2D1::Point2F(start.x, start.y),
							D2D1::Point2F(pt_mouse.x, pt_mouse.y),
							m_pBlackBrush,
							0.5f
						);
					}
				}
			}
			break;
		}
		case ELLIPSE:
		{
			if (flag == 1)
			{
				SetCursor(LoadCursor(NULL, IDC_CROSS));
				m_pRenderTarget->DrawEllipse(
					D2D1::Ellipse(D2D1::Point2F(start.x, start.y), (float)(pt_mouse.x - start.x), (float)(pt_mouse.y - start.y)),
					m_pBlackBrush,
					0.5f
				);
				break;
			}
		}
		case RECTANGLE:
		{
			if (flag == 1)
			{
				SetCursor(LoadCursor(NULL, IDC_CROSS));
				m_pRenderTarget->DrawRectangle(
					D2D1::RectF(start.x, start.y, pt_mouse.x, pt_mouse.y),
					m_pBlackBrush,
					0.5f
				);
				break;
			}
		}
		case BEZIER:
		{
			if (flag_bezier)
				SetCursor(LoadCursor(NULL, IDC_CROSS));
			POINT pt[4] = { start, control1, control2, pt_mouse };

			if (flag_bezier == 1)
			{
				// 用黑色实线绘制×,用红色突出点
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(start.x - 5, start.y - 5),
					D2D1::Point2F(start.x + 5, start.y + 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(start.x - 5, start.y + 5),
					D2D1::Point2F(start.x + 5, start.y - 5),
					m_pBlackBrush,
					0.5f
				);

				// 用红色突出绘制点
				m_pRenderTarget->DrawEllipse(
					D2D1::Ellipse(D2D1::Point2F(start.x, start.y), 1, 1),
					m_pRedBrush,
					0.5f
				);
			}

			else if (flag_bezier == 2)
			{
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(start.x - 5, start.y - 5),
					D2D1::Point2F(start.x + 5, start.y + 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(start.x - 5, start.y + 5),
					D2D1::Point2F(start.x + 5, start.y - 5),
					m_pBlackBrush,
					0.5f
				);

				// 用红色突出绘制点
				m_pRenderTarget->DrawEllipse(
					D2D1::Ellipse(D2D1::Point2F(start.x, start.y), 1, 1),
					m_pRedBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(control1.x - 5, control1.y - 5),
					D2D1::Point2F(control1.x + 5, control1.y + 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(control1.x - 5, control1.y + 5),
					D2D1::Point2F(control1.x + 5, control1.y - 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawEllipse(
					D2D1::Ellipse(D2D1::Point2F(control1.x, control1.y), 1, 1),
					m_pRedBrush,
					0.5f
				);
			}

			else if (flag_bezier == 3)
			{
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(start.x - 5, start.y - 5),
					D2D1::Point2F(start.x + 5, start.y + 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(start.x - 5, start.y + 5),
					D2D1::Point2F(start.x + 5, start.y - 5),
					m_pBlackBrush,
					0.5f
				);

				// 用红色突出绘制点
				m_pRenderTarget->DrawEllipse(
					D2D1::Ellipse(D2D1::Point2F(start.x, start.y), 1, 1),
					m_pRedBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(control1.x - 5, control1.y - 5),
					D2D1::Point2F(control1.x + 5, control1.y + 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(control1.x - 5, control1.y + 5),
					D2D1::Point2F(control1.x + 5, control1.y - 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawEllipse(
					D2D1::Ellipse(D2D1::Point2F(control1.x, control1.y), 1, 1),
					m_pRedBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(control2.x - 5, control2.y - 5),
					D2D1::Point2F(control2.x + 5, control2.y + 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(control2.x - 5, control2.y + 5),
					D2D1::Point2F(control2.x + 5, control2.y - 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawEllipse(
					D2D1::Ellipse(D2D1::Point2F(control2.x, control2.y), 1, 1),
					m_pRedBrush,
					0.5f
				);

				this->DrawBezier(D2D1::Point2F(start.x, start.y), D2D1::Point2F(control1.x, control1.y), D2D1::Point2F(control2.x, control2.y), D2D1::Point2F(pt_mouse.x, pt_mouse.y), m_pBlackBrush, 0.5f);
			}
			break;
		}

		case BLINE:
		{
			if (flag_bline)
				SetCursor(LoadCursor(NULL, IDC_CROSS));
			POINT pt[4] = { start, bcontrol1, bcontrol2, pt_mouse };

			if (flag_bline == 1)
			{
				// 用黑色实线绘制×,用红色突出点
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(start.x - 5, start.y - 5),
					D2D1::Point2F(start.x + 5, start.y + 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(start.x - 5, start.y + 5),
					D2D1::Point2F(start.x + 5, start.y - 5),
					m_pBlackBrush,
					0.5f
				);

				// 用红色突出绘制点
				m_pRenderTarget->DrawEllipse(
					D2D1::Ellipse(D2D1::Point2F(start.x, start.y), 1, 1),
					m_pRedBrush,
					0.5f
				);
			}

			else if (flag_bline == 2)
			{
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(start.x - 5, start.y - 5),
					D2D1::Point2F(start.x + 5, start.y + 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(start.x - 5, start.y + 5),
					D2D1::Point2F(start.x + 5, start.y - 5),
					m_pBlackBrush,
					0.5f
				);

				// 用红色突出绘制点
				m_pRenderTarget->DrawEllipse(
					D2D1::Ellipse(D2D1::Point2F(start.x, start.y), 1, 1),
					m_pRedBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(bcontrol1.x - 5, bcontrol1.y - 5),
					D2D1::Point2F(bcontrol1.x + 5, bcontrol1.y + 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(bcontrol1.x - 5, bcontrol1.y + 5),
					D2D1::Point2F(bcontrol1.x + 5, bcontrol1.y - 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawEllipse(
					D2D1::Ellipse(D2D1::Point2F(bcontrol1.x, bcontrol1.y), 1, 1),
					m_pRedBrush,
					0.5f
				);
			}

			else if (flag_bline == 3)
			{
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(start.x - 5, start.y - 5),
					D2D1::Point2F(start.x + 5, start.y + 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(start.x - 5, start.y + 5),
					D2D1::Point2F(start.x + 5, start.y - 5),
					m_pBlackBrush,
					0.5f
				);

				// 用红色突出绘制点
				m_pRenderTarget->DrawEllipse(
					D2D1::Ellipse(D2D1::Point2F(start.x, start.y), 1, 1),
					m_pRedBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(bcontrol1.x - 5, bcontrol1.y - 5),
					D2D1::Point2F(bcontrol1.x + 5, bcontrol1.y + 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(bcontrol1.x - 5, bcontrol1.y + 5),
					D2D1::Point2F(bcontrol1.x + 5, bcontrol1.y - 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawEllipse(
					D2D1::Ellipse(D2D1::Point2F(bcontrol1.x, bcontrol1.y), 1, 1),
					m_pRedBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(bcontrol2.x - 5, bcontrol2.y - 5),
					D2D1::Point2F(bcontrol2.x + 5, bcontrol2.y + 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(bcontrol2.x - 5, bcontrol2.y + 5),
					D2D1::Point2F(bcontrol2.x + 5, bcontrol2.y - 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawEllipse(
					D2D1::Ellipse(D2D1::Point2F(bcontrol2.x, bcontrol2.y), 1, 1),
					m_pRedBrush,
					0.5f
				);

				this->DrawBLine(D2D1::Point2F(start.x, start.y), D2D1::Point2F(bcontrol1.x, bcontrol1.y), D2D1::Point2F(bcontrol2.x, bcontrol2.y), D2D1::Point2F(pt_mouse.x, pt_mouse.y), m_pBlackBrush);
			}
			break;
		}

		case POLYLINE:
		{
			if (is_drawing_polyline)
			{
				SetCursor(LoadCursor(NULL, IDC_CROSS));

				// 如果poly_points非空，绘制它最后一个点到鼠标点的直线
				if (!poly_points.empty())
				{
					int distance = 10000;
					distance = (int)sqrt((double)pow((poly_points[0].x - pt_mouse.x), 2) + (double)pow((poly_points[0].y - pt_mouse.y), 2));
					if (poly_points.size() > 1 && distance < 10)
					{
						is_closed = true;

						// 绘制最后一个点到起点，用×和红点突出
						m_pRenderTarget->DrawLine(
							D2D1::Point2F(poly_points[0].x - 5, poly_points[0].y - 5),
							D2D1::Point2F(poly_points[0].x + 5, poly_points[0].y + 5),
							m_pBlackBrush,
							0.5f
						);

						m_pRenderTarget->DrawLine(
							D2D1::Point2F(poly_points[0].x - 5, poly_points[0].y + 5),
							D2D1::Point2F(poly_points[0].x + 5, poly_points[0].y - 5),
							m_pBlackBrush,
							0.5f
						);

						m_pRenderTarget->DrawEllipse(
							D2D1::Ellipse(D2D1::Point2F(poly_points[0].x, poly_points[0].y), 1, 1),
							m_pRedBrush,
							0.5f
						);

						m_pRenderTarget->DrawLine(
							poly_points.back(),
							D2D1::Point2F(poly_points[0].x, poly_points[0].y),
							m_pBlackBrush,
							0.5f
						);
					}
					else
					{
						m_pRenderTarget->DrawLine(
							poly_points.back(),
							D2D1::Point2F(pt_mouse.x, pt_mouse.y),
							m_pBlackBrush,
							0.5f
						);
					}
				}
			}

			break;
		}
		case CIRCLE:
		{
			if (flag == 1)
			{
				SetCursor(LoadCursor(NULL, IDC_CROSS));
				// 画一个圆，圆的半径是start点到pt_mouse点的距离

				double r = sqrt((double)pow((pt_mouse.x - start.x), 2) + (double)pow((pt_mouse.y - start.y), 2));
				if (type_circle == 0)
				{
					m_pRenderTarget->DrawEllipse(
						D2D1::Ellipse(D2D1::Point2F(start.x, start.y), r, r),
						m_pBlackBrush,
						0.5f
					);
				}
				else if (type_circle == 1)
				{
					Bresenham_circle(start.x, start.y, r, m_pBlackBrush);
				}
				break;
			}
			break;
		}

		default:
			break;
		}

		for (auto e : ellipses)
		{
			if (e->get_is_exist() && e->is_selected(pt_mouse.x, pt_mouse.y))
			{
				// 在底部显示椭圆中心坐标
				std::string str = "Center (x:" + std::to_string(e->get_center().x) + ", y:" + std::to_string(e->get_center().y) + ")";
				std::wstring widestr = std::wstring(str.begin(), str.end());
				const wchar_t* szText = widestr.c_str();

				// 将选中椭圆的圆心绘制一个斜的十字，十字交点为圆心
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(e->get_center().x - 5, e->get_center().y - 5),
					D2D1::Point2F(e->get_center().x + 5, e->get_center().y + 5),
					m_pBlackBrush,
					0.5f
				);
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(e->get_center().x - 5, e->get_center().y + 5),
					D2D1::Point2F(e->get_center().x + 5, e->get_center().y - 5),
					m_pBlackBrush,
					0.5f
				);

				// 将选中椭圆的圆心用红色点绘制
				m_pRenderTarget->DrawEllipse(
					D2D1::Ellipse(D2D1::Point2F(e->get_center().x, e->get_center().y), 1, 1),
					m_pRedBrush,
					0.5f
				);

				m_pRenderTarget->DrawText(
					szText,
					wcslen(szText),
					m_pTextFormat,
					D2D1::RectF(5 + rtSize.width / 2, rtSize.height - 20, rtSize.width, rtSize.height),
					m_pBlackBrush
				);
			}
		}

		for (auto c : circles)
		{
			if (c->get_is_exist() && c->is_selected(pt_mouse.x, pt_mouse.y))
			{
				std::string str = "Center (x:" + std::to_string(c->get_center().x) + ", y:" + std::to_string(c->get_center().y) + ")";

				std::wstring widestr = std::wstring(str.begin(), str.end());

				const wchar_t* szText = widestr.c_str();

				// 将选中圆的圆心绘制一个斜的十字，十字交点为圆心
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(c->get_center().x - 5, c->get_center().y - 5),
					D2D1::Point2F(c->get_center().x + 5, c->get_center().y + 5),
					m_pBlackBrush,
					0.5f
				);

				m_pRenderTarget->DrawLine(
					D2D1::Point2F(c->get_center().x - 5, c->get_center().y + 5),
					D2D1::Point2F(c->get_center().x + 5, c->get_center().y - 5),
					m_pBlackBrush,
					0.5f
				);

				// 将选中圆的圆心用红色点绘制
				m_pRenderTarget->DrawEllipse(
					D2D1::Ellipse(D2D1::Point2F(c->get_center().x, c->get_center().y), 1, 1),
					m_pRedBrush,
					0.5f
				);

				m_pRenderTarget->DrawText(
					szText,
					wcslen(szText),
					m_pTextFormat,
					D2D1::RectF(5 + rtSize.width / 2, rtSize.height - 20, rtSize.width, rtSize.height),
					m_pBlackBrush
				);
			}
		}

		std::ofstream out_line_file("line.txt", std::ios::app | std::ios::out);

		for (auto l : lines)
		{
			if (signal_rclk_mouse && l->is_selected(pt_mouse.x, pt_mouse.y))
			{
				out_line_file << l->get_is_exist() << " " << l->get_type() << " " << l->get_start().x << " " << l->get_start().y << " " << l->get_end().x << " " << l->get_end().y << std::endl;

				// 使得右键点击后直线立即被白色画刷重绘
				if (!l->get_is_exist())
				{
					for (int i = 0; i < 20; ++i)
						l->draw(m_pRenderTarget, m_pWhiteBrush, 1.5f);
				}
				signal_rclk_mouse = false;
			}
		}

		out_line_file.close();

		std::ofstream out_ellipse_file("ellipse.txt", std::ios::app | std::ios::out);

		for (auto e : ellipses)
		{
			if (signal_rclk_mouse && e->is_selected(pt_mouse.x, pt_mouse.y))
			{
				out_ellipse_file << e->get_is_exist() << " " << e->get_center().x << " " << e->get_center().y << " " << e->get_radiusX() << " " << e->get_radiusY() << std::endl;

				// 使得右键点击后椭圆立即被白色画刷重绘
				if (!e->get_is_exist())
				{
					for (int i = 0; i < 20; ++i)
						e->draw(m_pRenderTarget, m_pWhiteBrush, 0.5f);
				}
				signal_rclk_mouse = false;
			}
		}

		out_ellipse_file.close();

		std::ofstream out_rectangle_file("rectangle.txt", std::ios::app | std::ios::out);

		for (auto r : rectangles)
		{
			if (signal_rclk_mouse && r->is_selected(pt_mouse.x, pt_mouse.y))
			{
				if (state != FILL)
				{
					out_rectangle_file << r->get_is_exist() << " " << "0 " << r->get_left() << " " << r->get_top() << " " << r->get_right() << " " << r->get_bottom() << " " << r->get_lbleft() << " " << r->get_lbbottom() << " " << r->get_rtright() << " " << r->get_rttop() << std::endl;

					// 使得右键点击后矩形立即被白色画刷重绘
					if (!r->get_is_exist())
					{
						for (int i = 0; i < 20; ++i)
							r->draw(m_pRenderTarget, m_pWhiteBrush, 0.5f);
					}
				}
				else
				{
					out_rectangle_file << r->get_is_exist() << " " << r->get_is_filled() << " " << r->get_left() << " " << r->get_top() << " " << r->get_right() << " " << r->get_bottom() << " " << r->get_lbleft() << " " << r->get_lbbottom() << " " << r->get_rtright() << " " << r->get_rttop() << std::endl;

					// fill_rect(r->get_left(), r->get_top(), r->get_right(), r->get_bottom());

					vertices.clear();
					vertices.push_back({ (int)r->get_left(), (int)r->get_top() });
					vertices.push_back({ (int)r->get_rtright() - 1,(int)r->get_rttop() });
					vertices.push_back({ (int)r->get_right() - 1, (int)r->get_bottom() - 1 });
					vertices.push_back({ (int)r->get_lbleft(), (int)r->get_lbbottom() - 1 });

					scanline(vertices);
				}
				signal_rclk_mouse = false;
			}

			if (signal_dblclk && r->is_selected(pt_mouse.x, pt_mouse.y))
			{
				out_rectangle_file << r->get_is_exist() << " " << "1 " << r->get_left() << " " << r->get_top() << " " << r->get_right() << " " << r->get_bottom() << " " << r->get_lbleft() << " " << r->get_lbbottom() << " " << r->get_rtright() << " " << r->get_rttop() << std::endl;

				signal_dblclk = false;
			}
		}

		out_rectangle_file.close();

		std::ofstream out_bezier_file("bezier.txt", std::ios::app | std::ios::out);

		for (auto b : bezier)
		{
			if (signal_rclk_mouse && b->is_selected(pt_mouse.x, pt_mouse.y))
			{
				out_bezier_file << b->get_is_exist() << " " << b->get_d2d_point1().x << " " << b->get_d2d_point1().y << " " << b->get_d2d_point2().x << " " << b->get_d2d_point2().y << " " << b->get_d2d_point3().x << " " << b->get_d2d_point3().y << " " << b->get_d2d_point4().x << " " << b->get_d2d_point4().y << std::endl;

				// 使得右键点击后贝塞尔曲线立即被白色画刷重绘
				if (!b->get_is_exist())
				{
					this->DrawBezier(b->get_d2d_point1(), b->get_d2d_point2(), b->get_d2d_point3(), b->get_d2d_point4(), m_pWhiteBrush, 0.5f);
				}
				signal_rclk_mouse = false;
			}
		}

		out_bezier_file.close();

		std::ofstream out_bline_file("bline.txt", std::ios::app | std::ios::out);

		for (auto b : blines)
		{
			if (signal_rclk_mouse && b->is_selected(pt_mouse.x, pt_mouse.y))
			{
				out_bline_file << b->get_is_exist() << " " << b->get_d2d_point1().x << " " << b->get_d2d_point1().y << " " << b->get_d2d_point2().x << " " << b->get_d2d_point2().y << " " << b->get_d2d_point3().x << " " << b->get_d2d_point3().y << " " << b->get_d2d_point4().x << " " << b->get_d2d_point4().y << std::endl;

				// 使得右键点击后贝塞尔曲线立即被白色画刷重绘
				if (!b->get_is_exist())
				{
					this->DrawBLine(b->get_d2d_point1(), b->get_d2d_point2(), b->get_d2d_point3(), b->get_d2d_point4(), m_pWhiteBrush);
				}
				signal_rclk_mouse = false;
			}
		}

		out_bline_file.close();

		std::ofstream out_polyline_file("polyline.txt", std::ios::app | std::ios::out);

		for (auto p : polylines)
		{
			if (signal_rclk_mouse && p->is_selected(pt_mouse.x, pt_mouse.y))
			{
				if (state != FILL)
				{
					out_polyline_file << p->get_is_exist() << std::endl << "0" << std::endl;
					for (auto point : p->get_points())
					{
						out_polyline_file << point.x << " " << point.y << std::endl;
					}

					// 使得右键点击后多段线立即被白色画刷重绘
					if (!p->get_is_exist())
					{
						for (int i = 0; i < 20; ++i)
						{
							for (int i = 0; i < p->get_points().size() - 1; ++i)
							{
								m_pRenderTarget->DrawLine(
									p->get_points()[i],
									p->get_points()[i + 1],
									m_pWhiteBrush,
									0.5f
								);
							}
						}
					}
				}
				else if(state == FILL)
				{
					out_polyline_file << p->get_is_exist() << std::endl << p->get_is_filled() << std::endl;
					for (auto point : p->get_points())
					{
						out_polyline_file << point.x << " " << point.y << std::endl;
					}

					int len = p->get_points().size();

					fill_triangle(p->get_points()[0].x, p->get_points()[0].y, p->get_points()[1].x, p->get_points()[1].y, p->get_points()[2].x, p->get_points()[2].y);
				}
				signal_rclk_mouse = false;
			}
		}

		out_polyline_file.close();
		

		std::ofstream out_circle_file("circle.txt", std::ios::app | std::ios::out);
		 
		for (auto c : circles)
		{
			if (signal_rclk_mouse && c->is_selected(pt_mouse.x, pt_mouse.y))
			{
				out_circle_file << c->get_is_exist() << " " << c->get_type() << " " << c->get_center().x << " " << c->get_center().y << " " << c->get_radius() << std::endl;

				// 使得右键点击后圆立即被白色画刷重绘
				if (!c->get_is_exist())
				{
					for (int i = 0; i < 20; ++i)
					{
						m_pRenderTarget->DrawEllipse(
							D2D1::Ellipse(D2D1::Point2F(c->get_center().x, c->get_center().y), c->get_radius(), c->get_radius()),
							m_pWhiteBrush,
							0.5f
						);
					}
				}
				signal_rclk_mouse = false;
			}
		}

		out_circle_file.close();

		//std::vector<D2D1_POINT_2F> sp, cp;
		//cp.push_back({ 300, 100 });
		//cp.push_back({ 300, 400 });
		//cp.push_back({ 600, 400 });
		//cp.push_back({ 600, 100 });
		//cp.push_back({ 300, 100 });

		//sp.push_back({ 260, 140 });
		//sp.push_back({ 460, 450 });
		//sp.push_back({ 460, 300 });
		//sp.push_back({ 650, 300 });
		//sp.push_back({ 500, 60 });
		//sp.push_back({ 400, 120 });
		//sp.push_back({ 260, 140 });
		////std::vector<D2D1_POINT_2F> tmp_ = WeilerAtherton(sp, cp);

		////std::vector<D2D1_POINT_2F> sp, cp;
		////cp.push_back({ 100, 100 });
		////cp.push_back({ 100, 500 });
		////cp.push_back({ 300, 500 });
		////cp.push_back({ 300, 100 });
		////cp.push_back({ 100, 100 });

		////sp.push_back({ 50, 400 });
		////sp.push_back({ 150, 400 });
		////sp.push_back({ 75, 300 });
		////sp.push_back({ 150, 200 });
		////sp.push_back({ 75, 200 });
		////sp.push_back({ 150, 150 });
		////sp.push_back({ 50, 40 });
		////sp.push_back({ 50, 400 });
		//std::vector<D2D1_POINT_2F> tmp_clipped_polygon = WeilerAtherton(sp, cp);
		//std::vector<std::vector<D2D1_POINT_2F>> clipped_polygon;
		//std::vector<D2D1_POINT_2F> tmp_v;

		//for (auto tcp : tmp_clipped_polygon)
		//{
		//	tmp_v.push_back(tcp);

		//	if(tmp_v[0] == tcp && tmp_v.size() != 1)
		//	{
		//		clipped_polygon.push_back(tmp_v);
		//		tmp_v.clear();
		//	}
		//}

		//for (auto cpg : clipped_polygon)
		//{
		//	int len_cpg = cpg.size();
		//	for (int i = 0; i < len_cpg - 1; ++i)
		//		m_pRenderTarget->DrawLine(cpg[i], cpg[i + 1], m_pBlackBrush, 1.0f);
		//}

		//int len_cp = cp.size();
		//for (int i = 0; i < len_cp - 1; ++i)
		//	m_pRenderTarget->DrawLine(cp[i], cp[i + 1], m_pBLineBrush, 0.5f);

		//int len_sp = sp.size();
		//for (int i = 0; i < len_sp - 1; ++i)
		//	m_pRenderTarget->DrawLine(sp[i], sp[i + 1], m_pRedBrush, 0.5f);

		//sp.clear();
		//cp.clear();

		//POINT A = { 100, 100 }, C = { 300, 200 };
		//POINT P1 = { 150, 150 }, P2 = { 250, 250 };

		//std::vector<POINT> tmp_p = LiangBarsky(A, C, P1, P2);


		// 绘制交点
		std::vector<POINT> inter_points;
		std::vector<POINT> inter_points_le;
		std::vector<POINT> inter_points_ee;

		// 求所有线段的交点
		for (int i = 0; i < lines.size(); ++i)
		{
			for (int j = i + 1; j < lines.size(); ++j)
			{
				if (lines[i]->get_is_exist() && lines[j]->get_is_exist())
				{
					POINT p[4] = { lines[i]->get_start(), lines[i]->get_end(), lines[j]->get_start(), lines[j]->get_end() };
					if (is_intersect_ll(p[0], p[1], p[2], p[3]))
					{
						inter_points.push_back(get_intersect_ll(p[0], p[1], p[2], p[3]));
					}
				}
			}
		}

		// 用×绘制交点
		for (auto p : inter_points)
		{
			// 如果点到鼠标的距离小于10，则用红色×绘制
			float distance = sqrt((p.x - pt_mouse.x) * (p.x - pt_mouse.x) + (p.y - pt_mouse.y) * (p.y - pt_mouse.y));
			if (distance < 10)
			{
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(p.x - 5, p.y - 5),
					D2D1::Point2F(p.x + 5, p.y + 5),
					m_pBlackBrush,
					0.5f
				);
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(p.x - 5, p.y + 5),
					D2D1::Point2F(p.x + 5, p.y - 5),
					m_pBlackBrush,
					0.5f
				);

				// 用红心突出
				m_pRenderTarget->DrawEllipse(
					D2D1::Ellipse(D2D1::Point2F(p.x, p.y), 1, 1),
					m_pRedBrush,
					0.5f
				);

				// 在底部显示交点坐标
				std::string str = "Intersection (x:" + std::to_string(p.x) + ", y:" + std::to_string(p.y) + ")";
				std::wstring widestr = std::wstring(str.begin(), str.end());
				const wchar_t* szText = widestr.c_str();
				m_pRenderTarget->DrawText(
					szText,
					wcslen(szText),
					m_pTextFormat,
					D2D1::RectF(5 + rtSize.width / 2, rtSize.height - 40, rtSize.width, rtSize.height - 20),
					m_pBlackBrush
				);
			}
		}

		// 绘制线段与椭圆的交点
		for (auto l : lines)
		{
			for (auto e : ellipses)
			{
				if (l->get_is_exist() && e->get_is_exist())
				{
					get_intersect_le(l->get_start(), l->get_end(), e->get_ellipse(), inter_points_le);
				}
			}

			for (auto c : circles)
			{
				if (l->get_is_exist() && c->get_is_exist())
				{
					get_intersect_le(l->get_start(), l->get_end(), c->get_ellipse(), inter_points_le);
				}
			}
		}

		// 用×绘制交点
		for (auto p : inter_points_le)
		{
			// 如果点到鼠标的距离小于10，则用红色×绘制
			float distance = sqrt((p.x - pt_mouse.x) * (p.x - pt_mouse.x) + (p.y - pt_mouse.y) * (p.y - pt_mouse.y));
			if (distance < 10)
			{
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(p.x - 5, p.y - 5),
					D2D1::Point2F(p.x + 5, p.y + 5),
					m_pBlackBrush,
					0.5f
				);
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(p.x - 5, p.y + 5),
					D2D1::Point2F(p.x + 5, p.y - 5),
					m_pBlackBrush,
					0.5f
				);

				// 用红心突出
				m_pRenderTarget->DrawEllipse(
					D2D1::Ellipse(D2D1::Point2F(p.x, p.y), 1, 1),
					m_pRedBrush,
					0.5f
				);

				// 在底部显示交点坐标
				std::string str = "Intersection (x:" + std::to_string(p.x) + ", y:" + std::to_string(p.y) + ")";
				std::wstring widestr = std::wstring(str.begin(), str.end());
				const wchar_t* szText = widestr.c_str();
				m_pRenderTarget->DrawText(
					szText,
					wcslen(szText),
					m_pTextFormat,
					D2D1::RectF(5 + rtSize.width / 2, rtSize.height - 40, rtSize.width, rtSize.height - 20),
					m_pBlackBrush
				);
			}
		}

		// 绘制两圆交点
		for (auto e1 : circles)
		{
			for (auto e2 : circles)
			{
				get_intersect_ee(e1->get_ellipse(), e2->get_ellipse(), inter_points_ee);
			}
		}

		// 用×绘制交点
		for (auto p : inter_points_ee)
		{
			// 如果点到鼠标的距离小于10，则用红色×绘制
			float distance = sqrt((p.x - pt_mouse.x) * (p.x - pt_mouse.x) + (p.y - pt_mouse.y) * (p.y - pt_mouse.y));
			if (distance < 10)
			{
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(p.x - 5, p.y - 5),
					D2D1::Point2F(p.x + 5, p.y + 5),
					m_pBlackBrush,
					0.5f
				);
				m_pRenderTarget->DrawLine(
					D2D1::Point2F(p.x - 5, p.y + 5),
					D2D1::Point2F(p.x + 5, p.y - 5),
					m_pBlackBrush,
					0.5f
				);

				// 用红心突出
				m_pRenderTarget->DrawEllipse(
					D2D1::Ellipse(D2D1::Point2F(p.x, p.y), 1, 1),
					m_pRedBrush,
					0.5f
				);

				// 在底部显示交点坐标
				std::string str = "Intersection (x:" + std::to_string(p.x) + ", y:" + std::to_string(p.y) + ")";
				std::wstring widestr = std::wstring(str.begin(), str.end());
				const wchar_t* szText = widestr.c_str();
				m_pRenderTarget->DrawText(
					szText,
					wcslen(szText),
					m_pTextFormat,
					D2D1::RectF(5 + rtSize.width / 2, rtSize.height - 40, rtSize.width, rtSize.height - 20),
					m_pBlackBrush
				);
			}
		}

		res = m_pRenderTarget->EndDraw();	// 结束绘制

		if (res == D2DERR_RECREATE_TARGET)
		{
			res = S_OK;
			DiscardDeviceResources();
		}
	}

	return res;
}

void Melon::OnResize(UINT width, UINT height)
{
	if (m_pRenderTarget)
	{
		D2D1_SIZE_U size = D2D1::SizeU(width, height);	// 客户区大小

		m_pRenderTarget->Resize(size);	// 重置渲染目标大小
	}
}

LRESULT CALLBACK Melon::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT res = 0;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;	// 创建结构体指针
		Melon* pMelon = (Melon*)pcs->lpCreateParams;	// 获取窗口参数

		::SetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(pMelon)
		);

		res = 1;
	}
	else
	{
		Melon* pMelon = reinterpret_cast<Melon*>(static_cast<LONG_PTR>(
			::GetWindowLongPtrW(
				hwnd,
				GWLP_USERDATA
			)
			));

		bool wasHandled = false;

		if (pMelon)
		{
			switch (message)
			{
			case WM_SIZE:
			{
				UINT width = LOWORD(lParam);
				UINT height = HIWORD(lParam);
				pMelon->OnResize(width, height);
			}
			res = 0;
			wasHandled = true;
			break;

			case WM_DISPLAYCHANGE:
			{
				InvalidateRect(hwnd, nullptr, false);
			}
			res = 0;
			wasHandled = true;
			break;

			case WM_MBUTTONDOWN:
			{
				pMelon->OnRender();
				ValidateRect(hwnd, nullptr);
			}
			res = 0;
			wasHandled = true;
			break;

			case WM_LBUTTONDOWN:
			{
				switch (state)
				{
				case LINE:
				{
					std::fstream line_file("line.txt", std::ios::app | std::ios::out);
					if (!line_file.is_open())
					{
						exit(0);
					}
					line_file.clear();
					if (flag == -1)
					{
						start = { LOWORD(lParam), HIWORD(lParam) };
						if (type_line == 0)
						{
							line_file << true << " " << "0" << " " << start.x << " " << start.y << " ";
						}
						else if (type_line == 1)
						{
							line_file << true << " " << "1" << " " << start.x << " " << start.y << " ";
						}
					}
					else if (flag == 1 && is_perpendicular)	// 垂足
					{
						line_file << footpoint.x << " " << footpoint.y << std::endl;
						is_perpendicular = false;
					}
					else if (flag == 1 && !is_perpendicular)
					{
						end = { LOWORD(lParam), HIWORD(lParam) };
						line_file << end.x << " " << end.y << std::endl;
					}
					line_file.close();
					flag *= -1;
				}
				break;
				case ELLIPSE:
				{
					std::fstream ellipse_file("ellipse.txt", std::ios::app | std::ios::out);
					if (!ellipse_file.is_open())
					{
						exit(0);
					}
					if (flag == -1)
					{
						start = { LOWORD(lParam), HIWORD(lParam) };
						ellipse_file << true << " " << start.x << " " << start.y << " ";
					}
					else if (flag == 1)
					{
						end = { LOWORD(lParam), HIWORD(lParam) };
						ellipse_file << end.x - start.x << " " << end.y - start.y << std::endl;
					}
					ellipse_file.close();
					flag *= -1;
				}
				break;
				case RECTANGLE:
				{
					std::fstream rectangle_file("rectangle.txt", std::ios::app | std::ios::out);
					if (!rectangle_file.is_open())
					{
						exit(0);
					}
					if (flag == -1)
					{
						start = { LOWORD(lParam), HIWORD(lParam) };
						rectangle_file << true << " " << "0 " << start.x << " " << start.y << " ";
					}
					else if (flag == 1)
					{
						end = { LOWORD(lParam), HIWORD(lParam) };
						rectangle_file << end.x << " " << end.y << " " << start.x << " " << end.y << " " << end.x << " " << start.y << std::endl;
					}
					rectangle_file.close();
					flag *= -1;
				}
				break;
				case BEZIER:
				{
					++flag_bezier;
					if (flag_bezier == 5)
						flag_bezier = 0;

					std::fstream bezier_file("bezier.txt", std::ios::app | std::ios::out);
					if (!bezier_file.is_open())
					{
						exit(0);
					}

					switch (flag_bezier)
					{
					case 1:
					{
						start = { LOWORD(lParam), HIWORD(lParam) };
						bezier_file << true << " " << start.x << " " << start.y << " ";
						break;
					}
					case 2:
					{
						control1 = { LOWORD(lParam), HIWORD(lParam) };
						bezier_file << control1.x << " " << control1.y << " ";
						break;
					}
					case 3:
					{
						control2 = { LOWORD(lParam), HIWORD(lParam) };
						bezier_file << control2.x << " " << control2.y << " ";
						break;
					}
					case 4:
					{
						end = { LOWORD(lParam), HIWORD(lParam) };
						bezier_file << end.x << " " << end.y << std::endl;
						break;
					}
					default:
						break;
					}
					bezier_file.close();
				}
				break;
				case BLINE:
				{
					++flag_bline;
					if (flag_bline == 5)
						flag_bline = 0;

					std::fstream bline_file("bline.txt", std::ios::app | std::ios::out);
					if (!bline_file.is_open())
					{
						exit(0);
					}

					switch (flag_bline)
					{
					case 1:
					{
						start = { LOWORD(lParam), HIWORD(lParam) };
						bline_file << true << " " << start.x << " " << start.y << " ";
						break;
					}
					case 2:
					{
						bcontrol1 = { LOWORD(lParam), HIWORD(lParam) };
						bline_file << bcontrol1.x << " " << bcontrol1.y << " ";
						break;
					}
					case 3:
					{
						bcontrol2 = { LOWORD(lParam), HIWORD(lParam) };
						bline_file << bcontrol2.x << " " << bcontrol2.y << " ";
						break;
					}
					case 4:
					{
						end = { LOWORD(lParam), HIWORD(lParam) };
						bline_file << end.x << " " << end.y << std::endl;
						break;
					}
					default:
						break;
					}
					bline_file.close();
				}
				break;
				case POLYLINE:
				{
					std::fstream polyline_file("polyline.txt", std::ios::app | std::ios::out);
					if (!polyline_file.is_open())
					{
						exit(0);
					}

					if (is_drawing_polyline == false)	// 正在进行绘制多义线
					{
						is_drawing_polyline = true;

						polyline_file << "1" << std::endl;
						polyline_file << "0" << std::endl;

						// 写入鼠标坐标
						polyline_file << LOWORD(lParam) << " " << HIWORD(lParam) << std::endl;
						D2D1_POINT_2F poly_start_point = { LOWORD(lParam), HIWORD(lParam) };
						poly_points.push_back(poly_start_point);
					}
					else	// 正在进行绘制多义线，并且肯定不是第一个点
					{
						if (!is_closed)
						{
							// 写入鼠标坐标
							D2D1_POINT_2F poly_point = { LOWORD(lParam), HIWORD(lParam) };
							if (poly_point.x != poly_points.back().x && poly_point.y != poly_points.back().y)
							{
								poly_points.push_back(poly_point);
								polyline_file << LOWORD(lParam) << " " << HIWORD(lParam) << std::endl;
							}
						}
						else if (is_closed)
						{
							// 写入第一个点坐标
							D2D1_POINT_2F poly_point = { LOWORD(lParam), HIWORD(lParam) };
							if (poly_point.x != poly_points.back().x && poly_point.y != poly_points.back().y)
							{
								polyline_file << poly_points[0].x << " " << poly_points[0].y << std::endl;
								poly_points.push_back(poly_points[0]);
								is_closed = false;
							}
						}
					}

					polyline_file.close();
				}
				break;
				case CIRCLE:
				{
					std::fstream circle_file("circle.txt", std::ios::app | std::ios::out);
					if (!circle_file.is_open())
					{
						exit(0);
					}
					if (flag == -1)
					{
						start = { LOWORD(lParam), HIWORD(lParam) };
						if (type_circle == 0)
						{
							circle_file << true << " " << "0" << " " << start.x << " " << start.y << " ";
						}
						else if (type_circle == 1)
						{
							circle_file << true << " " << "1" << " " << start.x << " " << start.y << " ";
						}
					}
					else if (flag == 1)
					{
						end = { LOWORD(lParam), HIWORD(lParam) };
						// circle_file << end.x - start.x << " " << end.y - start.y << std::endl;
						// 写入圆的半径为end到start的距离
						circle_file << sqrt((double)pow(end.x - start.x, 2) + (double)pow(end.y - start.y, 2)) << std::endl;
					}
					circle_file.close();
					flag *= -1;
				}
				break;
				case TRANSLATE:
				{
					if (flag == -1)
					{
						old_point = { LOWORD(lParam), HIWORD(lParam) };
					}
					else
					{
						is_translate_finished = true;
					}
					flag *= -1;
				}
				break;
				case ROTATE:
				{
					if (flag == -1)
					{
						old_point = { LOWORD(lParam), HIWORD(lParam) };
					}
					else
					{
						is_rotate_finished = true;
					}
					flag *= -1;
				}
				break;
				case ZOOM:
				{
					if (flag == -1)
					{
						old_point = { LOWORD(lParam), HIWORD(lParam) };
					}
					else
					{
						is_zoom_finished = true;
					}
					flag *= -1;
				}
				break;
				case CLIP:
				{
					if (flag == -1)
					{
						old_point = { LOWORD(lParam), HIWORD(lParam) };
					}
					else
					{
						is_clip_finished = true;
					}
					flag *= -1;
				}
				break;
				default:
					break;
				}
				

				pMelon->OnRender();
				ValidateRect(hwnd, NULL);
			}
			res = 0;
			wasHandled = true;
			break;

			case WM_RBUTTONDOWN:
			{
				signal_rclk_mouse = true;
				pMelon->OnRender();
				ValidateRect(hwnd, nullptr);
			}
			res = 0;
			wasHandled = true;
			break;

			case WM_MOUSEMOVE:
			{
				pMelon->OnRender();
				ValidateRect(hwnd, NULL);
			}
			res = 0;
			wasHandled = true;
			break;

			case WM_LBUTTONDBLCLK:
			{
				signal_dblclk = true;
				pMelon->OnRender();
				ValidateRect(hwnd, nullptr);
			}
			res = 0;
			wasHandled = true;
			break;

			case WM_PAINT:
			{
				pMelon->OnRender();
				ValidateRect(hwnd, nullptr);
			}
			res = 0;
			wasHandled = true;
			break;

			// 响应菜单消息
			case WM_COMMAND:
			{
				switch (LOWORD(wParam))
				{
				case LINE:
				{
					state = LINE;
					type_line = 0;
					pMelon->OnRender();
					ValidateRect(hwnd, nullptr);
				}
				break;
				case BRESENHAM_LINE:
				{
					state = LINE;
					type_line = 1;
					pMelon->OnRender();
					ValidateRect(hwnd, nullptr);
				}
				break;
				case ELLIPSE:
				{
					state = ELLIPSE;
					pMelon->OnRender();
					ValidateRect(hwnd, nullptr);
				}
				break;
				case RECTANGLE:
				{
					state = RECTANGLE;
					pMelon->OnRender();
					ValidateRect(hwnd, nullptr);
				}
				break;
				case BEZIER:
				{
					state = BEZIER;
					pMelon->OnRender();
					ValidateRect(hwnd, nullptr);
				}
				break;
				case BLINE:
				{
					state = BLINE;
					pMelon->OnRender();
					ValidateRect(hwnd, nullptr);
				}
				break;
				case POLYLINE:
				{
					state = POLYLINE;
					pMelon->OnRender();
					ValidateRect(hwnd, nullptr);
				}
				break;
				case POLYLINE_OK:
				{
					is_drawing_polyline = false;
					poly_points.clear();
					pMelon->OnRender();
					ValidateRect(hwnd, nullptr);
				}
				break;
				case CIRCLE:
				{
					state = CIRCLE;
					type_circle = 0;
					pMelon->OnRender();
					ValidateRect(hwnd, nullptr);
				}
				break;
				case BRESENHAM_CIRCLE:
				{
					state = CIRCLE;
					type_circle = 1;
					pMelon->OnRender();
					ValidateRect(hwnd, nullptr);
				}
				break;
				case CLEAN:
				{
					is_clean = true;
					pMelon->OnRender();
					ValidateRect(hwnd, nullptr);
				}
				break;
				case PERPENDICULAR:
				{
					state = LINE;
					is_perpendicular = true;
					pMelon->OnRender();
					ValidateRect(hwnd, nullptr);
				}
				break;
				case SHOWGRID:
				{
					is_show_grid = !is_show_grid;
					pMelon->OnRender();
					ValidateRect(hwnd, nullptr);
				}
				break;
				case FILL:
				{
					state = FILL;
					pMelon->OnRender();
					ValidateRect(hwnd, nullptr);
				}
				break;
				case TRANSLATE:
				{
					state = TRANSLATE;
					pMelon->OnRender();
					ValidateRect(hwnd, nullptr);
				}
				break;
				case ROTATE:
				{
					state = ROTATE;
					pMelon->OnRender();
					ValidateRect(hwnd, nullptr);
				}
				break;
				case ZOOM:
				{
					state = ZOOM;
					pMelon->OnRender();
					ValidateRect(hwnd, nullptr);
				}
				break;
				case CLIP:
				{
					state = CLIP;
					pMelon->OnRender();
					ValidateRect(hwnd, nullptr);
				}
				break;

				default:
					break;
				}
			}
			res = 0;
			wasHandled = true;
			break;

			case WM_DESTROY:
			{
				PostQuitMessage(0);
			}
			res = 1;
			wasHandled = true;
			break;
			}
		}

		if (!wasHandled)
		{
			res = DefWindowProc(hwnd, message, wParam, lParam);
		}
	}

	return res;
}

void Melon::DrawBLine(D2D1_POINT_2F p0, D2D1_POINT_2F p1, D2D1_POINT_2F p2, D2D1_POINT_2F p3, ID2D1SolidColorBrush* m_pSolidColorBrush)
{
	D2D1_POINT_2F point;
	std::vector<D2D1_POINT_2F> points;

	for (double t = 0.0; t <= 1.0; t += 0.001)
	{
		// 以下为三次B样条曲线公式
		point.x = (1.0 / 6) * (-t * t * t + 3 * t * t - 3 * t + 1) * p0.x + (1.0 / 6) * (3 * t * t * t - 6 * t * t + 4) * p1.x + (1.0 / 6) * (-3 * t * t * t + 3 * t * t + 3 * t + 1) * p2.x + (1.0 / 6) * t * t * t * p3.x;
		point.y = (1.0 / 6) * (-t * t * t + 3 * t * t - 3 * t + 1) * p0.y + (1.0 / 6) * (3 * t * t * t - 6 * t * t + 4) * p1.y + (1.0 / 6) * (-3 * t * t * t + 3 * t * t + 3 * t + 1) * p2.y + (1.0 / 6) * t * t * t * p3.y;

		points.push_back(point);
	}

	for(auto p: points)
		put_pixel(static_cast<int>(p.x), static_cast<int>(p.y), m_pSolidColorBrush);
	points.clear();
}

void Melon::DrawBezier(D2D1_POINT_2F startPoint, D2D1_POINT_2F controlPoint1, D2D1_POINT_2F controlPoint2, D2D1_POINT_2F endPoint, ID2D1SolidColorBrush* m_pSolidColorBrush, float strokeWidth)
{
	ID2D1PathGeometry* pPathGeometry = nullptr;
	m_pDirect2dFactory->CreatePathGeometry(&pPathGeometry);

	ID2D1GeometrySink* pSink = nullptr;
	pPathGeometry->Open(&pSink);

	pSink->BeginFigure(startPoint, D2D1_FIGURE_BEGIN_FILLED);

	D2D1_BEZIER_SEGMENT bezierSegment = { controlPoint1, controlPoint2, endPoint };
	pSink->AddBezier(bezierSegment);

	pSink->EndFigure(D2D1_FIGURE_END_OPEN);
	pSink->Close();

	m_pRenderTarget->DrawGeometry(pPathGeometry, m_pSolidColorBrush, strokeWidth);

	SafeRelease(&pPathGeometry);
	SafeRelease(&pSink);
}

bool Melon::is_intersect_ll(POINT p1, POINT p2, POINT p3, POINT p4)
{
	if(max(p1.x, p2.x) < min(p3.x, p4.x) || max(p3.x, p4.x) < min(p1.x, p2.x) || max(p1.y, p2.y) < min(p3.y, p4.y) || max(p3.y, p4.y) < min(p1.y, p2.y))
		return false;

	int d1, d2, d3, d4;
	d1 = (p3.x - p4.x) * (p1.y - p3.y) - (p3.y - p4.y) * (p1.x - p3.x);
	d2 = (p3.x - p4.x) * (p2.y - p3.y) - (p3.y - p4.y) * (p2.x - p3.x);
	d3 = (p1.x - p2.x) * (p3.y - p1.y) - (p1.y - p2.y) * (p3.x - p1.x);
	d4 = (p1.x - p2.x) * (p4.y - p1.y) - (p1.y - p2.y) * (p4.x - p1.x);
	if (d1 * d2 <= 0 && d3 * d4 <= 0)
		return true;
	return false;
}

// 获取两线段交点
POINT Melon::get_intersect_ll(POINT p1, POINT p2, POINT p3, POINT p4)
{
	POINT p;
	p.x = ((p3.x - p4.x) * (p1.x * p2.y - p1.y * p2.x) - (p1.x - p2.x) * (p3.x * p4.y - p3.y * p4.x)) / ((p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x));
	p.y = ((p3.y - p4.y) * (p1.x * p2.y - p1.y * p2.x) - (p1.y - p2.y) * (p3.x * p4.y - p3.y * p4.x)) / ((p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x));
	return p;
}

bool Melon::is_intersect_ll(D2D1_POINT_2F p1, D2D1_POINT_2F p2, D2D1_POINT_2F p3, D2D1_POINT_2F p4)
{
	if (max(p1.x, p2.x) < min(p3.x, p4.x) || max(p3.x, p4.x) < min(p1.x, p2.x) || max(p1.y, p2.y) < min(p3.y, p4.y) || max(p3.y, p4.y) < min(p1.y, p2.y))
		return false;

	int d1, d2, d3, d4;
	d1 = (p3.x - p4.x) * (p1.y - p3.y) - (p3.y - p4.y) * (p1.x - p3.x);
	d2 = (p3.x - p4.x) * (p2.y - p3.y) - (p3.y - p4.y) * (p2.x - p3.x);
	d3 = (p1.x - p2.x) * (p3.y - p1.y) - (p1.y - p2.y) * (p3.x - p1.x);
	d4 = (p1.x - p2.x) * (p4.y - p1.y) - (p1.y - p2.y) * (p4.x - p1.x);

	if ((d1 == 0 && d2 == 0) || (d3 == 0 && d4 == 0)) {
		// Both segments are vertical
		// Add your special handling code here
		return false;
	}

	if (d1 * d2 < 0 && d3 * d4 < 0)
		return true;
	return false;
}

// 获取两线段交点
D2D1_POINT_2F Melon::get_intersect_ll(D2D1_POINT_2F p1, D2D1_POINT_2F p2, D2D1_POINT_2F p3, D2D1_POINT_2F p4)
{
	D2D1_POINT_2F p;
	p.x = ((p3.x - p4.x) * (p1.x * p2.y - p1.y * p2.x) - (p1.x - p2.x) * (p3.x * p4.y - p3.y * p4.x)) / ((p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x));
	p.y = ((p3.y - p4.y) * (p1.x * p2.y - p1.y * p2.x) - (p1.y - p2.y) * (p3.x * p4.y - p3.y * p4.x)) / ((p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x));
	return p;
}

void Melon::get_intersect_le(POINT p1, POINT p2, D2D1_ELLIPSE e, std::vector<POINT>& intersections)
{
	// 线段的参数
	float x1 = p1.x;
	float y1 = p1.y;
	float x2 = p2.x;
	float y2 = p2.y;

	// 椭圆的参数
	float centerX = e.point.x;
	float centerY = e.point.y;
	float radiusX = e.radiusX;
	float radiusY = e.radiusY;

	// 计算线段的方向向量
	float dx = x2 - x1;
	float dy = y2 - y1;

	// 计算线段的起点到椭圆中心的向量
	float ex = x1 - centerX;
	float ey = y1 - centerY;

	// 计算椭圆方程的系数
	float A = dx * dx / (radiusX * radiusX) + dy * dy / (radiusY * radiusY);
	float B = 2 * dx * ex / (radiusX * radiusX) + 2 * dy * ey / (radiusY * radiusY);
	float C = ex * ex / (radiusX * radiusX) + ey * ey / (radiusY * radiusY) - 1;

	// 计算判别式
	float discriminant = B * B - 4 * A * C;

	// 如果判别式为正，表示有两个交点
	if (discriminant > 0)
	{
		// 计算两个交点
		float t1 = (-B + sqrt(discriminant)) / (2 * A);
		float t2 = (-B - sqrt(discriminant)) / (2 * A);

		// 计算交点的坐标
		POINT intersection1;
		intersection1.x = x1 + t1 * dx;
		intersection1.y = y1 + t1 * dy;

		POINT intersection2;
		intersection2.x = x1 + t2 * dx;
		intersection2.y = y1 + t2 * dy;

		// 将交点存储在向量中
		if (intersection1.x >= min(x1, x2) && intersection1.x <= max(x1, x2) && intersection1.y >= min(y1, y2) && intersection1.y <= max(y1, y2))
			intersections.push_back(intersection1);
		if (intersection2.x >= min(x1, x2) && intersection2.x <= max(x1, x2) && intersection2.y >= min(y1, y2) && intersection2.y <= max(y1, y2))
			intersections.push_back(intersection2);
	}
	// 如果判别式为零，表示相切，只有一个交点
	else if (discriminant == 0)
	{
		// 计算相切点
		float t = -B / (2 * A);
		POINT intersection;
		intersection.x = x1 + t * dx;
		intersection.y = y1 + t * dy;

		// 将交点存储在向量中
		if (intersection.x >= min(x1, x2) && intersection.x <= max(x1, x2) && intersection.y >= min(y1, y2) && intersection.y <= max(y1, y2))
			intersections.push_back(intersection);
	}
}

void Melon::get_intersect_ee(D2D1_ELLIPSE e1, D2D1_ELLIPSE e2, std::vector<POINT>& intersections)
{
	// 获取椭圆e1的中心点和半长轴、短半轴
	D2D1_POINT_2F center1 = e1.point;
	float radiusX1 = e1.radiusX;
	float radiusY1 = e1.radiusY;

	// 获取椭圆e2的中心点和半长轴、短半轴
	D2D1_POINT_2F center2 = e2.point;
	float radiusX2 = e2.radiusX;
	float radiusY2 = e2.radiusY;

	// 计算两个椭圆的中心之间的距离
	float distance = sqrt((center2.x - center1.x) * (center2.x - center1.x) + (center2.y - center1.y) * (center2.y - center1.y));

	// 计算两个椭圆的半长轴之和
	float sumOfRadii = radiusX1 + radiusX2;

	// 如果两个椭圆的中心之间的距离小于等于半长轴之和，则两个椭圆相交
	if (distance <= sumOfRadii)
	{
		// 计算两个椭圆的交点
		float a = (radiusX1 * radiusX1 - radiusX2 * radiusX2 + distance * distance) / (2 * distance);
		float b = sqrt(radiusX1 * radiusX1 - a * a);

		D2D1_POINT_2F p2;
		p2.x = center1.x + a * (center2.x - center1.x) / distance;
		p2.y = center1.y + a * (center2.y - center1.y) / distance;
		D2D1_POINT_2F intersection1, intersection2;
		intersection1.x = p2.x + b * (center2.y - center1.y) / distance;
		intersection1.y = p2.y - b * (center2.x - center1.x) / distance;
		intersection2.x = p2.x - b * (center2.y - center1.y) / distance;
		intersection2.y = p2.y + b * (center2.x - center1.x) / distance;

		// 判断交点的个数，并将交点存储在向量中
		if (intersection1.x == intersection2.x && intersection1.y == intersection2.y)
		{
			intersections.push_back({ static_cast<LONG>(intersection1.x), static_cast<LONG>(intersection1.y) });
		}
		else
		{
			intersections.push_back({ static_cast<LONG>(intersection1.x), static_cast<LONG>(intersection1.y) });
			intersections.push_back({ static_cast<LONG>(intersection2.x), static_cast<LONG>(intersection2.y) });
		}
	}
}

void Melon::get_footpoint(POINT p, std::vector<POINT>& foot_points)
{
	for (auto l : lines)
	{
		// 获取线段的起点和终点
		POINT start = l->get_start();
		POINT end = l->get_end();

		// 计算线段的方向向量
		float dx = end.x - start.x;
		float dy = end.y - start.y;

		// 计算线段的起点到点p的向量
		float ex = p.x - start.x;
		float ey = p.y - start.y;

		// 计算垂足的坐标
		float t = (ex * dx + ey * dy) / (dx * dx + dy * dy);	// 数量积，线段上的垂足到线段起点的距离与线段长度的比值
		POINT foot_point;
		foot_point.x = start.x + t * dx;
		foot_point.y = start.y + t * dy;

		// 判断垂足是否在线段上
		if (foot_point.x >= min(start.x, end.x) && foot_point.x <= max(start.x, end.x) && foot_point.y >= min(start.y, end.y) && foot_point.y <= max(start.y, end.y))
		{
			foot_points.push_back(foot_point);
		}
	}
}

void Melon::fill_triangle(int x1, int y1, int x2, int y2, int x3, int y3)
{
	seed_fill((x1 + x2 + x3) / 3, (y1 + y2 + y3) / 3, x1, y1, x2, y2, x3, y3, m_pFillBrush);
}

void Melon::seed_fill(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3, ID2D1SolidColorBrush* brush)
{
	std::stack<POINT> points;

	int min_y = min(y1, min(y2, y3));
	int max_y = max(y1, max(y2, y3));

	row_has_seed.assign(max_y - min_y + 1, false);

	POINT seed_point = { x, y };
	points.push(seed_point);

	while (!points.empty())
	{
		POINT seed = points.top();
		points.pop();

		int left = seed.x;
		while (left >= 0 && is_in_triangle(left, seed.y, x1, y1, x2, y2, x3, y3))
		{
			--left;
		}

		int right = seed.x;
		while (right >= 0 && is_in_triangle(right, seed.y, x1, y1, x2, y2, x3, y3))
		{
			++right;
		}

		int x_start = left;
		int x_end = right;

		for (int xs = x_start; xs <= x_end; ++xs)
		{
			put_pixel(xs, seed.y, brush);
		}

		if (seed.y < max_y)
		{
			for (int xs = x_start; xs <= x_end; ++xs)
			{
				if (!row_has_seed[seed.y + 1 - min_y] && is_in_triangle(xs, seed.y + 1, x1, y1, x2, y2, x3, y3))
				{
					POINT new_seed{ xs, seed.y + 1 };
					points.push(new_seed);
					row_has_seed[seed.y + 1 - min_y] = true;
					break;
				}
			}
		}

		if (seed.y > min_y)
		{
			for (int xs = x_start; xs <= x_end; ++xs)
			{
				if (!row_has_seed[seed.y - 1 - min_y] && is_in_triangle(xs, seed.y - 1, x1, y1, x2, y2, x3, y3))
				{
					POINT new_seed{ xs, seed.y - 1 };
					points.push(new_seed);
					row_has_seed[seed.y - 1 - min_y] = true;
					break;
				}
			}
		}
	}

	return;
}

bool Melon::is_in_triangle(int x, int y, int x1, int y1, int x2, int y2, int x3, int y3)
{
	// 计算三个边的叉积
	int d1 = (x - x1) * (y1 - y2) - (x1 - x2) * (y - y1);
	int d2 = (x - x2) * (y2 - y3) - (x2 - x3) * (y - y2);
	int d3 = (x - x3) * (y3 - y1) - (x3 - x1) * (y - y3);

	// 如果点在三角形内部，那么这三个叉积的符号应该相同（或者都为0）
	if ((d1 >= 0 && d2 >= 0 && d3 >= 0) || (d1 <= 0 && d2 <= 0 && d3 <= 0))
		return true;

	return false;
}

void Melon::fill_triangle_scanline(int x1, int y1, int x2, int y2, int x3, int y3, ID2D1SolidColorBrush* brush)
{
	if (y1 > y2)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}
	if (y1 > y3)
	{
		std::swap(x1, x3);
		std::swap(y1, y3);
	}
	if (y2 > y3)
	{
		std::swap(x2, x3);
		std::swap(y2, y3);
	}

	for (int y = y1; y < y2; ++y)
	{
		int x_start = x1 + static_cast<int>((y - y1) * (x3 - x1) / (y3 - y1));
		int x_end = x1 + static_cast<int>((y - y1) * (x2 - x1) / (y2 - y1));

		if (x_start > x_end)
			std::swap(x_start, x_end);

		for (int x = x_start; x <= x_end; ++x)
			put_pixel(x, y, brush);
	}

	for (int y = y2 + 1; y < y3; ++y)
	{
		int x_start = x1 + static_cast<int>((y - y1) * (x3 - x1) / (y3 - y1));
		int x_end = x2 + static_cast<int>((y - y2) * (x3 - x2) / (y3 - y2));

		if (x_start > x_end)
			std::swap(x_start, x_end);

		for (int x = x_start; x <= x_end; ++x)
			put_pixel(x, y, brush);
	}

	// 绘制中间那条线
	if (y1 != y2 && y2 != y3)
	{
		int x_start = x1 + static_cast<int>((y2 - y1) * (x3 - x1) / (y3 - y1));
		int x_end = x2 + static_cast<int>((y2 - y2) * (x3 - x2) / (y3 - y2));

		if (x_start > x_end)
			std::swap(x_start, x_end);

		for (int x = x_start; x <= x_end; ++x)
			put_pixel(x, y2, brush);
	}

}

void Melon::scanline(std::vector<POINT> vertices)
{
	int max_y = 0;
	int min_y = 2000;
	int num = vertices.size();

	for (int i = 0; i < num; ++i)
	{
		if (vertices[i].y > max_y)
			max_y = vertices[i].y;
		if (vertices[i].y < min_y)
			min_y = vertices[i].y;
	}

	std::vector<Edge> ET, AET;
	Edge e;

	for (int i = 0; i < num; ++i)
	{
		e.x = (vertices[i].y > vertices[(i + 1) % num].y) ? vertices[(i + 1) % num].x : vertices[i].x;//ET的x表示下端点的x
		e.ymax = max(vertices[i].y, vertices[(i + 1) % num].y);
		e.ymin = min(vertices[i].y, vertices[(i + 1) % num].y);
		if ((vertices[i].y - vertices[(i + 1) % num].y) != 0)
			e.dx = (vertices[i].x - vertices[(i + 1) % num].x) * 1.0 / (vertices[i].y - vertices[(i + 1) % num].y);
		else
			e.dx = INT_MAX;

		ET.push_back(e);//插入到ET中
	}

	int num2 = ET.size();
	for (int y = min_y; y <= max_y; y++) 
	{
		for (int i = 0; i < num2; i++) 
		{
			if (y > ET[i].ymin && y <= ET[i].ymax) // 不取等号，可以避免水平线被计入AET
			{
				AET.push_back(ET[i]);
				ET[i].x += ET[i].dx; 
			}
		}

		std::sort(AET.begin(), AET.end(), [](Edge a, Edge b) {return a.x < b.x; });

		//交点配对,画线
		for (int i = 1; i < AET.size(); i += 2)
		{
			for(int xs = AET[i - 1].x; xs <= AET[i].x; ++xs)
				put_pixel(xs, y, m_pFillBrush);
		}

		AET.clear();
	}
}

void Melon::draw_4d_bezier(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int x5, int y5)
{
	// 着重绘制4个控制点，使用×，并红点突出
	m_pRenderTarget->DrawLine(D2D1::Point2F(x1 - 5, y1 - 5), D2D1::Point2F(x1 + 5, y1 + 5), m_pBlackBrush, 1.0f);
	m_pRenderTarget->DrawLine(D2D1::Point2F(x1 - 5, y1 + 5), D2D1::Point2F(x1 + 5, y1 - 5), m_pBlackBrush, 1.0f);
	m_pRenderTarget->DrawLine(D2D1::Point2F(x2 - 5, y2 - 5), D2D1::Point2F(x2 + 5, y2 + 5), m_pBlackBrush, 1.0f);
	m_pRenderTarget->DrawLine(D2D1::Point2F(x2 - 5, y2 + 5), D2D1::Point2F(x2 + 5, y2 - 5), m_pBlackBrush, 1.0f);
	m_pRenderTarget->DrawLine(D2D1::Point2F(x3 - 5, y3 - 5), D2D1::Point2F(x3 + 5, y3 + 5), m_pBlackBrush, 1.0f);
	m_pRenderTarget->DrawLine(D2D1::Point2F(x3 - 5, y3 + 5), D2D1::Point2F(x3 + 5, y3 - 5), m_pBlackBrush, 1.0f);
	m_pRenderTarget->DrawLine(D2D1::Point2F(x4 - 5, y4 - 5), D2D1::Point2F(x4 + 5, y4 + 5), m_pBlackBrush, 1.0f);
	m_pRenderTarget->DrawLine(D2D1::Point2F(x4 - 5, y4 + 5), D2D1::Point2F(x4 + 5, y4 - 5), m_pBlackBrush, 1.0f);
	m_pRenderTarget->DrawLine(D2D1::Point2F(x5 - 5, y5 - 5), D2D1::Point2F(x5 + 5, y5 + 5), m_pBlackBrush, 1.0f);
	m_pRenderTarget->DrawLine(D2D1::Point2F(x5 - 5, y5 + 5), D2D1::Point2F(x5 + 5, y5 - 5), m_pBlackBrush, 1.0f);
	m_pRenderTarget->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(x1, y1), 2.0f, 2.0f), m_pRedBrush, 1.0f);
	m_pRenderTarget->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(x2, y2), 2.0f, 2.0f), m_pRedBrush, 1.0f);
	m_pRenderTarget->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(x3, y3), 2.0f, 2.0f), m_pRedBrush, 1.0f);
	m_pRenderTarget->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(x4, y4), 2.0f, 2.0f), m_pRedBrush, 1.0f);
	m_pRenderTarget->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(x5, y5), 2.0f, 2.0f), m_pRedBrush, 1.0f);

	// 用虚线绘制5个控制点之间的连线
	m_pRenderTarget->DrawLine(D2D1::Point2F(x1, y1), D2D1::Point2F(x2, y2), m_pBLineBrush, 1.2f, m_pStrokeStyle);
	m_pRenderTarget->DrawLine(D2D1::Point2F(x2, y2), D2D1::Point2F(x3, y3), m_pBLineBrush, 1.2f, m_pStrokeStyle);
	m_pRenderTarget->DrawLine(D2D1::Point2F(x3, y3), D2D1::Point2F(x4, y4), m_pBLineBrush, 1.2f, m_pStrokeStyle);
	m_pRenderTarget->DrawLine(D2D1::Point2F(x4, y4), D2D1::Point2F(x5, y5), m_pBLineBrush, 1.2f, m_pStrokeStyle);

	// 绘制4次Bezier曲线
	for (double t = 0.0; t <= 1.0; t += 0.001)
	{
		// 使用4次Bezier曲线公式计算点
		D2D1_POINT_2F point;
		point.x = (1 - t) * (1 - t) * (1 - t) * (1 - t) * x1 + 4 * (1 - t) * (1 - t) * (1 - t) * t * x2 + 6 * (1 - t) * (1 - t) * t * t * x3 + 4 * (1 - t) * t * t * t * x4 + t * t * t * t * x5;
		point.y = (1 - t) * (1 - t) * (1 - t) * (1 - t) * y1 + 4 * (1 - t) * (1 - t) * (1 - t) * t * y2 + 6 * (1 - t) * (1 - t) * t * t * y3 + 4 * (1 - t) * t * t * t * y4 + t * t * t * t * y5;

		// 绘制像素点
		put_pixel(static_cast<int>(point.x), static_cast<int>(point.y), m_pBlackBrush);
	}

}

POINT Melon::zoom(POINT raw_point, POINT center, double factor)
{
	POINT new_point;

	new_point.x = center.x + (raw_point.x - center.x) * factor;
	new_point.y = center.y + (raw_point.y - center.y) * factor;

	return new_point;
}

D2D1_POINT_2F Melon::zoom(D2D1_POINT_2F raw_point, D2D1_POINT_2F center, double factor)
{
	D2D1_POINT_2F new_point;

	new_point.x = center.x + (raw_point.x - center.x) * factor;
	new_point.y = center.y + (raw_point.y - center.y) * factor;

	return new_point;
}

unsigned char Melon::get_code(POINT left_top, POINT right_bottom, POINT p)
{
	unsigned char code = 0x0;

	if (p.x < left_top.x)
		code |= 0x1;
	else if (p.x > right_bottom.x)
		code |= (0x1 << 1);

	if (p.y < left_top.y)
		code |= (0x1 << 2);
	else if (p.y > right_bottom.y)
		code |= (0x1 << 3);

	return code;
}

std::vector<POINT> Melon::code_clip(POINT left_top, POINT right_bottom, POINT p1, POINT p2)
{
	std::vector<POINT> points;
	points.clear();

	unsigned char code1 = get_code(left_top, right_bottom, p1);
	unsigned char code2 = get_code(left_top, right_bottom, p2);
	unsigned char code;

	double dx = p2.x - p1.x;
	double dy = p2.y - p1.y;
	double x = 0, y = 0;
	
	// 使用Cohen-Sutherland算法进行裁剪，而不是中点分割裁剪算法
	while (code1 != 0 || code2 != 0)
	{
		if ((code1 & code2) != 0)
			return points;
		
		code = code1 != 0 ? code1 : code2;

		if (code & 0x1)	
		{
			x = left_top.x;
			if(dx != 0)
				y = p1.y + (left_top.x - p1.x) * dy / dx;
			else
				y = p1.y;
		}
		else if (code & 0x2)
		{
			x = right_bottom.x;
			if (dx != 0)
				y = p1.y + (right_bottom.x - p1.x) * dy / dx;
			else
				y = p1.y;
		}
		else if (code & 0x4)
		{
			y = left_top.y;
			if (dy != 0)
				x = p1.x + (left_top.y - p1.y) * dx / dy;
			else
				x = p1.x;
		}
		else if (code & 0x8)
		{
			y = right_bottom.y;
			if (dy != 0)
				x = p1.x + (right_bottom.y - p1.y) * dx / dy;
			else
				x = p1.x;
		}

		if (code == code1)
		{
			p1.x = x;
			p1.y = y;
			code1 = get_code(left_top, right_bottom, p1);
		}
		else
		{
			p2.x = x;
			p2.y = y;
			code2 = get_code(left_top, right_bottom, p2);
		}
	}

	points.push_back(p1);
	points.push_back(p2);

	return points;
}

std::vector<POINT> Melon::LiangBarsky(POINT left_top, POINT right_bottom, POINT p1, POINT p2)
{
	std::vector<POINT> points;
	double t0 = 0.0, t1 = 1.0, dx = p2.x - p1.x, dy = p2.y - p1.y;
	std::vector<double> p = { -dx, dx, -dy, dy };
	std::vector<LONG> q = { p1.x - left_top.x, right_bottom.x - p1.x, p1.y - left_top.y, right_bottom.y - p1.y };

	for (int i = 0; i < 4; ++i)
	{
		if (p[i] == 0)	// 线段平行于裁剪边
		{
			if (q[i] < 0)	// 线段在裁剪窗口外
				return {};
		}
		else
		{
			double t = q[i] / p[i];
			if (p[i] < 0)	// 入点
			{
				if (t > t1)	// 线段不在裁剪窗口内
					return {};
				else if (t > t0)	// 参数为P1, R, T三个点中参数最大值
					t0 = t;
			}
			else
			{
				if (t < t0)
					return {};
				else if (t < t1)
					t1 = t;
			}
		}
	}

	points.push_back({ static_cast<LONG>(p1.x + t0 * dx), static_cast<LONG>(p1.y + t0 * dy) });
	points.push_back({ static_cast<LONG>(p1.x + t1 * dx), static_cast<LONG>(p1.y + t1 * dy) });

	return points;
}

bool is_intersection(D2D1_POINT_2F point, std::vector<D2D1_POINT_2F> polygon)
{
	// 如果在polygon中没找到point，返回true
	if (std::find(polygon.begin(), polygon.end(), point) == polygon.end())
		return true;
	return false;
}


std::vector<D2D1_POINT_2F> Melon::WeilerAtherton(std::vector<D2D1_POINT_2F> sp, std::vector<D2D1_POINT_2F> cp)
{
	std::vector<D2D1_POINT_2F> points;
	points.clear();

	int size1 = sp.size();
	int size2 = cp.size();

	std::vector<D2D1_POINT_2F> inter_points;
	std::vector<D2D1_POINT_2F> sp_list, cp_list;
	D2D1_POINT_2F prime_point = { 0, 0 };
	bool is_starting = false;

	sp_list.push_back(sp[0]);
	for (int i = 0; i < size1 - 1; ++i)
	{
		inter_points.clear();

		for (int j = 0; j < size2 - 1; ++j)
		{
			if(is_intersect_ll(sp[i], sp[i+1], cp[j], cp[j+1]))
				inter_points.push_back(get_intersect_ll(sp[i], sp[i + 1], cp[j], cp[j + 1]));
		}

		// 按照到点sp[i]的距离从小到大排序inter_points
		std::sort(inter_points.begin(), inter_points.end(), [sp, i](D2D1_POINT_2F a, D2D1_POINT_2F b) {return get_distance(a, sp[i]) < get_distance(b, sp[i]); });

		for(auto ip : inter_points)
			sp_list.push_back(ip);
		sp_list.push_back(sp[i + 1]);
	}

	cp_list.push_back(cp[0]);
	for (int i = 0; i < size2 - 1; ++i)
	{
		inter_points.clear();

		for (int j = 0; j < size1 - 1; ++j)
		{
			if (is_intersect_ll(cp[i], cp[i + 1], sp[j], sp[j + 1]))
				inter_points.push_back(get_intersect_ll(cp[i], cp[i + 1], sp[j], sp[j + 1]));
		}
		if (is_intersect_ll(cp[i], cp[i + 1], sp[size1 - 1], sp[0]))
			inter_points.push_back(get_intersect_ll(cp[i], cp[i + 1], sp[size1 - 1], sp[0]));

		// 按照到点cp[i]的距离从小到大排序inter_points
		std::sort(inter_points.begin(), inter_points.end(), [cp, i](D2D1_POINT_2F a, D2D1_POINT_2F b) {return get_distance(a, cp[i]) < get_distance(b, cp[i]); });

		for (auto ip : inter_points)
			cp_list.push_back(ip);
		cp_list.push_back(cp[i + 1]);
	}

	std::vector<bool> sp_visited(sp_list.size(), false);
	// 把sp_list中的交点标记为已访问
	for (auto sl : sp_list)
	{
		if (!is_intersection(sl, sp))
		{
			sp_visited[std::distance(sp_list.begin(), std::find(sp_list.begin(), sp_list.end(), sl))] = true;
		}
	}
	sp_visited[sp_visited.size() - 1] = true;

	for (std::vector<D2D1_POINT_2F>::iterator it1 = sp_list.begin();;)
	{
		auto index = std::distance(sp_list.begin(), it1);

		bool is_finished = true;
		for (auto sv : sp_visited)	// 
		{
			if (sv == false)
			{
				is_finished = false;
				break;
			}
		}

		if (is_finished)
			break;

		// 如果已经访问过了，或者不是交点，就跳过
		if (sp_visited[index])
		{
			++it1;
			if (it1 == sp_list.end())
				it1 = sp_list.begin();

			continue;
		}

		if (!is_starting)	// 记录搜索起点
		{
			prime_point = *it1;
			is_starting = true;
		}

		sp_visited[index] = true;
		points.push_back(*it1);

		++it1;
		if (it1 == sp_list.end())
			it1 = sp_list.begin();
		while (!is_intersection(*it1, sp))	// 直到找到一个交点
		{
			points.push_back(*it1);
			++it1;
			if (it1 == sp_list.end())
				it1 = sp_list.begin();
		}

		// 此时it1指向一个交点
		auto it2 = std::find(cp_list.begin(), cp_list.end(), *it1);
		points.push_back(*it1);

		auto index2 = std::distance(sp_list.begin(), it1);
		sp_visited[index2] = true;

		while(true)
		{
			++it2;
			if (it2 == cp_list.end())
				it2 = cp_list.begin();

			if (*it2 == prime_point && is_starting)
			{
				points.push_back(*it2);
				prime_point = { 0, 0 };
				is_starting = false;
				break;
			}

			if (is_intersection(*it2, cp))
				break;	
		}

		it1 = std::find(sp_list.begin(), sp_list.end(), *it2);
	}

	return points;
}
