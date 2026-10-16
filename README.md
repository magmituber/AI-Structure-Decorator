# AI Structure Decorator

A Geometry Dash Geode mod that uses Gemini to generate decorative object placements around a selected structure in the level editor.

## Intended workflow
1. Install the mod in Geode.
2. Open the mod settings and enter your own Gemini API key.
3. Set the decoration style and maximum object count.
4. In the level editor, select the structure or group you want to decorate.
5. Press the AI button and review the generated decoration before saving.

## Build
GitHub Actions builds Android32 and Android64 .geode packages on pushes to main and can also be run manually. Open the repository's Actions tab, select Build Geode mod, and open a successful run to download its artifacts.

## API key safety
The API key is stored in local Geode mod settings. Never commit your key into this repository or share it in screenshots. Gemini requests are sent from the game to Google's Generative Language API.

## Current limitations
- AI placements are constrained to the selected object's bounding box and a limited object-ID range.
- This initial implementation must pass GitHub Actions compilation and in-game testing before it is considered stable.
- Inspect generated objects and use undo before saving if needed.

## References
- https://docs.geode-sdk.org/
- https://docs.geode-sdk.org/getting-started/create-mod/
- https://github.com/geode-sdk/build-geode-mod
