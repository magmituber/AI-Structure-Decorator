# Magmituber AI Structure Decorator

A Geometry Dash editor utility for Geode. The current implementation provides a white preview/selection helper for selected blocks, slopes, and spikes. It does **not yet** perform Gemini-powered automatic decoration; a successful build only confirms that the current source compiles.

## Android build

GitHub Actions builds the **Android64** `.geode` package only (no Android32 target). Pushes to `main` start the workflow automatically. Open the repository's Actions tab, select the latest **Build Geode mod** run, and download its artifact after the run succeeds.

## Test safely

1. Install the artifact that matches your Geometry Dash and Geode versions.
2. Test in a copied level first.
3. Check the workflow result before treating the artifact as a successful build.

## Current status

This is an early editor utility. The mobile AI prompt, preview workflow, and real Gemini-backed structure decoration still need to be implemented and tested.