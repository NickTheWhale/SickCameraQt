#include "Frameset.h"

frameset::Frameset::Frameset(const frameset::Frame& depth, const frameset::Frame& intensity, const frameset::Frame& state) :
	m_depth(depth), m_intensity(intensity), m_state(state)
{
}

frameset::Frameset::Frameset()
{
}

frameset::Frameset::~Frameset()
{
}

const frameset::Frame& frameset::Frameset::depth() const
{
	return m_depth;
}

const frameset::Frame& frameset::Frameset::intensity() const
{
	return m_intensity;
}

const frameset::Frame& frameset::Frameset::state() const
{
	return m_state;
}

const bool frameset::Frameset::isEmpty() const
{
	return m_depth.size().isEmpty() || m_intensity.size().isEmpty() || m_state.size().isEmpty();
}

const bool frameset::Frameset::isUniform() const
{
	return m_depth.size() == m_intensity.size() && m_intensity.size() == m_state.size();
}
