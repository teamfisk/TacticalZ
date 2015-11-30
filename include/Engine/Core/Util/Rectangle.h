#ifndef Rectangle_h__
#define Rectangle_h__

#include <algorithm>

struct Rectangle
{
	Rectangle()
		: X(0), Y(0), Width(0), Height(0) { }

	Rectangle(int width, int height)
		: X(0), Y(0), Width(width), Height(height) { }

	Rectangle(int x, int y, int width, int height)
		: X(x), Y(y), Width(width), Height(height) { }

	/*Rectangle(const Rectangle &rect)
	: X(rect.X), Y(rect.Y), Width(rect.Width), Height(rect.Height) { }*/

	int X;
	int Y;
	int Width;
	int Height;

	virtual int Left() const { return X; }
	virtual void SetLeft(int left)
	{
		//Width += X - left;
		X = left;
	}
	virtual int Right() const { return X + Width; }
	virtual void SetRight(int right)
	{
		Width = right - X;
	}
	virtual int Top() const { return Y; }
	virtual void SetTop(int top)
	{
		//Height += Y - top;
		Y = top;
	}
	virtual int Bottom() const { return Y + Height; }
	virtual void SetBottom(int bottom)
	{
		Height = bottom - Y;
	}

	Rectangle& operator+=(const Rectangle &rhs)
	{
		SetLeft(std::min(Left(), rhs.Left()));
		SetRight(std::max(Right(), rhs.Right()));
		SetTop(std::min(Top(), rhs.Top()));
		SetBottom(std::max(Bottom(), rhs.Bottom()));
	}

	static bool Intersects(const Rectangle &r1, const Rectangle &r2)
	{
		return !(r2.Left() > r1.Right() || r2.Right() < r1.Left() || r2.Top() > r1.Bottom() || r2.Bottom() < r1.Top());
	}
};

inline bool operator==(const Rectangle &r1, const Rectangle &r2)
{
	return (r1.X == r2.X) && (r1.Y == r2.Y) && (r1.Width == r2.Width) && (r1.Height == r2.Height);
}

inline bool operator!=(const Rectangle &lhs, const Rectangle &rhs)
{
	return !(lhs == rhs);
}

inline bool operator<(const Rectangle &lhs, const Rectangle &rhs)
{
	return (lhs.Width < rhs.Width) && (lhs.Height < rhs.Height);
}

inline bool operator>(const Rectangle &lhs, const Rectangle &rhs)
{
	return rhs < lhs;
}

inline bool operator<=(const Rectangle &lhs, const Rectangle &rhs)
{
	return !(lhs > rhs);
}

inline bool operator>=(const Rectangle &lhs, const Rectangle &rhs)
{
	return !(lhs < rhs);
}

inline Rectangle operator+(Rectangle lhs, const Rectangle &rhs)
{
	lhs += rhs;
	return lhs;
}

#endif // Util_Rectangle_h__
