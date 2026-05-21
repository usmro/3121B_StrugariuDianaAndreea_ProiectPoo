#pragma once

// ── TMDB Poster Integration ────────────────────────────────────────────────────
// Get a FREE API key at https://www.themoviedb.org/settings/api
// Paste the "API Key (v3 auth)" below. Leave empty to disable.
#define TMDB_API_KEY "fcb031ae142ac3fb270cec31c987f5cd"

// ── In-app Trailer (Qt WebEngine) ─────────────────────────────────────────────
// Qt WebEngine embeds YouTube directly inside the app.
// Requires Qt WebEngine to be installed (included in standard Qt installer).
//
// If you get linker errors about Qt6WebEngineWidgets:
//   → Set USE_WEBENGINE to 0  (trailer opens in browser instead)
//
// To enable: make sure Qt6WebEngineWidgets is in your vcxproj Additional Dependencies
//            and that Qt WebEngine was selected during Qt installation.
#define USE_WEBENGINE 0
