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
        rev = "v0.2.19";
        hash = "sha256-6pdyQpHwYwUBeeHVuH/YTYdlOAtRgvrSuFE6Wlh+BnM=";
      };
      entt-src = pkgs.fetchFromGitHub {
        owner = "skypjack"; repo = "entt";
        rev = "v3.12.2";
        hash = "sha256-gzoea3IbmpkIZYrfTZA6YgcnDU5EKdXF5Y7Yz2Uaj4A=";
      };
      solanaceae_util-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_util";
        rev = "17b4cce69b1e1712239700b4b13b242e61ca7d62";
        hash = "sha256-QNLZgByoZsrXtAWk1i6O1bUkheWop/Oi/V9ggANFcgE=";
      };
      solanaceae_contact-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_contact";
        rev = "e40271670b4df96a8d02f32a1ba61a838419db48";
        hash = "sha256-a/G1hpkZlnRF0Reg3PgJuToZclukImVDff3CpzSGz4k=";
      };
      solanaceae_message3-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_message3";
        rev = "7c28b232a46ebede9d6f09bc6eafb49bacfa99ea";
        hash = "sha256-Ufb8qs/Tz2nHmUOMt8gsNDjf3+8YyoT7sTwYnsFQkC0=";
      };
      solanaceae_plugin-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_plugin";
        rev = "82cfb6d4920a2d6eb19e3f3560b20ec281a5fa81";
        hash = "sha256-3Bx9FHHze7MDxvHBEd3Ya/m05X5qMugoV4MUs1vtcZs=";
      };
      solanaceae_toxcore-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_toxcore";
        rev = "cf3679018be3f90db0f2f1e9433a966692976421";
        hash = "sha256-XnsQR6TYhZWNSVjxe4wJdSD2WVdGM8ZHGmzrDlDyJC4=";
      };
      solanaceae_tox-src = pkgs.fetchFromGitHub {
        owner = "Green-Sky"; repo = "solanaceae_tox";
        rev = "ce81ef7cf7cea2fe2091912c9eafe787cbba6100";
        hash = "sha256-2NwD5Dv6almBRNb8FTuzmiGDWFNZdPPIwJ1etWETpLE=";
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
