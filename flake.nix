{
  description = "totato flake";

  # https://youtu.be/7ZeTP_S6ZVI

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/release-23.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs { inherit system; };

      # get deps
      # TODO: move to inputs and lock file
      toxcore-src = pkgs.fetchFromGitHub {
        owner = "TokTok"; repo = "c-toxcore";
        fetchSubmodules = true;
        rev = "f1df709b8792da4c0e946d826b11df77d565064d"; # iphydf:ordered-events pr
        hash = "sha256-P3+Y+IBy932TtR+j25G6yzQqfDLvdLv/c+9fvF1b5LY=";
      };
      entt-src = pkgs.fetchFromGitHub {
        owner = "skypjack"; repo = "entt";
        rev = "v3.12.2";
        hash = "sha256-gzoea3IbmpkIZYrfTZA6YgcnDU5EKdXF5Y7Yz2Uaj4A=";
      };
      solanaceae_util-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_util";
        rev = "2b20c2d2a45ad1005e794c704b3fc831ca1d3830";
        hash = "sha256-eLDIFQOA3NjdjK1MPpdNJl9mYQgtJK9IV4CaOyenae8=";
      };
      solanaceae_contact-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_contact";
        rev = "2d73c7272c3c254086fa067ccfdfb4072c20f7c9";
        hash = "sha256-t6UWKMvWqXGvmsjx1JSc6fOawrzzXndaOpBntoKhLW0=";
      };
      solanaceae_message3-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_message3";
        rev = "1a036c2321e06d4c36f3e2148e67dfe6aa379296";
        hash = "sha256-vcUN7tsy6E1P5juhW7pj9OUtZLytZcpPijlIj/iTBgk=";
      };
      solanaceae_plugin-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_plugin";
        rev = "96bab0200f5b13671756abe7e3132ed78aaa2a40";
        hash = "sha256-UTOdTiZLsiFs/3SZdrxfynX9OYVlDXEeG6VFIHXIxlA=";
      };
      solanaceae_toxcore-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_toxcore";
        rev = "54084b5a53e1617ff9b0c225880b0f1d60fe65ea";
        hash = "sha256-yuOuZZAacnUUPbxL01sUx/r210E8TJQfgVZms6YROVk=";
      };
      solanaceae_tox-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_tox";
        rev = "c01d91144ce10486ff6e98a2e6e8cc5e20a5c412";
        hash = "sha256-oCDx/bK433AZ5+xx0kq/NudylTMQK+6ycddaQRse/+0=";
      };

      pname = "totato";
      version = "0.0.0-dev";

      src = ./.;

      nativeBuildInputs = with pkgs; [
        cmake
        ninja
        pkg-config
      ];

      cmakeFlags = [
        #"-DFETCHCONTENT_SOURCE_DIR_TOXCORE=${pkgs.libtoxcore.src}"
        "-DFETCHCONTENT_SOURCE_DIR_TOXCORE=${toxcore-src}"
        "-DFETCHCONTENT_SOURCE_DIR_ENTT=${entt-src}" # the version is important
        "-DFETCHCONTENT_SOURCE_DIR_SOLANACEAE_UTIL=${solanaceae_util-src}"
        "-DFETCHCONTENT_SOURCE_DIR_SOLANACEAE_CONTACT=${solanaceae_contact-src}"
        "-DFETCHCONTENT_SOURCE_DIR_SOLANACEAE_MESSAGE3=${solanaceae_message3-src}"
        "-DFETCHCONTENT_SOURCE_DIR_SOLANACEAE_PLUGIN=${solanaceae_plugin-src}"
        "-DFETCHCONTENT_SOURCE_DIR_SOLANACEAE_TOXCORE=${solanaceae_toxcore-src}"
        "-DFETCHCONTENT_SOURCE_DIR_SOLANACEAE_TOX=${solanaceae_tox-src}"
        "-DFETCHCONTENT_SOURCE_DIR_JSON=${pkgs.nlohmann_json.src}" # we care less about version here
      ];

      # TODO: replace with install command
      installPhase = ''
        mkdir -p $out/bin
        mv bin/totato $out/bin
      '';
    in {
      packages.default = pkgs.stdenv.mkDerivation {
        inherit pname version src nativeBuildInputs cmakeFlags installPhase;

        # static libsodium, because I can
        buildInputs = with pkgs.pkgsStatic; [
          libsodium
        ];
      };

      # all static, kinda useless, since dlopen() wont work to load plugins
      packages.static = pkgs.pkgsStatic.stdenv.mkDerivation {
        inherit pname version src nativeBuildInputs cmakeFlags installPhase;

        buildInputs = with pkgs.pkgsStatic; [
          libsodium
        ];
      };

      #apps.default = {
        #type = "app";
        #program = "${self.packages.${system}.default}/bin/tomato";
      #};
    }
  );
}
