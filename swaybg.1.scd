swaybg(1)

# NAME

swaybg - Background for Wayland

# SYNOPSIS

*swaybg* [options...]

Displays a background image on all outputs of your Wayland session.

Without an output specified, appearance options apply to all outputs.
Per-output appearance options can be set by passing _-o, --output_ followed by
these options.

# OPTIONS

*-c, --color* <[#]rrggbb>
	Set the background color.

*-h, --help*
	Show help message and quit.

*-i, --image* <path>
	Set the background image.

*-m, --mode* <mode>
	Scaling mode for images: _stretch_, _fill_, _fit_, _center_, or _tile_.
	Default is _stretch_. Use the additional mode _solid\_color_ to display
	only the background color, even if a background image is specified.

*-o, --output* <name>
	Select an output to configure. Subsequent appearance options will only
	apply to this output. The special value _\*_ selects all outputs.

*-s, --socket* <path>
	Listen on a Unix socket for runtime cache and image-change commands from
	*swaybgctl*.

*-v, --version*
	Show the version number and quit.

# RUNTIME IMAGE CHANGES

Start swaybg with an IPC socket:

```
swaybg --socket "$XDG_RUNTIME_DIR/swaybg.sock" --image /path/to/image.jpg
```

Cache and show images from another process:

```
swaybgctl "$XDG_RUNTIME_DIR/swaybg.sock" cache forest /path/to/forest.jpg
swaybgctl "$XDG_RUNTIME_DIR/swaybg.sock" show forest
```

An image path can also be displayed immediately without caching it:

```
swaybgctl "$XDG_RUNTIME_DIR/swaybg.sock" /path/to/image.jpg
```

Available commands are:

*set* <path>
	Decode and display an uncached image. Passing the path directly without the
	_set_ command is equivalent.

*cache* <id> <path>
	Decode an image and retain its Cairo surface under _id_. IDs may contain
	letters, numbers, periods, hyphens, and underscores.

*show* <id>
	Display a cached image on all configured outputs without decoding it again.

*next*
	Show the next cached image in insertion order, wrapping to the first entry.
	If no cache entry is active, show the first entry.

*prev*
	Show the previous cached image in insertion order, wrapping to the last
	entry. If no cache entry is active, show the last entry.

*drop* <id>
	Release one cached image. If it is currently displayed, the committed
	wallpaper remains visible and can be reloaded from disk when required.

*clear*
	Release all cached images without changing the committed wallpaper.

*status*
	List cached IDs, dimensions, paths, and the active cache entry.

Image changes preserve each output's scaling mode. Color-only outputs switch
to _stretch_ mode when an image is shown.

# AUTHORS

Maintained by Simon Ser <contact@emersion.fr>, who is assisted by other open
source contributors. For more information about swaybg development, see
<https://github.com/swaywm/swaybg>.
