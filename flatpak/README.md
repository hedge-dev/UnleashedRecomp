Build
```sh
flatpak-builder --force-clean --user --install-deps-from=flathub --repo=repo --install builddir io.github.hedge_dev.unleashedrecomp.json
```

Bundle
```sh
flatpak build-bundle repo io.github.hedge_dev.unleashedrecomp.flatpak io.github.hedge_dev.unleashedrecomp --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo
```

