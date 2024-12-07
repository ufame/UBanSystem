@BanCommand(const player_id) {
  if (read_argc() > 4) {
    client_print(player_id, print_console, "[Ban Command] Syntax: amx_ban time ^"#userid OR Name OR steamid^" ^"reason^"");

    return;
  }

  enum args {
    BanTime[10],
    BanTarget[MAX_AUTHID_LENGTH],
    BanReason[64]
  };

  new args[args];

  read_argv(1, args[BanTime], charsmax(args[BanTime]));
  read_argv(2, args[BanTarget], charsmax(args[BanTarget]));
  read_argv(3, args[BanReason], charsmax(args[BanReason]));

  new time = ParseBanTime(args[BanTime]);
  new target = cmd_target(player_id, args[BanTarget]);

  if (!target) {
    client_print(player_id, print_console, "[Ban Command] Target not found.");

    return;
  }

  BanAction(player_id, target, time, args[BanReason]);
}

