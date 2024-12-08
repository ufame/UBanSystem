UpdateUserName(const player_id, const name[]) {
  new escapedName[MAX_NAME_LENGTH * 2];
  MysqlEscapeString(escapedName, charsmax(escapedName), name);

  new steamId[MAX_AUTHID_LENGTH];
  get_user_authid(player_id, steamId, MAX_AUTHID_LENGTH - 1);

  new dbQuery[512];
  formatex(dbQuery, charsmax(dbQuery), "SET @user_id = (SELECT id FROM users WHERE steam = '%s'); ", steamId);
  add(dbQuery, charsmax(dbQuery), fmt("INSERT INTO names_history (user_id, user_name) VALUES (@user_id, '%s') ON DUPLICATE KEY UPDATE updated_at = CURRENT_TIMESTAMP;", escapedName));

  SQL_ThreadQuery(DbHandle, "@UpdateUserNameHandler", dbQuery);
}

@UpdateUserNameHandler(const failstate, const Handle: query, const error[], const errnum, const data[], const size, const Float: queuetime) {
  if (failstate != TQUERY_SUCCESS) {
    SQL_ThreadError(query, error, errnum, queuetime);
    return;
  }
}