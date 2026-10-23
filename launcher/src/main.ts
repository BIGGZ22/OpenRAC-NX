import "@fontsource-variable/inter";
import "@fontsource-variable/orbitron";
import "@fontsource/audiowide";
import "@fontsource/chakra-petch/600.css";
import { mount } from "svelte";
import "./app.css";
import "./layouts/compact.css";
import App from "./App.svelte";
import { onAndroid } from "./lib/api";
import { app } from "./lib/app.svelte";
import { startPad } from "./lib/pad";
import { applyPalette, storedPalette } from "./lib/palettes";
import { startRipples } from "./lib/ripple";

const target = document.getElementById("app");
if (!target) throw new Error("index.html has no #app element");

// Android: its own layout and margins (app.css, Header.svelte), its panels without a blurred
// backdrop (the WebView redraws one over moving content with a visible flicker), and a ripple
// under the finger on each control.
if (onAndroid) {
  document.documentElement.dataset.platform = "android";
  startRipples();
}

// The theme of the last start, so the first paint has it (App.svelte follows the settings).
applyPalette(storedPalette());

// A controller drives the whole launcher; B and Back return to the games from any other page.
startPad(() => {
  if (app.page === "library") return false;
  app.page = "library";
  return true;
}, !onAndroid);

export default mount(App, { target });
