#include "Vector4.h"
#include "Vector3.h"
#include "Vector2.h"
#include <math.h>

namespace Engine
{
	Vector4::Vector4(void)
		: X(0.0f), Y(0.0f), Z(0.0f), W(0.0f)
	{ }

	Vector4::Vector4(float x, float y, float z, float w)
		: X(x), Y(y), Z(z), W(w)
	{ }

	Vector4::Vector4(const Vector2 &vector2)
		: X(vector2.X), Y(vector2.Y), Z(0.0f), W(0.0f)
	{ }

	Vector4::Vector4(const Vector3 &vector3)
		: X(vector3.X), Y(vector3.Y), Z(vector3.Z), W(0.0f)
	{ }

	float Vector4::GetLength(void) const
	{
		return sqrt(X * X + Y * Y + Z * Z + W * W);
	}

	float Vector4::GetLengthSquared(void) const
	{
		return (X * X + Y * Y + Z * Z + W * W);
	}

	void Vector4::Normalize(void)
	{
		float length = this->GetLength();
		X /= length;
		Y /= length;
		Z /= length;
		W /= length;
	}
		
	Vector4 Vector4::operator+(const Vector4 &other) const
	{
		return Vector4(this->X + other.X,
			this->Y + other.Y,
			this->Z + other.Z,
			this->W + other.W);
	}

	Vector4 Vector4::operator+=(const Vector4 &other)
	{
		this->X += other.X;
		this->Y += other.Y;
		this->Z += other.Z;
		this->W += other.W;
		return *this;
	}

	Vector4 Vector4::operator-(const Vector4 &other) const
	{
		return Vector4(this->X - other.X,
			this->Y - other.Y,
			this->Z - other.Z,
			this->W - other.W);
	}

	Vector4 Vector4::operator-=(const Vector4 &other)
	{
		this->X -= other.X;
		this->Y -= other.Y;
		this->Z -= other.Z;
		this->W -= other.W;
		return *this;
	}

	Vector4 Vector4::operator*(float scalar) const
	{
		return Vector4(this->X * scalar,
			this->Y * scalar,
			this->Z * scalar,
			this->W * scalar);
	}

	Vector4 Vector4::operator*=(float scalar)
	{
		this->X *= scalar;
		this->Y *= scalar;
		this->Z *= scalar;
		this->W *= scalar;
		return *this;
	}

	float Vector4::operator*(const Vector4 &other) const
	{
		return (this->X * other.X + 
				this->Y * other.Y +
				this->Z * other.Z +
				this->W * other.W);
	}

	bool Vector4::operator<(const Vector4 &other) const
	{
		return (this->X < other.X) && (this->Y < other.Y) && (this->Z < other.Z) && (this->W < other.W);
	}

	bool Vector4::operator<=(const Vector4 &other) const
	{
		return (this->X <= other.X) && (this->Y <= other.Y) && (this->Z <= other.Z) && (this->W <= other.W);
	}

	bool Vector4::operator>(const Vector4 &other) const
	{
		return (this->X > other.X) && (this->Y > other.Y) && (this->Z > other.Z) && (this->W > other.W);
	}

	bool Vector4::operator>=(const Vector4 &other) const
	{
		return (this->X >= other.X) && (this->Y >= other.Y) && (this->Z >= other.Z) && (this->W >= other.W);
	}

	bool Vector4::operator==(const Vector4 &other) const
	{
		return (this->X == other.X) && (this->Y == other.Y) && (this->Z == other.Z) && (this->W == other.W);
	}

	bool Vector4::operator!=(const Vector4 &other) const
	{
		return (this->X != other.X) || (this->Y != other.Y) || (this->Z != other.Z) || (this->W != other.W);
	}

	float Vector4::Dot(const Vector4 &left, const Vector4 &right)
	{
		return left * right;
	}

	Vector4 Vector4::Normalize(const Vector4 &vector)
	{
		Vector4 result(vector);
		result.Normalize();
		return result;
	}
}