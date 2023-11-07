#pragma once

#include "Assignment.h"

class Lab05 : public Assignment {
public:
	Lab05() : Assignment("Lab 05") { }
	virtual ~Lab05() { }

	virtual void init(); 
	virtual void render(s_ptr<Texture> screen);
	virtual void renderUI();
};
