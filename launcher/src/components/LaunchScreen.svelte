<script lang="ts">
  // Android: a game starting. Its disc spins on the whole screen, over its picture, from the press
  // of Play until the game's window covers the launcher (app.launching, showLaunch). Only
  // transform and opacity move, so it stays smooth while the game's process starts beside it.
  import { app } from "$lib/app.svelte";
  import { gameArt, hideMissing } from "$lib/media";
  import { enter, reduced } from "$lib/motion";
  import { themeStyle } from "$lib/themes";
  import { fade, fly } from "svelte/transition";

  const launch = $derived(app.launching);
  const art = $derived(launch ? gameArt(launch.game) : null);
</script>

{#if launch}
  <div
    class="launch"
    role="status"
    aria-live="polite"
    style={themeStyle(launch.game)}
    transition:fade={{ duration: reduced ? 0 : 220 }}
  >
    {#if art}<img class="art" src={art} alt="" decoding="async" onerror={hideMissing} />{/if}
    <div class="shade"></div>
    <div class="middle" in:fly|global={enter(60)}>
      <div class="disc" aria-hidden="true">
        <div class="spin"><span class="shine"></span></div>
      </div>
      <h1>{launch.title}</h1>
      <p>Starting<span class="dots"><i>.</i><i>.</i><i>.</i></span></p>
    </div>
  </div>
{/if}

<style>
  .launch {
    position: fixed;
    inset: 0;
    z-index: 60;
    display: grid;
    place-items: center;
    overflow: hidden;
    background: linear-gradient(160deg, var(--from), var(--to));
  }

  .art {
    position: absolute;
    inset: 0;
    width: 100%;
    height: 100%;
    object-fit: cover;
    filter: blur(14px) saturate(1.1);
    transform: scale(1.12);
    animation: drift 6s var(--ease) both;
  }

  .shade {
    position: absolute;
    inset: 0;
    background: radial-gradient(
      circle at 50% 42%,
      color-mix(in srgb, var(--ink-deep) 35%, transparent),
      color-mix(in srgb, var(--ink-deep) 88%, transparent) 70%
    );
  }

  .middle {
    position: relative;
    display: grid;
    justify-items: center;
    gap: 14px;
    text-align: center;
    padding: 0 24px;
  }

  /* The disc: a DVD's silver with its rainbow, the hub and the hole, spinning up to speed. */
  .disc {
    width: min(42vh, 260px);
    aspect-ratio: 1;
    border-radius: 50%;
    filter: drop-shadow(0 18px 40px rgb(0 0 0 / 0.55));
    animation: arrive 0.6s var(--ease) both;
  }

  .spin {
    position: relative;
    width: 100%;
    height: 100%;
    border-radius: 50%;
    background:
      radial-gradient(circle, transparent 0 9%, rgb(255 255 255 / 0.55) 9.5% 10.5%, transparent 11%),
      radial-gradient(circle, transparent 0 17%, rgb(210 214 222 / 0.9) 17.5% 30%, transparent 30.5%),
      conic-gradient(
        from 20deg,
        rgb(201 204 214),
        rgb(233 182 255),
        rgb(143 214 255),
        rgb(183 255 212),
        rgb(255 241 168),
        rgb(255 179 168),
        rgb(201 204 214),
        rgb(159 178 255),
        rgb(183 255 212),
        rgb(201 204 214)
      );
    -webkit-mask: radial-gradient(circle, transparent 0 9%, black 9.5%);
    mask: radial-gradient(circle, transparent 0 9%, black 9.5%);
    /* A turn speeding up, then turns at that speed. */
    animation:
      spin 1.2s cubic-bezier(0.4, 0, 1, 1) both,
      spin 0.9s linear 1.2s infinite;
    will-change: transform;
  }

  /* A light across the disc's face, which stays put while the disc turns under it. */
  .shine {
    position: absolute;
    inset: 0;
    border-radius: 50%;
    background: conic-gradient(
      from -30deg,
      transparent 0 10%,
      rgb(255 255 255 / 0.55) 14%,
      transparent 20% 60%,
      rgb(255 255 255 / 0.35) 64%,
      transparent 70%
    );
    animation:
      counter 1.2s cubic-bezier(0.4, 0, 1, 1) both,
      counter 0.9s linear 1.2s infinite;
  }

  h1 {
    margin: 10px 0 0;
    font-family: var(--title-font);
    font-weight: var(--title-weight);
    font-size: 1.5rem;
    color: var(--text);
    text-wrap: balance;
  }

  p {
    margin: 0;
    color: var(--soft);
    font-weight: 500;
  }

  .dots i {
    font-style: normal;
    animation: blink 1.2s infinite both;
  }

  .dots i:nth-child(2) {
    animation-delay: 0.2s;
  }

  .dots i:nth-child(3) {
    animation-delay: 0.4s;
  }

  @keyframes arrive {
    from {
      opacity: 0;
      transform: scale(0.82) translateY(12px);
    }
  }

  @keyframes spin {
    to {
      transform: rotate(360deg);
    }
  }

  @keyframes counter {
    to {
      transform: rotate(-360deg);
    }
  }

  @keyframes drift {
    from {
      transform: scale(1.2);
      opacity: 0;
    }
  }

  @keyframes blink {
    0%,
    100% {
      opacity: 0.2;
    }
    40% {
      opacity: 1;
    }
  }
</style>
