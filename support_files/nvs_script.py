import os

Import("env")   # type: ignore

def set_nvs():
    flag_path = os.path.join(env['PROJECT_DIR'], '.pio', 'build', env['PIOENV'], 'nvs_flag.txt')
    os.makedirs(os.path.dirname(flag_path), exist_ok=True)
    with open(flag_path, 'w') as f:
        f.write("NVS flag created!\n")
    print(f"[INFO] NVS Flag created at: {flag_path}")

def before_upload(source, target, env):
    print("[NVS file] NVS flag set to upload")
    board_config = env.BoardConfig()
    mcu = (board_config.get("build.mcu") or env.get("BOARD_MCU") or "").lower()
    if mcu == "esp32p4":
        merged_bin = os.path.join(env["PROJECT_DIR"], f"Launcher-{env['PIOENV']}.bin")
        env.Replace(
            UPLOADERFLAGS=[
                "--chip",
                mcu,
                "--port",
                '"$UPLOAD_PORT"',
                "--baud",
                "$UPLOAD_SPEED",
                "--before",
                board_config.get("upload.before_reset", "default-reset"),
                "--after",
                board_config.get("upload.after_reset", "hard-reset"),
                "write-flash",
                "-z",
                "--flash-mode",
                "${__get_board_flash_mode(__env__)}",
                "--flash-freq",
                "${__get_board_f_image(__env__)}",
                "--flash-size",
                "detect",
            ]
        )
        env.Replace(UPLOADCMD=f'$UPLOADER $UPLOADERFLAGS 0x0 "{merged_bin}"')
        return

    bin_path = os.path.join(env['PROJECT_DIR'], 'support_files', 'UiFlow2_nvs.bin')
    env.Append(UPLOADERFLAGS=[0x9000, bin_path])

env.AddPreAction("upload", before_upload)
set_nvs()
