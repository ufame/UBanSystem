@UnbanCommand(const player_id) {
  if (read_argc() > 2) {
    client_print(player_id, print_console, "[UnBan Command] Syntax: amx_unban ^"steamid^"");

    return;
  }

  new steamId[MAX_AUTHID_LENGTH];
  read_argv(1, steamId, MAX_AUTHID_LENGTH - 1);

  UnBanAction(player_id, steamId);
}