---
title: Environment Setup
kind: chapter
part: I
number: 2
order: 3
---

I would like to share my development environment setup before we started the journey.
I use `nix-shell` along with `direnv` to setup the development environment.

For nix, please checkout the [Nix Pills](https://nixos.org/guides/nix-pills/00-preface.html)
for more details.

Install the `direnv` package. In the project root, create `shell.nix`, this would define
a nix-shell:

```nix
{ pkgs ? import <nixpkgs> {} }:

let
  llvmPkgs = pkgs.llvmPackages_latest;
in
pkgs.mkShell {
  buildInputs = [
    llvmPkgs.llvm.dev
    llvmPkgs.clang
    llvmPkgs.bintools

    pkgs.cmake
    pkgs.pkg-config
    pkgs.flex
    pkgs.zlib
    pkgs.libffi
  ];

  # ponytail: nix zlib paths so cmake's find_package resolves ZLIB::ZLIB for LLVM
  shellHook = ''
    export ZLIB_ROOT="${pkgs.zlib.dev}"
    export LIBCLANG_PATH="${llvmPkgs.libclang.lib}/lib"
    echo "Entering C++ development environment"
  '';
}
```

It would install the clang toolchain, llvm headers and libraries, `flex`, `zlib`, and `libffi` libraries,
and export the environment variables for discovery.


Then in the `.envrc`, simply add

```
use nix
```

this would instruct `direnv` to load nix-shell defined in `shell.nix`.

Enable direnv as `direnv allow .`, the nix-shell will populate the dev environment.
Once you change the directory, this environment is unloaded automatically. Quite neat.
