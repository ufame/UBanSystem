ParseBanTime(const value[]) {
  new result = 0;

  new multipliers[] = { 1, 60, 3600, 86400, 2592000, 31104000 };
  new multiplierIndex = 0;
  new currentNumber = 0;

  for (new i = 0; value[i] != EOS; i++) {
    if (isdigit(value[i])) {
      currentNumber = currentNumber * 10 + (value[i] - '0');
    } else {
      switch (value[i]) {
        case 'i': multiplierIndex = 1;
        case 'h': multiplierIndex = 2;
        case 'd': multiplierIndex = 3;
        case 'm': multiplierIndex = 4;
        case 'y': multiplierIndex = 5;
        default: break;
      }

      result += currentNumber * multipliers[multiplierIndex];
      currentNumber = 0;
    }
  }

  result += currentNumber * multipliers[multiplierIndex];

  return result;
}

UserKick(const player_id, const reason[]) {
  server_cmd("kick #%d ^"%s^"", get_user_userid(player_id), reason);
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
