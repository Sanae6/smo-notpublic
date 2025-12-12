# This is an example flake.nix for a Switch project based on devkitA64.
# It will work on any devkitPro example with a Makefile out of the box.
{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    devkitNix = {
      url = "github:bandithedoge/devkitNix";
      inputs = {
        nixpkgs.follows = "nixpkgs";
        flake-utils.follows = "flake-utils";
      };
    };
  };

  outputs =
    {
      nixpkgs,
      flake-utils,
      devkitNix,
      ...
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          inherit system;
          # devkitNix provides an overlay with the toolchains
          overlays = [ devkitNix.overlays.default ];
        };
      in
      with pkgs;
      {
        formatter = nixfmt-tree;
        devShells.default = mkShell.override { stdenv = pkgs.devkitNix.stdenvA64; } {
          buildInputs = [
            cmake
            ninja
            curl
            ncftp

            llvmPackages.bintools-unwrapped
          ];
        };
      }
    );
}
