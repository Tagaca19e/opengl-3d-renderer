#pragma once

#include "Assignment.h"

class Lab03 : public Assignment {
public:
	Lab03() : Assignment("Lab 03") { }
	virtual ~Lab03() { }

	virtual void render(s_ptr<Texture> screen);
	virtual void renderUI();
};
