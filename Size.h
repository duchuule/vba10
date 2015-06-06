#ifndef SIZE_H_
#define SIZE_H_

namespace Engine
{
	struct Size
	{
		unsigned int Width, Height;
		Size(unsigned int width, unsigned int height)
			: Width(width), Height(height)
		{ }
		~Size(void) { }
	};
}

#endif