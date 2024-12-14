#include <amxmodx>
#include <amxmisc>
#include <reapi>
#include <json>
#include <sqlx>
#include <time>

public stock const PluginName[] = "Ufame's Ban System";
public stock const PluginVersion[] = "0.1.0";
public stock const PluginAuthor[] = "ufame";
public stock const PluginDescription[] = "Open source ban system for AmxModX";

const TASK_KICK_ID = 17829;

enum EDatabaseConfig {
  DbType[16],
  DbHost[32],
  DbUser[32],
  DbPassword[64],
  DbDatabase[64],
}

enum EAccessFlagsConfig {
  AccessFlags_Immunity,
  AccessFlags_Ban,
  AccessFlags_BanOthers,
  AccessFlags_UnBan_Self,
  AccessFlags_UnBan_Others
}

enum ESettingsConfig {
  bool: Settings_KickAfterBan,
  Float: Settings_KickAfterBan_Time
};

new DatabaseConfig[EDatabaseConfig];
new AccessFlagsConfig[EAccessFlagsConfig];
new SettingsConfig[ESettingsConfig];

new Handle: DbHandle = Empty_Handle;

#include "UBanSystem/utils.inl"

#include "UBanSystem/actions/users/create.inl"
#include "UBanSystem/actions/users/update.inl"
#include "UBanSystem/actions/users/check.inl"

#include "UBanSystem/actions/ban.inl"
#include "UBanSystem/actions/unban.inl"

#include "UBanSystem/commands/ban.inl"
#include "UBanSystem/commands/unban.inl"

public plugin_init() {
  register_dictionary("time.txt");

  ReadMainConfig();
  ConnectionTest();

  RegisterHookChain(RG_CBasePlayer_SetClientUserInfoName, "@CBasePlayer_SetClientUserInfoName", .post = 1);

  register_concmd("amx_ban", "@BanCommand");
  register_concmd("amx_unban", "@UnbanCommand");
}

public client_authorized(player_id, const authid[]) {
  CreateOrUpdateUser(player_id, authid);
  CheckUserIsBanned(player_id, authid);
}

@CBasePlayer_SetClientUserInfoName(const player_id, infobuffer[], newName[]) {
  if (equal(infobuffer, newName))
    return;

  UpdateUserName(player_id, newName);
}

ReadMainConfig() {
  new configFile[64];
  get_configsdir(configFile, charsmax(configFile));
  add(configFile, charsmax(configFile), "/UBanSystem/main.json");

  new JSON: config = json_parse(
    configFile,
    .is_file = true,
    .with_comments = true
  );

  if (config == Invalid_JSON) {
    set_fail_state("JSON synax error in config file (%s).", configFile);
  }

  if (!json_is_object(config)) {
    set_fail_state("JSON config must contain a valid object (%s).", configFile);
  }

  json_object_get_string(config, "database.type", DatabaseConfig[DbType], charsmax(DatabaseConfig[DbType]), .dot_not = true);
  json_object_get_string(config, "database.host", DatabaseConfig[DbHost], charsmax(DatabaseConfig[DbHost]), .dot_not = true);
  json_object_get_string(config, "database.user", DatabaseConfig[DbUser], charsmax(DatabaseConfig[DbUser]), .dot_not = true);
  json_object_get_string(config, "database.password", DatabaseConfig[DbPassword], charsmax(DatabaseConfig[DbPassword]), .dot_not = true);
  json_object_get_string(config, "database.database", DatabaseConfig[DbDatabase], charsmax(DatabaseConfig[DbDatabase]), .dot_not = true);

  AccessFlagsConfig[AccessFlags_Immunity] = LoadAccessFlags(config, "access_flags.immunity");
  AccessFlagsConfig[AccessFlags_Ban] = LoadAccessFlags(config, "access_flags.ban");
  AccessFlagsConfig[AccessFlags_BanOthers] = LoadAccessFlags(config, "access_flags.ban_others");
  AccessFlagsConfig[AccessFlags_UnBan_Self] = LoadAccessFlags(config, "access_flags.unban_self");
  AccessFlagsConfig[AccessFlags_UnBan_Others] = LoadAccessFlags(config, "access_flags.unban_others");

  SettingsConfig[Settings_KickAfterBan] = json_object_get_bool(config, "settings.kick_after_ban", .dot_not = true);
  SettingsConfig[Settings_KickAfterBan_Time] = json_object_get_real(config, "settings.kick_after_ban_time", .dot_not = true);

  json_free(config);
}

ConnectionTest() {
  SQL_SetAffinity(DatabaseConfig[DbType]);
  DbHandle = SQL_MakeDbTuple(
    DatabaseConfig[DbHost],
    DatabaseConfig[DbUser],
    DatabaseConfig[DbPassword],
    DatabaseConfig[DbDatabase]
  );

  SQL_SetCharset(DbHandle, "utf8");

  new errorMessage[512], errorCode;
  new Handle: connectionHandle = SQL_Connect(DbHandle, errorCode, errorMessage, charsmax(errorMessage));

  if (connectionHandle == Empty_Handle) {
    SQL_FreeHandle(DbHandle);
    set_fail_state("[Ban Connection Test] Connection error[%d]: %s", errorCode, errorMessage);

    return;
  }

  SQL_FreeHandle(connectionHandle);
}
