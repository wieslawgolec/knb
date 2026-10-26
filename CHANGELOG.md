# Changelog

## 0.3.0 (2026-10-26)

- OpenSSL TLS support (auto-detect / `--with-ssl` / `--without-ssl`)
- SASL PLAIN (`sasluser` / `saslpass` config options)
- Config options: `ssl`, `sasluser`, `saslpass`
- Port 6697 enables TLS automatically for a server entry
- Modernized `configure.sh` (pkg-config, Homebrew OpenSSL paths, macOS/FreeBSD)
- GitHub Actions CI (Ubuntu, macOS, FreeBSD)
- Fixed Makefile generation when SSL is disabled (missing separator on Ubuntu CI)
- Documentation: README.md only; legacy `README` removed; CHANGELOG → CHANGELOG.md
- Version bump from 0.2.6

---

## Historical (0.2.x and earlier)

See git history and the previous CHANGELOG file for full 2005–2011 entries (0.0.1b through 0.2.5).

Key 0.2.x notes retained for reference:

- 0.2.5: delay cfg option, alphabet option, randomness in checks
- 0.2.4: reason cfg option, IPv6 default packages
- 0.2.3: ctcpreply, default ctcp reply to all
- 0.2.0: improved op/deop/voice, kick/ban, binary protector, queue improvements
