#ifndef VECTOR3_H_
#define VECTOR3_H_

namespace Engine
{
	struct Vector2;

	struct Vector3
	{
		union
		{
			struct
			{
				float X;
				float Y;
				float Z;
			};
			float XYZ[3];
		};

		Vector3(void);
		Vector3(float x, float y, float z);
		Vector3(const Vector2 &vector2);

		float GetLength(void) const;
		float GetLengthSquared(void) const;
		void Normalize(void);
		
		Vector3 operator+(const Vector3 &other) const;
		Vector3 operator+=(const Vector3 &other);
		Vector3 operator-(const Vector3 &other) const;
		Vector3 operator-=(const Vector3 &other);
		Vector3 operator*(float scalar) const;
		Vector3 operator*=(float scalar);
		float operator*(const Vector3 &other) const;
		bool operator<(const Vector3 &other) const;
		bool operator<=(const Vector3 &other) const;
		bool operator>(const Vector3 &other) const;
		bool operator>=(const Vector3 &other) const;
		bool operator==(const Vector3 &other) const;
		bool operator!=(const Vector3 &other) const;

		static Vector3 Cross(const Vector3 &left, const Vector3 &right);
		static float Dot(const Vector3 &left, const Vector3 &right);
		static Vector3 Normalize(const Vector3 &vector);
	};
}

#endif