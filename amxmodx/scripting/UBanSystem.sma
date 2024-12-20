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

enum DatabaseConfig_Struct {
  DbConfig_Type[16],
  DbConfig_Host[32],
  DbConfig_User[32],
  DbConfig_Password[64],
  DbConfig_Database[64],
};

enum AccessFlagsConfig_Struct {
  AccessFlags_Immunity,
  AccessFlags_Ban,
  AccessFlags_BanOthers,
  AccessFlags_Unban_Self,
  AccessFlags_Unban_Others
};

enum SettingsConfig_Struct {
  bool: Settings_KickAfterBan,
  Float: Settings_KickAfterBan_Time
};

new DatabaseConfig[DatabaseConfig_Struct];
new AccessFlagsConfig[AccessFlagsConfig_Struct];
new SettingsConfig[SettingsConfig_Struct];

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

  json_object_get_string(config, "database.type", DatabaseConfig[DbConfig_Type], charsmax(DatabaseConfig[DbConfig_Type]), .dot_not = true);
  json_object_get_string(config, "database.host", DatabaseConfig[DbConfig_Host], charsmax(DatabaseConfig[DbConfig_Host]), .dot_not = true);
  json_object_get_string(config, "database.user", DatabaseConfig[DbConfig_User], charsmax(DatabaseConfig[DbConfig_User]), .dot_not = true);
  json_object_get_string(config, "database.password", DatabaseConfig[DbConfig_Password], charsmax(DatabaseConfig[DbConfig_Password]), .dot_not = true);
  json_object_get_string(config, "database.database", DatabaseConfig[DbConfig_Database], charsmax(DatabaseConfig[DbConfig_Database]), .dot_not = true);

  AccessFlagsConfig[AccessFlags_Immunity] = LoadAccessFlags(config, "access_flags.immunity");
  AccessFlagsConfig[AccessFlags_Ban] = LoadAccessFlags(config, "access_flags.ban");
  AccessFlagsConfig[AccessFlags_BanOthers] = LoadAccessFlags(config, "access_flags.ban_others");
  AccessFlagsConfig[AccessFlags_Unban_Self] = LoadAccessFlags(config, "access_flags.unban_self");
  AccessFlagsConfig[AccessFlags_Unban_Others] = LoadAccessFlags(config, "access_flags.unban_others");

  SettingsConfig[Settings_KickAfterBan] = json_object_get_bool(config, "settings.kick_after_ban", .dot_not = true);
  SettingsConfig[Settings_KickAfterBan_Time] = json_object_get_real(config, "settings.kick_after_ban_time", .dot_not = true);

  json_free(config);
}

ConnectionTest() {
  SQL_SetAffinity(DatabaseConfig[DbConfig_Type]);
  DbHandle = SQL_MakeDbTuple(
    DatabaseConfig[DbConfig_Host],
    DatabaseConfig[DbConfig_User],
    DatabaseConfig[DbConfig_Password],
    DatabaseConfig[DbConfig_Database]
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
