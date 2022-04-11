#pragma once

#include "BaseCube.h"
#include "Renderer/DataTypes.h"
#include "Renderer/Renderable.h"

class SunCube : public BaseCube
{
public:
	SunCube() = default;

	virtual void Update(_In_ FLOAT deltaTime) override;
};