## Configuration
Change configuration in ```config.yaml```
```yaml
# config.yaml
---
upstream_dns: 1.1.1.1 # working dns server ip address
blacklist_response: resolve # not_found or refused or resolve
resolve_ip: 1.2.3.4 # only if blacklist_response equals 'resolve', else delete whole line
blacklist: # array of blacklist's items
    - example.com
    # ...
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

## Build

```bash
chmod +x build.sh
./build.sh <debug or release> # debug by default

# Run
./build/<debug or release>/dnsproxy
```

## Dependencies (Third-party Libraries)

-   [uthash](https://troydhanson.github.io/uthash/) (BSD license), included in `third_party/uthash/`.

    No additional installation is needed.
    See `LICENSES/utash-LICENSE.md` for more details.

-   [libyaml](https://github.com/yaml/libyaml) (MIT License).

    See `LICENSES/libyaml-LICENSE.md` for more details.

-   [C-Thread-Pool](https://github.com/Pithikos/C-Thread-Pool) (MIT LICENSE)

    See `LICENSES/c-thread-pool-LICENSE.md` for more details.
