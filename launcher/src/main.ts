import "@fontsource-variable/inter";
import "@fontsource-variable/orbitron";
import "@fontsource/audiowide";
import { mount } from "svelte";
import "./app.css";
import "./layouts/compact.css";
import App from "./App.svelte";
import { onAndroid } from "./lib/api";
import { app } from "./lib/app.svelte";
import { startPad } from "./lib/pad";

const target = document.getElementById("app");
if (!target) throw new Error("index.html has no #app element");

// A controller drives the whole launcher; B and Back return to the games from any other page.
startPad(() => {
  if (app.page === "library") return false;
  app.page = "library";
  return true;
}, !onAndroid);

export default mount(App, { target });
