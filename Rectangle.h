#ifndef RECTANGLE_H_
#define RECTANGLE_H_

#include "Point.h"
#include "Size.h"

namespace Engine
{
	struct Rectangle
	{
	public:
		union
		{
			struct
			{
				int X;
				int Y;
				unsigned int Width;
				unsigned int Height;
			};
			struct
			{
				Point TopLeft;
				Size Size;
			};
		};

		Rectangle();
		Rectangle(int x, int y, unsigned int width, unsigned int height);
		Rectangle(const Point &topleft, const Engine::Size &size);

		~Rectangle();

		Point GetBottomRight(void) const;
		void GetBottomRight(Point *p) const;
		void SetBottomRight(Point p);

		bool Contains(const Rectangle &other) const;
		bool Intersects(const Rectangle &other) const;
	};
}

#endif