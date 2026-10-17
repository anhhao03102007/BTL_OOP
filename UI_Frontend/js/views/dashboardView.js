import { getIcon, renderBrand } from '../icons.js';
import { transactions } from '../data.js';
import { state } from '../state.js';

export function renderDashboardView() {
  const quickActions = [
    { label: "Chuyển tiền", icon: "send", page: "transfer" },
    { label: "Nạp tiền", icon: "plus", page: "topup" },
    { label: "Thanh toán", icon: "card", page: "payment" },
    { label: "Tất cả", icon: "grid", page: "services" }
  ];

  return `
    <main class="min-h-screen bg-[#f3ecdf] pb-24 text-[#3d2d23] lg:pb-8">
      <!-- Header -->
      <header class="sticky top-0 z-20 border-b border-[#e6dacb] bg-[#f8f2e8]/90 backdrop-blur-xl">
        <div class="mx-auto flex h-20 max-w-[1280px] items-center justify-between px-5 sm:px-8">
          ${renderBrand("OOPBANK")}
          <nav class="hidden items-center gap-2 lg:flex">
            <button class="btn-nav flex items-center gap-2 rounded-xl bg-[#eadfd1] px-4 py-2.5 text-sm font-bold text-[#67452f]" data-page="home">
              ${getIcon("home", "size-4")}
              Trang chủ
            </button>
            <button class="btn-nav rounded-xl px-4 py-2.5 text-sm font-semibold text-[#806e61] transition hover:bg-[#eee4d7]" data-page="accounts">
              Tài khoản
            </button>
            <button class="btn-nav rounded-xl px-4 py-2.5 text-sm font-semibold text-[#806e61] transition hover:bg-[#eee4d7]" data-page="services">
              Dịch vụ
            </button>
          </nav>
          <div class="flex items-center gap-2">
            <button
              class="btn-nav relative grid size-10 place-items-center rounded-full border border-[#ddcfc0] bg-[#fffaf1] text-[#674d3b] transition hover:bg-white"
              data-page="notifications"
              aria-label="Thông báo"
            >
              ${getIcon("bell", "size-4.5")}
              <span class="absolute right-2 top-2 size-1.5 rounded-full bg-[#b45f4d]"></span>
            </button>
            <button
              id="btn-logout"
              class="ml-1 flex items-center gap-3 rounded-full bg-[#e9ddce] p-1 pr-3 text-left"
              title="Đăng xuất"
            >
              <span class="grid size-9 place-items-center rounded-full bg-[#765038] text-xs font-bold text-white">NA</span>
              <span class="hidden text-xs font-bold text-[#4b3527] sm:block">Minh Anh</span>
            </button>
          </div>
        </div>
      </header>

      <!-- Nội dung Dashboard -->
      <div class="mx-auto max-w-[1280px] px-5 py-7 sm:px-8 sm:py-10">
        <div class="mb-7 flex items-end justify-between gap-4">
          <div>
            <p class="text-sm text-[#8a7668]">Chào buổi sáng,</p>
            <h1 class="font-display mt-1 text-3xl text-[#3b2a20] sm:text-4xl">Nguyễn Minh Anh</h1>
          </div>
          <p class="hidden text-xs font-semibold text-[#9a887a] sm:block">Thứ Hai, 02 tháng 02, 2026</p>
        </div>

        <div class="grid gap-6 xl:grid-cols-[1.45fr_0.8fr]">
          <!-- Thẻ Số dư -->
          <section class="relative overflow-hidden rounded-[28px] bg-[#684630] p-6 text-white shadow-[0_20px_50px_rgba(78,49,31,0.18)] sm:p-8">
            <div class="absolute -right-20 -top-28 size-80 rounded-full border-[55px] border-white/5"></div>
            <div class="absolute -bottom-24 right-32 size-56 rounded-full border-[38px] border-[#d5b18c]/10"></div>
            <div class="relative">
              <div class="flex items-start justify-between">
                <div>
                  <p class="text-xs font-semibold uppercase tracking-[0.18em] text-white/60">Tổng số dư khả dụng</p>
                  <div class="mt-3 flex items-center gap-3">
                    <p class="text-3xl font-bold tracking-[-0.03em] sm:text-4xl">
                      ${state.showBalance ? "128.650.000 ₫" : "••••••••••"}
                    </p>
                    <button
                      id="btn-toggle-balance"
                      class="rounded-full p-2 text-white/65 transition hover:bg-white/10 hover:text-white"
                      type="button"
                    >
                      ${getIcon(state.showBalance ? "eye" : "eyeOff", "size-5")}
                    </button>
                  </div>
                </div>
                <div class="grid size-11 place-items-center rounded-2xl bg-white/10">
                  ${getIcon("wallet", "size-5 text-[#e3c6a8]")}
                </div>
              </div>

              <div class="mt-9 flex flex-wrap gap-x-10 gap-y-4 border-t border-white/15 pt-5">
                <div>
                  <p class="text-[10px] font-semibold uppercase tracking-[0.15em] text-white/50">Tài khoản thanh toán</p>
                  <p class="mt-1.5 text-sm font-bold">0123 6868 8888</p>
                </div>
                <div>
                  <p class="text-[10px] font-semibold uppercase tracking-[0.15em] text-white/50">Chi nhánh</p>
                  <p class="mt-1.5 text-sm font-bold">OOPBank Hà Nội</p>
                </div>
              </div>
            </div>
          </section>

          <!-- Thao tác nhanh -->
          <section class="rounded-[28px] border border-[#e2d6c7] bg-[#fffaf2] p-6 sm:p-7">
            <div class="flex items-center justify-between">
              <h2 class="font-display text-xl">Thao tác nhanh</h2>
              <button class="btn-nav text-xs font-bold text-[#8a5c3e]" data-page="services">Chỉnh sửa</button>
            </div>
            <div class="mt-6 grid grid-cols-4 gap-3">
              ${quickActions.map(action => `
                <button
                  class="btn-nav group flex flex-col items-center gap-2.5 text-center"
                  data-page="${action.page}"
                  type="button"
                >
                  <span class="grid size-12 place-items-center rounded-2xl bg-[#eee3d6] text-[#72503a] transition group-hover:-translate-y-1 group-hover:bg-[#765038] group-hover:text-white">
                    ${getIcon(action.icon, "size-5")}
                  </span>
                  <span class="text-[10px] font-bold leading-4 text-[#6f5b4e] sm:text-xs">${action.label}</span>
                </button>
              `).join('')}
            </div>
          </section>
        </div>

        <!-- Tiết kiệm & Khoản vay -->
        <div class="mt-6 grid gap-6 md:grid-cols-2">
          <!-- Sổ tiết kiệm -->
          <section class="rounded-[28px] border border-[#e2d6c7] bg-[#fffaf2] p-6 sm:p-7">
            <div class="flex items-start justify-between gap-4">
              <div class="flex items-center gap-3">
                <span class="grid size-11 place-items-center rounded-2xl bg-[#e8dac9] text-[#745039]">
                  ${getIcon("wallet", "size-5")}
                </span>
                <div>
                  <p class="text-[10px] font-bold uppercase tracking-[0.17em] text-[#9a806e]">Tích lũy sinh lời</p>
                  <h2 class="font-display mt-1 text-xl">Sổ tiết kiệm</h2>
                </div>
              </div>
              <span class="rounded-full bg-[#e8f0e4] px-3 py-1.5 text-[10px] font-bold text-[#536b4d]">Đang hoạt động</span>
            </div>
            <div class="mt-6 grid grid-cols-2 gap-4">
              <div>
                <p class="text-[10px] font-semibold uppercase tracking-[0.12em] text-[#9a887a]">Tiền gốc</p>
                <p class="mt-2 text-xl font-bold text-[#493429]">80.000.000 ₫</p>
              </div>
              <div>
                <p class="text-[10px] font-semibold uppercase tracking-[0.12em] text-[#9a887a]">Lãi suất</p>
                <p class="mt-2 text-xl font-bold text-[#52704b]">5,8% / năm</p>
              </div>
            </div>
            <div class="mt-5 flex items-center justify-between rounded-2xl bg-[#f2e9dd] px-4 py-3">
              <div>
                <p class="text-[10px] text-[#927f71]">Ngày đáo hạn</p>
                <p class="mt-1 text-xs font-bold">18/08/2026</p>
              </div>
              <div class="text-right">
                <p class="text-[10px] text-[#927f71]">Lãi dự kiến</p>
                <p class="mt-1 text-xs font-bold text-[#52704b]">+2.320.000 ₫</p>
              </div>
            </div>
            <button
              class="btn-nav mt-5 flex w-full items-center justify-center gap-2 rounded-xl border border-[#bfa991] py-3 text-xs font-bold text-[#67452f] transition hover:bg-[#765038] hover:text-white"
              data-page="savings"
            >
              ${getIcon("plus", "size-4")}
              Mở sổ tiết kiệm mới
            </button>
          </section>

          <!-- Khoản vay -->
          <section class="rounded-[28px] border border-[#e2d6c7] bg-[#fffaf2] p-6 sm:p-7">
            <div class="flex items-start justify-between gap-4">
              <div class="flex items-center gap-3">
                <span class="grid size-11 place-items-center rounded-2xl bg-[#ead9d3] text-[#855446]">
                  ${getIcon("card", "size-5")}
                </span>
                <div>
                  <p class="text-[10px] font-bold uppercase tracking-[0.17em] text-[#9a806e]">Vay tiêu dùng</p>
                  <h2 class="font-display mt-1 text-xl">Khoản vay</h2>
                </div>
              </div>
              <span class="rounded-full bg-[#f2e5dc] px-3 py-1.5 text-[10px] font-bold text-[#895943]">Còn 14 kỳ</span>
            </div>
            <div class="mt-6 flex items-end justify-between gap-4">
              <div>
                <p class="text-[10px] font-semibold uppercase tracking-[0.12em] text-[#9a887a]">Dư nợ còn lại</p>
                <p class="mt-2 text-xl font-bold text-[#493429]">45.600.000 ₫</p>
              </div>
              <p class="text-xs font-bold text-[#7c5a45]">38% đã trả</p>
            </div>
            <div class="mt-4 h-2 overflow-hidden rounded-full bg-[#eadfd2]">
              <div class="h-full w-[38%] rounded-full bg-[#9b6a4c]"></div>
            </div>
            <div class="mt-5 flex items-center justify-between rounded-2xl bg-[#f2e9dd] px-4 py-3">
              <div>
                <p class="text-[10px] text-[#927f71]">Kỳ thanh toán tới</p>
                <p class="mt-1 text-xs font-bold">15/02/2026</p>
              </div>
              <div class="text-right">
                <p class="text-[10px] text-[#927f71]">Số tiền cần trả</p>
                <p class="mt-1 text-xs font-bold text-[#895943]">3.250.000 ₫</p>
              </div>
            </div>
            <button
              class="btn-nav mt-5 flex w-full items-center justify-center gap-2 rounded-xl bg-[#765038] py-3 text-xs font-bold text-white transition hover:bg-[#5e3f2c]"
              data-page="loan"
            >
              Thanh toán khoản vay
              ${getIcon("arrow", "size-4")}
            </button>
          </section>
        </div>

        <!-- Lịch sử giao dịch & Banner -->
        <div class="mt-6 grid gap-6 lg:grid-cols-[1.3fr_0.7fr]">
          <section class="rounded-[28px] border border-[#e2d6c7] bg-[#fffaf2] p-5 sm:p-7">
            <div class="flex items-center justify-between">
              <div>
                <h2 class="font-display text-2xl">Giao dịch gần đây</h2>
                <p class="mt-1 text-xs text-[#927f71]">Cập nhật theo thời gian thực</p>
              </div>
              <button class="btn-nav text-xs font-bold text-[#8a5c3e] hover:underline" data-page="transactions">
                Xem tất cả
              </button>
            </div>
            <div class="mt-5 divide-y divide-[#eadfd2]">
              ${transactions.map(item => `
                <div class="flex items-center gap-3 py-4 first:pt-1">
                  <span class="grid size-10 shrink-0 place-items-center rounded-xl bg-[#f0e5d8] text-[#79543c]">
                    ${getIcon(item.icon, "size-4.5")}
                  </span>
                  <div class="min-w-0 flex-1">
                    <p class="truncate text-sm font-bold text-[#493429]">${item.name}</p>
                    <p class="mt-1 text-[11px] text-[#9a887a]">${item.detail}</p>
                  </div>
                  <p class="text-xs font-bold sm:text-sm ${item.amount.startsWith('+') ? 'text-[#52704b]' : 'text-[#59473c]'}">
                    ${item.amount}
                  </p>
                </div>
              `).join('')}
            </div>
          </section>

          <section class="relative overflow-hidden rounded-[28px] bg-[#d7b894] p-7">
            <div class="absolute -bottom-16 -right-16 size-52 rounded-full bg-white/20"></div>
            <p class="text-[10px] font-bold uppercase tracking-[0.2em] text-[#6b4a34]">Tiết kiệm thông minh</p>
            <h2 class="font-display mt-4 max-w-xs text-3xl leading-tight text-[#38281f]">Sinh lời mỗi ngày, vững vàng tương lai.</h2>
            <p class="mt-3 max-w-xs text-xs leading-5 text-[#684b38]">Lãi suất hấp dẫn lên đến 5,8%/năm với Tiết kiệm OOPBank.</p>
            <button class="btn-nav relative mt-7 flex items-center gap-2 rounded-xl bg-[#4f3728] px-5 py-3 text-xs font-bold text-white transition hover:bg-[#39271d]" data-page="savings">
              Khám phá ngay
              ${getIcon("arrow", "size-4")}
            </button>
          </section>
        </div>
      </div>

      <!-- Menu Mobile bên dưới -->
      <nav class="fixed inset-x-3 bottom-3 z-30 flex items-center justify-around rounded-2xl border border-[#e2d6c7] bg-[#fffaf2]/95 px-3 py-2 shadow-[0_12px_35px_rgba(70,45,29,0.16)] backdrop-blur-xl lg:hidden">
        ${[
          { label: "Trang chủ", icon: "home", page: "home", active: true },
          { label: "Tài khoản", icon: "wallet", page: "accounts", active: false },
          { label: "Chuyển tiền", icon: "send", page: "transfer", active: false },
          { label: "Tiện ích", icon: "grid", page: "services", active: false }
        ].map(item => `
          <button
            class="btn-nav flex flex-col items-center gap-1 rounded-xl px-3 py-1.5 text-[9px] font-bold ${item.active ? "bg-[#eee3d6] text-[#6c4933]" : "text-[#9a887a]"}"
            data-page="${item.page}"
          >
            ${getIcon(item.icon, "size-4.5")}
            ${item.label}
          </button>
        `).join('')}
      </nav>
    </main>
  `;
}

export function bindDashboardEvents() {
  document.getElementById('btn-logout')?.addEventListener('click', () => {
    state.setLoggedIn(false); // Đăng xuất -> Quay lại trang đăng nhập
  });

  document.getElementById('btn-toggle-balance')?.addEventListener('click', () => {
    state.toggleBalanceVisibility();
  });

  document.querySelectorAll('.btn-nav').forEach(btn => {
    btn.addEventListener('click', (e) => {
      const page = e.currentTarget.getAttribute('data-page');
      if (page) state.setCurrentPage(page);
    });
  });
}