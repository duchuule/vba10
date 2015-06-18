#include "Point.h"

namespace Engine
{
	Point::Point(void)
		: X(0), Y(0)
	{ }

	Point::Point(int x, int y)
		: X(x), Y(y)
	{ }

	bool Point::operator<(const Point &other) const
	{
		return (this->X < other.X) && (this->Y < other.Y);
	}

	bool Point::operator<=(const Point &other) const
	{
		return (this->X <= other.X) && (this->Y <= other.Y);
	}

	bool Point::operator>(const Point &other) const
	{
		return (this->X > other.X) && (this->Y > other.Y);
	}

	bool Point::operator>=(const Point &other) const
	{
		return (this->X >= other.X) && (this->Y >= other.Y);
	}

	bool Point::operator==(const Point &other) const
	{
		return (this->X == other.X) && (this->Y == other.Y);
	}

	bool Point::operator!=(const Point &other) const
	{
		return (this->X != other.X) || (this->Y != other.Y);
	}

	Point::~Point(void)
	{ }
}