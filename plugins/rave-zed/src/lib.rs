use zed_extension_api::{self as zed, Result};

struct RaveExtension {

}


impl RaveExtension {}


impl zed::Extension for RaveExtension {

	fn new() -> Self {
		Self {}
	}


	fn language_server_command(
		&mut self,
		_language_server_id: &zed::LanguageServerId,
		_worktree: &zed::Worktree
	) -> Result<zed::Command> {
		Err(format!("Not implemented yet!"))
	}


}

zed::register_extension!(RaveExtension);
