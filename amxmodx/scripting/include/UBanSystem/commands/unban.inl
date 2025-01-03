@UnbanCommand(const player_id) {
  if (!CheckUserAccess(player_id, AccessFlags_Unban_Self) && !CheckUserAccess(player_id, AccessFlags_Unban_Others))
    return;

  if (read_argc() > 2) {
    client_print(player_id, print_console, "[Unban Command] Syntax: amx_unban ^"steamid^"");

    return;
  }

  new targetSteamId[MAX_AUTHID_LENGTH];
  read_argv(1, targetSteamId, MAX_AUTHID_LENGTH - 1);

  new adminSteamId[MAX_AUTHID_LENGTH];
  get_user_authid(player_id, adminSteamId, MAX_AUTHID_LENGTH - 1);

  UnbanAction(player_id, adminSteamId, targetSteamId, CheckUserAccess(player_id, AccessFlags_Unban_Others));
}