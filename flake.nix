{
  description = "totato flake";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/release-23.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs { inherit system; };
      toxcore-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky";
        repo = "c-toxcore";
        fetchSubmodules = true;
        rev = "d4b06edc2a35bad51b0f0950d74f61c8c70630ab"; # ngc_events
        hash = "sha256-P7wTojRQRtvTx+h9+QcFdO381hniWWpAy5Yv63KWWZA=";
      };
    in {
      packages.default = pkgs.stdenv.mkDerivation {
        pname = "totato";
        version = "0.0.0-dev";

        src = ./.;

        nativeBuildInputs = with pkgs; [
          cmake
          ninja
          pkg-config
          git # cmake FetchContent
        ];

        cmakeFlags = [
          #"-DFETCHCONTENT_SOURCE_DIR_TOXCORE=${pkgs.libtoxcore.src}"
          "-DFETCHCONTENT_SOURCE_DIR_TOXCORE=${toxcore-src}"
        ];

        buildInputs = with pkgs; [
          #(libsodium.override { stdenv = pkgs.pkgsStatic.stdenv; })
          #pkgsStatic.libsodium
          libsodium
        ];

        # TODO: replace with install command
        installPhase = ''
          mkdir -p $out/bin
          mv bin/totato $out/bin
        '';
      };

      #apps.default = {
        #type = "app";
        #program = "${self.packages.${system}.default}/bin/tomato";
      #};
    }
  );
}
