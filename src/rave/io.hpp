#pragma once

#include "rave/common.hpp"

namespace Rave {

	struct SignalHandler {
		void(*on_terminate)();
	};


	void InitIO();

	int ReadIO(char* n, int len);
	int WriteIO(const char* buffer, int len);

	// For program signaling.
	bool SignalInit(SignalHandler* handler = nullptr);
	bool SignalPoll();
};
