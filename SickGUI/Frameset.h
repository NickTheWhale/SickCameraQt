#pragma once
#include "Frame.h"

namespace frameset
{
	class Frameset
	{
	public:
		Frameset(const frameset::Frame& depth, const frameset::Frame& intensity, const frameset::Frame& state);
		Frameset();
		~Frameset();

		const frameset::Frame& depth() const;
		const frameset::Frame& intensity() const;
		const frameset::Frame& state() const;

		const bool isEmpty() const;
		const bool isUniform() const;

	private:
		frameset::Frame m_depth;
		frameset::Frame m_intensity;
		frameset::Frame m_state;
	};
}