#pragma once


#include "BaseCube.h"


class CenterCube : public BaseCube
{
public:
	CenterCube() = default;

	virtual void Update(_In_ FLOAT deltaTime) override;
};