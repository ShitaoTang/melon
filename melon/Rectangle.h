#pragma once

#include "Shape.h"

class Rectangles :public Shape
{
private:
	typedef struct RECT
	{
		float left, top, right, bottom;
		float lbleft, lbbottom, rtright, rttop;
	}RECT;

	RECT rect;
	bool is_exist;
	bool is_filled;
	bool selected;

public:
	Rectangles(bool exist, bool is_filled, float left, float top, float right, float bottom, float lbleft, float lbbottom, float rtright, float rttop);
	~Rectangles() {}
	void draw(ID2D1HwndRenderTarget* pRenderTarget, ID2D1SolidColorBrush* pSolidColorBrush, float strokeWidth);
	
	bool is_selected(float x, float y);
	bool is_selected(D2D1_POINT_2F start, D2D1_POINT_2F end, float x, float y);

	void set_selected_t(bool is_selected) { this->selected = is_selected; }
	bool get_selected_t() { return this->selected; }

	bool get_is_exist() { return is_exist; }
	void set_is_exist(bool is_exist) { this->is_exist = is_exist; }

	bool get_is_filled() { return is_filled; }
	void set_is_filled(bool is_filled) { this->is_filled = is_filled; }

	POINT get_center() { return { (int)(rect.left + rect.right + rect.lbleft + rect.rtright) >> 2, (int)(rect.top + rect.bottom + rect.rttop + rect.lbbottom) >> 2 }; }
	float get_width() { return rect.right - rect.left; }
	float get_height() { return rect.bottom - rect.top; }
	float get_left() { return rect.left; }
	float get_top() { return rect.top; }
	float get_right() { return rect.right; }
	float get_bottom() { return rect.bottom; }
	float get_lbleft() { return rect.lbleft; }
	float get_lbbottom() { return rect.lbbottom; }
	float get_rtright() { return rect.rtright; }
	float get_rttop() { return rect.rttop; }
	POINT get_left_top(){ return { (int)rect.left, (int)rect.top }; }
	POINT get_right_bottom() { return { (int)rect.right, (int)rect.bottom }; }
	POINT get_left_bottom(){ return { (int)rect.lbleft, (int)rect.lbbottom }; }
	POINT get_right_top(){ return { (int)rect.rtright, (int)rect.rttop }; }

	void set_center(POINT center) { rect.left = center.x; rect.top = center.y; }
	void set_width(float width) { rect.right = rect.left + width; }
	void set_height(float height) { rect.bottom = rect.top + height; }
	void set_left(float left) { rect.left = left; }
	void set_top(float top) { rect.top = top; }
	void set_right(float right) { rect.right = right; }
	void set_bottom(float bottom) { rect.bottom = bottom; }
	void set_lbleft(float lbleft) { rect.lbleft = lbleft; }
	void set_lbbottom(float lbbottom) { rect.lbbottom = lbbottom; }
	void set_rtright(float rtright) { rect.rtright = rtright; }
	void set_rttop(float rttop) { rect.rttop = rttop; }

	// ͼ�α任
	void translate(float dx, float dy);
	void rotate(POINT old_point, POINT mouse);
	void zoom(double factor);

	bool is_in_rect(D2D_RECT_F r);
};