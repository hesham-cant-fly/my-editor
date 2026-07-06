{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell.override { stdenv = pkgs.clangStdenv; }
{
  nativeBuildInputs = with pkgs; [];
  packages = with pkgs; [
    clang-tools
    pkg-config
    gf
  ];

  buildInputs = with pkgs; [
    glibc
    glibc.static
    ncurses
    llvm
  ];

  shellHook = ''
    export CLINK_FLAGS=$(pkg-config --cflags --libs ncurses)
  '';
}
