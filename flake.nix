{
  description = "totato flake";

  # https://youtu.be/7ZeTP_S6ZVI

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/release-24.11";
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
        rev = "v0.2.20";
        hash = "sha256-jJk3K1wDG84NtMQBmNvSDRMPYUsYIzjE0emiZ4Ugbhk=";
      };
      entt-src = pkgs.fetchFromGitHub {
        owner = "skypjack"; repo = "entt";
        rev = "v3.12.2";
        hash = "sha256-gzoea3IbmpkIZYrfTZA6YgcnDU5EKdXF5Y7Yz2Uaj4A=";
      };
      solanaceae_util-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_util";
        rev = "6cbcc9463ce3c4344e06d74d7df67175ada83b5f";
        hash = "sha256-puWLMzCWLBQEsslIJE9sGA7fqp8v93J4zixyu64TqhY=";
      };
      solanaceae_contact-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_contact";
        rev = "e2917c497c91f91f8febcd1f43e462fad8359305";
        hash = "sha256-gM9aT+lk+TmgiLyIeaGvHSpo55C+RNEnJvzFPLXrVMc=";
      };
      solanaceae_message3-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_message3";
        rev = "e55fb46027f16a1bc078f797ae9fcc7609d15659";
        hash = "sha256-BdwHBCXWLiVL26djyasQEPW1PbwKCkeXdUivkPaQD3c=";
      };
      solanaceae_plugin-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_plugin";
        rev = "54cd23433df4acedede51e932f27d16fe4f35548";
        hash = "sha256-Yy58w1PJFzIiN8kjqe7zerG9HewcdlBcN9P/YsjFCQs=";
      };
      solanaceae_toxcore-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_toxcore";
        rev = "727c341899a82c911a27a5cac6d09bb23ce06b1d";
        hash = "sha256-pI0ZKX6h/DMC9m0z4yC38kqGRP34ES12U9LcuO14fO0=";
      };
      solanaceae_tox-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_tox";
        rev = "8ad10978b96837eb7949f32ef433c5b37c2aa458";
        hash = "sha256-0eV8tf1CDG1xHxKmDHBMSP/5hMoB2qRagOj1meHcEFY=";
      };
      solanaceae_object_store-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_object_store";
        rev = "18d2888e3452074245375f329d90520ac250b595";
        hash = "sha256-vfFepPHj58c8YBSa8G8bJjD6gcCj65T0Kx5aEhqsjok=";
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
