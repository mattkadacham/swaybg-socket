# Development environment

swaybg is a C11 project built with Meson and Ninja.

## Required packages

On Debian 13, install the compiler, build tools, and required development
libraries with:

```sh
sudo apt install \
  build-essential meson ninja-build pkg-config \
  libwayland-dev wayland-protocols libcairo2-dev
```

Optional dependencies:

```sh
sudo apt install libgdk-pixbuf-2.0-dev scdoc
```

- `libgdk-pixbuf-2.0-dev` enables image formats other than PNG.
- `scdoc` generates the manual page.

## Configure and build

```sh
meson setup build
ninja -C build
meson test -C build
```

Configuring the build generates the Wayland protocol headers and
`build/compile_commands.json`. Clangd uses that compilation database to find
the correct C standard, include paths, generated headers, and feature flags.

Helix should detect `clangd` automatically when it is installed. Check the C
language setup with:

```sh
hx --health c
```
