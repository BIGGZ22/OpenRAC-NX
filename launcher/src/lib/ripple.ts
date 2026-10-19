// Android's touch feedback: a press on a control sends a wave out from the finger, clipped to the
// control's shape (app.css, .ripples and .ripple), as the buttons of Android's own apps do.

const CONTROLS = 'button, .button, [role="menuitem"], label.option, label.setting';

function press(event: PointerEvent) {
  const target = event.target instanceof Element ? event.target.closest<HTMLElement>(CONTROLS) : null;
  if (!target || (target as HTMLButtonElement).disabled) return;
  const box = target.getBoundingClientRect();
  // Wide enough to cover the control from any point inside it.
  const size = 2 * Math.hypot(box.width, box.height);
  if (getComputedStyle(target).position === "static") target.style.position = "relative";
  const clip = document.createElement("span");
  clip.className = "ripples";
  clip.setAttribute("aria-hidden", "true");
  const wave = document.createElement("span");
  wave.className = "ripple";
  wave.style.left = `${event.clientX - box.left}px`;
  wave.style.top = `${event.clientY - box.top}px`;
  wave.style.width = wave.style.height = `${size}px`;
  clip.append(wave);
  target.append(clip);
  const done = () => {
    clip.remove();
  };
  wave.addEventListener("animationend", done);
  // Without animations (reduced motion) there is no end to wait for.
  setTimeout(done, 800);
}

export function startRipples() {
  document.addEventListener("pointerdown", press, { passive: true });
}
