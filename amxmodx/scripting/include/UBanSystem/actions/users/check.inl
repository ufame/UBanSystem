CheckUserIsBanned(const player_id) {
  new steamId[MAX_AUTHID_LENGTH];
  get_user_authid(player_id, steamId, MAX_AUTHID_LENGTH - 1);

  new dbQuery[512];
  formatex(dbQuery, charsmax(dbQuery), "SELECT reason FROM users JOIN bans ON bans.user_id = users.id WHERE bans.unban_timestamp > CURRENT_TIMESTAMP", steamId);

  new data[1];
  data[0] = player_id;

  SQL_ThreadQuery(DbHandle, "@CheckUserisBannedHandler", dbQuery, data, sizeof data);
}

@CheckUserisBannedHandler(const failstate, const Handle: query, const error[], const errnum, const data[], const size, const Float: queuetime) {
  if (failstate != TQUERY_SUCCESS) {
    SQL_ThreadError(query, error, errnum, queuetime);
    return;
  }

  new player_id = data[0];

  if (!is_user_connected(player_id))
    return;

  if (!SQL_NumResults(query))
    return;

  new reason[256];
  SQL_ReadResult(query, 0, reason, charsmax(reason));

  UserKick(player_id, reason);
}