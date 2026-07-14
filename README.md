# swaybg

swaybg is a wallpaper utility for Wayland compositors. It is compatible with
any Wayland compositor which implements the wlr-layer-shell protocol and
`wl_output` version 4.

See the man page, `bgsock(1)`, for instructions on using this fork.

## Changing the image at runtime

Start bgsock. It listens on `$XDG_RUNTIME_DIR/swaybg.sock` by default:

    bgsock --image /path/to/image.jpg

With no appearance options, bgsock starts with a black background so it can be
controlled entirely through `bgctl`:

    bgsock

Use `--socket <path>` to listen elsewhere or `--no-socket` to disable IPC.

Change the image immediately without caching it:

    bgctl /path/to/new-image.jpg
    bgctl --mode fill /path/to/new-image.jpg

Cache decoded images under reusable IDs:

    bgctl --mode fill cache forest /path/to/forest.jpg
    bgctl --mode fit cache city /path/to/city.jpg

Switch images without decoding them again:

    bgctl show forest
    bgctl next
    bgctl prev

Inspect or release cached images:

    bgctl status
    bgctl drop city
    bgctl clear

`--mode` accepts the same `stretch`, `fill`, `fit`, `center`, and `tile` values
as bgsock. An uncached image uses the supplied mode immediately. A cached image
remembers its mode, and `show`, `next`, and `prev` apply it whenever that entry
is selected. If no mode is supplied, each output keeps its current scaling
mode; a color-only output switches to `stretch` when it receives an image.

The cache retains the decoded image, so selecting it does not read or decode
the file again. Scaling and cropping are still calculated when it is shown
because those depend on each output's current dimensions and scale. Dropping
or clearing the active cache entry leaves the committed wallpaper visible;
bgsock reloads its path if an output later needs to be redrawn.

`next` and `prev` follow cache insertion order and wrap at either end. If an
uncached image is active, they start at the first or last cached image.

`bgctl` connects to the same default socket. Use `--socket <path>` to connect
to a bgsock instance listening elsewhere.

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
