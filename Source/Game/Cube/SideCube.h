#pragma once

#include "BaseCube.h"


class SideCube : public BaseCube
{
public:
	SideCube() = default;

	virtual void Update(_In_ FLOAT deltaTime) override;

private:
	FLOAT m_deltaTime;
};
