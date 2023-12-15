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
      toxcore-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "c-toxcore";
        fetchSubmodules = true;
        rev = "adbd5b32d85d9c13800f5ece17c0a9dce99faacd"; # ngc_events
        hash = "sha256-imXy3++MKMDgftfJn5TdocxsrXewDSHVgLT6X7WtVo4=";
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
        rev = "eeab3109e76907a0e3e8c526956ff0fa9b3bd3a2";
        hash = "sha256-knW8S4VK/U3xAWFyczSNXwx2ZA9hq2XSyr39Xh2Nsgs=";
      };
      solanaceae_toxcore-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_toxcore";
        rev = "d05875f489577e9a2c26234810058b41c3236cf7";
        hash = "sha256-uQVEkTn9ww/LVhPOqrq/iKIFSaDC6/BNrYGNUg+LrzA=";
      };
      solanaceae_tox-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_tox";
        rev = "89e74b35f83d888f8aa2e5230811b8a5e2b101a7";
        hash = "sha256-PQw2290ahYfU13tHGzBttwrvZBXK+wKh6UF4xfUaRWQ=";
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
