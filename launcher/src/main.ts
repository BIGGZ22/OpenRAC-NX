import "@fontsource-variable/inter";
import "@fontsource-variable/orbitron";
import "@fontsource/audiowide";
import "@fontsource/chakra-petch/600.css";
import { mount } from "svelte";
import "./app.css";
import "./layouts/compact.css";
import App from "./App.svelte";

const target = document.getElementById("app");
if (!target) throw new Error("index.html has no #app element");

export default mount(App, { target });
