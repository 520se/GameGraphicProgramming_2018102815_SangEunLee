#include "MyCube.h"

void MyCube::Update(_In_ FLOAT deltaTime)
{
	m_deltaTime += deltaTime;

	XMMATRIX mScale = XMMatrixScaling(0.3f, 0.3f, 0.3f);
	XMMATRIX mTranslate = XMMatrixTranslation(-4.0f, 0.0f, 0.0f);
	XMMATRIX mSpin = XMMatrixRotationZ(-m_deltaTime);
	XMMATRIX mOrbit = XMMatrixRotationY(m_deltaTime * 2.0f);

	m_world = mScale * mSpin * mTranslate * mOrbit;
}