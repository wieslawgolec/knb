# knb

```
 _  __      _
| |/ /_ __ | |__
| ' /| '_ \| '_ \
| . \| | | | |_) |
|_|\_\_| |_|_.__/
```

**Keep nick bot** — a small IRCNet-oriented nick keeper written in C.

It stays connected, watches a list of preferred nicks with `ISON`, and claims them when they free up. Optional channels, vhosts, and a simple userfile complete the picture.

**Version:** 0.3.0  
**License:** GPL-style / historical psotnic-related lineage (see source headers)

---

## Features

- IRCNet-style servers (including modern **2.11+ / 2.12.0-pre**)
- Optional **OpenSSL TLS** (port **6697** or `ssl on`)
- **SASL PLAIN** authentication (`sasluser` / `saslpass`)
- IPv4 and optional IPv6 (`server` / `server6`)
- Nick list + optional altnick / nickappend
- Channels, CTCP handling, userfile masks
- Optional binary integrity validator (SHA-256)
- GitHub Actions CI (Ubuntu, macOS, FreeBSD)

Plain TCP behaviour is unchanged; SSL and SASL are additive.

---

## Requirements

- C compiler (`gcc` or compatible)
- `make` / `gmake` and `bash`
- Optional: OpenSSL development package for TLS (`libssl-dev`, `openssl`, etc.)
- Optional: `pkg-config`

---

## Build

```bash
cd src
./configure [options]
make dynamic          # recommended
```

Binary is installed as `../knb-<version>-<os>` with a `../knb` symlink.

### Configure options

| Option | Description |
|--------|-------------|
| `--with-ssl` | Force OpenSSL (error if not found) |
| `--without-ssl` | Disable TLS |
| `--with-ipv6` | Enable IPv6 (default on most platforms) |
| `--without-ipv6` | Disable IPv6 |
| `--without-validator` | Do not enable the binary protector |
| `--help` | Show help |

SSL is **auto-detected** when neither `--with-ssl` nor `--without-ssl` is given.

### Other targets

```bash
make static
make debug
make release
make clean
make src
```

---

## Configuration

Examples live under `cfg-examples/`. Symlink or copy one to `conf` (or pass the path as the last argument).

Important options:

| Option | Meaning |
|--------|---------|
| `nick` | Primary nick (required) |
| `nicks` | Space-separated nick list to keep (required) |
| `server` / `server6` | Hostname `[port]` (required; default port 6667) |
| `ssl` | `on` / `off` — force TLS for servers (also auto on port **6697**) |
| `sasluser` | SASL account / username |
| `saslpass` | SASL password (PLAIN) |
| `channel` | Channel `[key]` |
| `vhost` | Bind address |
| `ident` / `realname` | USER line fields |
| `altnick` / `nickappend` | Alternate nick behaviour |
| `logfile` / `pidfile` / `userfile` | Paths |

First user mask: message the bot with `!new` (or on a shared channel).

Commands start with `.`, `!`, or `@` — see `!help` once online.

---

## SSL / TLS

- Build with OpenSSL (`--with-ssl` or auto-detect).
- Port **6697** enables TLS automatically for that server entry.
- Or set `ssl on` in the config to prefer TLS for configured servers.
- Handshake runs on the established TCP socket before NICK/USER (and before SASL CAP).
- Certificate verification is currently disabled (same class of tool as cloner).

---

## SASL

```text
sasluser youraccount
saslpass secret
```

On connect the bot sends `CAP LS 302`, requests `sasl` when advertised, and authenticates with **PLAIN**. If the network does not offer SASL, registration continues without it.

---

## Usage

```bash
./knb conf
./knb -v
./knb -u          # self-update helper (legacy)
./knb -a conf1 conf2   # install crontab entries
```

---

## Notes

- Prefer packages from CI or `make release` if a local build fails on an exotic OS.
- Linking warnings on some platforms can be ignored when the binary still runs.
- See [CHANGELOG.md](CHANGELOG.md) for history.

---

## License / history

Parts of the integrity validator descend from early psotnic code (later GPL; see `src/validator.c` header). Overall project remains a personal/small-network IRC utility — use responsibly and in line with network policy.
