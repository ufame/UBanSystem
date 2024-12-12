UnBanAction(const player_id, const unbanUserSteamId[MAX_AUTHID_LENGTH]) {
  new steamId[MAX_AUTHID_LENGTH];
  get_user_authid(player_id, steamId, MAX_AUTHID_LENGTH -1);

  new dbQuery[512];

  formatex(dbQuery, charsmax(dbQuery), "SET @admin_id = (SELECT id FROM users WHERE steam = '%s');", steamId);
  add(dbQuery, charsmax(dbQuery), fmt("SET @user_id = (SELECT id FROM users WHERE steam = '%s');", unbanUserSteamId));
  add(dbQuery, charsmax(dbQuery), fmt("UPDATE bans SET unban_timestamp = CURRENT_TIMESTAMP, unbanned_by_user_id = @admin_id "));
  add(dbQuery, charsmax(dbQuery), fmt("WHERE user_id = @user_id AND (unban_timestamp IS NULL OR unban_timestamp > CURRENT_TIMESTAMP);"));
  add(dbQuery, charsmax(dbQuery), fmt("SELECT user_name FROM names_history WHERE user_id = @user_id ORDER BY updated_at DESC LIMIT 1"));

  new data[1];
  data[0] = get_user_userid(player_id);

  SQL_ThreadQuery(DbHandle, "@UnBanActionHandler", dbQuery, data, sizeof data);
}

@UnBanActionHandler(const failstate, const Handle: query, const error[], const errnum, const data[], const size, const Float: queuetime) {
  if (failstate != TQUERY_SUCCESS) {
    SQL_ThreadError(query, error, errnum, queuetime);
    return;
  }

  new userId = data[0];
  new player_id = find_player_ex(FindPlayer_MatchUserId, userId);

  if (!player_id)
    return;

  if (!SQL_NumResults(query))
    return;

  new userName[MAX_NAME_LENGTH];
  SQL_ReadResult(query, 0, userName, MAX_NAME_LENGTH - 1);

  client_print_color(0, print_team_default, "^4*** ^3%n ^1unbanned ^3%s^1.", player_id, userName);
}