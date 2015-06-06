#ifndef VECTOR2_H_
#define VECTOR2_H_

namespace Engine
{

	struct Vector2
	{
	public:
		union
		{
			struct
			{
				float X;
				float Y;
			};
			float XY[2];
		};

		Vector2(void);
		Vector2(float x, float y);

		float GetLength(void) const;
		float GetLengthSquared(void) const;
		void Normalize(void);
		
		Vector2 operator+(const Vector2 &other) const;
		Vector2 operator+=(const Vector2 &other);
		Vector2 operator-(const Vector2 &other) const;
		Vector2 operator-=(const Vector2 &other);
		Vector2 operator*(float scalar) const;
		Vector2 operator*=(float scalar);
		float operator*(const Vector2 &other) const;
		bool operator<(const Vector2 &other) const;
		bool operator<=(const Vector2 &other) const;
		bool operator>(const Vector2 &other) const;
		bool operator>=(const Vector2 &other) const;
		bool operator==(const Vector2 &other) const;
		bool operator!=(const Vector2 &other) const;

		static float Dot(const Vector2 &left, const Vector2 &right);
		static Vector2 Normalize(const Vector2 &vector);
	};
}

#endif