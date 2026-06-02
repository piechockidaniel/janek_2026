#pragma once
#include <Arduino.h>

// Self-contained control UI - no external CDN, works on the robot's own
// network without internet access. Embeds the camera feed by URL from
// secrets.h so the brain doesn't have to proxy video.
const char WEB_UI_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>Robo control</title>
<style>
  *{box-sizing:border-box;-webkit-tap-highlight-color:transparent;user-select:none}
  body{margin:0;background:#111;color:#eee;font-family:system-ui,sans-serif;display:flex;flex-direction:column;align-items:center;padding:8px}
  h1{margin:6px 0;font-size:18px;font-weight:500}
  #stream{width:100%;max-width:480px;background:#222;border-radius:8px;aspect-ratio:4/3;object-fit:cover}
  .grid{display:grid;grid-template-columns:repeat(3,72px);grid-template-rows:repeat(3,72px);gap:8px;margin:12px 0}
  button{background:#333;color:#eee;border:1px solid #555;border-radius:8px;font-size:24px;font-weight:500;touch-action:none}
  button:active{background:#0a84ff;border-color:#0a84ff}
  .row{display:flex;gap:8px;margin-bottom:8px}
  .row button{flex:1;height:48px;font-size:14px}
  .stop{background:#a00;border-color:#a00}
  .explore{background:#0a84ff;border-color:#0a84ff}
  .reset{background:#666}
  #status{font-family:ui-monospace,monospace;font-size:12px;background:#222;padding:8px;border-radius:8px;width:100%;max-width:480px;white-space:pre-wrap;line-height:1.5}
  .ok{color:#5dd}.warn{color:#fa0}.bad{color:#f55}
</style>
</head>
<body>
<h1>Robo control</h1>
<img id="stream" alt="camera offline">
<div class="grid">
  <div></div>
  <button data-cmd="drive" data-lin="1"  data-ang="0">&#9650;</button>
  <div></div>
  <button data-cmd="drive" data-lin="0"  data-ang="1">&#9664;</button>
  <button data-cmd="stop" class="stop">STOP</button>
  <button data-cmd="drive" data-lin="0"  data-ang="-1">&#9654;</button>
  <div></div>
  <button data-cmd="drive" data-lin="-1" data-ang="0">&#9660;</button>
  <div></div>
</div>
<div class="row">
  <button class="explore" data-cmd="explore">Explore</button>
  <button class="reset"   data-cmd="reset">Reset failsafe</button>
</div>
<div id="status">connecting...</div>

<script>
const status = document.getElementById('status');
const stream = document.getElementById('stream');
stream.src = '__CAMERA_URL__';

function send(cmd, lin, ang) {
  const params = new URLSearchParams({cmd});
  if (lin !== undefined) params.set('lin', lin);
  if (ang !== undefined) params.set('ang', ang);
  params.set('_', Date.now());  // cache buster - some mobile browsers cache aggressively
  // Image() trick instead of fetch() - works in EVERY browser including
  // Mi Browser on HyperOS, older Samsung Internet, embedded WebViews.
  // Browser fetches the URL thinking it's an image, fails silently on
  // the JSON response - we don't care, we just need the request to fire.
  new Image().src = '/cmd?' + params.toString();
}

// Tap-and-hold drive: send the command while pressed, send stop on release.
// 'click' is added as a fallback for browsers that don't reliably emit
// touchstart/mousedown on the first interaction (e.g. some Mi Browser builds).
document.querySelectorAll('button[data-cmd]').forEach(b => {
  const press = e => {
    e.preventDefault();
    send(b.dataset.cmd, b.dataset.lin, b.dataset.ang);
  };
  const release = e => {
    e.preventDefault();
    if (b.dataset.cmd === 'drive') send('stop');
  };
  b.addEventListener('touchstart', press, {passive:false});
  b.addEventListener('mousedown',  press);
  b.addEventListener('touchend',   release);
  b.addEventListener('mouseup',    release);
  b.addEventListener('mouseleave', release);
  // Click fallback: only fires for non-drive commands (reset, stop, explore)
  // since drive uses tap-and-hold semantics and click already triggered via mousedown.
  if (b.dataset.cmd !== 'drive') {
    b.addEventListener('click', e => {
      e.preventDefault();
      send(b.dataset.cmd);
    });
  }
});

function colorize(mode) {
  if (mode === 'Failsafe') return 'bad';
  if (mode === 'Alert' || mode === 'Avoiding') return 'warn';
  return 'ok';
}

async function poll() {
  try {
    const r = await fetch('/status', {cache:'no-store'});
    const s = await r.json();
    const fail = s.mode === 'Failsafe'
      ? `\n<span class="bad">fail  ${s.failReason}</span>`
      : '';
    status.innerHTML =
      `<span class="${colorize(s.mode)}">mode  ${s.mode}</span>${fail}\n` +
      `batt  ${s.battery.toFixed(2)} V\n` +
      `front ${s.front} cm   back ${s.back} cm\n` +
      `left  ${s.left} cm   right ${s.right} cm\n` +
      `pwm   L=${s.pwmL}  R=${s.pwmR}\n` +
      `wd    sensor=${s.msSinceSensor}/${s.sensorTimeoutMs}ms  cmd=${s.msSinceCmd}/${s.commandTimeoutMs}ms\n` +
      `up    ${s.uptime}s`;
  } catch(e) {
    status.textContent = 'lost connection';
  }
}
setInterval(poll, 2000);
poll();
// Heartbeat removed - Mi Browser queued too many requests.
// Drive command is sent once on press, stop on release. That's enough.
</script>
</body>
</html>)HTML";
