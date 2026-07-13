# swaybg

swaybg is a wallpaper utility for Wayland compositors. It is compatible with
any Wayland compositor which implements the wlr-layer-shell protocol and
`wl_output` version 4.

See the man page, `swaybg(1)`, for instructions on using swaybg.

## Changing the image at runtime

Start swaybg. It listens on `$XDG_RUNTIME_DIR/swaybg.sock` by default:

    swaybg --image /path/to/image.jpg

With no appearance options, swaybg starts with a black background so it can be
controlled entirely through `swaybgctl`:

    swaybg

Use `--socket <path>` to listen elsewhere or `--no-socket` to disable IPC.

Change the image immediately without caching it:

    swaybgctl /path/to/new-image.jpg

Cache decoded images under reusable IDs:

    swaybgctl cache forest /path/to/forest.jpg
    swaybgctl cache city /path/to/city.jpg

Switch images without decoding them again:

    swaybgctl show forest
    swaybgctl next
    swaybgctl prev

Inspect or release cached images:

    swaybgctl status
    swaybgctl drop city
    swaybgctl clear

`show` applies the image to all configured outputs without recreating their
Wayland surfaces. Each output keeps its current scaling mode. A color-only
output switches to `stretch` mode when it receives an image. Dropping or
clearing the active cache entry leaves the committed wallpaper visible; swaybg
reloads its path if an output later needs to be redrawn.

`next` and `prev` follow cache insertion order and wrap at either end. If an
uncached image is active, they start at the first or last cached image.

`swaybgctl` connects to the same default socket. Use `--socket <path>` to
connect to a swaybg instance listening elsewhere.

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
