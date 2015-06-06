#ifndef VECTOR4_H_
#define VECTOR4_H_

namespace Engine
{
	struct Vector2;
	struct Vector3;

	struct Vector4
	{
	public:
		union
		{
			struct
			{
				float X;
				float Y;
				float Z;
				float W;
			};
			float XYZW[4];
		};

		Vector4(void);
		Vector4(float x, float y, float z, float w);
		Vector4(const Vector2 &vector2);
		Vector4(const Vector3 &vector3);

		float GetLength(void) const;
		float GetLengthSquared(void) const;
		void Normalize(void);
		
		Vector4 operator+(const Vector4 &other) const;
		Vector4 operator+=(const Vector4 &other);
		Vector4 operator-(const Vector4 &other) const;
		Vector4 operator-=(const Vector4 &other);
		Vector4 operator*(float scalar) const;
		Vector4 operator*=(float scalar);
		float operator*(const Vector4 &other) const;
		bool operator<(const Vector4 &other) const;
		bool operator<=(const Vector4 &other) const;
		bool operator>(const Vector4 &other) const;
		bool operator>=(const Vector4 &other) const;
		bool operator==(const Vector4 &other) const;
		bool operator!=(const Vector4 &other) const;

		static float Dot(const Vector4 &left, const Vector4 &right);
		static Vector4 Normalize(const Vector4 &vector);
	};
}

#endif