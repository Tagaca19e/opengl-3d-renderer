#pragma once

#include "Assignment.h"

class Lab06 : public Assignment {
public:
	Lab06() : Assignment("Lab 06", true) { }
	virtual ~Lab06() { }

	virtual void init();
	virtual void render(s_ptr<Framebuffer> framebuffer);
	virtual void renderUI();
};
