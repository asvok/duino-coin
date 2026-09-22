# Personal Duino-Coin development

This repository uses `master` for reviewed changes and `dev` for local work.
The official repository is a read-only update source. Do not push branches or
open pull requests to `duino-coin/duino-coin` for personal configuration.

## Update `dev` from official releases

1. Check out `dev` in your local clone and make sure it has no uncommitted changes.
2. Run `bash scripts/update-dev-from-official.sh`.
3. If Git reports a conflict, resolve it on `dev` and finish the merge.
4. Build the target board: `pio run -e ESP8266-D1-Mini`.
5. Push `dev` to **your** repository, `asvok/duino-coin`, and open a pull request
   from `dev` to `master` there when the changes are ready.

The script fetches and merges from the official `master` branch. It never pushes
to either repository.

## Private device settings

Copy `ESP_Code/Settings.local.example.h` to `ESP_Code/Settings.local.h` and edit
the copy. The local file and new `.pio/` output are ignored by Git. Keep Wi-Fi
passwords and mining keys out of commits, pull requests, screenshots, and build
logs. The public `Settings.h` contains safe example values.
