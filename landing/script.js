/* ============================================
   SOLAR 303 — Script
   ============================================ */

// ── PARTICLE BACKGROUND ──────────────────────
(function () {
  const canvas = document.getElementById('bg-canvas');
  const ctx = canvas.getContext('2d');
  let W, H, particles = [];

  function resize() {
    W = canvas.width  = window.innerWidth;
    H = canvas.height = window.innerHeight;
  }
  window.addEventListener('resize', resize);
  resize();

  class Particle {
    constructor() { this.reset(true); }
    reset(init = false) {
      this.x  = Math.random() * W;
      this.y  = init ? Math.random() * H : H + 4;
      this.vx = (Math.random() - 0.5) * 0.25;
      this.vy = -(Math.random() * 0.4 + 0.1);
      this.sz = Math.random() * 1.5 + 0.4;
      this.a  = Math.random() * 0.4 + 0.05;
      this.t  = Math.random() * Math.PI * 2;
      this.ts = Math.random() * 0.015 + 0.004;
      this.orange = Math.random() > 0.6;
    }
    update() {
      this.x += this.vx;
      this.y += this.vy;
      this.t += this.ts;
      const alpha = this.a * (0.6 + 0.4 * Math.sin(this.t));
      if (this.orange) {
        ctx.fillStyle = `rgba(255,153,0,${alpha})`;
      } else {
        ctx.fillStyle = `rgba(0,229,255,${alpha * 0.7})`;
      }
      ctx.beginPath();
      ctx.arc(this.x, this.y, this.sz, 0, Math.PI * 2);
      ctx.fill();
      if (this.y < -4) this.reset();
    }
  }

  for (let i = 0; i < 120; i++) particles.push(new Particle());

  // Grid lines
  function drawGrid() {
    const spacing = 60;
    ctx.strokeStyle = 'rgba(255,153,0,0.025)';
    ctx.lineWidth = 1;
    for (let x = 0; x < W; x += spacing) {
      ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, H); ctx.stroke();
    }
    for (let y = 0; y < H; y += spacing) {
      ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(W, y); ctx.stroke();
    }
  }

  function loop() {
    ctx.clearRect(0, 0, W, H);
    drawGrid();
    particles.forEach(p => p.update());
    requestAnimationFrame(loop);
  }
  loop();
})();

// ── NAVBAR SCROLL ────────────────────────────
(function () {
  const nav = document.getElementById('navbar');
  window.addEventListener('scroll', () => {
    nav.classList.toggle('scrolled', window.scrollY > 60);
  });
})();

// ── MOBILE NAV ───────────────────────────────
(function () {
  const btn = document.getElementById('hamburger');
  const nav = document.getElementById('mobile-nav');
  btn.addEventListener('click', () => {
    nav.classList.toggle('open');
  });
  nav.querySelectorAll('a').forEach(a => {
    a.addEventListener('click', () => nav.classList.remove('open'));
  });
})();

// ── SCROLL REVEAL ────────────────────────────
(function () {
  const els = document.querySelectorAll('.reveal');
  const io = new IntersectionObserver((entries) => {
    entries.forEach(e => {
      if (e.isIntersecting) {
        e.target.classList.add('visible');
        io.unobserve(e.target);
      }
    });
  }, { threshold: 0.1 });
  els.forEach(el => io.observe(el));
})();

// ── ACID AUDIO ENGINE — WAV Samples ───────────────────────────────────────────
(function () {

  const SAMPLES = [
    'samples/SOLAR.wav',
    'samples/I AM SOLAR.wav',
    'samples/AM I SOLAR.wav',
    'samples/ITS SOLAR LANGUAGE.wav',
  ];

  let currentPreset = 1;   // I AM SOLAR is first/active by default
  let playing = false;
  let audioEl = null;

  function getAudio() {
    if (!audioEl) {
      audioEl = new Audio();
      audioEl.loop = true;
      audioEl.volume = 0.9;
    }
    return audioEl;
  }

  function startAcid() {
    const a = getAudio();
    a.src = SAMPLES[currentPreset];
    a.currentTime = 0;
    a.play().catch(() => {});
  }

  function stopAcid() {
    if (audioEl) {
      audioEl.pause();
      audioEl.currentTime = 0;
    }
  }

  function switchPreset(idx) {
    currentPreset = idx;
    if (playing) {
      const a = getAudio();
      a.src = SAMPLES[currentPreset];
      a.currentTime = 0;
      a.play().catch(() => {});
    }
  }

  const btn = document.getElementById('acid-btn');
  btn.addEventListener('click', () => {
    playing = !playing;
    if (playing) {
      startAcid();
      btn.classList.add('playing');
      btn.innerHTML = '<span class="btn-icon">■</span> STOP ACID';
    } else {
      stopAcid();
      btn.classList.remove('playing');
      btn.innerHTML = '<span class="btn-icon">▶</span> HEAR THE ACID';
    }
  });

  document.querySelectorAll('.preset-btn').forEach(b => {
    b.addEventListener('click', () => {
      const idx = parseInt(b.dataset.preset);
      document.querySelectorAll('.preset-btn').forEach(x => x.classList.remove('active'));
      b.classList.add('active');
      switchPreset(idx);
    });
  });

})();

// ── HUD DATA LIVE UPDATE ─────────────────────
(function () {
  function rnd(min, max) {
    return (Math.random() * (max - min) + min).toFixed(2);
  }
  const vals = {
    '.hd-1 .hd-val.sm:nth-child(4)': () => `FREQ · ${rnd(430, 450)} Hz`,
    '.hd-1 .hd-val.sm:last-child':   () => `BPM · ${rnd(129, 131)}`,
    '.hd-2 .hd-val.sm:nth-child(4)': () => `CUTOFF · ${rnd(0.60, 0.85)}`,
    '.hd-2 .hd-val.sm:last-child':   () => `RESO · ${rnd(0.75, 0.95)}`,
    '.hd-4 .hd-val.sm:nth-child(4)': () => `DRIVE · ${rnd(0.50, 0.80)}`,
  };

  setInterval(() => {
    Object.entries(vals).forEach(([sel, fn]) => {
      const el = document.querySelector(sel);
      if (el) el.textContent = fn();
    });
  }, 1800);
})();

// ── FEAT CARD MOUSE GLOW ─────────────────────
(function () {
  document.querySelectorAll('.feat-card').forEach(card => {
    card.addEventListener('mousemove', e => {
      const r = card.getBoundingClientRect();
      card.style.setProperty('--mx', ((e.clientX - r.left) / r.width  * 100) + '%');
      card.style.setProperty('--my', ((e.clientY - r.top)  / r.height * 100) + '%');
    });
  });
})();
