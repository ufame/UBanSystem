#include <amxmodx>
#include <amxmisc>
#include <reapi>
#include <json>
#include <sqlx>
#include <time>

public stock const PluginName[] = "Ufame's Ban System";
public stock const PluginVersion[] = "0.0.1";
public stock const PluginAuthor[] = "ufame";
public stock const PluginDescription[] = "Open source ban system for AmxModX";

enum EDatabaseConfig {
  DbType[16],
  DbHost[32],
  DbUser[32],
  DbPassword[64],
  DbDatabase[64],
}

new DatabaseConfig[EDatabaseConfig];

new Handle: DbHandle = Empty_Handle;

#include "UBanSystem/utils.inl"

#include "UBanSystem/actions/users/create.inl"
#include "UBanSystem/actions/users/update.inl"
#include "UBanSystem/actions/users/check.inl"

#include "UBanSystem/actions/ban.inl"

#include "UBanSystem/commands/ban.inl"

public plugin_init() {
  register_dictionary("time.txt");

  ReadMainConfig();
  ConnectionTest();

  RegisterHookChain(RG_CBasePlayer_SetClientUserInfoName, "@CBasePlayer_SetClientUserInfoName", .post = 1);

  register_concmd("amx_ban", "@BanCommand");
}

public client_putinserver(player_id) {
  CreateOrUpdateUser(player_id);

  CheckUserIsBanned(player_id);
}

@CBasePlayer_SetClientUserInfoName(const player_id, infobuffer[], newName[]) {
  if (equal(infobuffer, newName))
    return;

  UpdateUserName(player_id, newName);
}

ReadMainConfig() {
  new configFile[64];
  get_configsdir(configFile, charsmax(configFile));
  add(configFile, charsmax(configFile), "/main.json");

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

  json_object_get_string(config, "database.type", DatabaseConfig[DbType], charsmax(DatabaseConfig[]), .dot_not = true);
  json_object_get_string(config, "database.host", DatabaseConfig[DbHost], charsmax(DatabaseConfig[]), .dot_not = true);
  json_object_get_string(config, "database.user", DatabaseConfig[DbUser], charsmax(DatabaseConfig[]), .dot_not = true);
  json_object_get_string(config, "database.password", DatabaseConfig[DbPassword], charsmax(DatabaseConfig[]), .dot_not = true);
  json_object_get_string(config, "database.database", DatabaseConfig[DbDatabase], charsmax(DatabaseConfig[]), .dot_not = true);

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

  new errorMessage[512], errorCode;
  new Handle: connectionHandle = SQL_Connect(DbHandle, errorCode, errorMessage, charsmax(errorMessage));

  if (connectionHandle == Empty_Handle) {
    SQL_FreeHandle(DbHandle);
    set_fail_state("[Ban Connection Test] Connection error[%d]: %s", errorCode, errorMessage);

    return;
  }

  SQL_FreeHandle(connectionHandle);
}
