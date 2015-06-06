#ifndef POINT_H_
#define POINT_H_

namespace Engine
{
	struct Point
	{
		int X, Y;

		Point(void);
		Point(int x, int y);
		~Point(void);

		bool operator<(const Point &other) const;
		bool operator<=(const Point &other) const;
		bool operator>(const Point &other) const;
		bool operator>=(const Point &other) const;
		bool operator==(const Point &other) const;
		bool operator!=(const Point &other) const;
	};
}

#endif