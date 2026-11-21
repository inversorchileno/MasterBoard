# Master Board

This repository is **Master Board** (called LibertyBoard until version 2.0), a modified version of OpenBoard (GPLv3) owned by Luis, who builds it for his own use on Windows 11. Before any work, read:

1. [docs/MASTERBOARD.md](docs/MASTERBOARD.md) (in Spanish): what Master Board is, every agreed change, how it is built and published, the way of working with Luis, how to test on Linux and the pending ideas.
2. [AGENTS.md](AGENTS.md) and [CODING_STYLE.md](CODING_STYLE.md): the OpenBoard rules for code, commits and data safety, which still apply.

Where they differ, for this repository:

- Luis is the maintainer. Pushing to `master` is how a change reaches him: it starts the Windows build and publishes a release that his **About → Update** menu installs. Push only when he asks for a build; when he says not to compile yet, commit without pushing to `master`.
- Talk to Luis in Spanish, in plain words: he does not program.
- New files go in both qmake (`src/*/*.pri`, used for Windows) and CMake (`src/*/CMakeLists.txt`, used for the Linux tests).
- New user-visible texts are translated into all languages of `resources/i18n` in a separate `i18n:` commit.
