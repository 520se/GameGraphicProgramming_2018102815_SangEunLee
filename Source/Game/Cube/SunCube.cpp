#include "Suncube.h"

void SunCube::Update(_In_ FLOAT deltaTime)
{
	m_world = XMMatrixRotationY(deltaTime);
}