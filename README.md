# DNS Proxy Server

A multithreaded DNS proxy server written in C, supporting:

-   Domain blacklist filtering
-   Configurable responses for blacklisted domains:
    -   `not_found` (NXDOMAIN)
    -   `refused`
    -   `resolve` (with custom IP address)
-   High-performance non-blocking I/O using `epoll`
-   Thread pool for concurrent request processing

## Table of Contents

-   [How It Works](#how-it-works)
-   [Configuration](#configuration)
-   [Building](#building)
-   [Running the Proxy](#running-the-proxy)
-   [Testing](#testing)
-   [Features](#features)
-   [Limitations](#limitations)
-   [Dependencies](#dependencies-third-party-libraries)

## How It Works

1. The server reads configuration from `config.yaml`.
2. It listens for DNS queries on UDP port **1053** by default (can be changed in `main.c`).
3. For each incoming query:
    - The domain is parsed from the request.
    - The domain is checked against the blacklist.
    - If blacklisted, the server replies with the configured response type.
    - If not blacklisted, the request is forwarded to the upstream DNS server.
    - The upstream response is relayed back to the client.

## Configuration

Configuration is provided via a YAML file (`config.yaml`):

```yaml
# config.yaml
---
upstream_dns: 1.1.1.1 # IP address of the upstream DNS server
blacklist_response: resolve # one of: not_found | refused | resolve
resolve_ip: 1.2.3.4 # used only if blacklist_response is 'resolve'
blacklist:
    - example.com
    - test.com
```

#### upstream_dns:

-   Value:
    -   ip address

#### blacklist_response:

-   Value (one of):
    -   not_found
    -   refused
    -   resolve

| If value is resolve then also need to specify resolve_ip

#### blacklist:

-   Value:
    -   array of addresses

## Building

```bash
chmod +x build.sh
./build.sh [debug|release]  # defaults to debug if not specified
```

## Running the Proxy

#### Note: the server is binded to port 1053 by default. You can change it in `main.c`

```bash
./build/debug/dnsproxy
# or
./build/release/dnsproxy
```

## Testing

The server can be tested using tools like `dig`s:

```bash
dig @127.0.0.1 -p 1053 example.com
```

or run prepared tests

```bash
chmod +x test.sh
./test.sh
```

## Features

-   High-performance non-blocking I/O using epoll
-   Thread pool for parallel query handling
-   Configurable blacklist behavior
-   Lightweight and dependency-minimal
-   Easy debugging and logging support

## Limitations

-   No wildcard support in blacklist (exact match only)
-   No DNS caching
-   Configuration is static — cannot be reloaded at runtime (but you can change the configuration and restart server)

## Dependencies (Third-party Libraries)

-   [uthash](https://troydhanson.github.io/uthash/) (BSD license), included in `third_party/uthash/`.

    No additional installation is needed.
    See `LICENSES/utash-LICENSE.md` for more details.

-   [libyaml](https://github.com/yaml/libyaml) (MIT License).

    See `LICENSES/libyaml-LICENSE.md` for more details.

-   [C-Thread-Pool](https://github.com/Pithikos/C-Thread-Pool) (MIT LICENSE)

    See `LICENSES/c-thread-pool-LICENSE.md` for more details.
