#include "CommandManager.hpp"


namespace psycle {
	namespace host {

		CommandManager::CommandManager()
			: max_undo_(30),
			  max_redo_(30) {
		}

		CommandManager::~CommandManager() {
			while (!undo_queue_.empty()) {
				CommandUndoable* cmd = undo_queue_.back();
				undo_queue_.pop_back();
				delete cmd;
			}
			while (!redo_queue_.empty()) {
				CommandUndoable* cmd = redo_queue_.back();
				redo_queue_.pop_back();
				delete cmd;
			}
		}

		void CommandManager::InvokeCommand(Command* cmd) {
		}

		void CommandManager::InvokeCommand(CommandUndoable* cmd) {			
			undo_queue_.push_back(cmd);
			if (undo_queue_.size() > max_undo_) {
				CommandUndoable* cmd = undo_queue_.front();
				undo_queue_.pop_front();
				delete cmd;
			}
		}

		void CommandManager::Undo() {
			if (!undo_queue_.empty()) {
				CommandUndoable* cmd = undo_queue_.back();
				undo_queue_.pop_back();
				cmd->Undo();
				if (redo_queue_.size() > max_redo_) {
					CommandUndoable* cmd = redo_queue_.front();
					redo_queue_.pop_front();
					delete cmd;
				} else {
					redo_queue_.push_back(cmd);
				}
			}
		}

		void CommandManager::Redo() {
			if (!redo_queue_.empty()) {
				CommandUndoable* cmd = redo_queue_.back();
				redo_queue_.pop_back();
				cmd->Redo();
				if (undo_queue_.size() > max_undo_) {
					CommandUndoable* cmd = undo_queue_.front();
					undo_queue_.pop_front();
					delete cmd;
				} else {
					undo_queue_.push_back(cmd);
				}	
			}
		}

	}	// namespace host
}	// namespace psycle
