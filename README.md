# StringMap for Godot

This is a class for mapping a multiline string with masks for:
- strings
- brackets
- comments (currently "#" is hardcoded)

This is a port of a GDScript class I use all of the time for script processing. The C++ is much faster, especially as the
string grows to 1k+ lines. 

## Install

This plugin is compiled GDExtesion, download the latest package from releases.

You can also use [gdaddon](https://github.com/brohd11/gdaddon) to manage the
addon. It is a TUI package/repo manager that can install and update addons for
you.
`curl -fsSL https://raw.githubusercontent.com/brohd11/gdaddon/main/install.sh | sh`

Then, you can add the addon and install:
```
cd ~/your/project/
gdaddon install <owner/repo>
```
You can also install from the TUI instead of the command.
