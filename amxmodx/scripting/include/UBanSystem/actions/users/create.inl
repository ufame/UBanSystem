CreateOrUpdateUser(const player_id, const authid[]) {
  enum Player {
    name[MAX_NAME_LENGTH],
    ip[MAX_IP_LENGTH],
  };

  new playerData[Player];

  get_user_name(player_id, playerData[name], MAX_NAME_LENGTH - 1);
  get_user_ip(player_id, playerData[ip], MAX_IP_LENGTH - 1, .without_port = 1);

  new escapedName[MAX_NAME_LENGTH * 2];
  MysqlEscapeString(escapedName, charsmax(escapedName), playerData[name]);

  new dbQuery[512];

  formatex(dbQuery, charsmax(dbQuery), "INSERT INTO users (steam) VALUES ('%s') ON DUPLICATE KEY UPDATE updated_at = CURRENT_TIMESTAMP; ", authid);
  add(dbQuery, charsmax(dbQuery), fmt("SET @user_id = (SELECT id FROM users WHERE steam = '%s'); ", authid));
  add(dbQuery, charsmax(dbQuery), fmt("INSERT INTO ips_history (user_id, ip) VALUES (@user_id, '%s') ON DUPLICATE KEY UPDATE updated_at = CURRENT_TIMESTAMP; ", playerData[ip]));
  add(dbQuery, charsmax(dbQuery), fmt("INSERT INTO names_history (user_id, user_name) VALUES (@user_id, '%s') ON DUPLICATE KEY UPDATE updated_at = CURRENT_TIMESTAMP;", escapedName));

  SQL_ThreadQuery(DbHandle, "@CreateOrUpdateUserHandler", dbQuery);
}

@CreateOrUpdateUserHandler(const failstate, const Handle: query, const error[], const errnum, const data[], const size, const Float: queuetime) {
  if (failstate != TQUERY_SUCCESS) {
    SQL_ThreadError(query, error, errnum, queuetime);
    return;
  }
}
