{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell.override { stdenv = pkgs.clangStdenv; }
{
  nativeBuildInputs = with pkgs; [
    clang
    clang-tools
  ];
  buildInputs = with pkgs; [
    glibc
    glibc.static
    llvm
  ];
}
