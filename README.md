# WireScope (Wireshark-like Starter)

Starter project structure for a packet capture desktop app using:
- C++17
- Qt6 (MVC style organization)
- MySQL
- Docker

## Project Structure

- `src/controllers`: app controllers (business flow)
- `src/models`: packet/domain models
- `src/views`: Qt UI windows/widgets
- `src/services`: app services (future capture/parsing services)
- `src/db`: database access logic
- `src/capture`: packet capture engine (future libpcap/npcap integration)
- `include/...`: headers mirrored by module
- `docker/init.sql`: MySQL initialization script

## Run MySQL with Docker

```bash
docker compose up -d mysql
```

## Build App in Docker

```bash
docker compose build app
docker compose run --rm app
```

Inside the app container:

```bash
cmake -S . -B build
cmake --build build -j
```

## Next Steps

1. Implement packet sniffing in `src/capture` (Npcap on Windows).
2. Convert captured packets to `Packet` model objects.
3. Insert packets into MySQL using the DB layer.
4. Show packet list/details in the Qt view.
to run:
docker compose down -v
docker compose up --build --force-recreate