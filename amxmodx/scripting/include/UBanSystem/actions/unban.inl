#include <amxmodx>
#include <sqlx>

enum _: UnbanActionData_Struct {
  Data_AdminUserId,
  Data_PlayerSteamId[MAX_AUTHID_LENGTH]
}

UnbanAction(const player_id, const adminSteamId[], const targetSteamId[], const bool: canUnbanAny = false) { 
  new dbQuery[512];

  add(dbQuery, charsmax(dbQuery), "UPDATE bans ");
  add(dbQuery, charsmax(dbQuery), fmt("JOIN users AS target_user ON target_user.steam = '%s' ", targetSteamId));
  add(dbQuery, charsmax(dbQuery), fmt("JOIN users AS admin_user ON admin_user.steam = '%s' ", adminSteamId));
  add(dbQuery, charsmax(dbQuery), "SET bans.unban_timestamp = CURRENT_TIMESTAMP, ");
  add(dbQuery, charsmax(dbQuery), "bans.unbanned_by_user_id = admin_user.id ");
  add(dbQuery, charsmax(dbQuery), "WHERE bans.user_id = target_user.id ");
  add(dbQuery, charsmax(dbQuery), "AND (bans.unban_timestamp IS NULL OR bans.unban_timestamp > CURRENT_TIMESTAMP) ");

  if (!canUnbanAny) {
    add(dbQuery, charsmax(dbQuery), "AND bans.admin_id = admin_user.id");
  }

  add(dbQuery, charsmax(dbQuery), ";");

  new data[UnbanActionData_Struct];
  data[Data_AdminUserId] = get_user_userid(player_id);
  copy(data[Data_PlayerSteamId], MAX_AUTHID_LENGTH - 1, targetSteamId);

  SQL_ThreadQuery(DbHandle, "@UnbanActionHandler", dbQuery, data, sizeof data);
}

@UnbanActionHandler(const failstate, const Handle: query, const error[], const errnum, const data[UnbanActionData_Struct], const size, const Float: queuetime) {
  if (failstate != TQUERY_SUCCESS) {
    SQL_ThreadError(query, error, errnum, queuetime);
    return;
  }

  new userId = data[Data_AdminUserId];
  new player_id = find_player_ex(FindPlayer_MatchUserId, userId);

  if (!player_id)
    return;

  if (!SQL_AffectedRows(query)) {
    client_print(player_id, print_console, "[UnBan Action] No active ban found for the provided SteamID.");

    return;
  }

  new dbQuery[512];

  add(dbQuery, charsmax(dbQuery), fmt("SELECT names_history.user_name "));
  add(dbQuery, charsmax(dbQuery), fmt("FROM names_history "));
  add(dbQuery, charsmax(dbQuery), fmt("JOIN users ON users.steam = '%s' ", data[Data_PlayerSteamId]));
  add(dbQuery, charsmax(dbQuery), fmt("WHERE names_history.user_id = users.id "));
  add(dbQuery, charsmax(dbQuery), fmt("ORDER BY names_history.updated_at DESC LIMIT 1;"));

  SQL_ThreadQuery(DbHandle, "@UnbanActionInfoHandler", dbQuery, data, sizeof data);
}

@UnbanActionInfoHandler(const failstate, const Handle: query, const error[], const errnum, const data[UnbanActionData_Struct], const size, const Float: queuetime) {
  if (failstate != TQUERY_SUCCESS) {
    SQL_ThreadError(query, error, errnum, queuetime);
    return;
  }

  new userId = data[Data_AdminUserId];
  new player_id = find_player_ex(FindPlayer_MatchUserId, userId);

  if (!player_id)
    return;

  new userName[MAX_NAME_LENGTH];
  SQL_ReadResult(query, 0, userName, MAX_NAME_LENGTH - 1);

  client_print_color(0, print_team_default, "^4*** ^3%n^1 unbanned ^3%s^1.", player_id, userName);
}