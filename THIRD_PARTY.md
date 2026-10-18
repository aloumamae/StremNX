# Third-party software

StremNX is built with the following libraries, installed from the devkitPro package repositories and linked
statically into `stremnx.nro`. Each one is the work of its respective authors and is used under its own license.

| Library | Used for | License |
|---|---|---|
| [libnx](https://github.com/switchbrew/libnx) | Nintendo Switch system services | ISC |
| [mpv](https://mpv.io) (libmpv) | Video playback | GPL-2.0-or-later / LGPL-2.1-or-later (depends on the build) |
| [FFmpeg](https://ffmpeg.org) | Audio/video decoding (used by mpv) | LGPL-2.1-or-later / GPL-2.0-or-later (depends on the build) |
| [libass](https://github.com/libass/libass) | Subtitle rendering (used by mpv) | ISC |
| [libplacebo](https://code.videolan.org/videolan/libplacebo) | Video rendering (used by mpv) | LGPL-2.1-or-later |
| [dav1d](https://code.videolan.org/videolan/dav1d) | AV1 decoding (used by FFmpeg) | BSD-2-Clause |
| [SDL2](https://libsdl.org), SDL2_ttf, SDL2_image | Graphics, fonts, images | zlib |
| [FreeType](https://freetype.org), [HarfBuzz](https://harfbuzz.github.io) | Font rendering | FreeType License / MIT |
| [libjpeg-turbo](https://libjpeg-turbo.org), [libpng](http://www.libpng.org), [libwebp](https://developers.google.com/speed/webp) | Image decoding | BSD-style / libpng / BSD-3-Clause |
| [curl](https://curl.se) | HTTP(S) requests | curl (MIT-style) |
| [mbedTLS](https://github.com/Mbed-TLS/mbedtls) | TLS (used by curl) | Apache-2.0 |
| [Mesa](https://mesa3d.org) | OpenGL implementation | MIT |
| [zlib](https://zlib.net), [bzip2](https://sourceware.org/bzip2/), [xz/liblzma](https://tukaani.org/xz/), [zstd](https://github.com/facebook/zstd), [lz4](https://github.com/lz4/lz4) | Compression | zlib / BSD-style / 0BSD / BSD / BSD |
| [FriBidi](https://github.com/fribidi/fribidi), [Expat](https://libexpat.github.io) | Text layout / XML | LGPL-2.1 / MIT |

Because GPL-licensed components are linked into the binary, StremNX as a whole is distributed under the
GNU General Public License v3.0 (see [LICENSE](LICENSE)), and its complete source code is available in this
repository.

## Not included

- **Fonts**: the Nintendo Switch system font is read from the console at runtime and is not distributed.
- **Content and addons**: StremNX ships with only the default Stremio catalog (Cinemeta) enabled. All other addons
  are installed by the user.
- **Trademarks**: "Stremio" and "Nintendo Switch" are trademarks of their respective owners. StremNX is not
  affiliated with or endorsed by them.
