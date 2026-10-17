export function getIcon(name, className = "size-5") {
  const paths = {
    arrow: `<path d="M5 12h14"/><path d="m13 6 6 6-6 6"/>`,
    bell: `<path d="M18 8a6 6 0 0 0-12 0c0 7-3 7-3 9h18c0-2-3-2-3-9"/><path d="M10 21h4"/>`,
    card: `<rect x="2" y="5" width="20" height="14" rx="3"/><path d="M2 10h20M6 15h4"/>`,
    check: `<path d="m5 12 4 4L19 6"/>`,
    grid: `<rect x="3" y="3" width="7" height="7" rx="2"/><rect x="14" y="3" width="7" height="7" rx="2"/><rect x="3" y="14" width="7" height="7" rx="2"/><rect x="14" y="14" width="7" height="7" rx="2"/>`,
    home: `<path d="m3 11 9-8 9 8"/><path d="M5 10v11h14V10M9 21v-6h6v6"/>`,
    eye: `<path d="M2.5 12s3.5-6 9.5-6 9.5 6 9.5 6-3.5 6-9.5 6-9.5-6Z"/><circle cx="12" cy="12" r="2.5"/>`,
    eyeOff: `<path d="m3 3 18 18"/><path d="M10.6 6.2A10 10 0 0 1 12 6c6 0 9.5 6 9.5 6a17 17 0 0 1-2.1 2.8"/><path d="M6.6 6.6C4 8.2 2.5 12 2.5 12s3.5 6 9.5 6a9 9 0 0 0 3.2-.6"/>`,
    lock: `<rect x="4" y="10" width="16" height="11" rx="2"/><path d="M8 10V7a4 4 0 0 1 8 0v3"/>`,
    mail: `<rect x="3" y="5" width="18" height="14" rx="2"/><path d="m3 7 9 6 9-6"/>`,
    plus: `<path d="M12 5v14M5 12h14"/>`,
    phone: `<path d="M7 2h10a2 2 0 0 1 2 2v16a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2Z"/><path d="M10 18h4"/>`,
    send: `<path d="m22 2-7 20-4-9-9-4Z"/><path d="M22 2 11 13"/>`,
    shield: `<path d="M12 22s8-3.5 8-10V5l-8-3-8 3v7c0 6.5 8 10 8 10Z"/><path d="m9 12 2 2 4-5"/>`,
    user: `<circle cx="12" cy="8" r="4"/><path d="M4 22a8 8 0 0 1 16 0"/>`,
    wallet: `<path d="M4 6h14a2 2 0 0 1 2 2v12H4a2 2 0 0 1-2-2V5a3 3 0 0 1 3-3h13"/><path d="M15 12h7v5h-7a2.5 2.5 0 0 1 0-5Z"/>`
  };

  return `<svg aria-hidden="true" class="${className}" fill="none" viewBox="0 0 24 24" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round">${paths[name] || ''}</svg>`;
}

export function renderBrand(name = "OOPBANK") {
  return `
    <div class="flex items-center gap-3">
      <div class="grid size-10 place-items-center rounded-full border border-[#71543d]/25 bg-[#fffaf1]/70">
        <svg aria-hidden="true" class="size-6 text-[#71543d]" viewBox="0 0 32 32" fill="none">
          <path d="M6 12 16 5l10 7M8 13h16M10 15v8m6-8v8m6-8v8M7 26h18" stroke="currentColor" stroke-width="1.7" stroke-linecap="round" />
        </svg>
      </div>
      <div>
        <div class="font-display text-xl leading-none tracking-[0.08em] text-[#3d2d23]">${name}</div>
        <div class="mt-1 text-[9px] font-semibold uppercase tracking-[0.3em] text-[#8d735e]">Private Banking</div>
      </div>
    </div>
  `;
}

export function renderField({ id, label, icon, type = "text", placeholder, actionHtml = "" }) {
  return `
    <label class="block" for="${id}">
      <span class="mb-2 block text-[11px] font-bold uppercase tracking-[0.16em] text-[#655246]">${label}</span>
      <span class="group flex h-13 items-center gap-3 rounded-2xl border border-[#d9cdbf] bg-[#fffdf8] px-4 transition focus-within:border-[#9b6a45] focus-within:ring-3 focus-within:ring-[#9b6a45]/10">
        ${getIcon(icon, "size-4.5 shrink-0 text-[#9a8575] transition group-focus-within:text-[#805638]")}
        <input
          id="${id}"
          class="min-w-0 flex-1 bg-transparent text-sm text-[#35271f] outline-none placeholder:text-[#ad9e91]"
          type="${type}"
          placeholder="${placeholder}"
          required
        />
        ${actionHtml}
      </span>
    </label>
  `;
}