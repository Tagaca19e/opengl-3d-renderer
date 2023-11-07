#pragma once

#include "Assignment.h"

class Lab04 : public Assignment {
public:
	Lab04() : Assignment("Lab 04") { }
	virtual ~Lab04() { }

	virtual void render(s_ptr<Texture> screen);
	virtual void renderUI();
};