import { getIcon, renderBrand, renderField } from '../icons.js';
import { pageDetails, transactions } from '../data.js';
import { state } from '../state.js';

export function renderFeatureView() {
  const page = state.currentPage;
  const detail = pageDetails[page] || pageDetails.transfer;
  const isFormPage = ["transfer", "topup", "payment", "savings", "loan"].includes(page);

  const serviceItems = [
    { page: "transfer", label: "Chuyển tiền", icon: "send" },
    { page: "topup", label: "Nạp điện thoại", icon: "phone" },
    { page: "payment", label: "Thanh toán", icon: "card" },
    { page: "savings", label: "Tiết kiệm", icon: "wallet" },
    { page: "loan", label: "Khoản vay", icon: "card" },
    { page: "accounts", label: "Tài khoản", icon: "user" }
  ];

  return `
    <main class="min-h-screen bg-[#f3ecdf] text-[#3d2d23]">
      <!-- Header -->
      <header class="border-b border-[#e6dacb] bg-[#f8f2e8]/95">
        <div class="mx-auto flex h-20 max-w-[1120px] items-center justify-between px-5 sm:px-8">
          <button id="btn-back" class="flex items-center gap-2 rounded-xl px-3 py-2 text-xs font-bold text-[#6e5140] transition hover:bg-[#eadfd1]" type="button">
            ${getIcon("arrow", "size-4 rotate-180")}
            Quay lại
          </button>
          ${renderBrand("OOPBANK")}
          <button id="btn-home" class="grid size-10 place-items-center rounded-full bg-[#eadfd1] text-[#6e5140]" type="button" aria-label="Về trang chủ">
            ${getIcon("home", "size-4.5")}
          </button>
        </div>
      </header>

      <div class="mx-auto max-w-[1120px] px-5 py-8 sm:px-8 sm:py-12">
        <div class="mb-8 flex items-center gap-4">
          <span class="grid size-14 place-items-center rounded-2xl bg-[#e5d6c5] text-[#72503a]">
            ${getIcon(detail.icon, "size-6")}
          </span>
          <div>
            <p class="text-[10px] font-bold uppercase tracking-[0.2em] text-[#9a6c4d]">${detail.eyebrow}</p>
            <h1 class="font-display mt-1 text-3xl sm:text-4xl">${detail.title}</h1>
          </div>
        </div>
        <p class="-mt-4 mb-8 max-w-xl text-sm leading-6 text-[#806e61]">${detail.description}</p>

        <!-- Dịch vụ -->
        ${page === 'services' ? `
          <div class="grid gap-4 sm:grid-cols-2 lg:grid-cols-3">
            ${serviceItems.map(item => `
              <button
                class="btn-feature-nav group flex items-center gap-4 rounded-2xl border border-[#e2d6c7] bg-[#fffaf2] p-5 text-left transition hover:-translate-y-1 hover:shadow-[0_15px_35px_rgba(76,49,31,0.1)]"
                data-page="${item.page}"
                type="button"
              >
                <span class="grid size-11 place-items-center rounded-xl bg-[#eee3d6] text-[#72503a] group-hover:bg-[#765038] group-hover:text-white">
                  ${getIcon(item.icon, "size-5")}
                </span>
                <span class="flex-1 text-sm font-bold">${item.label}</span>
                ${getIcon("arrow", "size-4 text-[#a79283]")}
              </button>
            `).join('')}
          </div>
        ` : ''}

        <!-- Tài khoản -->
        ${page === 'accounts' ? `
          <div class="grid gap-5 md:grid-cols-2">
            ${[
              { name: "Tài khoản thanh toán", number: "0123 6868 8888", balance: "128.650.000 ₫", primary: true },
              { name: "Tài khoản dự phòng", number: "0123 6868 9999", balance: "12.500.000 ₫", primary: false }
            ].map(acc => `
              <section class="rounded-[26px] p-6 ${acc.primary ? "bg-[#684630] text-white" : "border border-[#e2d6c7] bg-[#fffaf2]"}">
                <div class="flex items-center justify-between">
                  ${getIcon("wallet", "size-5")}
                  ${acc.primary ? `<span class="rounded-full bg-white/10 px-3 py-1 text-[9px] font-bold uppercase tracking-wider">Mặc định</span>` : ''}
                </div>
                <p class="mt-8 text-xs ${acc.primary ? "text-white/60" : "text-[#927f71]"}">${acc.name}</p>
                <p class="mt-2 text-2xl font-bold">${acc.balance}</p>
                <p class="mt-5 text-xs tracking-[0.16em] ${acc.primary ? "text-white/70" : "text-[#806e61]"}">${acc.number}</p>
              </section>
            `).join('')}
          </div>
        ` : ''}

        <!-- Thông báo & Tất cả giao dịch -->
        ${(page === 'notifications' || page === 'transactions') ? `
          <section class="rounded-[26px] border border-[#e2d6c7] bg-[#fffaf2] p-5 sm:p-7">
            <div class="divide-y divide-[#eadfd2]">
              ${(page === 'transactions'
                ? [...transactions, ...transactions.slice(1)]
                : [
                    { name: "Giao dịch thành công", detail: "Bạn đã chuyển 2.500.000 ₫ đến Nguyễn Hoàng Nam.", amount: "2 phút trước", icon: "check" },
                    { name: "Nhắc lịch thanh toán", detail: "Khoản vay sẽ đến hạn vào ngày 15/02/2026.", amount: "Hôm qua", icon: "bell" },
                    { name: "Ưu đãi mới dành cho bạn", detail: "Hoàn tiền 10% khi thanh toán hóa đơn cuối tuần.", amount: "28/01", icon: "card" }
                  ]
              ).map(item => `
                <div class="flex items-center gap-4 py-4 first:pt-0 last:pb-0">
                  <span class="grid size-11 shrink-0 place-items-center rounded-xl bg-[#eee3d6] text-[#72503a]">
                    ${getIcon(item.icon, "size-5")}
                  </span>
                  <div class="min-w-0 flex-1">
                    <p class="text-sm font-bold">${item.name}</p>
                    <p class="mt-1 truncate text-xs text-[#927f71]">${item.detail}</p>
                  </div>
                  <span class="text-xs font-bold text-[#806e61]">${item.amount}</span>
                </div>
              `).join('')}
            </div>
          </section>
        ` : ''}

        <!-- Form thực hiện giao dịch -->
        ${isFormPage ? `
          <div class="grid gap-6 lg:grid-cols-[1fr_0.55fr]">
            <section class="rounded-[28px] border border-[#e2d6c7] bg-[#fffaf2] p-6 sm:p-8">
              ${state.formSuccess ? `
                <div class="py-10 text-center">
                  <span class="mx-auto grid size-16 place-items-center rounded-full bg-[#e6efe1] text-[#52704b]">
                    ${getIcon("check", "size-8")}
                  </span>
                  <h2 class="font-display mt-5 text-2xl">Giao dịch thành công</h2>
                  <p class="mt-2 text-sm text-[#806e61]">Yêu cầu của bạn đã được OOPBank ghi nhận.</p>
                  <button id="btn-back-home" class="mt-7 rounded-xl bg-[#765038] px-6 py-3 text-sm font-bold text-white" type="button">
                    Về trang chủ
                  </button>
                </div>
              ` : `
                <form id="feature-form" class="space-y-5">
                  <h2 class="font-display text-2xl">
                    ${page === "transfer" ? "Thông tin người nhận" : ""}
                    ${page === "topup" ? "Thông tin thuê bao" : ""}
                    ${page === "payment" ? "Thông tin hóa đơn" : ""}
                    ${page === "savings" ? "Mở sổ tiết kiệm mới" : ""}
                    ${page === "loan" ? "Thanh toán kỳ vay" : ""}
                  </h2>

                  ${page === "transfer" ? `
                    ${renderField({ id: "receiver", label: "Số tài khoản người nhận", icon: "user", placeholder: "Nhập số tài khoản" })}
                    ${renderField({ id: "bank", label: "Ngân hàng thụ hưởng", icon: "shield", placeholder: "Nhập tên ngân hàng" })}
                  ` : ''}

                  ${page === "topup" ? renderField({ id: "topup-phone", label: "Số điện thoại", icon: "phone", type: "tel", placeholder: "0901 234 567" }) : ''}
                  ${page === "payment" ? renderField({ id: "bill-code", label: "Mã khách hàng / Hóa đơn", icon: "card", placeholder: "Nhập mã tra cứu" }) : ''}

                  <label class="block">
                    <span class="mb-2 block text-[11px] font-bold uppercase tracking-[0.16em] text-[#655246]">
                      ${page === "savings" ? "Số tiền gửi" : page === "loan" ? "Số tiền thanh toán" : "Số tiền"}
                    </span>
                    <span class="flex h-13 items-center rounded-2xl border border-[#d9cdbf] bg-white px-4 focus-within:border-[#9b6a45]">
                      <input class="min-w-0 flex-1 bg-transparent text-sm outline-none" type="number" min="10000" placeholder="0" required />
                      <strong class="text-xs">VND</strong>
                    </span>
                  </label>

                  ${page === "savings" ? `
                    <label class="block">
                      <span class="mb-2 block text-[11px] font-bold uppercase tracking-[0.16em] text-[#655246]">Kỳ hạn</span>
                      <select class="h-13 w-full rounded-2xl border border-[#d9cdbf] bg-white px-4 text-sm outline-none">
                        <option>3 tháng · 4,5%/năm</option>
                        <option>6 tháng · 5,8%/năm</option>
                        <option>12 tháng · 6,1%/năm</option>
                      </select>
                    </label>
                  ` : ''}

                  <button class="flex h-13 w-full items-center justify-center gap-2 rounded-2xl bg-[#765038] text-sm font-bold text-white transition hover:bg-[#5e3f2c]" type="submit">
                    Xác nhận
                    ${getIcon("arrow", "size-4")}
                  </button>
                </form>
              `}
            </section>

            <!-- Cột thông tin nguồn tiền -->
            <aside class="h-fit rounded-[26px] bg-[#e6d5c1] p-6">
              <p class="text-[10px] font-bold uppercase tracking-[0.18em] text-[#80614c]">Nguồn tiền</p>
              <p class="mt-4 text-xs text-[#806e61]">Tài khoản thanh toán</p>
              <p class="mt-1 text-sm font-bold">0123 6868 8888</p>
              <div class="my-5 h-px bg-[#cdb69f]"></div>
              <p class="text-xs text-[#806e61]">Số dư khả dụng</p>
              <p class="mt-2 text-xl font-bold">128.650.000 ₫</p>
            </aside>
          </div>
        ` : ''}
      </div>
    </main>
  `;
}

export function bindFeatureEvents() {
  document.getElementById('btn-back')?.addEventListener('click', () => state.setCurrentPage('home'));
  document.getElementById('btn-home')?.addEventListener('click', () => state.setCurrentPage('home'));
  document.getElementById('btn-back-home')?.addEventListener('click', () => state.setCurrentPage('home'));

  document.querySelectorAll('.btn-feature-nav').forEach(btn => {
    btn.addEventListener('click', (e) => {
      const page = e.currentTarget.getAttribute('data-page');
      if (page) state.setCurrentPage(page);
    });
  });

  document.getElementById('feature-form')?.addEventListener('submit', (e) => {
    e.preventDefault();
    state.setFormSuccess(true);
  });
}