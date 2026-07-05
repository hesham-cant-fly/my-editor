{
  description = "my-editor - A text editor written in C";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "my-editor";
          version = "0.1.0";
          src = ./.;
          buildPhase = ''
            make
          '';
          installPhase = ''
            mkdir -p $out/bin
            cp my-editor $out/bin/
          '';
          meta = {
            description = "A text editor wirtten in C";
            license = pkgs.lib.licenses.mit;
            platforms = pkgs.lib.platforms.linux;
          };
        };
        devShells.default = import ./shell.nix { inherit pkgs; };
      }
    );
}
