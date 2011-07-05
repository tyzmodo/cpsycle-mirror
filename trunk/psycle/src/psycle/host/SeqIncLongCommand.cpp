#include <psycle/host/detail/project.private.hpp>
#include "SeqIncLongCommand.hpp"
#include "SeqView.hpp"

namespace psycle { namespace host {
		
SeqIncLongCommand::SeqIncLongCommand(SequencerView* seq_view) 
	: SeqHelperCommand(seq_view) {
}

void SeqIncLongCommand::Execute() {
	SeqHelperCommand::PrepareUndoStorage();
	// Execute
	seq_view()->OnInclong();
	SeqHelperCommand::PrepareRedoStorage();
}

}}
