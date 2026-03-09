{
  description = "totato flake";

  # https://youtu.be/7ZeTP_S6ZVI

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/release-25.11";
    flake-utils.url = "github:numtide/flake-utils";
    solanaceae_util-src = {
      flake = false; url = "github:Green-Sky/solanaceae_util";
    };
    solanaceae_contact-src = {
      flake = false; url = "github:Green-Sky/solanaceae_contact";
    };
    solanaceae_message3-src = {
      flake = false; url = "github:Green-Sky/solanaceae_message3";
    };
    solanaceae_plugin-src = {
      flake = false; url = "github:Green-Sky/solanaceae_plugin";
    };
    solanaceae_toxcore-src = {
      flake = false; url = "github:Green-Sky/solanaceae_toxcore";
    };
    solanaceae_tox-src = {
      flake = false; url = "github:Green-Sky/solanaceae_tox";
    };
    solanaceae_object_store-src = {
      flake = false; url = "github:Green-Sky/solanaceae_object_store";
    };
  };

  outputs = {
    self, nixpkgs, flake-utils,
    solanaceae_util-src,
    solanaceae_contact-src,
    solanaceae_message3-src,
    solanaceae_plugin-src,
    solanaceae_toxcore-src,
    solanaceae_tox-src,
    solanaceae_object_store-src
  }:
    flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs { inherit system; };

      # get deps
      # TODO: move to inputs and lock file
      toxcore-src = pkgs.fetchFromGitHub {
        owner = "TokTok"; repo = "c-toxcore";
        fetchSubmodules = true;
        rev = "v0.2.22";
        hash = "sha256-Oi0AYV252KPF6omiErCXZvQlxkWvubk0eiegc5OMQHM=";
      };
      entt-src = pkgs.fetchFromGitHub {
        owner = "skypjack"; repo = "entt";
        rev = "v3.12.2";
        hash = "sha256-gzoea3IbmpkIZYrfTZA6YgcnDU5EKdXF5Y7Yz2Uaj4A=";
      };

      pname = "totato";
      version = "0.2.0-dev";

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
        "-DFETCHCONTENT_SOURCE_DIR_SOLANACEAE_OBJECT_STORE=${solanaceae_object_store-src}"
        "-DFETCHCONTENT_SOURCE_DIR_JSON=${pkgs.nlohmann_json.src}" # we care less about version here
        "-DFETCHCONTENT_SOURCE_DIR_ZSTD=${pkgs.zstd.src}"
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
