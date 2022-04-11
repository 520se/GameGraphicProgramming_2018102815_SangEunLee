#pragma once

#include "BaseCube.h"


class MyCube : public BaseCube
{
public:
	MyCube() = default;

	virtual void Update(_In_ FLOAT deltaTime) override;

private:
	FLOAT m_deltaTime;
};
