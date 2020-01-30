Notepad2-mod-jrb - A fork of Notepad2-mod aimed mainly at providing
                   better regular expressions support

Changes compared to the official Notepad2:
* Code folding
* Support for bookmarks
* Option to mark all occurrences of a word
* Word auto-completion
* Syntax highlighting support for AutoHotkey, AutoIt3, AviSynth, Bash,
  CMake, CoffeeScript, Inno Setup, LaTeX, Lua, Markdown, NSIS, Ruby,
  Tcl, YAML and VHDL scripts
* Improved support for NFO ANSI art
* Other various minor changes and tweaks

Changes compared to the Notepad2-mod fork:
* Switch to the Onigmo Regex Engine (https://github.com/k-takata/Onigmo)
* File-change notifications received while in the background are deferred
  until window is shown again
* More keywords being recognized for PowerShell syntax highlighting

Supported Operating Systems:
* Windows 7, 8, 8.1 and 10, for both 32-bit and 64-bit
* Support for Windows Vista has not been tested
* Windows XP is *not supported*

License:
* A 3-Clause BSD-like license; details in the "License.txt" file.

Notes:
* If you find any bugs or have any suggestions for the implemented
  lexers (and not only) feel free to provide patches/pull requests.
  Without patches or pull requests chances are that nothing will be
  fixed/implemented.
* No new features are expected to be added. You prefer new toys? Then
  have a look at Notepad3 (https://github.com/rizonesoft/Notepad3) or
  consider switching to a modern code editor like Atom (https://atom.io)
  or Visual Studio Code (https://code.visualstudio.com).
* This fork is not expected to be localized either.

All contributors to the Notepad2 fork:
* Kai Liu (Notepad2-mod main contributor)
* RL Vision (for the Bookmarks feature)
* Aleksandar Lekov
* Bruno Barbieri
* Everyone else on GitHub; discover them at
  https://github.com/XhmikosR/notepad2-mod/graphs/contributors

Contributors to this Notepad2-mod-jrb fork:
* Rainer Kottenhoff (https://github.com/RaiKoHoff), Rizonesoft
  (https://www.rizonesoft.com) and the Notepad3 project for the Onigmo
  integration and bindings
* Jonathan R.-Brochu (https://github.com/johnwait) for the fork itself

More information:
* Official Notepad3 website:   https://www.rizonesoft.com/downloads/notepad3/
* Notepad3 repository:         https://github.com/rizonesoft/Notepad3/
* Onigmo repository:           https://github.com/k-takata/Onigmo/
* Official Notepad2-mod site:  https://xhmikosr.github.io/notepad2-mod/
* Notepad2-mod repository:     https://github.com/XhmikosR/notepad2-mod/
* Code folding usage guide:    https://github.com/XhmikosR/notepad2-mod/wiki/Code-Folding-Usage
* Official Notepad2 website:   http://www.flos-freeware.ch/notepad2.html

Keyboard shortcuts changed for Notepad2-mod:
* [Ctrl]+[Alt]+[F2]            Expand selection to next match.
* [Ctrl]+[Alt]+[Shift]+[F2]    Expand selection to previous match.
* [Ctrl]+[Shift]+[Enter]       New line with toggled auto indent option.

Keyboard shortcuts changed for Notepad2-mod-jrb:
* [Ctrl]+[Shift]+[F12]         Customize syntax schemes.

Build notes:
* It's recommended you use MSVC 2017 or later to build Notepad2-mod-jrb.