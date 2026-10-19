# White Structure Preview

A Geometry Dash editor mod for Geode that displays a white outline preview over selected blocks, slopes, and spikes.

## Use
1. Install the matching Android build through Geode.
2. Open a level in the editor and select one or more objects.
3. Tap **PREVIEW** to show the outline.
4. Tap **CONFIRM** to clear the preview. This does not alter or duplicate the original objects.
5. Drag the button group to reposition it.

## Build
GitHub Actions builds only the Android64 `.geode` package on pushes to `main`, using the Geode SDK version specified in `mod.json` to keep the SDK and Geometry Dash bindings compatible. Open the Actions tab and download the artifact from a successful run.

## Status
This is an early version. A successful build verifies compilation, not all in-game behavior. Test in a copied level before relying on it.
