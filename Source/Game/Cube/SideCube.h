#pragma once

#include "Common.h"

#include "BaseCube.h"


class SideCube : public BaseCube
{
public:
	SideCube() = default;

	virtual void Update(_In_ FLOAT deltaTime) override;
};
