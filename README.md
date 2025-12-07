# PortOps

Portable operator library (C99) providing a stable ABI for basic numerical operators across platforms (ARM/ARM NEON, x86/AVX, RISC-V, HiFi5, etc.).

This repository contains a small portable operator library skeleton (C99) that:

- Exposes a stable, C-compatible ABI via a function table (ops_table_t).
- Ships with reference (portable) implementations for:
  - elementwise multiply / divide (float32)
  - elementwise natural log (float32)
  - biquad IIR (Direct Form II, float32)
  - radix-2 iterative FFT (in-place, float32 complex)
- Provides an initialization hook so platform-specific optimized implementations (NEON/AVX/RISC-V/HIFI5) can register and replace the generic implementations at runtime or at build time.

Build:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./examples/ops_demo
```

License: MIT
