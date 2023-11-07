#pragma once

#include "Assignment.h"

class Lab02 : public Assignment {
	public:
		Lab02() : Assignment("Lab 02") { }
		virtual ~Lab02() { }

		virtual void render(s_ptr<Texture> screen);
		virtual void renderUI();
};
