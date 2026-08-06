{
       inputs={
                nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
                flake-parts.url = "github:hercules-ci/flake-parts";
		}; 

        outputs=inputs@{self,nixpkgs,flake-parts,...}:
        flake-parts.lib.mkFlake {inherit inputs;} {
                
                systems=["x86_64-linux" "x86_64-darwin" "aarch64-darwin" "aarch64-linux"];
                perSystem= {pkgs, system, ...}:{
					devShells.default=pkgs.mkShell {
						buildInputs=[
							pkgs.libgcc
							pkgs.clang
						];
					};
					packages.default=pkgs.stdenv.mkDerivation {
						name="cppmake";
						version="1.0";
						src=./.;
						buildInputs=[
							pkgs.clang
						];
						buildPhase=''
							clang++ compile.cpp -o compile
							clang++ install.cpp -o install
							./compile
							echo "HELLO FROM BUILDER"
						'';
						installPhase=''
							./install $out
					'';
					};
				};
        };
}
