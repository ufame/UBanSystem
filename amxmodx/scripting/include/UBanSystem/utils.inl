ParseBanTime(const value[]) {
  new result = 0
  new currentNumber = 0;

  if (isdigit(value[0]) && str_to_num(value[0]) <= 0)
    return 0;

  static units[][] = {
    { 's', 1 },          // Секунды
    { 'i', 60 },         // Минуты
    { 'h', 3600 },       // Часы
    { 'd', 86400 },      // Дни
    { 'w', 604800 },     // Недели
    { 'm', 2592000 },    // Месяцы
    { 'y', 31104000 }    // Годы
  };

  for (new i = 0; value[i] != EOS; i++) {
    if (isdigit(value[i])) {
      currentNumber = currentNumber * 10 + (value[i] - '0');
    } else {
      for (new j = 0; j < sizeof(units); j++) {
        if (units[j][0] == value[i]) {
          result += currentNumber * units[j][1];
          currentNumber = 0;
          break;
        }
      }
    }
  }

  result += currentNumber;

  return result;
}

UserKick(const player_id, const reason[]) {
  server_cmd("kick #%d ^"%s^"", get_user_userid(player_id), reason);
}

bool: CheckUserAccess(const player_id, const EAccessFlagsConfig: access) {
  return bool: (get_user_flags(player_id) & AccessFlagsConfig[access]);
}

FindPlayerId(const args[]) {
  new player_id = 0;

  player_id = find_player_ex(FindPlayer_MatchName, args);

  if (!player_id)
    player_id = find_player_ex(FindPlayer_MatchAuthId, args);

  if (!player_id)
    player_id = find_player_ex(FindPlayer_MatchUserId, args);

  return player_id;
}

LoadAccessFlags(const JSON: config, const key[]) {
  new flags[32];
  json_object_get_string(config, key, flags, charsmax(flags), .dot_not = true);

  return read_flags(flags);
}

MysqlEscapeString(output[], len, const source[]) {
  static const inChars[][] = { "\\", "\0", "\n", "\r", "\x1a", "'", "^"" };
  static const outChars[][] = { "\\\\", "\\0", "\\n", "\\r", "\Z", "\'", "\^"" };

  copy(output, len, source);

  for (new i = 0; i < sizeof inChars; i++) {
    replace_string(output, len, inChars[i], outChars[i]);
  }
}

SQL_ThreadError(const Handle: query, const error[], const errorCode, const Float: queuetime) {
  new dbQuery[512];
  SQL_GetQueryString(query, dbQuery, charsmax(dbQuery));

  abort(AMX_ERR_GENERAL, "[SB SQL Error] Queue time: %.4f", queuetime);
  abort(AMX_ERR_GENERAL, "[SB SQL Error] Error[%d]: %s", errorCode, error);
  abort(AMX_ERR_GENERAL, "[SB SQL Error] Query with error: %s", dbQuery);
}
