#include "Vector3.h"
#include "Vector2.h"
#include <math.h>

namespace Engine
{
	Vector3::Vector3(void)
		: X(0.0f), Y(0.0f), Z(0.0f)
	{ }

	Vector3::Vector3(float x, float y, float z)
		: X(x), Y(y), Z(z)
	{ }

	Vector3::Vector3(const Vector2 &vector2)
		: X(vector2.X), Y(vector2.Y), Z(0.0f)
	{ }

	float Vector3::GetLength(void) const
	{
		return sqrt(X * X + Y * Y + Z * Z);
	}

	float Vector3::GetLengthSquared(void) const
	{
		return (X * X + Y * Y + Z * Z);
	}

	void Vector3::Normalize(void)
	{
		float length = this->GetLength();
		this->X /= length;
		this->Y /= length;
		this->Z /= length;
	}
		
	Vector3 Vector3::operator+(const Vector3 &other) const
	{
		return Vector3(this->X + other.X,
			this->Y + other.Y,
			this->Z + other.Z);
	}

	Vector3 Vector3::operator+=(const Vector3 &other)
	{
		this->X += other.X;
		this->Y += other.Y;
		this->Z += other.Z;
		
		return Vector3(*this);
	}

	Vector3 Vector3::operator-(const Vector3 &other) const
	{
		return Vector3(this->X - other.X,
			this->Y - other.Y,
			this->Z - other.Z);
	}

	Vector3 Vector3::operator-=(const Vector3 &other)
	{
		this->X -= other.X;
		this->Y -= other.Y;
		this->Z -= other.Z;
		
		return Vector3(*this);
	}

	Vector3 Vector3::operator*(float scalar) const
	{
		return Vector3(this->X * scalar,
			this->Y * scalar,
			this->Z * scalar);
	}

	Vector3 Vector3::operator*=(float scalar)
	{
		this->X *= scalar;
		this->Y *= scalar;
		this->Z *= scalar;
		return Vector3(*this);
	}

	float Vector3::operator*(const Vector3 &other) const
	{
		return (this->X * other.X + this->Y * other.Y + this->Z * other.Z);
	}

	bool Vector3::operator<(const Vector3 &other) const
	{
		return (this->X < other.X) && (this->Y < other.Y) && (this->Z < other.Z);
	}

	bool Vector3::operator<=(const Vector3 &other) const
	{
		return (this->X <= other.X) && (this->Y <= other.Y) && (this->Z <= other.Z);
	}

	bool Vector3::operator>(const Vector3 &other) const
	{
		return (this->X > other.X) && (this->Y > other.Y) && (this->Z > other.Z);
	}

	bool Vector3::operator>=(const Vector3 &other) const
	{
		return (this->X >= other.X) && (this->Y >= other.Y) && (this->Z >= other.Z);
	}

	bool Vector3::operator==(const Vector3 &other) const
	{
		return (this->X == other.X) && (this->Y == other.Y) && (this->Z == other.Z);
	}

	bool Vector3::operator!=(const Vector3 &other) const
	{
		return (this->X != other.X) || (this->Y != other.Y) || (this->Z != other.Z);
	}

	Vector3 Vector3::Cross(const Vector3 &left, const Vector3 &right)
	{
		return Vector3(left.Y * right.Z - left.Z * right.Y,
			left.Z * right.X - left.X * right.Z,
			left.X * right.Y - left.Y * right.X);
	}

	float Vector3::Dot(const Vector3 &left, const Vector3 &right)
	{
		return left * right;
	}

	Vector3 Vector3::Normalize(const Vector3 &vector)
	{
		Vector3 result(vector);
		result.Normalize();
		return result;
	}
}