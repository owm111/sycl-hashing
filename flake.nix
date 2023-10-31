{
  description = "Parallel hashing benchmark";

  inputs = {
    nixpkgs = {
      /* First commit with opensycl */
      url = "github:NixOS/nixpkgs/925cfb87a3353c5dec4a9435fbb8ea7b9a9b10c9";
    };
  };

  outputs = {self, nixpkgs}: {
    lib = {
      pkgsForEach = nixpkgs: cfg: systems: attrs: let
        sys2attrset = system: {
          name = system;
          value = let
            wholeCfg = cfg // {
              inherit system;
            };
            pkgs = import nixpkgs wholeCfg;
            apply = _: f: f pkgs system;
          in builtins.mapAttrs apply attrs;
        };
      in builtins.listToAttrs (map sys2attrset systems);

      pkgsForEachDefaults = self.lib.pkgsForEach nixpkgs {} [
        "x86_64-linux"
      ];
    };

    packages = self.lib.pkgsForEachDefaults {
      texlive = pkgs: sys: pkgs.texlive.combine {
        inherit (pkgs.texlive) scheme-medium;
        inherit (self.packages.${sys}) noweb;
      };
      noweb = pkgs: sys: pkgs.noweb.overrideAttrs (old: {
        version = "2.13";
        src = pkgs.fetchFromGitHub {
          owner = "nrnrnr";
          repo = "noweb";
          rev = "v2_13";
          sha256 = "COcWyrYkheRaSr2gqreRRsz9SYRTX2PSl7km+g98ljs=";
        };
        patches = [];
      });
    };

    devShells = self.lib.pkgsForEachDefaults {
      default = pkgs: sys: pkgs.mkShell {
        nativeBuildInputs = with pkgs; [
          pkgs.ghostscript_headless # for ps2pdf
          pkgs.groff
          pkgs.opensyclWithRocm
          pkgs.plan9port # for grap
          self.packages.${sys}.noweb
          self.packages.${sys}.texlive
        ];
      };
    };
  };
}
