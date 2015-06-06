#ifndef COLOR_H_
#define COLOR_H_

namespace Engine
{
	struct Color
	{
		union
		{
			struct
			{
				float R;
				float G;
				float B;
				float A;
			};
			float RGBA[4];
		};

	public:
		Color()
			: R(0.0f), G(0.0f), B(0.0f), A(0.0f)
		{ }

		Color(float r, float g, float b, float a)
			: R(r), B(b), G(g), A(a)
		{ }
	};

	struct Color4i
	{
		union
		{
			struct
			{
				UCHAR R;
				UCHAR G;
				UCHAR B;
				UCHAR A;
			};
			UINT RGBA;
			UCHAR Array[4];
		};
	};
}

#endif