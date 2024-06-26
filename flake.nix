{
  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  outputs = { flake-utils, nixpkgs, ... }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        nativeBuildInputs = with pkgs; [
          clang
          pkg-config
          check
          bats
        ];
      in
      {
        devShells = {
          default = pkgs.mkShell {
            buildInputs = with pkgs; [
              clang-tools
            ] ++ nativeBuildInputs;
          };
        };
        packages = {
          default = pkgs.stdenv.mkDerivation {
            name = "ccgen";
            src = ./.;

            buildPhase = "make build/ccgen";
            installPhase = "mkdir -p $out/bin && mv build/ccgen $out/bin";

            nativeBuildInputs = nativeBuildInputs;
          };
        };
      }
    );
}



