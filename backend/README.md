# Smart Bus Tracker — Backend

A Drogon (C++17) REST + WebSocket backend for real-time bus tracking, backed by PostgreSQL.

This tree has been fixed up from the original upload: leaked credentials removed,
password hashing replaced, role-based authorization added, and every previously-empty
stub file (services, repository, geo utils, WebSocket controller) implemented and
tested end-to-end against a real Postgres instance.

## 1. Prerequisites

- CMake ≥ 3.16, a C++17 compiler (GCC/Clang)
- PostgreSQL (13+ recommended)
- Drogon dev package and its dependencies. On Ubuntu/Debian:

```bash
sudo apt-get install -y build-essential cmake libdrogon-dev libssl-dev libpq-dev \
    libjsoncpp-dev libsqlite3-dev libmariadb-dev libmariadb-dev-compat \
    libhiredis-dev libyaml-cpp-dev zlib1g-dev uuid-dev libbrotli-dev libc-ares-dev
```

(`jwt-cpp` is fetched automatically at configure time via CMake `FetchContent` — no
separate install needed, but the build machine needs network access to GitHub.)

## 2. Configure secrets

```bash
cp .env.example .env
# then edit .env:
#   DB_HOST, DB_PORT, DB_NAME, DB_USER, DB_PASSWORD, DB_SSLMODE
#   JWT_SECRET   (generate one with: openssl rand -hex 32)
```

`.env` is git-ignored. `config.json` itself contains no secrets — it only has
`${VAR}` placeholders that `main.cc` expands from the environment (loading `.env`
first if present) before handing the config to Drogon.

In a real deployment, set these as actual environment variables on the host/container
instead of shipping a `.env` file.

## 3. Create the database

```bash
createdb busdb
psql -d busdb -f database/init.sql
```

(`init.sql` now enables the `pgcrypto` extension itself, which `gen_random_uuid()`
requires — a fresh Postgres doesn't have it on by default.)

## 4. Build

```bash
mkdir -p build && cd build
cmake ..
cmake --build . -j$(nproc)
```

Produces `build/SmartBusTracker`.

## 5. Run

```bash
cd backend    # so config.json / .env are found via relative paths
./build/SmartBusTracker
```

Health check: `GET /api/health` → `{"status":"ok"}`

## API summary

| Method | Path | Auth | Notes |
|---|---|---|---|
| POST | `/api/v1/auth/register` | none | Always registers as PASSENGER — self-registration can't pick a role |
| POST | `/api/v1/auth/login` | none | Returns a JWT |
| GET | `/api/v1/fleet/buses` | any logged-in user | |
| POST | `/api/v1/fleet/buses` | ADMIN | |
| GET | `/api/v1/routes` | any logged-in user | |
| POST | `/api/v1/routes` | ADMIN | |
| GET | `/api/v1/routes/{routeId}/stops` | any logged-in user | |
| POST | `/api/v1/routes/{routeId}/stops` | ADMIN | |
| GET | `/api/v1/tracking/buses/{busId}/location` | any logged-in user | Latest known position |
| GET | `/api/v1/tracking/buses/{busId}/history?limit=50` | any logged-in user | |
| POST | `/api/v1/tracking/buses/{busId}/location` | DRIVER/CONDUCTOR/ADMIN | `speed_kmh` optional — derived from the previous point if omitted |
| WS | `/ws/tracking?token=<JWT>` | any logged-in user (token as query param) | `{"action":"subscribe","bus_id":...}` to receive live updates; `{"action":"push_location",...}` (staff only) to publish a position |

There's currently no admin endpoint to promote a user to DRIVER/CONDUCTOR/ADMIN —
do that directly in the database for now (`UPDATE users SET role_id = ... WHERE
email = ...`), or add such an endpoint (ADMIN-only) as a next step.

## What was fixed from the original codebase

- **Leaked live Supabase DB credentials and a JWT signing key**, committed in
  `config.json`, `src/models/model.json`, and `.env` — removed, replaced with
  `${VAR}` placeholders / an ignored `.env`. **If you're reusing the original
  database, rotate its password — assume it was already compromised.**
- **Unsalted SHA-256 password hashing** → salted PBKDF2-HMAC-SHA256 (210,000
  iterations, OWASP 2023 minimum), via OpenSSL's EVP API.
- **Hardcoded JWT secret in source** → read from `JWT_SECRET` env var, required
  at startup (the app refuses to start without it).
- **No role-based authorization** (any authenticated user could create buses/
  routes/stops) → `AdminOnlyFilter` / `StaffOnlyFilter` now gate those endpoints;
  self-registration can no longer pick its own `role_id`.
- **Six empty stub files** (`AuthService`, `RouteService`, `TrackingService`,
  `TrackingRepo`, `GeoUtils`, `TrackingWebSocketCtrl`) — the tracking feature the
  project is named for didn't exist at all. All implemented: haversine distance/
  speed/ETA math, a `bus_locations` repository, a service that derives speed from
  consecutive readings and broadcasts updates, and a real `/ws/tracking` WebSocket
  channel with subscribe/push actions.
- **No input validation** — UUID path params, lat/lon ranges, email format,
  password length, license plate/capacity bounds, and implausible speed values
  (e.g. a bad GPS jump computing to 1000+ km/h, which used to crash the insert
  against the DB's `DECIMAL(5,2)` column) are now all checked, with clear 400s.
- **Hardcoded macOS Homebrew path** in `CMakeLists.txt` — now conditional on
  `APPLE` and probes common locations, so it also configures on Linux/CI.
- **No tests / empty `tests/`** — the entire API surface (register/login incl.
  duplicate email and privilege-escalation attempts, role-gated fleet/route
  creation, tracking HTTP endpoints incl. speed derivation and rejection of
  implausible values, and the WebSocket subscribe/push/broadcast flow incl.
  rejecting unauthenticated connections and non-staff pushes) was manually
  exercised end-to-end against a real PostgreSQL instance during this pass —
  see the "Verification" section below. Automated tests are still worth adding
  as a next step; none exist yet.
- Removed the committed `build/` directory (hundreds of MB of vendored
  dependency source that had been accidentally checked in) and added a
  `.gitignore`.

## Verification performed

Built successfully with `cmake --build .` (zero errors). Ran against a local
PostgreSQL 16 instance and confirmed via curl / a small Python WebSocket script:

- Registration, duplicate-email rejection (409), and confirmation that supplying
  `role_id` in the register request is ignored (new users are always PASSENGER).
- Login success/failure, and that a wrong password is rejected.
- PASSENGER blocked (403) from creating buses/routes/stops and from pushing a
  tracking update; ADMIN allowed; requests with no token rejected (401).
- Invalid UUID path params and out-of-range lat/lon rejected (400).
- Speed correctly derived from two consecutive location readings; an
  out-of-range client-supplied speed rejected (400); an implausible derived
  speed no longer crashes the insert (it's now dropped instead of stored).
- WebSocket: unauthenticated connections rejected at handshake; a subscribed
  client receives a live broadcast when another client pushes a location for
  that bus; a PASSENGER's `push_location` attempt over the socket is rejected.

## Known gaps / suggested next steps

- No endpoint to promote users to DRIVER/CONDUCTOR/ADMIN (currently a manual
  DB update) — worth adding as an ADMIN-only endpoint.
- No automated test suite (see above) — the `tests/` directory is still empty;
  the checks above were exploratory/manual, not committed as repeatable tests.
- No rate limiting on `/auth/login` or `/auth/register` (brute-force / spam risk).
- ETA calculation (`GeoUtils::etaSeconds`) is implemented but not yet wired into
  an endpoint — nothing currently calls it.
- `redis_clients` config was removed since nothing in the codebase used it; add
  it back if/when you build caching or pub/sub on top of Redis.
