1) run scripts/win32/generate_sln.sh
2) run scripts/win32/fix_static_link.sh
3) build Lightpack.sln with MSBuild / VisualStudio
???) run translations?
4) run scripts/win32/prepare_installer.sh
5) build dist_windows/script.iss (64bit) or script32.iss (32bit) with ISCC (InnoSetup compiler)