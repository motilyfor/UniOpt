
## 构建 benchmark 可执行：

```bash
./build.sh --release --compiler clang --target benchmark
```

## Benchmark

仓库内置了一个轻量级 benchmark 可执行，直接走 `audioeffectstore` 的公开 API，用于测量和比较模块算力。

典型运行方式：

```bash
./build.sh --release --compiler clang --target benchmark
./out/linux-x64-Release-clang/benchmark --module rln --frames 48,120,240,480,960 --iterations 40000 --warmup 4000
```

分组对比单模块和模块链：

```bash
./out/linux-x64-Release-clang/benchmark --cases gain;rln;gain+rln --frames 48,240,960 --iterations 40000 --warmup 4000
```

导出适合回归比较的 CSV 文件：

```bash
./out/linux-x64-Release-clang/benchmark --cases gain;rln;gain+rln --csv benchmark_regression.csv
```

输出字段说明：

- `case_name`: 对比组名称，默认等于模块链字符串
- `module_chain`: 模块链，使用 `+` 连接
- `frames`: 每次 `apply` 的帧数
- `iterations`: 正式测量迭代次数
- `warmup`: 预热迭代次数
- `sample_rate`: 采样率
- `channels`: 声道数
- `elapsed_ms`: 总耗时（毫秒）
- `ns_per_frame`: 每帧耗时（纳秒），适合比较优化前后变化
- `samples_per_sec`: 每秒处理采样数
- `realtime_x`: 相对实时 48 kHz 播放链路的倍数
- `checksum`: 输出校验值，防止基准被编译器消除

可选参数：

- `--module gain|rln|gain,rln` - 模块链（默认: rln）
- `--cases gain;rln;gain+rln` - 一次运行多个对比组；组内模块使用 `+` 连接，组间使用 `;` 分隔
- `--frames 48,120,240,480,960` - 每次 apply 的帧大小列表（默认: 48,120,240,480,960）
- `--iterations 40000` - 测量迭代次数（默认: 40000）
- `--warmup 4000` - 预热迭代次数（默认: 4000）
- `--sample-rate 48000` - 采样率，用于 realtime_x 计算（默认: 48000）
- `--csv benchmark_regression.csv` - 把结果写入 CSV 文件，同时标准输出也会打印同样的 CSV

建议做算力优化时统一使用 `Release` 构建，并固定输入规模、迭代次数和 CPU 环境。
