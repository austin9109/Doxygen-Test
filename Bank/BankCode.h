#pragma once

enum class EBankCode
{
	EBC_Alishan,
	EBC_XueMountain,
	EBC_GreenIsland,
	EBC_XiaoLiuqiu,
	EBC_NoneExist,
};

enum class ERetLogin
{
	ERL_Valid,
	ERL_NonExistBank,
	ERL_WrongFormat,
	ERL_NoAccount,
	ERL_WrongPassword,
	ERL_AlreadyLogin,
	ERL_ConnectionFailed,
	ERL_Failed,
};