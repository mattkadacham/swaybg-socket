# swaybg

swaybg is a wallpaper utility for Wayland compositors. It is compatible with
any Wayland compositor which implements the wlr-layer-shell protocol and
`wl_output` version 4.

See the man page, `swaybg(1)`, for instructions on using swaybg.

## Changing the image at runtime

Start swaybg with a Unix socket:

    swaybg --socket "$XDG_RUNTIME_DIR/swaybg.sock" --image /path/to/image.jpg

Then replace the image without recreating the Wayland surfaces:

    swaybgctl "$XDG_RUNTIME_DIR/swaybg.sock" /path/to/new-image.jpg

The new image is applied to all configured outputs. Each output keeps its
current scaling mode. A color-only output switches to `stretch` mode when it
receives an image.

## Release Signatures

Releases are signed with [E88F5E48](https://keys.openpgp.org/search?q=34FF9526CFEF0E97A340E2E40FDE7BE0E88F5E48)
and published [on GitHub](https://github.com/swaywm/swaybg/releases). swaybg
releases are managed independently of sway releases.

## Installation

### From Packages

swaybg is available in many distributions. Try installing the "swaybg"
package for yours.

### Compiling from Source

Install dependencies:

* meson \*
* wayland
* wayland-protocols \*
* cairo
* gdk-pixbuf2 (optional: image formats other than PNG)
* [scdoc](https://git.sr.ht/~sircmpwn/scdoc) (optional: man pages) \*
* git (optional: version information) \*

_\* Compile-time dep_

Run these commands:

    meson build/
    ninja -C build/
    sudo ninja -C build/ install
