#pragma once
#include "PatHelperCommand.hpp"

namespace psycle {
	namespace host {

		class PatDeleteCommand : public PatHelperCommand {
		public:
			PatDeleteCommand(class PatternView* pat_view);
			~PatDeleteCommand() {}

			virtual void Execute();

		};

	}	// namespace host
}	// namespace psycle