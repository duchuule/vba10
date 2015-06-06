#include "pch.h"
#include "Rectangle.h"

namespace Engine
{
	Rectangle::Rectangle()
		: X(0), Y(0), Width(0), Height(0), TopLeft(Point(0, 0)), Size(Engine::Size(0, 0))
	{ }

	Rectangle::Rectangle(int x, int y, unsigned int width, unsigned int height)
		: X(x), Y(y), Width(width), Height(height), TopLeft(Point(x, y)), Size(Engine::Size(width, height))
	{ }

	Rectangle::Rectangle(const Point &topleft, const Engine::Size &size)
		: TopLeft(topleft), Size(size), X(topleft.X), Y(topleft.Y), Width(size.Width), Height(size.Height)
	{ }

	Rectangle::~Rectangle()
	{ }

	Point Rectangle::GetBottomRight(void) const
	{
		return Point(this->X + this->Width, 
			this->Y + this->Height);
	}

	void Rectangle::GetBottomRight(Point *p) const
	{
		p->X = this->X + this->Width;
		p->Y = this->Y + this->Height;
	}

	void Rectangle::SetBottomRight(Point p)
	{
		if(p < this->TopLeft)
		{
			Point tmp;
			tmp = this->TopLeft;
			this->TopLeft = p;
			p = tmp;
		}
		this->Width = p.X - this->TopLeft.X;
		this->Height = p.Y - this->TopLeft.Y;
	}

	bool Rectangle::Contains(const Rectangle &other) const
	{
		Point P1 = this->TopLeft;
		Point P2 = this->GetBottomRight();
		Point otherP1 = other.TopLeft;
		Point otherP2 = other.GetBottomRight();
		return (P1 <= otherP1 && 
				P2 >= otherP2);
	}

	bool Rectangle::Intersects(const Rectangle &other) const
	{
		Point P1 = this->TopLeft;
		Point P2 = this->GetBottomRight();
		Point otherP1 = other.TopLeft;
		Point otherP2 = other.GetBottomRight();
		return ((P1 < otherP1 && P2 > otherP1) ||
				(P1 < otherP2 && P2 > otherP2));
	}
}