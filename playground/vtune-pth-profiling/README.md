# Performance profiling

## Installation

Let's use docker containers:
```bash
docker run -it \
    --device=/dev/dri \
    --name=vtune-interpolate \
    -v $PWD:/interpolate-tensoriterator \
    -v $PWD/../:/workspace \
    -w /interpolate-tensoriterator \
    -v /home/user/Documents/ml/pytorch/:/pytorch \
    --network=host --security-opt seccomp:unconfined --shm-size 16G \
    intel/oneapi-basekit
```

### Standalone tool download

```bash
https://registrationcenter-download.intel.com/akdlm/irc_nas/17527/l_oneapi_vtune_p_2021.1.2.150_offline.sh
```

## Run Hotspots profiling




## Refs:

- https://gist.github.com/mingfeima/e08310d7e7bb9ae2a693adecf2d8a916
- https://github.com/intel/oneapi-containers