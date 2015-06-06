#include "pch.h"
#include "Vector2.h"
#include <math.h>

namespace Engine
{
	Vector2::Vector2(void)
		: X(0.0f), Y(0.0f)
	{ }

	Vector2::Vector2(float x, float y)
		: X(x), Y(y)
	{ }

	float Vector2::GetLength(void) const
	{
		return sqrt(X * X + Y * Y);
	}

	float Vector2::GetLengthSquared(void) const
	{
		return (X * X + Y * Y);
	}

	void Vector2::Normalize(void)
	{
		float length = this->GetLength();
		X /= length;
		Y /= length;
	}
		
	Vector2 Vector2::operator+(const Vector2 &other) const
	{
		return Vector2(this->X + other.X,
			this->Y + other.Y);
	}

	Vector2 Vector2::operator+=(const Vector2 &other)
	{
		this->X += other.X;
		this->Y += other.Y;
		return *this;
	}

	Vector2 Vector2::operator-(const Vector2 &other) const
	{
		return Vector2(this->X - other.X,
			this->Y - other.Y);
	}

	Vector2 Vector2::operator-=(const Vector2 &other)
	{
		this->X -= other.X;
		this->Y -= other.Y;
		return *this;
	}

	Vector2 Vector2::operator*(float scalar) const
	{
		return Vector2(this->X * scalar,
			this->Y * scalar);
	}

	Vector2 Vector2::operator*=(float scalar)
	{
		this->X *= scalar;
		this->Y *= scalar;
		return *this;
	}

	float Vector2::operator*(const Vector2 &other) const
	{
		return (this->X * other.X + 
				this->Y * other.Y);
	}

	bool Vector2::operator<(const Vector2 &other) const
	{
		return (this->X < other.X) && (this->Y < other.Y);
	}

	bool Vector2::operator<=(const Vector2 &other) const
	{
		return (this->X <= other.X) && (this->Y <= other.Y);
	}

	bool Vector2::operator>(const Vector2 &other) const
	{
		return (this->X > other.X) && (this->Y > other.Y);
	}

	bool Vector2::operator>=(const Vector2 &other) const
	{
		return (this->X >= other.X) && (this->Y >= other.Y);
	}

	bool Vector2::operator==(const Vector2 &other) const
	{
		return (this->X == other.X) && (this->Y == other.Y);
	}

	bool Vector2::operator!=(const Vector2 &other) const
	{
		return (this->X != other.X) || (this->Y != other.Y);
	}

	float Vector2::Dot(const Vector2 &left, const Vector2 &right)
	{
		return left * right;
	}

	Vector2 Vector2::Normalize(const Vector2 &vector)
	{
		Vector2 result(vector);
		result.Normalize();
		return result;
	}
}