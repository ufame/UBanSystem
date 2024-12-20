enum _: BanActionData_Struct {
  BanData_AdminId,
  BanData_TargetId,
  BanData_Time,
  BanData_Reason[64]
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
  add(dbQuery, charsmax(dbQuery), fmt("VALUES (@target_id, @admin_id, '%s', %d, CURRENT_TIMESTAMP, ", reason, time));
  
  if (time) {
    add(dbQuery, charsmax(dbQuery), fmt("DATE_ADD(CURRENT_TIMESTAMP, INTERVAL %d SECOND));", time));
  } else {
    add(dbQuery, charsmax(dbQuery), fmt("NULL);"));
  }

  new data[BanActionData_Struct];

  data[BanData_AdminId] = get_user_userid(player_id);
  data[BanData_TargetId] = get_user_userid(target_id);
  data[BanData_Time] = time;
  copy(data[BanData_Reason], charsmax(data[BanData_Reason]), reason);

  SQL_ThreadQuery(DbHandle, "@BanActionHandler", dbQuery, data, sizeof data);
}

@BanActionHandler(const failstate, const Handle: query, const error[], const errnum, const data[BanActionData_Struct], const size, const Float: queuetime) {
  if (failstate != TQUERY_SUCCESS) {
    SQL_ThreadError(query, error, errnum, queuetime);
    return;
  }

  new player_id = find_player_ex(FindPlayer_MatchUserId, data[BanData_AdminId]);
  new target_id = find_player_ex(FindPlayer_MatchUserId, data[BanData_TargetId]);

  new formattedTime[32];
  get_time_length(player_id, data[BanData_Time], timeunit_seconds, formattedTime, charsmax(formattedTime));

  client_print_color(0, print_team_default,
    "^4*** ^3%n ^1banned ^3%n ^1for ^4%s^1. Reason: ^4%s^1.",
    player_id, target_id, data[BanData_Time] ? formattedTime : "forever", data[BanData_Reason]
  );
  
  if (SettingsConfig[Settings_KickAfterBan]) {
    set_task(SettingsConfig[Settings_KickAfterBan_Time], "@Task_KickAfterBan", target_id + TASK_KICK_ID, data[BanData_Reason], sizeof data[BanData_Reason]);
  }
}

@Task_KickAfterBan(const reason[], id) {
  id -= TASK_KICK_ID;

  if (!is_user_connected(id))
    return;

  UserKick(id, reason);
}
