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
