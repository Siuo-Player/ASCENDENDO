# ASCENDENDO — Third-Party Notices

This repository contains project-owned source together with third-party code and creative assets. The root `LICENSE` applies only to ASCENDENDO-owned code; it does not relicense third-party material.

## Vendored code

| Component | Version / source | License | Local material |
|---|---|---|---|
| doctest | 2.5.0 | MIT | `external/doctest/doctest.h` |
| stb | upstream single-header distribution | MIT or Public Domain (attribution preserved by upstream terms) | `external/stb/stb_image.h`, `external/stb/stb_truetype.h` |
| GLFW | 3.4 | GLFW license (zlib-style permissive license) | `external/glfw/` |

The local vendored files remain third-party material and must retain their upstream notices. See the upstream license texts for the authoritative terms:

- doctest: https://github.com/doctest/doctest/blob/master/LICENSE.txt
- stb: https://github.com/nothings/stb/blob/master/LICENSE
- GLFW: https://github.com/glfw/glfw/blob/master/LICENSE.md

## Font asset

`Game/Assets/Fonts/UIFont.ttf` is the Orbitron font and is distributed under the SIL Open Font License 1.1.

Copyright notice from the Orbitron distribution:

> Copyright 2018 The Orbitron Project Authors (https://github.com/theleagueof/orbitron), with Reserved Font Name: "Orbitron"

Authoritative license text:

- https://github.com/google/fonts/blob/main/ofl/orbitron/OFL.txt

The Orbitron font must not be treated as MIT-licensed project code.

## Project code

ASCENDENDO-owned source code is licensed under the MIT License in the repository root. That license does not change the license of third-party code, fonts, images, or other external material.

## Provenance boundary

This file records the current known third-party material and attribution boundaries. It is not a claim that the repository contains no other third-party material. New vendored material must be reviewed for source, version, license, attribution requirements, and compatibility before distribution.
