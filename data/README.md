# Data Folder Guide

This folder is for mock JSON files that students must create themselves.

- Example payloads and shapes are documented in `data/mock_payloads.md` with complete samples.
- The repo intentionally does not include ready-made JSON files so you build them yourself.
- Use the exact filenames and paths as documented (tests and scripts rely on them).
- Tip: validate your files with a simple JSON linter/validator before opening a PR.

Files you should create (examples):

- `data/mock_state.json`
- `data/mock_lobby_wait.json`
- `data/mock_recruit_state_advanced.json`
- `data/mock_combat_log_advanced.json`
- `data/mock_error_not_enough_coins.json`
- `data/mock_error_board_full.json`

Notes:

- For offline runs, your app can accept a switch like `--offline <path>` and load one of these files.
- If the JSON structure changes, also update `mock_payloads.md` via PR so everyone stays in sync.
