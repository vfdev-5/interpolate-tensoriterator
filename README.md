# Prototype Torch Interpolate with TensorIterator

FMassa's code : https://github.com/fmassa/vision-1/commit/407e0430e14ca688b2fb6f03ec1122ba46527553

## Goals

- ND downsampling/upsampling with TensorIterator, mode: linear, nearest, cubic
- Benchmark implementations vs original pytorch ones
- Improve previous algorithms

### Step 6: Back to basics (#5 by Francisco)

- [x] Test the code with older compiler like gcc 5.4
- [x] Inspect assembly code
- [ ] Specialization tricks: https://github.com/pytorch/pytorch/blob/9cec8ae146c0b95b0f5dcd1c62ea4e83ee32f90c/aten/src/ATen/native/cpu/Loops.h#L387
- [x] Explore C10_RESTRICT : https://en.wikipedia.org/wiki/Restrict
  -  https://wiki.sei.cmu.edu/confluence/display/c/EXP43-C.+Avoid+undefined+behavior+when+using+restrict-qualified+pointers


## Development

<details>

<summary>
Click here for details
</summary>


```bash
docker run --rm -it \
    --name=tv-interpolate \
    -v $PWD:/interpolate-tensoriterator \
    -v $PWD/../:/workspace \
    -w /interpolate-tensoriterator \
    -v /home/user/Documents/ml/pytorch/:/pytorch \
    --network=host --security-opt seccomp:unconfined --privileged --shm-size 16G \
    nvidia/cuda:11.1-cudnn8-devel-ubuntu20.04 \
    /bin/bash
```
```
apt-get update && ln -fs /usr/share/zoneinfo/America/New_York /etc/localtime && \
    apt-get install -y tzdata && \
    dpkg-reconfigure --frontend noninteractive tzdata && \
    apt-get install -y git cmake python3 python3-pip numactl && \
    ln -s /usr/bin/python3 /usr/bin/python && \
    ln -s /usr/bin/pip3 /usr/bin/pip && \
    pip install numpy typing_extensions
```

</details>

See [playground](playground) for details step by step.

### Step 6 - Linear interpolation

- Install GCC 7.3 as PyTorch Nightly

```bash
echo "deb http://archive.ubuntu.com/ubuntu/ bionic main\n" >> /etc/apt/sources.list
echo "deb http://archive.ubuntu.com/ubuntu/ bionic universe\n" >> /etc/apt/sources.list
apt-get update
apt-get install g++-7=7.3.0-16ubuntu3 gcc-7-base=7.3.0-16ubuntu3 gcc-7=7.3.0-16ubuntu3 cpp-7=7.3.0-16ubuntu3 libgcc-7-dev=7.3.0-16ubuntu3 libstdc++-7-dev=7.3.0-16ubuntu3 libasan4=7.3.0-16ubuntu3 libubsan0=7.3.0-16ubuntu3 libcilkrts5=7.3.0-16ubuntu3
```

- Build

```bash
cd step_six && mkdir -p build && cd $_
export CC=/usr/bin/gcc-7
export CXX=/usr/bin/g++-7
export TORCH_PATH=/tmp/libtorch
cmake -DTORCH_DIR=$TORCH_PATH ..
make
```

```bash
make && ./bench 20000
```

#### How to produce benchmarks:

Configure PR_TORCH_PATH for PR PyTorch build inside `run_python_pr_bench.sh`:
```
export PR_TORCH_PATH=/workspace/pth-linear-interp/
```

and run:
```
sh run_python_pr_bench.sh
> pr_vs_pth_results.md
```



### Step 7 - Cubic/Nearest/Linear interpolations


#### Cubic interpolation

```bash
cd step_seven/cubic && mkdir -p build && cd $_
export TORCH_PATH=/pytorch/torch
cmake -DTORCH_DIR=$TORCH_PATH ..
make
```

```bash
make && ./bench
```
