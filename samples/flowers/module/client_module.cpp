#include "client_module.h"
#include <filesystem>

#include "flower_version.h"

namespace flower
{
	ClientModule::ClientModule() noexcept
	{
		this->reset();
	}

	ClientModule::~ClientModule() noexcept
	{
	}

	bool
	ClientModule::isLogin() const noexcept
	{
		return !token.empty();
	}

	void
	ClientModule::reset() noexcept
	{
		if (std::filesystem::exists("./config/public.pem"))
			ssl = std::filesystem::canonical("./config/public.pem").string();
		else
			ssl.clear();

		token.clear();
		username.clear();
		password.clear();
		vip = false;
		coin = 0;
#if _DEBUG
		host = "39.99.178.49";
#else
		host = "39.99.178.49";
#endif
		port = 443;
		autoLogin = true;
		version = FLOWERS_VERSION;
	}

	void 
	ClientModule::load(octoon::runtime::json& reader) noexcept
	{
		if (reader.find("autoLogin") != reader.end())
			this->autoLogin = reader["autoLogin"];
		if (reader.find("username") != reader.end())
			this->username = reader["username"].get<octoon::runtime::json::string_t>();
		if (this->autoLogin)
		{
			if (reader.find("token") != reader.end())
				this->token = reader["token"].get<octoon::runtime::json::string_t>();
		}
	}

	void 
	ClientModule::save(octoon::runtime::json& writer) noexcept
	{
		writer["autoLogin"] = this->autoLogin;
		writer["username"] = this->username;
		writer["token"] = this->autoLogin ? this->token : "";
	}
}