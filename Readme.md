# Notepad2-jrb

> *A fork of Notepad2-mod aimed mainly at providing better regular expressions support*

[//]: # "[![Build status](https://img.shields.io/appveyor/ci/XhmikosR/notepad2-mod/master.svg)](https://ci.appveyor.com/project/XhmikosR/notepad2-mod/branch/master)"
[//]: # "[![Coverity Scan Build Status](https://img.shields.io/coverity/scan/1113.svg)](https://scan.coverity.com/projects/1113)"

## Changes compared to the official Notepad2:

* Code folding
* Support for bookmarks
* Option to mark all occurrences of a word
* Word auto-completion
* Syntax highlighting support for AutoHotkey, AutoIt3, AviSynth, Bash, CMake, CoffeeScript,
  Inno Setup, LaTeX, Lua, Markdown, NSIS, Ruby, Tcl, YAML and VHDL scripts
* Improved support for NFO ANSI art
* Other various minor changes and tweaks

## Changes compared to the Notepad2-mod fork:

* Switch to the [Onigmo Regex Engine](https://github.com/k-takata/Onigmo)
* More keywords being recognized for PowerShell syntax highlighting

## Supported Operating Systems:

* Windows 7, 8, 8.1 and 10, for both 32-bit and 64-bit
* Support for Windows Vista has not been tested
* Windows XP is **_not supported_**

## License:

* A [3-Clause BSD](https://opensource.org/licenses/BSD-3-Clause)-like license; details in the `License.txt` file.

## [Notepad2-mod Screenshots](https://xhmikosr.github.io/notepad2-mod/screenshots)

## Notes:

* If you find any bugs or have any suggestions for the implemented lexers (and **not** only)
  feel free to **provide patches/pull requests**. Without patches or pull requests chances are
  that nothing will be fixed/implemented.
* No new features are expected to be added. You prefer new toys? Then have a look at
  **[Notepad3](https://github.com/rizonesoft/Notepad3)** or consider switching to a modern
  code editor like **[Atom](https://atom.io)** or **[Visual Studio Code](https://code.visualstudio.com)**.
* This fork is not expected to be **localized** either.

## Contributors to the Notepad2 fork:

* [Kai Liu](http://code.kliu.org/misc/notepad2/) (Notepad2-mod main contributor)
* [RL Vision](http://www.rlvision.com/notepad2/about.asp) (for the Bookmarks feature)
* Aleksandar Lekov
* Bruno Barbieri
* Everyone else on [GitHub](https://github.com/XhmikosR/notepad2-mod/graphs/contributors)

## More information:

* [Official Notepad2-mod website](https://xhmikosr.github.io/notepad2-mod/)
* [Notepad2-mod repository (archived)](https://github.com/XhmikosR/notepad2-mod/)
* [Code folding usage guide](https://github.com/XhmikosR/notepad2-mod/wiki/Code-Folding-Usage)
* [Official Notepad2 website](http://www.flos-freeware.ch/notepad2.html)

## Changed keyboard shortcuts compared to Notepad2:

* <kbd>Ctrl+Alt+F2</kbd>       Expand selection to next match.
* <kbd>Ctrl+Alt+Shift+F2</kbd> Expand selection to previous match.
* <kbd>Ctrl+Shift+Enter</kbd>  New line with toggled auto indent option.

## Changed keyboard shortcuts compared to the Notepad2-mod fork:

* <kbd>Ctrl+Shift+F12</kbd>    Customize syntax schemes. *(previously was <kbd>Ctrl+F12</kbd>)*

It's recommended you use [MSVC 2017 or later](https://visualstudio.microsoft.com/vs/whatsnew) to build Notepad2-jrb.
