# Khronos platform headers

Vendored MIT-licensed headers used by DPF's MSVC OpenGL build. Avoids unchecked
configure-time downloads from redirecting registry endpoints.

- `GL/glext.h`: KhronosGroup/OpenGL-Registry,
  commit `6af574a14089ccfee87efe230ebcdd8742859813`, `api/GL/glext.h`.
- `KHR/khrplatform.h`: KhronosGroup/EGL-Registry,
  commit `8e56e86e65687697ef54114cdb1afabdfac756bf`, `api/KHR/khrplatform.h`.

Original copyright/SPDX declarations and license terms remain in each header.
