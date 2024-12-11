enum _: BanParameters {
  AdminId,
  TargetId,
  Time,
  Reason[64]
};

BanAction(const player_id, const target_id, const time, const reason[]) {
  if (!is_user_connected(target_id)) {
    client_print(player_id, print_console, "[Ban Action] Player %n has been disconnected.", target_id);

    return;
  }

  new adminSteam[MAX_AUTHID_LENGTH];
  new targetSteam[MAX_AUTHID_LENGTH];

  get_user_authid(player_id, adminSteam, MAX_AUTHID_LENGTH - 1);
  get_user_authid(target_id, targetSteam, MAX_AUTHID_LENGTH - 1);

  new dbQuery[2048];

  formatex(dbQuery, charsmax(dbQuery), "SET @admin_id = (SELECT id FROM users WHERE steam = '%s'); ", adminSteam);
  add(dbQuery, charsmax(dbQuery), fmt("SET @target_id = (SELECT id FROM users WHERE steam = '%s'); ", targetSteam));
  add(dbQuery, charsmax(dbQuery), fmt("INSERT INTO bans (user_id, admin_id, reason, ban_duration, ban_timestamp, unban_timestamp) "));
  add(dbQuery, charsmax(dbQuery), fmt("VALUES (@target_id, @admin_id, '%s', %d, CURRENT_TIMESTAMP, DATE_ADD(CURRENT_TIMESTAMP, INTERVAL %d SECONDS));", reason, time, time));

  log_amx(dbQuery);

  new data[BanParameters];

  data[AdminId] = get_user_userid(player_id);
  data[TargetId] = get_user_userid(target_id);
  data[Time] = time;
  copy(data[Reason], charsmax(data[Reason]), reason);

  SQL_ThreadQuery(DbHandle, "@BanActionHandler", dbQuery, data, sizeof data);
}

@BanActionHandler(const failstate, const Handle: query, const error[], const errnum, const data[BanParameters], const size, const Float: queuetime) {
  if (failstate != TQUERY_SUCCESS) {
    SQL_ThreadError(query, error, errnum, queuetime);
    return;
  }

  new player_id = find_player_ex(FindPlayer_MatchUserId, data[AdminId]);
  new target_id = find_player_ex(FindPlayer_MatchUserId, data[TargetId]);

  new formattedTime[32];
  get_time_length(player_id, data[Time], timeunit_minutes, formattedTime, charsmax(formattedTime));

  client_print_color(0, print_team_default, "^4*** ^3%n ^1banned ^3%n ^1for ^4%s^1. Reason: ^4%s^1.", player_id, target_id, formattedTime, data[Reason]);
  UserKick(player_id, data[Reason]);
}
