## Configuration

``` yaml
---
upstream_dns: 1.1.1.1 # ip address

blacklist_response: resolve # not_found or refused or resolve
resolve_ip: 1.2.3.4 # only if blacklist_response equals 'resolve'

blacklist: # array of blacklist's items
  - ads.com
  - youtube.com
```

#### upstream_dns:
- Value:
  - ip address

#### blacklist_response:
- Value (one of):
  - not_found
  - refused
  - resolve

| If value if resolve then also need to specify resolve_ip

#### blacklist:
- Value:
  - array of addresses

## Build

```bash
chmod +x build.sh
./build.sh <debug or release> # debug by default

# Run
./build/<debug or release>/bin/dnsproxy
```



## Third-party Libraries
- [uthash](https://troydhanson.github.io/uthash/) (BSD license), included in `third_party/uthash/`.

No additional installation is needed.

- [libyaml](https://github.com/yaml/libyaml) (MIT License).
See `LICENSES/libyaml-LICENSE.txt` for more details.