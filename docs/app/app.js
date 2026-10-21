import { createStaff } from "./staff.js";
import { createData } from "./data.js";

// Wait until Qt publishes its C++ bridges; in a regular browser this resolves immediately.
await globalThis.__OOPBANK_BRIDGES_READY__;

const app = document.querySelector("#app");

function getBridge(name) {
  return globalThis[name]
    ?? globalThis.oopbank?.[name]
    ?? globalThis.oopbankBridge?.[name]
    ?? null;
}

function hasBackend() {
  return Boolean(getBridge("userBridge"));
}

function getAuthBridge() {
  return getBridge("authBridge");
}

function parseBridgeResponse(value) {
  return typeof value === "string" ? JSON.parse(value) : value;
}

function invokeBridgeMethod(bridge, methodName, args) {
  return new Promise((resolve, reject) => {
    let settled = false;
    const finish = (callback, value) => {
      if (settled) return;
      settled = true;
      clearTimeout(timer);
      callback(value);
    };
    const timer = setTimeout(() => finish(reject, new Error("Hết thời gian chờ xác thực.")), 15000);
    const callback = (value) => finish(resolve, value);

    try {
      const result = bridge[methodName](...args, callback);
      if (result && typeof result.then === "function") {
        result.then(callback, (error) => finish(reject, error));
      } else if (result !== undefined) {
        callback(result);
      }
    } catch (error) {
      finish(reject, error);
    }
  });
}

async function callBackend(bridgeName, methodName, payload) {
  const bridge = getBridge(bridgeName);
  if (!bridge || typeof bridge[methodName] !== "function") {
    throw new Error(`Backend ${bridgeName}.${methodName} chưa sẵn sàng.`);
  }
  const response = parseBridgeResponse(await invokeBridgeMethod(
    bridge,
    methodName,
    [JSON.stringify(payload)],
  ));
  if (!response || response.success !== true) {
    throw new Error(response?.message || "Backend không xác nhận thao tác thành công.");
  }
  return response;
}

const state = {
  loggedIn: false,
  sessionToken: globalThis.__OOPBANK_SESSION_TOKEN__ ?? "",
  role: "CUSTOMER", // CUSTOMER | TELLER | ADMIN
  authMode: "login",
  page: "home",
  showBalance: true,
  showPassword: false,
  billType: "electricity",
  billProviders: [],
  billProvidersLoaded: false,
  billProvidersError: "",
  selectedMessage: 0,
  transactionSuccess: false,
  showPinModal: false, // State quản lý Pop-up PIN/OTP
  pendingTransaction: null, // Lưu dữ liệu form chờ xác thực PIN
  loanTab: "apply", // apply | docs | review | disburse | manage
  openSchedule: null, // id khoản vay đang mở lịch trả nợ
  docAppId: null, // đơn vay đang bổ sung hồ sơ
  balance: 128650000, // số dư tài khoản thanh toán (mock, được nạp từ DB khi có backend)
  profile: { name: "Nguyễn Minh Anh", email: "minhanh@email.com", phone: "0901234567", branch: "OOPBank Hà Nội" },
  accounts: [
    { id: 1, name: "Tài khoản thanh toán", number: "0123 6868 8888", balance: 128650000, primary: true },
    { id: 2, name: "Tài khoản dự phòng", number: "0123 6868 9999", balance: 12500000, primary: false },
  ],
  savings: [],
  msgOpen: false, // hộp thư nổi đang mở
  msgExpanded: null, // tin nhắn đang xem chi tiết trong hộp thư nổi
  msgBanner: null, // tin nhắn mới đang hiện dạng banner
};

const icons = {
  arrow: '<path d="M5 12h14"/><path d="m13 6 6 6-6 6"/>',
  back: '<path d="M19 12H5"/><path d="m11 18-6-6 6-6"/>',
  bell: '<path d="M18 8a6 6 0 0 0-12 0c0 7-3 7-3 9h18c0-2-3-2-3-9"/><path d="M10 21h4"/>',
  card: '<rect x="2" y="5" width="20" height="14" rx="3"/><path d="M2 10h20M6 15h4"/>',
  check: '<path d="m5 12 4 4L19 6"/>',
  eye: '<path d="M2.5 12s3.5-6 9.5-6 9.5 6 9.5 6-3.5 6-9.5 6-9.5-6Z"/><circle cx="12" cy="12" r="2.5"/>',
  eyeOff: '<path d="m3 3 18 18"/><path d="M10.6 6.2A10 10 0 0 1 12 6c6 0 9.5 6 9.5 6"/><path d="M6.6 6.6C4 8.2 2.5 12 2.5 12s3.5 6 9.5 6"/>',
  grid: '<rect x="3" y="3" width="7" height="7" rx="2"/><rect x="14" y="3" width="7" height="7" rx="2"/><rect x="3" y="14" width="7" height="7" rx="2"/><rect x="14" y="14" width="7" height="7" rx="2"/>',
  home: '<path d="m3 11 9-8 9 8"/><path d="M5 10v11h14V10M9 21v-6h6v6"/>',
  lock: '<rect x="4" y="10" width="16" height="11" rx="2"/><path d="M8 10V7a4 4 0 0 1 8 0v3"/>',
  mail: '<rect x="3" y="5" width="18" height="14" rx="2"/><path d="m3 7 9 6 9-6"/>',
  phone: '<path d="M7 2h10a2 2 0 0 1 2 2v16a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2Z"/><path d="M10 18h4"/>',
  plus: '<path d="M12 5v14M5 12h14"/>',
  send: '<path d="m22 2-7 20-4-9-9-4Z"/><path d="M22 2 11 13"/>',
  shield: '<path d="M12 22s8-3.5 8-10V5l-8-3-8 3v7c0 6.5 8 10 8 10Z"/><path d="m9 12 2 2 4-5"/>',
  user: '<circle cx="12" cy="8" r="4"/><path d="M4 22a8 8 0 0 1 16 0"/>',
  wallet: '<path d="M4 6h14a2 2 0 0 1 2 2v12H4a2 2 0 0 1-2-2V5a3 3 0 0 1 3-3h13"/><path d="M15 12h7v5h-7a2.5 2.5 0 0 1 0-5Z"/>',
  key: '<path d="m21 2-2 2m-1.5 1.5L14 9l-1.5-1.5L11 9l-1.5-1.5L8 9 2 15v5h5l1.5-1.5L7 17l1.5-1.5L7 14l5.5-5.5z"/>',
  clock: '<circle cx="12" cy="12" r="9"/><path d="M12 7v5l3 2"/>',
  file: '<path d="M14 3H7a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2V8z"/><path d="M14 3v5h5M9 13h6M9 17h4"/>',
  settings: '<circle cx="12" cy="12" r="3"/><path d="M12 2v3M12 19v3M2 12h3M19 12h3M5 5l2 2M17 17l2 2M19 5l-2 2M7 17l-2 2"/>',
};

function icon(name) {
  return `<svg class="icon" aria-hidden="true" viewBox="0 0 24 24">${icons[name] || icons.grid}</svg>`;
}

function brand() {
  return `
    <div class="brand">
      <span class="brand-mark">${icon("home")}</span>
      <span><span class="brand-name">OOPBANK</span><span class="brand-sub">Private Banking</span></span>
    </div>`;
}

function field(id, label, iconName, placeholder, type = "text", val = "") {
  const passwordAction = type === "password"
    ? `<button class="password-toggle" type="button" data-action="toggle-password" aria-label="Hiện mật khẩu">${icon(state.showPassword ? "eyeOff" : "eye")}</button>`
    : "";
  const actualType = type === "password" && state.showPassword ? "text" : type;
  return `
    <div class="field">
      <label for="${id}">${label}</label>
      <div class="input-wrap">${icon(iconName)}<input id="${id}" name="${id}" type="${actualType}" value="${val}" placeholder="${placeholder}" required />${passwordAction}</div>
    </div>`;
}

function renderAuth() {
  const register = state.authMode === "register";
  app.innerHTML = `
    <main class="auth-shell">
      <section class="auth-card">
        <aside class="auth-visual">
          <div class="visual-top">${brand()}</div>
          <div class="visual-copy">
            <h2 class="display">Vững tâm hôm nay,<br />thịnh vượng ngày mai.</h2>
            <p>Đồng hành cùng bạn kiến tạo một tương lai tài chính bền vững với dịch vụ ngân hàng riêng tư và tận tâm.</p>
          </div>
        </aside>
        <section class="auth-panel">
          <button class="auth-help" type="button">Trợ giúp</button>
          <div class="auth-form-wrap">
            <span class="eyebrow">${register ? "Bắt đầu hành trình" : "Chào mừng trở lại"}</span>
            <h1 class="display">${register ? "Tạo tài khoản mới" : "Đăng nhập tài khoản"}</h1>
            <p class="subcopy">${register ? "Chỉ mất vài phút để gia nhập OOPBank." : "Quản lý tài chính của bạn an toàn, mọi lúc mọi nơi."}</p>
            <div class="tabs">
              <button class="tab ${register ? "" : "active"}" data-auth-mode="login">Đăng nhập</button>
              <button class="tab ${register ? "active" : ""}" data-auth-mode="register">Đăng ký</button>
            </div>
            <form id="auth-form" class="form-stack">
              ${register ? field("username", "Tên đăng nhập", "user", "Tên đăng nhập") : ""}
              ${register ? field("name", "Họ và tên", "user", "Nguyễn Minh Anh") : ""}
              ${field("contact", register ? "Địa chỉ email" : "Tên đăng nhập hoặc số điện thoại", "mail", register ? "tenban@email.com" : "Tên đăng nhập")}
              ${register ? field("phone", "Số điện thoại", "phone", "0901 234 567", "tel") : ""}
              ${register ? field("cccd", "Số CCCD", "shield", "Nhập 12 số căn cước công dân") : ""}
              ${field("password", "Mật khẩu", "lock", register ? "Tối thiểu 8 ký tự" : "Nhập mật khẩu của bạn", "password")}
              ${register ? "" : '<div class="forgot-row"><button class="link-button" type="button" data-action="forgot-pass">Quên mật khẩu?</button></div>'}
              <button class="primary" type="submit">${register ? "Tạo tài khoản" : "Đăng nhập an toàn"} ${icon("arrow")}</button>
            </form>
          </div>
          <footer class="auth-footer">© 2026 OOPBank</footer>
        </section>
      </section>
    </main>`;
}

const transactions = [
  ["Nguyễn Hoàng Nam", "Chuyển tiền · Hôm nay, 09:42", "-2.500.000 ₫", "send"],
  ["Lương tháng 01/2026", "Tiền vào · 28/01, 08:15", "+25.000.000 ₫", "wallet"],
  ["The Coffee House", "Thanh toán · 27/01, 18:20", "-85.000 ₫", "card"],
];

function transactionRows(all = false) {
  const rows = all ? transactions : transactions.slice(0, 3);
  return rows.map(([name, detail, amount, iconName]) => `
    <div class="transaction">
      <span class="icon-box">${icon(iconName)}</span>
      <div class="info"><p>${name}</p><small>${detail}</small></div>
      <span class="amount ${amount.startsWith("+") ? "positive" : ""}">${amount}</span>
    </div>`).join("");
}

// Thanh điều hướng khách hàng — cùng cấu trúc [trang, biểu tượng, nhãn] với Teller/Admin (staff.js)
const NAV_CUSTOMER = [["home", "home", "Trang chủ"], ["accounts", "wallet", "Tài khoản"], ["services", "grid", "Dịch vụ"], ["user-settings", "settings", "Cài đặt"]];
// Trang con của dịch vụ → mục điều hướng cha được tô sáng
const NAV_PARENT = { transactions: "accounts", transfer: "services", topup: "services", payment: "services", savings: "services", "apply-loan": "services", "loan-pay": "services" };
const navActive = () => NAV_PARENT[state.page] || state.page;
const initials = (name) => { const w = String(name).trim().split(/\s+/); return ((w[0]?.[0] || "") + (w.length > 1 ? w[w.length - 1][0] : "")).toUpperCase(); };
const shortName = (name) => String(name).trim().split(/\s+/).slice(-2).join(" ");

function appHeader() {
  const items = NAV_CUSTOMER.map(([p, ic, t]) => `<button class="nav-button ${navActive() === p ? "active" : ""}" data-page="${p}">${icon(ic)} ${t}</button>`).join("");
  return `
    <header class="app-header">
      <div class="header-inner">
        ${brand()}
        <nav class="desktop-nav">${items}</nav>
        <div class="header-actions">
          <span class="role-chip customer">KHÁCH HÀNG</span>
          <button class="circle-button" data-page="notifications" aria-label="Thông báo">${icon("bell")}${messages.some((m) => m.unread) ? '<i class="unread"></i>' : ""}</button>
          <button class="avatar" data-action="logout" title="Đăng xuất"><b>${escHtml(initials(state.profile.name))}</b><span>${escHtml(shortName(state.profile.name))}</span></button>
        </div>
      </div>
    </header>`;
}

function mobileNav() {
  return `
    <nav class="mobile-nav">
      <button class="${state.page === "home" ? "active" : ""}" data-page="home">${icon("home")}Trang chủ</button>
      <button class="${state.page === "accounts" ? "active" : ""}" data-page="accounts">${icon("wallet")}Tài khoản</button>
      <button class="${state.page === "transfer" ? "active" : ""}" data-page="transfer">${icon("send")}Chuyển tiền</button>
      <button class="${state.page === "services" ? "active" : ""}" data-page="services">${icon("grid")}Tiện ích</button>
    </nav>`;
}

function renderHome() {
  app.innerHTML = `
    ${appHeader()}
    <main class="page-container">
      <div class="welcome">
        <div><p>${greeting()},</p><h1 class="display">${escHtml(state.profile.name)}</h1></div>
        <time>${todayText()}</time>
      </div>
      <div class="grid-main">
        <section class="balance-card">
          <span class="balance-label">Tổng số dư khả dụng</span>
          <div class="balance-line">
            <span class="balance">${state.showBalance ? vnd(state.balance) : "••••••••••"}</span>
            <button class="ghost-button" data-action="toggle-balance">${icon(state.showBalance ? "eye" : "eyeOff")}</button>
          </div>
          <div class="account-meta">
            <div><small>${escHtml(primaryAcc().name)}</small><b>${escHtml(primaryAcc().number)}</b></div>
            <div><small>Chi nhánh</small><b>${escHtml(state.profile.branch)}</b></div>
          </div>
        </section>
        <section class="surface">
          <div class="surface-title"><h2 class="display">Thao tác nhanh</h2><button class="text-action" data-page="services">Chỉnh sửa</button></div>
          <div class="quick-grid">
            ${quickButton("transfer", "send", "Chuyển tiền")}
            ${quickButton("topup", "plus", "Nạp tiền")}
            ${quickButton("payment", "card", "Thanh toán")}
            ${quickButton("transactions", "clock", "Giao dịch")}
            ${quickButton("savings", "wallet", "Tiết kiệm")}
            ${quickButton("loan", "card", "Khoản vay")}
          </div>
        </section>
      </div>
      <div class="content-grid single">
        <section class="surface promo">
          <span class="eyebrow">Tiết kiệm thông minh</span>
          <h2 class="display">Sinh lời mỗi ngày, vững vàng tương lai.</h2>
          <p>Lãi suất hấp dẫn lên đến 6,1%/năm với Tiết kiệm OOPBank.</p>
          <button class="primary" data-page="savings">Khám phá ngay ${icon("arrow")}</button>
        </section>
      </div>
    </main>
    ${mobileNav()}`;
}

function quickButton(page, iconName, label) {
  return `<button class="quick" data-page="${page}"><span>${icon(iconName)}</span>${label}</button>`;
}

function productCard(page, iconName, eyebrow, title, value, note, button, progress = false) {
  return `
    <section class="surface">
      <div class="product-head">
        <div class="product-title"><span class="icon-box">${icon(iconName)}</span><div><span class="eyebrow">${eyebrow}</span><h3>${title}</h3></div></div>
        <span class="status">${progress ? "Còn 14 kỳ" : "Đang hoạt động"}</span>
      </div>
      <p class="product-value">${value}</p><p class="product-note">${note}</p>
      ${progress ? '<div class="progress"><i></i></div>' : ""}
      <button class="secondary" data-page="${page}">${button}</button>
    </section>`;
}

const pageDetails = {
  transfer: ["Giao dịch 24/7", "Chuyển tiền", "Chuyển tiền nhanh chóng và an toàn đến mọi ngân hàng.", "send"],
  topup: ["Nạp tiền tức thì", "Nạp tiền điện thoại", "Nạp tiền cho bạn và người thân với nhiều ưu đãi.", "phone"],
  payment: ["Thanh toán hóa đơn", "Thanh toán", "Thanh toán hóa đơn Điện, Nước và Internet tại một nơi.", "card"],
  services: ["OOPBank tiện ích", "Tất cả dịch vụ", "Khám phá đầy đủ các dịch vụ tài chính dành cho bạn.", "grid"],
  accounts: ["Tài sản của tôi", "Tài khoản", "Theo dõi số dư và quản lý các tài khoản OOPBank.", "wallet"],
  notifications: ["Cập nhật mới nhất", "Thông báo", "Tin nhắn tiền vào, chuyển tiền và xác thực OTP.", "bell"],
  transactions: ["Lịch sử tài chính", "Tất cả giao dịch", "Theo dõi chi tiết biến động số dư trên tài khoản.", "grid"],
  savings: ["Tích lũy sinh lời", "Sổ tiết kiệm", "Mở sổ trực tuyến và tất toán chủ động.", "wallet"],
  loan: ["Quản lý tín dụng", "Khoản vay", "Theo dõi dư nợ và thanh toán các kỳ vay đúng hạn.", "card"],
  "apply-loan": ["Tín dụng linh hoạt", "Vay vốn", "Đăng ký, bổ sung hồ sơ, theo dõi xử lý, giải ngân và quản lý khoản vay tại một nơi.", "card"],
  "loan-pay": ["Quản lý tín dụng", "Thanh toán kỳ vay", "Thanh toán kỳ vay đến hạn từ tài khoản thanh toán.", "card"],
  "user-settings": ["An toàn & Bảo mật", "Cài đặt tài khoản", "Quản lý thông tin cá nhân, mật khẩu và mã PIN giao dịch.", "settings"],
};

function featureShell(content) {
  const detail = pageDetails[state.page] || ["OOPBank", "Dịch vụ", "Hệ thống ngân hàng số.", "grid"];
  return `
    ${appHeader()}
    <main class="feature-container">
      <button class="back" data-page="home">${icon("back")} Quay lại</button>
      <div class="feature-title"><span class="icon-box">${icon(detail[3])}</span><div><span class="eyebrow">${detail[0]}</span><h1 class="display">${detail[1]}</h1></div></div>
      <p class="feature-description">${detail[2]}</p>
      ${content}
    </main>
    ${mobileNav()}
    ${renderPinModal()}`;
}

// 1. MODAL XÁC THỰC MÃ PIN & OTP (BẢO MẬT GIAO DỊCH)
function renderPinModal() {
  if (!state.showPinModal) return "";
  const needsOtp = state.pendingTransaction && state.pendingTransaction.type === "transfer" && state.pendingTransaction.amount > 50000000;
  return `
    <div class="modal-backdrop">
      <div class="modal-card">
        <div class="modal-head">
          <span class="icon-box">${icon("shield")}</span>
          <div>
            <h3>Xác thực giao dịch</h3>
            <small>Vui lòng nhập mã PIN giao dịch để hoàn tất</small>
          </div>
        </div>
        ${pendingSummary()}
        <form id="pin-modal-form" class="form-stack">
          <div class="field">
            <label for="modal-pin">Mã PIN giao dịch (6 số)</label>
            <div class="input-wrap">
              ${icon("key")}
              <input id="modal-pin" name="pin" type="password" maxlength="6" pattern="[0-9]{6}" placeholder="••••••" required autofocus />
            </div>
          </div>
          ${needsOtp ? `
            <div class="field">
              <label for="modal-otp">Mã OTP xác thực (> 50 triệu VNĐ) · xem trong hộp thư góc dưới màn hình</label>
              <div class="input-wrap">
                ${icon("shield")}
                <input id="modal-otp" name="otp" type="text" maxlength="6" placeholder="Nhập 6 số OTP" required />
              </div>
            </div>` : ""}
          <div class="modal-actions">
            <button class="secondary" type="button" data-action="close-modal">Hủy bỏ</button>
            <button class="primary" type="submit">Xác nhận giao dịch ${icon("check")}</button>
          </div>
        </form>
      </div>
    </div>`;
}

function renderServices() {
  const services = [
    ["transfer", "send", "Chuyển tiền"], ["topup", "phone", "Nạp điện thoại"],
    ["payment", "card", "Thanh toán"], ["savings", "wallet", "Tiết kiệm"],
    ["apply-loan", "card", "Nộp đơn vay mới"], ["user-settings", "settings", "Cài đặt & Bảo mật"],
  ];
  renderFeature(services.map(([page, iconName, label]) => `
    <button class="service-item" data-page="${page}"><span class="icon-box">${icon(iconName)}</span><b>${label}</b>${icon("arrow")}</button>`).join(""), "service-grid");
}

function renderFeature(content, className = "") {
  app.innerHTML = featureShell(`<section class="${className}">${content}</section>`);
}

function renderAccounts() {
  const accounts = state.accounts.map((a) => [escHtml(a.name), escHtml(a.number), vnd(a.primary ? state.balance : a.balance), a.primary]);
  renderFeature(accounts.map(([name, number, balance, primary]) => `
    <article class="account-card ${primary ? "primary-account" : ""}">
      ${icon("wallet")}<p class="product-note">${name}</p><p class="product-value">${balance}</p><small>${number}</small>
    </article>`).join(""), "account-grid");
}

const messages = [
  { title: "Nhận tiền thành công", preview: "+5.000.000 ₫ từ TRẦN HOÀNG NAM", time: "09:42", icon: "wallet", content: "Tài khoản 0123 6868 8888 vừa nhận được 5.000.000 ₫ từ TRẦN HOÀNG NAM.", reference: "FT26020209421238" },
  { title: "Chuyển tiền thành công", preview: "-2.500.000 ₫ đến NGUYỄN MINH KHÔI", time: "Hôm qua", icon: "send", content: "Bạn đã chuyển thành công 2.500.000 ₫ đến tài khoản NGUYỄN MINH KHÔI tại OOPBank.", reference: "FT26020118201467" },
  { title: "Mã xác thực OTP", preview: "Mã OTP của bạn là 482913", time: "28/01", icon: "shield", content: "Mã OTP để xác thực giao dịch của bạn là 482913. Mã có hiệu lực trong 2 phút. Tuyệt đối không cung cấp mã này cho bất kỳ ai.", reference: "Bảo mật giao dịch" },
];

messages.forEach((m) => { m.unread = true; });
messages[2].otp = "482913";

function renderNotifications() {
  const message = messages[state.selectedMessage];
  const list = messages.map((item, index) => `
    <button class="message-item ${state.selectedMessage === index ? "active" : ""}" data-message="${index}">
      <span class="icon-box">${icon(item.icon)}</span>
      <span class="message-info"><strong>${item.title}</strong><span>${item.preview}</span></span>
      <time class="message-time">${item.time}</time>
    </button>`).join("");
  markRead(message);
  const otp = message.otp
    ? `<div class="otp">${message.otp.split("").map((number) => `<b>${number}</b>`).join("")}</div>${message.expiresAt ? otpTimer(message) : ""}` : "";
  renderFeature(`
    <section class="inbox">
      <div class="message-list"><div class="inbox-title"><h2 class="display">Hộp tin nhắn</h2><span class="badge">${messages.filter((m) => m.unread).length} chưa đọc</span></div>${list}</div>
      <article class="message-detail">
        <div class="message-detail-head"><span class="icon-box">${icon(message.icon)}</span><div><span class="eyebrow">OOPBank thông báo</span><h2 class="display">${message.title}</h2></div></div>
        <div class="message-body">${message.content}${otp}</div>
        <footer class="message-footer"><div><span class="eyebrow">Mã tham chiếu</span><br /><strong>${message.reference}</strong></div><button class="secondary" style="width:auto;margin:0">Đánh dấu đã đọc</button></footer>
      </article>
    </section>`);
}

function renderTransactions() {
  renderFeature(`<div>${transactionRows(true)}</div>`, "surface");
}

function formField(id, label, placeholder, iconName = "card", type = "text") {
  return `<div class="field"><label for="${id}">${label}</label><div class="input-wrap">${icon(iconName)}<input id="${id}" name="${id}" type="${type}" placeholder="${placeholder}" required /></div></div>`;
}

// 4. TRANG CÀI ĐẶT HỒ SƠ VÀ BẢO MẬT (USER SERVICE)
function renderUserSettings() {
  const content = `
    <div class="grid-main">
      <section class="form-card">
        <h2 class="display">Cập nhật hồ sơ cá nhân</h2>
        <form id="update-profile-form">
          ${field("fullname", "Họ và tên", "user", "Họ và tên", "text", escHtml(state.profile.name))}
          ${field("email", "Địa chỉ Email", "mail", "email@example.com", "email", escHtml(state.profile.email))}
          ${field("phone", "Số điện thoại", "phone", "0901234567", "tel", escHtml(state.profile.phone))}
          <button class="primary" type="submit">Lưu thay đổi ${icon("check")}</button>
        </form>
      </section>

      <section class="form-card">
        <h2 class="display">Đổi mật khẩu & Mã PIN</h2>
        <form id="change-pass-form" class="form-stack">
          <span class="eyebrow">Mật khẩu đăng nhập</span>
          ${field("old_password", "Mật khẩu hiện tại", "lock", "••••••••", "password")}
          ${field("new_password", "Mật khẩu mới", "lock", "Tối thiểu 8 ký tự", "password")}
          <button class="secondary" type="submit">Cập nhật mật khẩu</button>
        </form>
        <hr class="divider" />
        <form id="change-pin-form" class="form-stack">
          <span class="eyebrow">Mã PIN giao dịch (6 số)</span>
          ${field("old_pin", "Mã PIN hiện tại", "key", "••••••", "password")}
          ${field("new_pin", "Mã PIN mới 6 số", "key", "••••••", "password")}
          <button class="secondary" type="submit">Đổi mã PIN</button>
        </form>
      </section>
    </div>`;
  renderFeature(content);
}

function renderTransactionForm() {
  if (state.transactionSuccess) {
    const content = `
      <div class="success"><span class="success-icon">${icon("check")}</span><h2 class="display">Giao dịch thành công</h2><p>Yêu cầu của bạn đã được C++ Backend ghi nhận và xử lý thành công.</p><button class="primary" data-page="home">Về trang chủ</button>${state.page === "apply-loan" ? '<button class="secondary" data-loan-tab="review">Theo dõi xử lý hồ sơ</button>' : ""}</div>`;
    app.innerHTML = featureShell(`<div class="feature-layout"><section class="form-card">${content}</section>${sourceCard()}</div>`);
    return;
  }

  let fields = "";
  let heading = "";

  if (state.page === "transfer") {
    heading = "Thông tin người nhận";
    fields = formField("receiver", "Số tài khoản người nhận", "Nhập số tài khoản", "user") + formField("bank", "Ngân hàng thụ hưởng", "Nhập tên ngân hàng", "shield");
  } else if (state.page === "topup") {
    heading = "Thông tin thuê bao";
    fields = formField("topup-phone", "Số điện thoại", "0901 234 567", "phone", "tel");
  } else if (state.page === "payment") {
    heading = "Thanh toán hóa đơn";
    fields = billSelector() + providerSelector() + formField("bill-code", "Mã khách hàng / Hóa đơn", "Nhập mã tra cứu", "card");
  } 
  // 2. FORM MỞ SỔ TIẾT KIỆM (KÈM PHƯƠNG THỨC ĐÁO HẠN & TẤT TOÁN)
  else if (state.page === "savings") {
    heading = "Mở sổ tiết kiệm mới";
    fields = formField("saving-amount", "Số tiền gửi (VNĐ)", "Tối thiểu 1.000.000", "wallet", "number") + `
      <div class="field">
        <label for="term">Kỳ hạn gửi</label>
        <div class="input-wrap"><select id="term" name="term"><option value="3">3 tháng · 4,5%/năm</option><option value="6" selected>6 tháng · 5,8%/năm</option><option value="12">12 tháng · 6,1%/năm</option></select></div>
      </div>
      <div class="field">
        <label for="maturity-option">Phương thức đáo hạn</label>
        <div class="input-wrap">
          <select id="maturity-option" name="maturity_option">
            <option value="RENEW_ALL">Tự động tái tục cả gốc và lãi (RENEW_ALL)</option>
            <option value="RENEW_PRINCIPAL">Tái tục gốc, lãi chuyển về TK thanh toán (RENEW_PRINCIPAL)</option>
            <option value="CLOSE_TO_ACCOUNT">Tất toán toàn bộ về TK thanh toán khi hết hạn (CLOSE_TO_ACCOUNT)</option>
          </select>
        </div>
      </div>`;
  } 
  // 3. FORM NỘP ĐƠN XIN VAY (APPLY LOAN)
  else if (state.page === "apply-loan") {
    heading = "Đăng ký khoản vay mới";
    fields = formField("loan-amount", "Số tiền cần vay (VNĐ)", "Ví dụ: 50000000", "wallet", "number") + `
      <div class="field">
        <label for="loan-term">Kỳ hạn vay (tháng)</label>
        <div class="input-wrap"><select id="loan-term" name="loan_term">${termOptions()}</select></div>
      </div>
      <div class="field">
        <label for="calc-method">Phương thức trả nợ</label>
        <div class="input-wrap">
          <select id="calc-method" name="calc_method">
            <option value="DECREASING">Dư nợ giảm dần</option>
            <option value="FIXED">Niên kim cố định (Gốc + Lãi chia đều)</option>
          </select>
        </div>
      </div>` + formField("collateral", "Thông tin tài sản đảm bảo", "Ví dụ: Bảng lương / Giấy tờ xe / Bất động sản", "shield");
  } else {
    heading = "Thanh toán kỳ vay";
  }

  const needsAmount = !["payment", "savings", "apply-loan"].includes(state.page);
  const amount = needsAmount ? formField("amount", state.page === "loan-pay" ? "Số tiền thanh toán" : "Số tiền (VNĐ)", "0 VND", "wallet", "number") : "";

  // Bổ sung nút tất toán sổ tiết kiệm trực tiếp
  const closeSavingsAction = state.page === "savings" ? `
    <div class="close-savings-box">
      <hr />
      <span class="eyebrow">Sổ tiết kiệm đang hoạt động</span>
      ${state.savings.filter((item) => item.status === "ACTIVE").map((item) => `
        <div class="row">
          <div class="info"><p>${escHtml(item.savings_id)}</p><small>${vnd(item.principal_vnd)} · ${item.term_months} tháng · ${item.interest_rate}%/năm</small></div>
          <button class="secondary" type="button" data-action="close-savings-request" data-savings-id="${escHtml(item.savings_id)}">Tất toán</button>
        </div>`).join("") || `<p class="product-note">Chưa có sổ tiết kiệm đang hoạt động.</p>`}
    </div>` : "";

  const content = `
    ${state.page === "apply-loan" ? loanTabs() : ""}
    <div class="feature-layout">
      <section class="form-card">
        <h2 class="display">${heading}</h2>
        <form id="transaction-form">
          ${fields}${amount}
          <button class="primary" type="submit">Tiếp tục xác thực ${icon("arrow")}</button>
        </form>
        ${closeSavingsAction}
      </section>
      ${sourceCard()}
    </div>`;
  app.innerHTML = featureShell(content);
}

function billSelector() {
  return `
    <div class="field"><span class="field-label">Chọn loại hóa đơn</span>
      <div class="bill-types">
        ${billButton("electricity", "plus", "Điện")}
        ${billButton("water", "wallet", "Nước")}
        ${billButton("internet", "grid", "Internet")}
      </div>
    </div>`;
}

function billButton(type, iconName, label) {
  return `<button class="bill-type ${state.billType === type ? "active" : ""}" type="button" data-bill="${type}">${icon(iconName)}${label}</button>`;
}

function providersForBillType() {
  const serviceType = { electricity: "ELECTRICITY", water: "WATER", internet: "INTERNET" }[state.billType];
  return state.billProviders.filter((provider) => String(provider.service_type).toUpperCase() === serviceType);
}

function providerSelector() {
  const fallback = {
    electricity: ["EVN Hà Nội", "EVN TP. Hồ Chí Minh", "EVN Miền Trung"],
    water: ["Nước sạch Hà Nội", "SAWACO", "Cấp nước Đà Nẵng"],
    internet: ["Viettel Internet", "VNPT", "FPT Telecom"],
  };
  const options = hasBackend()
    ? providersForBillType().map((provider) => `<option value="${escHtml(provider.provider_id)}">${escHtml(provider.provider_name)}</option>`).join("")
    : fallback[state.billType].map((name) => `<option value="${escHtml(name)}">${escHtml(name)}</option>`).join("");
  const placeholder = hasBackend() && !state.billProvidersLoaded
    ? "<option value=''>Đang tải nhà cung cấp…</option>"
    : hasBackend() && !options
      ? `<option value="">${state.billProvidersError ? "Không tải được nhà cung cấp" : "Database chưa có nhà cung cấp cho loại hóa đơn này"}</option>`
      : options;
  return `
    <div class="field"><label for="provider">Nhà cung cấp</label><div class="input-wrap">
      <select id="provider" name="provider" ${hasBackend() && !options ? "disabled" : ""}>${placeholder}</select>
    </div></div>`;
}

async function loadBillProviders() {
  const result = await callBackend("billBridge", "getProviders", { session_token: state.sessionToken });
  state.billProviders = result.providers || [];
  state.billProvidersLoaded = true;
  state.billProvidersError = "";
  if (state.page === "payment") {
    const customerCode = document.querySelector("#bill-code")?.value || "";
    render();
    const codeInput = document.querySelector("#bill-code");
    if (codeInput) codeInput.value = customerCode;
  }
}

function ensureBillProviders() {
  if (state.page !== "payment" || !hasBackend() || state.billProvidersLoaded) return;
  state.billProvidersLoaded = true;
  loadBillProviders().catch((error) => {
    state.billProvidersError = error?.message || String(error);
    if (state.page === "payment") {
      const customerCode = document.querySelector("#bill-code")?.value || "";
      render();
      const codeInput = document.querySelector("#bill-code");
      if (codeInput) codeInput.value = customerCode;
    }
    showToast(`Không tải được nhà cung cấp hóa đơn: ${state.billProvidersError}`);
  });
}

function sourceCard() {
  return `<aside class="source-card"><span class="eyebrow">Nguồn tiền</span><p>Tài khoản thanh toán</p><b>0123 6868 8888</b><hr /><p>Số dư khả dụng</p><strong>${vnd(state.balance)}</strong></aside>`;
}

// ===== HỘP THƯ NỔI (góc dưới màn hình): nhận tiền · chuyển tiền · mã OTP (mô phỏng) =====
const msgHost = document.createElement("div");
msgHost.id = "msgbox";
document.body.append(msgHost);

const SENDERS = ["TRẦN HOÀNG NAM", "LÊ THU HÀ", "PHẠM QUỐC BẢO", "VŨ THANH TÂM", "ĐỖ MINH KHOA"];
const RECEIVERS = ["NGUYỄN MINH KHÔI", "BÙI NGỌC LAN", "HOÀNG ĐỨC ANH", "TRƯƠNG GIA HÂN"];
const pick = (list) => list[Math.floor(Math.random() * list.length)];
const makeRef = () => `FT${Date.now().toString().slice(-14)}`;
const stampShort = () => { const d = new Date(); return `${pad(d.getDate())}/${pad(d.getMonth() + 1)}, ${nowHM()}`; };
const timerText = (exp) => { const r = Math.ceil((exp - Date.now()) / 1000); return r <= 0 ? "Mã đã hết hạn" : `Hiệu lực còn ${Math.floor(r / 60)}:${pad(r % 60)}`; };
const otpTimer = (m) => `<small class="otp-timer" data-exp="${m.expiresAt}">${timerText(m.expiresAt)}</small>`;
const msgIcon = (m) => icon(m.icon || "bell");

let bannerTimer;
function showBanner(m) {
  state.msgBanner = m;
  clearTimeout(bannerTimer);
  bannerTimer = setTimeout(() => { state.msgBanner = null; renderMsgbox(); }, 4500);
  bannerTimer?.unref?.();
}

function pushMessage(m) {
  messages.unshift({ unread: true, time: "Vừa xong", ...m });
  state.selectedMessage = 0;
  if (state.msgOpen) state.msgExpanded = messages[0];
  else showBanner(messages[0]);
  renderMsgbox();
}

function recordPayment(tx) {
  const to = escHtml(tx.data?.receiver || tx.data?.["topup-phone"] || "");
  state.balance -= tx.amount;
  if (tx.type === "transfer") {
    transactions.unshift([`Chuyển đến TK ${to}`, `Chuyển tiền · ${stampShort()}`, `-${vnd(tx.amount)}`, "send"]);
    pushMessage({ title: "Chuyển tiền thành công", preview: `-${vnd(tx.amount)} đến TK ${to}`, icon: "send", content: `Bạn đã chuyển thành công ${vnd(tx.amount)} đến tài khoản ${to}. Số dư khả dụng: ${vnd(state.balance)}.`, reference: makeRef() });
  } else {
    transactions.unshift([`Nạp tiền ${to}`, `Nạp tiền · ${stampShort()}`, `-${vnd(tx.amount)}`, "phone"]);
    pushMessage({ title: "Nạp tiền điện thoại thành công", preview: `-${vnd(tx.amount)} · ${to}`, icon: "phone", content: `Bạn đã nạp ${vnd(tx.amount)} cho số ${to}. Số dư khả dụng: ${vnd(state.balance)}.`, reference: makeRef() });
  }
}

function sendOtp(label = "xác thực giao dịch") {
  const code = String(Math.floor(100000 + Math.random() * 900000));
  pushMessage({ title: "Mã xác thực OTP", preview: `Mã OTP của bạn là ${code}`, icon: "shield", content: `Mã OTP để ${label} của bạn là ${code}. Mã có hiệu lực trong 2 phút. Tuyệt đối không cung cấp mã này cho bất kỳ ai.`, reference: "Bảo mật giao dịch", otp: code, expiresAt: Date.now() + 120000 });
}

function maybeSendOtp() {
  const tx = state.pendingTransaction;
  if (tx && tx.type !== "DISBURSE" && tx.amount > 50000000) sendOtp("xác thực giao dịch");
}

// Mô phỏng thông báo từ hệ thống (nút trong hộp thư)
function simulate(kind) {
  if (kind === "receive") {
    const amount = pick([500000, 1000000, 2000000, 5000000, 12000000]);
    const from = pick(SENDERS);
    state.balance += amount;
    transactions.unshift([from, `Tiền vào · ${stampShort()}`, `+${vnd(amount)}`, "wallet"]);
    pushMessage({ title: "Nhận tiền thành công", preview: `+${vnd(amount)} từ ${from}`, icon: "wallet", content: `Tài khoản 0123 6868 8888 vừa nhận được ${vnd(amount)} từ ${from}. Số dư khả dụng: ${vnd(state.balance)}.`, reference: makeRef() });
  } else if (kind === "transfer") {
    const amount = pick([250000, 500000, 1500000, 2500000]);
    const to = pick(RECEIVERS);
    if (amount > state.balance) return showToast("Số dư khả dụng không đủ để mô phỏng chuyển tiền");
    state.balance -= amount;
    transactions.unshift([`Chuyển đến ${to}`, `Chuyển tiền · ${stampShort()}`, `-${vnd(amount)}`, "send"]);
    pushMessage({ title: "Chuyển tiền thành công", preview: `-${vnd(amount)} đến ${to}`, icon: "send", content: `Bạn đã chuyển thành công ${vnd(amount)} đến tài khoản ${to} tại OOPBank. Số dư khả dụng: ${vnd(state.balance)}.`, reference: makeRef() });
  } else {
    sendOtp("xác thực giao dịch");
  }
  // cập nhật số dư đang hiển thị, tránh dựng lại trang có form đang nhập dở
  if (["home", "accounts", "transactions", "notifications"].includes(state.page)) renderPage();
}

function renderMsgbox() {
  if (!state.loggedIn || state.role !== "CUSTOMER") { msgHost.innerHTML = ""; return; }
  const unread = messages.filter((m) => m.unread).length;
  const detail = (m) => `<div class="msg-detail"><p>${m.content}</p>${m.otp ? `<div class="otp">${m.otp.split("").map((n) => `<b>${n}</b>`).join("")}</div>${m.expiresAt ? otpTimer(m) : ""}` : ""}<small>Mã tham chiếu: ${m.reference}</small></div>`;
  const items = messages.map((m) => `
    <div class="msg-entry ${m.unread ? "unread" : ""}">
      <button class="msg-item" data-msg-open="${messages.indexOf(m)}" aria-expanded="${state.msgExpanded === m}">
        <span class="icon-box">${msgIcon(m)}</span>
        <span class="msg-text"><strong>${m.title}</strong><span>${m.preview}</span></span>
        <time>${m.time}</time>
      </button>${state.msgExpanded === m ? detail(m) : ""}
    </div>`).join("");
  const banner = state.msgBanner && !state.msgOpen
    ? `<button class="msg-banner" data-msg="openpanel"><span class="icon-box">${msgIcon(state.msgBanner)}</span><span class="msg-text"><strong>${state.msgBanner.title}</strong><span>${state.msgBanner.preview}</span></span></button>` : "";
  const panel = state.msgOpen ? `
    <section class="msg-panel" role="dialog" aria-label="Hộp thư OOPBank">
      <header><div><b>Hộp thư OOPBank</b><small>${unread ? `${unread} tin chưa đọc` : "Đã đọc tất cả"}</small></div>
        <div class="msg-head-actions"><button class="msg-link" data-msg="readall">Đọc tất cả</button><button class="msg-close" data-msg="toggle" aria-label="Đóng hộp thư">×</button></div></header>
      <div class="msg-list">${items}</div>
      <footer><span class="msg-sim-label">Mô phỏng thông báo</span>
        <div class="msg-sim"><button data-msg-sim="receive">${icon("wallet")} Nhận tiền</button><button data-msg-sim="transfer">${icon("send")} Chuyển tiền</button><button data-msg-sim="otp">${icon("shield")} Mã OTP</button></div>
        <button class="msg-link" data-page="notifications">Xem tất cả thông báo</button></footer>
    </section>` : "";
  const list = msgHost.querySelector(".msg-list");
  const keep = list ? list.scrollTop : 0;
  msgHost.innerHTML = `${banner}${panel}<button class="msg-fab" data-msg="toggle" aria-label="Hộp thư${unread ? `, ${unread} tin chưa đọc` : ""}" aria-expanded="${state.msgOpen}">${icon("mail")}${unread ? `<i class="msg-count">${unread}</i>` : ""}</button>`;
  const fresh = msgHost.querySelector(".msg-list");
  if (fresh) fresh.scrollTop = keep;
}

msgHost.addEventListener("click", (event) => {
  const t = event.target;
  const sim = t.closest("[data-msg-sim]");
  const item = t.closest("[data-msg-open]");
  const nav = t.closest("[data-page]");
  const act = t.closest("[data-msg]")?.dataset.msg;
  if (sim) return simulate(sim.dataset.msgSim);
  if (item) {
    const m = messages[Number(item.dataset.msgOpen)];
    state.msgExpanded = state.msgExpanded === m ? null : m;
    markRead(m);
    return render();
  }
  if (nav) {
    state.page = nav.dataset.page;
    state.transactionSuccess = false;
    state.msgOpen = false;
    window.scrollTo({ top: 0, behavior: "smooth" });
    render();
    ensureBillProviders();
    return loadCurrent();
  }
  if (act === "toggle") { state.msgOpen = !state.msgOpen; state.msgBanner = null; }
  else if (act === "openpanel") { state.msgOpen = true; state.msgBanner = null; }
  else if (act === "readall") messages.forEach(markRead);
  render();
});

// đồng hồ đếm ngược hiệu lực OTP
const otpClock = setInterval(() => {
  document.querySelectorAll("[data-exp]").forEach((el) => { el.textContent = timerText(Number(el.dataset.exp)); });
}, 1000);
otpClock?.unref?.();

// ===== VAY VỐN: đăng ký · bổ sung hồ sơ · xử lý · giải ngân · quản lý (dữ liệu MOCK, dùng chung với GDV/Admin) =====
const vnd = (n) => `${Number(n).toLocaleString("vi-VN")} ₫`;
const pad = (n) => String(n).padStart(2, "0");
const fmtDate = (iso) => iso.split("-").reverse().join("/");
const nowHM = () => new Date().toTimeString().slice(0, 5);
const nowFull = () => { const d = new Date(); return `${pad(d.getDate())}/${pad(d.getMonth() + 1)}/${d.getFullYear()} ${nowHM()}`; };
const fmtSize = (b) => (b >= 1048576 ? `${(b / 1048576).toFixed(1).replace(".", ",")} MB` : `${Math.max(1, Math.round(b / 1024))} KB`);
const escHtml = (v) => String(v).replace(/[&<>"']/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[c]));

function showToast(message) {
  const el = document.createElement("div");
  el.className = "toast";
  el.textContent = message;
  document.body.append(el);
  setTimeout(() => el.remove(), 2400);
}

const CUSTOMER_NAME = "Nguyễn Minh Anh";
const LOAN_RATE = 12; // %/năm (mock) — lãi suất mặc định cho hồ sơ cũ
// Cấu hình kỳ hạn vay (Admin thiết lập ở mục Cấu hình): chỉ các kỳ hạn đang bật mới hiện ở form đăng ký vay
const loanTerms = [
  { months: 3, enabled: true, rate: 10.5 },
  { months: 6, enabled: true, rate: 11 },
  { months: 12, enabled: true, rate: 12 },
];
const termRate = (months) => loanTerms.find((t) => t.months === months)?.rate ?? LOAN_RATE;
const termOptions = () => {
  const on = loanTerms.filter((t) => t.enabled);
  return on.map((t, i) => `<option value="${t.months}" ${i === on.length - 1 ? "selected" : ""}>${t.months} tháng · ${t.rate}%/năm</option>`).join("");
};
const LOAN_STEPS = ["Tiếp nhận hồ sơ", "Thẩm định", "Phê duyệt", "Giải ngân"];
const LOAN_TABS = [["apply", "Đăng ký vay"], ["docs", "Bổ sung hồ sơ"], ["review", "Xử lý hồ sơ"], ["disburse", "Giải ngân"], ["manage", "Quản lý vay"]];
const DOC_STATUS = { DONE: ["Đã xác nhận", "ok"], REVIEW: ["Đang kiểm tra", "role"], MISSING: ["Cần bổ sung", "warn"] };
const NOTICE = {
  APPROVED: ["ok", "Hồ sơ đã được phê duyệt"],
  REJECTED: ["warn", "Hồ sơ bị từ chối"],
  INFO: ["warn", "Ngân hàng yêu cầu bổ sung hồ sơ"],
  SUBMITTED: ["role", "Đã gửi bổ sung hồ sơ"],
};

// Bộ giấy tờ chuẩn; statuses theo thứ tự: CCCD, thu nhập, tài sản đảm bảo, cư trú
const DOC_BASE = [
  ["cccd", "shield", "CCCD gắn chip (2 mặt)"],
  ["income", "wallet", "Sao kê lương / HĐLĐ 3 tháng gần nhất"],
  ["collateral", "file", "Giấy tờ tài sản đảm bảo"],
  ["address", "home", "Xác nhận nơi cư trú"],
];
const DOC_HINT = { DONE: "Đã được ngân hàng xác nhận", REVIEW: "Đã gửi, đang chờ kiểm tra", MISSING: "Chưa có tệp" };
// Tệp mô phỏng cho các giấy tờ đã nộp: [tên tệp, loại, dung lượng]
const DOC_FILE = {
  cccd: ["cccd-2-mat.jpg", "image/jpeg", "1,8 MB"],
  income: ["sao-ke-luong-3-thang.pdf", "application/pdf", "642 KB"],
  collateral: ["giay-to-tai-san.pdf", "application/pdf", "2,3 MB"],
  address: ["xac-nhan-cu-tru.pdf", "application/pdf", "310 KB"],
};
const docsWith = (statuses) => DOC_BASE.map(([id, ic, name], i) => {
  const has = statuses[i] !== "MISSING";
  const [file, mime, size] = has ? DOC_FILE[id] : ["", "", ""];
  return { id, icon: ic, name, status: statuses[i], hint: DOC_HINT[statuses[i]], file, mime, size, at: has ? "14/10/2026 08:40" : "", url: "" };
});

// stage = số bước đã hoàn tất: 0..2 đang xử lý, 3 = đã duyệt chờ giải ngân, 4 = đã giải ngân, -1 = bị từ chối
const loanApps = [
  { id: "LN20261014001", who: CUSTOMER_NAME, phone: "0901 234 567", type: "Vay tiêu dùng", amt: 50000000, term: 24, rate: LOAN_RATE, method: "DECREASING", purpose: "Mua sắm nội thất", collateral: "Bảng lương", income: "18.000.000 ₫/tháng", stage: 1, needInfo: false, resubmitted: false, result: null, docs: docsWith(["DONE", "MISSING", "MISSING", "REVIEW"]) },
  { id: "LN20261016003", who: CUSTOMER_NAME, phone: "0901 234 567", type: "Vay tiêu dùng", amt: 120000000, term: 36, rate: LOAN_RATE, method: "FIXED", purpose: "Sửa nhà", collateral: "Giấy tờ xe", income: "18.000.000 ₫/tháng", stage: 3, needInfo: false, resubmitted: false, result: { type: "APPROVED", note: "Khoản vay đã được phê duyệt. Vui lòng vào mục Giải ngân để nhận tiền.", by: "Quản trị viên", time: "09:12" }, docs: docsWith(["DONE", "DONE", "DONE", "DONE"]) },
  { id: "LN20261015002", who: "Phạm Quốc Bảo", phone: "0966 777 888", type: "Vay thế chấp", amt: 900000000, term: 180, rate: 9.5, method: "DECREASING", purpose: "Mua căn hộ", collateral: "Sổ hồng căn hộ Q.7", income: "65.000.000 ₫/tháng", stage: 1, needInfo: false, resubmitted: false, result: null, docs: docsWith(["DONE", "DONE", "REVIEW", "DONE"]) },
  { id: "LN20261012003", who: "Lê Thu Hà", phone: "0933 456 789", type: "Vay tiêu dùng", amt: 20000000, term: 12, rate: LOAN_RATE, method: "FIXED", purpose: "Học phí", collateral: "Bảng lương", income: "12.000.000 ₫/tháng", stage: 4, needInfo: false, resubmitted: false, result: { type: "APPROVED", note: "Đã phê duyệt và giải ngân.", by: "Quản trị viên", time: "09:12" }, docs: docsWith(["DONE", "DONE", "DONE", "DONE"]) },
];
const activeLoans = [
  { id: "LN20250811007", type: "Vay tiêu dùng", principal: 60000000, outstanding: 45600000, rate: 12, total: 24, left: 14, pay: 3250000, due: "2026-03-05" },
];

const mine = () => loanApps.filter((a) => a.who === state.profile.name);
const missingDocs = (a) => a.docs.filter((d) => d.status === "MISSING").length;
const openApps = () => mine().filter((a) => a.stage >= 0 && a.stage < 2);
const docApp = () => { const list = openApps(); return list.find((a) => a.id === state.docAppId) || list[0]; };
const monthlyPay = (a) => Math.round(a.amt / a.term + (a.amt * (a.rate || LOAN_RATE)) / 1200);
const appChip = (a) => (a.stage < 0 ? ["Bị từ chối", "warn"] : a.stage === 3 ? ["Chờ giải ngân", "ok"] : a.needInfo ? ["Cần bổ sung", "warn"] : [LOAN_STEPS[a.stage], "role"]);
const emptyCard = (text) => `<section class="form-card"><p class="product-note">${text}</p></section>`;

function noticeHtml(a) {
  if (!a.result) return "";
  const [cls, title] = NOTICE[a.result.type];
  return `<div class="notice ${cls}"><b>${title}</b><p>${escHtml(a.result.note)}</p><small>${a.result.by ? `${escHtml(a.result.by)} · ` : ""}${a.result.time}</small></div>`;
}

// Gửi kết quả / yêu cầu của GDV-Admin về hộp thông báo của khách hàng
function notifyCustomer(a, kind, note) {
  if (a.who !== state.profile.name) return;
  pushMessage({ title: NOTICE[kind][1], preview: `${a.id} · ${vnd(a.amt)}`, icon: kind === "APPROVED" ? "check" : "bell", content: `${escHtml(note)} (Khoản vay ${a.id} · ${vnd(a.amt)}).`, reference: a.id });
}

function loanTabs() {
  const badge = {
    docs: openApps().reduce((n, a) => n + missingDocs(a), 0),
    disburse: mine().filter((a) => a.stage === 3).length,
  };
  return `<div class="loan-tabs" role="tablist">${LOAN_TABS.map(([key, label]) => `
    <button class="tab ${state.loanTab === key ? "active" : ""}" role="tab" aria-selected="${state.loanTab === key}" data-loan-tab="${key}">${label}${badge[key] ? `<i class="tab-badge">${badge[key]}</i>` : ""}</button>`).join("")}</div>`;
}

function loanDocsView() {
  const target = docApp();
  if (!target) return emptyCard("Hiện không có hồ sơ nào cần bổ sung giấy tờ.");
  const list = openApps();
  const picker = list.length > 1 ? `<div class="loan-tabs sub">${list.map((a) => `<button class="tab ${a.id === target.id ? "active" : ""}" data-doc-app="${a.id}">${a.id}</button>`).join("")}</div>` : "";
  const missing = missingDocs(target);
  const notice = target.result?.type === "INFO" ? noticeHtml(target) : "";
  const rows = target.docs.map((d) => {
    const [text, cls] = DOC_STATUS[d.status];
    const upload = d.status === "DONE" || hasBackend() ? "" : `<label class="btn-sm upload">${d.status === "MISSING" ? "Tải lên" : "Thay tệp"}<input type="file" accept=".pdf,.jpg,.jpeg,.png" data-doc="${d.id}" hidden /></label>`;
    return `<div class="row"><span class="icon-box">${icon(d.icon)}</span>
      <div class="info"><p>${escHtml(d.name)}</p><small>${d.file ? escHtml(d.file) : d.hint}</small></div>
      <span class="chip ${cls}">${text}</span>${upload}</div>`;
  }).join("");
  return `
    ${picker}${notice}
    <div class="feature-layout">
      <section class="form-card">
        <h2 class="display">Bổ sung hồ sơ vay</h2>
        <p class="product-note">Hồ sơ ${target.id} · ${vnd(target.amt)} · ${target.term} tháng</p>
        ${hasBackend() ? '<p class="notice warn">Database chưa có nơi lưu tệp hồ sơ vay. Chức năng tải lên đang tắt để tránh báo đã nộp khi tệp chỉ nằm trong bộ nhớ trình duyệt.</p>' : ""}
        <div class="doc-list">${rows}</div>
        <button class="primary" data-action="submit-docs" ${missing || hasBackend() ? "disabled" : ""}>Gửi hồ sơ bổ sung ${icon("arrow")}</button>
      </section>
      <aside class="source-card"><span class="eyebrow">Tiến độ hồ sơ</span><p>Đã nộp</p><strong>${target.docs.length - missing}/${target.docs.length} giấy tờ</strong><hr /><p>${hasBackend() ? "Chờ cấu hình kho tài liệu để nộp hồ sơ trực tuyến." : "Định dạng PDF, JPG, PNG · tối đa 5 MB mỗi tệp. Hồ sơ đủ giấy tờ sẽ được chuyển sang bước thẩm định."}</p></aside>
    </div>`;
}

function loanReviewView() {
  const apps = mine().filter((a) => a.stage < 4);
  if (!apps.length) return emptyCard("Bạn chưa có hồ sơ vay nào đang xử lý.");
  const notes = [
    "Hồ sơ đã được ghi nhận, đang chờ phân công chuyên viên.",
    "Chuyên viên đang thẩm định thu nhập và tài sản đảm bảo. Dự kiến 1–2 ngày làm việc.",
    "Hồ sơ đang chờ cấp có thẩm quyền phê duyệt.",
    "Khoản vay đã được phê duyệt. Bạn có thể xác nhận giải ngân.",
  ];
  return `<div class="stack">${apps.map((a) => {
    const [ct, cc] = appChip(a);
    const timeline = a.stage < 0 ? "" : `<ol class="timeline">${LOAN_STEPS.map((s, i) => `<li class="${i < a.stage ? "done" : i === a.stage ? "current" : ""}"><span class="dot">${i < a.stage ? icon("check") : i + 1}</span><b>${s}</b></li>`).join("")}</ol>`;
    const missing = missingDocs(a);
    const action = a.stage === 3
      ? '<button class="btn-sm" data-loan-tab="disburse">Đi tới giải ngân</button>'
      : a.stage >= 0 && missing ? `<button class="btn-sm" data-loan-tab="docs" data-doc-app="${a.id}">Bổ sung hồ sơ (${missing})</button>` : "";
    return `<section class="form-card">
      <div class="loan-head"><div><span class="eyebrow">${a.id}</span><h2 class="display">${vnd(a.amt)}</h2><p class="product-note">${a.type} · ${a.term} tháng</p></div><span class="chip ${cc}">${ct}</span></div>
      ${noticeHtml(a)}${timeline}
      ${a.stage >= 0 ? `<p class="product-note">${notes[a.stage]}</p>` : ""}${action ? `<div class="loan-actions">${action}</div>` : ""}
    </section>`;
  }).join("")}</div>`;
}

function loanDisburseView() {
  const ready = mine().filter((a) => a.stage === 3);
  if (!ready.length) return emptyCard("Chưa có khoản vay nào đủ điều kiện giải ngân. Khoản vay sẽ xuất hiện tại đây sau khi được phê duyệt.");
  const cards = ready.map((a) => `
    <section class="form-card">
      <div class="loan-head"><div><span class="eyebrow">${a.id}</span><h2 class="display">${vnd(a.amt)}</h2><p class="product-note">${a.type} · đã được phê duyệt</p></div><span class="chip ok">Chờ giải ngân</span></div>
      <form id="disburse-form">
        <input type="hidden" name="loanId" value="${a.id}" />
        <div class="loan-facts">
          <div><small>Kỳ hạn</small><b>${a.term} tháng</b></div>
          <div><small>Lãi suất</small><b>${a.rate}%/năm</b></div>
          <div><small>Trả hàng tháng (ước tính)</small><b>${vnd(monthlyPay(a))}</b></div>
          <div><small>Phí giải ngân</small><b>0 ₫</b></div>
        </div>
        <div class="field"><label for="dest-${a.id}">Tài khoản nhận tiền</label><div class="input-wrap"><select id="dest-${a.id}" name="dest">${state.accounts.map((account) => `<option value="${escHtml(account.id)}">${escHtml(account.number)} · ${escHtml(account.name)}</option>`).join("")}</select></div></div>
        <button class="primary" type="submit">Xác nhận giải ngân ${icon("arrow")}</button>
      </form>
    </section>`).join("");
  return `
    <div class="feature-layout">
      <div class="stack">${cards}</div>
      <aside class="source-card"><span class="eyebrow">Tài khoản nhận</span><p>Tài khoản thanh toán</p><b>0123 6868 8888</b><hr /><p>Số dư hiện tại</p><strong>${vnd(state.balance)}</strong></aside>
    </div>`;
}

function scheduleRows(l) {
  const [y, m, d] = l.due.split("-").map(Number);
  return Array.from({ length: Math.min(4, l.left) }, (_, i) => {
    const dt = new Date(y, m - 1 + i, d);
    return `<div class="row"><span class="icon-box">${icon("clock")}</span><div class="info"><p>Kỳ ${l.total - l.left + 1 + i}/${l.total}</p><small>${pad(dt.getDate())}/${pad(dt.getMonth() + 1)}/${dt.getFullYear()}</small></div><b class="amount">${vnd(l.pay)}</b></div>`;
  }).join("");
}

function loanManageView() {
  const loans = activeLoans.map((l) => {
    const pct = Math.round(((l.total - l.left) / l.total) * 100);
    const open = state.openSchedule === l.id;
    return `<section class="surface">
      <div class="product-head"><div class="product-title"><span class="icon-box">${icon("card")}</span><div><span class="eyebrow">${l.id}</span><h3>${l.type}</h3></div></div><span class="status">Còn ${l.left} kỳ</span></div>
      <p class="product-value">${vnd(l.outstanding)}</p>
      <p class="product-note">Dư nợ gốc còn lại · Gốc ban đầu ${vnd(l.principal)} · Lãi ${l.rate}%/năm</p>
      <div class="progress"><i style="width:${pct}%"></i></div>
      <p class="product-note">Đã trả ${l.total - l.left}/${l.total} kỳ · Kỳ tới ${vnd(l.pay)} ngày ${fmtDate(l.due)}</p>
      <div class="loan-actions"><button class="primary" data-page="loan-pay">Thanh toán kỳ này</button><button class="secondary" data-action="toggle-schedule" data-id="${l.id}">${open ? "Ẩn" : "Xem"} lịch trả nợ</button></div>
      ${open ? `<div class="schedule">${scheduleRows(l)}</div>` : ""}
    </section>`;
  }).join("");
  const pending = mine().filter((a) => a.stage < 4).map((a) => {
    const [ct, cc] = appChip(a);
    return `<div class="row"><span class="icon-box">${icon("file")}</span>
      <div class="info"><p>${vnd(a.amt)} · ${a.term} tháng</p><small>${a.id} · ${a.type}</small></div>
      <span class="chip ${cc}">${ct}</span>
      <button class="btn-sm" data-loan-tab="review">Theo dõi</button></div>`;
  }).join("");
  return `<div class="stack">
    ${loans || emptyCard("Bạn chưa có khoản vay nào đang hoạt động.")}
    <section class="surface">
      <div class="surface-title"><h2 class="display">Đơn vay đang xử lý</h2><button class="text-action" data-loan-tab="apply">Đăng ký vay mới</button></div>
      ${pending || '<p class="product-note">Không có đơn vay nào đang xử lý.</p>'}
    </section>
  </div>`;
}

function renderLoanHub() {
  const views = { docs: loanDocsView, review: loanReviewView, disburse: loanDisburseView, manage: loanManageView };
  app.innerHTML = featureShell(`${loanTabs()}${views[state.loanTab]()}`);
}

// Thông tin khoản vay hiển thị trong hộp xác thực trước khi khách thực hiện giao dịch
function pendingSummary() {
  const tx = state.pendingTransaction;
  if (!tx || !["apply-loan", "DISBURSE"].includes(tx.type)) return "";
  const loan = tx.type === "DISBURSE" ? loanApps.find((a) => a.id === tx.loanId) : null;
  const term = loan ? loan.term : Number(tx.data?.loan_term) || 12;
  const rate = loan ? loan.rate : termRate(term);
  const monthly = Math.round(tx.amount / term + (tx.amount * rate) / 1200);
  return `<div class="loan-facts modal-facts">
    <div><small>Số tiền</small><b>${vnd(tx.amount)}</b></div><div><small>Kỳ hạn</small><b>${term} tháng</b></div>
    <div><small>Lãi suất</small><b>${rate}%/năm</b></div><div><small>Trả hàng tháng (ước tính)</small><b>${vnd(monthly)}</b></div>
    <div><small>Tổng lãi (ước tính)</small><b>${vnd(monthly * term - tx.amount)}</b></div></div>`;
}

function registerLoanApp(data, amount) {
  const stamp = new Date().toISOString().slice(0, 10).replace(/-/g, "");
  const id = `LN${stamp}${String(loanApps.length + 1).padStart(3, "0")}`;
  loanApps.push({ id, who: state.profile.name, phone: state.profile.phone, type: "Vay tiêu dùng", amt: amount, term: Number(data?.loan_term) || 12, rate: termRate(Number(data?.loan_term) || 12), method: data?.calc_method || "DECREASING", purpose: "—", collateral: data?.collateral || "—", income: "Chưa khai báo", stage: 0, needInfo: false, resubmitted: false, result: null, docs: docsWith(["MISSING", "MISSING", "MISSING", "MISSING"]) });
  state.docAppId = id;
}

function completeDisbursement(id) {
  const a = loanApps.find((x) => x.id === id);
  if (!a) return;
  a.stage = 4;
  const due = new Date();
  due.setDate(Math.min(due.getDate(), 28));
  due.setMonth(due.getMonth() + 1);
  activeLoans.push({ id: a.id, type: a.type, principal: a.amt, outstanding: a.amt, rate: a.rate, total: a.term, left: a.term, pay: monthlyPay(a), due: `${due.getFullYear()}-${pad(due.getMonth() + 1)}-${pad(due.getDate())}` });
  state.loanTab = "manage";
  state.pendingTransaction = null;
  state.balance += a.amt;
  transactions.unshift([`Giải ngân khoản vay ${a.id}`, `Tiền vào · ${stampShort()}`, `+${vnd(a.amt)}`, "wallet"]);
  pushMessage({ title: "Giải ngân thành công", preview: `+${vnd(a.amt)} từ khoản vay ${a.id}`, icon: "wallet", content: `Khoản vay ${a.id} đã được giải ngân ${vnd(a.amt)} vào tài khoản 0123 6868 8888. Số dư khả dụng: ${vnd(state.balance)}.`, reference: a.id });
  showToast(`Đã giải ngân ${vnd(a.amt)} vào tài khoản thanh toán`);
}

// ===== NẠP DỮ LIỆU TỪ DATABASE LÊN GIAO DIỆN (chi tiết ở data.js / schema.sql) =====
// Có backend C++ (QWebChannel) → mỗi lần vào trang, data.js gọi database và đổ dữ liệu vào các "sink" dưới đây.
// Không có backend (mở trên trình duyệt) → chế độ mô phỏng, giữ nguyên dữ liệu mẫu trong file này.
const replaceAll = (arr, rows) => arr.splice(0, arr.length, ...rows);
const greeting = () => { const h = new Date().getHours(); return h < 11 ? "Chào buổi sáng" : h < 14 ? "Chào buổi trưa" : h < 18 ? "Chào buổi chiều" : "Chào buổi tối"; };
const todayText = () => new Date().toLocaleDateString("vi-VN", { weekday: "long", day: "2-digit", month: "long", year: "numeric" });
const primaryAcc = () => state.accounts.find((a) => a.primary) || state.accounts[0] || { name: "Tài khoản thanh toán", number: "—" };

const loadBar = document.createElement("div");
loadBar.id = "loadbar";
document.body.append(loadBar);

const data = createData({
  sinks: {
    profile: (p) => { state.profile = p; },
    accounts: (list) => { state.accounts = list; const main = list.find((a) => a.primary) || list[0]; state.balance = main?.balance ?? 0; },
    savings: (items) => replaceAll(state.savings, items),
    transactions: (rows) => replaceAll(transactions, rows),
    messages: (rows) => { replaceAll(messages, rows); state.msgExpanded = null; state.msgBanner = null; },
    loanTerms: (rows) => replaceAll(loanTerms, rows),
    loanApps: (rows) => replaceAll(loanApps, rows),
    activeLoans: (rows) => replaceAll(activeLoans, rows),
    people: (v) => staff.hydrate("people", v),
    audit: (v) => staff.hydrate("audit", v),
    config: (v) => staff.hydrate("config", v),
    stats: (v) => staff.hydrate("stats", v),
  },
  onStatus: ({ status }) => loadBar.classList.toggle("on", status === "loading"),
  getSessionToken: () => state.sessionToken || globalThis.__OOPBANK_SESSION_TOKEN__ || "",
});

function markRead(m) {
  if (!m.unread) return;
  m.unread = false;
  if (m.id != null) data.mutate("notify.markRead", { ids: [m.id] }).catch(() => {}); // ghi lại vào DB (bỏ qua khi mô phỏng)
}

const pageKey = () => (state.role === "CUSTOMER" ? state.page : `staff:${staff.page}`);

// Nạp dữ liệu cho trang đang mở rồi vẽ lại (không vẽ lại khi người dùng đang nhập liệu hoặc đang mở hộp PIN)
function loadCurrent() {
  if (!state.loggedIn || state.role !== "CUSTOMER") return;
  const key = pageKey();
  data.load(key).then(({ changed, errors }) => {
    if (errors.length) showToast("Không tải được dữ liệu: " + errors.map((e) => e.message || e.query).join("; "));
    const busy = /^(INPUT|TEXTAREA|SELECT)$/.test(document.activeElement?.tagName ?? "") || state.showPinModal;
    if (changed && key === pageKey() && !busy) render();
  });
}
document.addEventListener("visibilitychange", () => { if (!document.hidden) loadCurrent(); });

function logoutSession() {
  const authBridge = getAuthBridge();
  if (state.sessionToken && typeof authBridge?.logout === "function") {
    invokeBridgeMethod(authBridge, "logout", [state.sessionToken]).catch(() => {});
  }
  state.sessionToken = "";
  globalThis.__OOPBANK_SESSION_TOKEN__ = "";
  data.setSessionToken("");
  state.loggedIn = false;
  state.role = "CUSTOMER";
  state.page = "home";
}

const staff = createStaff({ state, icons, icon, brand, render: () => render(), callBackend,
  hasBackend: () => Boolean(getBridge("userBridge")), onLogout: logoutSession,
  loans: { apps: loanApps, docStatus: DOC_STATUS, notify: notifyCustomer, message: pushMessage, terms: loanTerms } });

function render() {
  renderPage();
  renderMsgbox();
}

function renderPage() {
  if (!state.loggedIn) return renderAuth();
  if (state.role !== "CUSTOMER") return staff.render();
  if (state.page === "home") return renderHome();
  if (state.page === "services") return renderServices();
  if (state.page === "accounts") return renderAccounts();
  if (state.page === "notifications") return renderNotifications();
  if (state.page === "transactions") return renderTransactions();
  if (state.page === "user-settings") return renderUserSettings();
  if (state.page === "apply-loan" && state.loanTab !== "apply") return renderLoanHub();
  renderTransactionForm();
}

app.addEventListener("click", (event) => {
  const staffPageBefore = staff.page;
  if (staff.click(event)) {
    if (staff.page !== staffPageBefore) loadCurrent();
    return;
  }
  const pageButton = event.target.closest("[data-page]");
  const authButton = event.target.closest("[data-auth-mode]");
  const actionButton = event.target.closest("[data-action]");
  const billButtonElement = event.target.closest("[data-bill]");
  const messageButton = event.target.closest("[data-message]");

  const loanTabButton = event.target.closest("[data-loan-tab]");
  const docAppButton = event.target.closest("[data-doc-app]");

  if (loanTabButton) {
    state.page = "apply-loan";
    state.loanTab = loanTabButton.dataset.loanTab;
    if (loanTabButton.dataset.docApp) state.docAppId = loanTabButton.dataset.docApp;
    state.transactionSuccess = false;
    window.scrollTo({ top: 0, behavior: "smooth" });
    render();
    loadCurrent();
  } else if (docAppButton) {
    state.docAppId = docAppButton.dataset.docApp;
    render();
  } else if (pageButton) {
    let target = pageButton.dataset.page;
    if (target === "loan") { target = "apply-loan"; state.loanTab = "manage"; }
    else if (target === "apply-loan") state.loanTab = "apply";
    state.page = target;
    state.transactionSuccess = false;
    window.scrollTo({ top: 0, behavior: "smooth" });
    render();
    ensureBillProviders();
    loadCurrent();
  } else if (authButton) {
    state.authMode = authButton.dataset.authMode;
    render();
  } else if (billButtonElement) {
    state.billType = billButtonElement.dataset.bill;
    render();
  } else if (messageButton) {
    state.selectedMessage = Number(messageButton.dataset.message);
    render();
  } else if (actionButton?.dataset.action === "toggle-balance") {
    state.showBalance = !state.showBalance;
    render();
  } else if (actionButton?.dataset.action === "toggle-password") {
    state.showPassword = !state.showPassword;
    render();
  } else if (actionButton?.dataset.action === "logout") {
    logoutSession();
    render();
  } else if (actionButton?.dataset.action === "close-modal") {
    state.showPinModal = false;
    render();
  } else if (actionButton?.dataset.action === "close-savings-request") {
    const savingsId = actionButton.dataset.savingsId;
    const saving = state.savings.find((item) => item.savings_id === savingsId);
    if (!saving) return showToast("Không tìm thấy sổ tiết kiệm đang hoạt động.");
    state.pendingTransaction = { type: "CLOSE_SAVINGS", savingsId, amount: Number(saving.principal_vnd || 0) };
    state.showPinModal = true;
    render();
  } else if (actionButton?.dataset.action === "toggle-schedule") {
    state.openSchedule = state.openSchedule === actionButton.dataset.id ? null : actionButton.dataset.id;
    render();
  } else if (actionButton?.dataset.action === "submit-docs") {
    if (hasBackend()) return showToast("Chưa thể nộp hồ sơ: hệ thống chưa có kho lưu tài liệu vay.");
    const target = docApp();
    if (!target) return;
    if (missingDocs(target)) return showToast("Vui lòng tải lên đủ giấy tờ còn thiếu");
    if (target.stage === 0) target.stage = 1;
    target.needInfo = false;
    target.resubmitted = true;
    target.result = { type: "SUBMITTED", note: "Bạn đã gửi bổ sung hồ sơ. Ngân hàng sẽ kiểm tra và phản hồi sớm.", time: nowHM() };
    state.loanTab = "review";
    showToast("Đã gửi hồ sơ bổ sung");
    render();
  } else if (actionButton?.dataset.action === "forgot-pass") {
    showToast("Đặt lại mật khẩu tạm khóa: hệ thống chưa có kênh gửi và xác minh OTP an toàn.");
  }
});

app.addEventListener("change", (event) => {
  const input = event.target;
  if (!input.matches?.("input[type=file][data-doc]") || !input.files[0]) return;
  if (hasBackend()) return showToast("Tải hồ sơ chưa được nối với kho tài liệu.");
  const file = input.files[0];
  if (file.size > 5 * 1024 * 1024) return showToast("Tệp vượt quá 5 MB, vui lòng chọn tệp nhỏ hơn");
  const doc = docApp()?.docs.find((d) => d.id === input.dataset.doc);
  if (!doc) return;
  doc.status = "REVIEW";
  doc.file = file.name;
  doc.mime = file.type;
  doc.size = fmtSize(file.size);
  doc.at = nowFull();
  try { doc.url = URL.createObjectURL(file); } catch { doc.url = ""; }
  showToast("Đã tải lên " + file.name);
  render();
});

app.addEventListener("submit", async (event) => {
  event.preventDefault();
  if (await staff.submit(event)) return;

  // 1. Đăng nhập / Đăng ký
  if (event.target.id === "auth-form") {
    if (state.authMode === "login") {
      const identifier = document.querySelector("#contact")?.value ?? "";
      const password = document.querySelector("#password")?.value ?? "";
      const authBridge = getAuthBridge();

      if (typeof authBridge?.login === "function") {
        try {
          const response = parseBridgeResponse(await invokeBridgeMethod(
            authBridge,
            "login",
            [JSON.stringify({ identifier, password })],
          ));
          if (!response?.success) {
            showToast(response?.message || "Đăng nhập thất bại.");
            return;
          }

          const session = response.session ?? {};
          if (!session.session_token) {
            showToast("Backend đăng nhập chưa trả session token.");
            return;
          }

          state.sessionToken = session.session_token;
          data.setSessionToken(state.sessionToken);
          state.loggedIn = true;
          state.role = String(session.role || "CUSTOMER").toUpperCase();
          state.profile = { ...state.profile, name: session.full_name || state.profile.name };
          if (state.role === "CUSTOMER") state.page = "home";
          else staff.login(state.role);
        } catch (error) {
          showToast("Không thể đăng nhập: " + (error?.message || String(error)));
          return;
        }
      } else {
        // Chế độ xem thử trên trình duyệt.
        state.loggedIn = true;
        state.page = "home";
        staff.login(identifier);
      }
    } else {
      try {
        const formData = new FormData(event.target);
        const result = await callBackend("authBridge", "registerCustomer", {
          username: String(formData.get("username") || "").trim(),
          password: String(formData.get("password") || ""),
          fullname: String(formData.get("name") || "").trim(),
          email: String(formData.get("contact") || "").trim(),
          cccd: String(formData.get("cccd") || "").trim(),
          phone: String(formData.get("phone") || "").trim(),
        });
        state.authMode = "login";
        render();
        showToast(result.message || "Đăng ký tài khoản thành công. Vui lòng đăng nhập.");
      } catch (error) {
        showToast("Không thể đăng ký: " + (error?.message || String(error)));
        return;
      }
    }
    render();
    loadCurrent();
  }

  // 2. Submit form giao dịch -> Mở Modal xác thực PIN & OTP
  if (event.target.id === "transaction-form") {
    const formData = new FormData(event.target);
    const values = Object.fromEntries(formData.entries());
    const amountVal = Number(formData.get("amount") || formData.get("saving-amount") || formData.get("loan-amount") || 0);
    if (state.page === "topup") return showToast("Nạp điện thoại chưa được nối với nhà cung cấp thanh toán; chưa ghi nợ tài khoản.");
    if (state.page === "loan-pay") return showToast("Thanh toán kỳ vay chưa sẵn sàng: backend chưa có API đối soát lịch trả nợ.");
    if (["transfer", "savings"].includes(state.page) && amountVal > state.balance) return showToast("Số dư khả dụng không đủ để thực hiện giao dịch");
    if (state.page === "apply-loan") {
      try {
        await callBackend("loanBridge", "applyLoan", {
          session_token: state.sessionToken,
          amount_vnd: amountVal,
          term_months: Number(values.loan_term || 12),
          calc_method: values.calc_method === "FIXED" ? "FLAT" : (values.calc_method || "DECREASING"),
          collateral_info: String(values.collateral || "").trim(),
        });
        state.transactionSuccess = true;
        render();
        loadCurrent();
      } catch (error) { showToast("Không thể nộp đơn vay: " + (error?.message || String(error))); }
      return;
    }
    if (state.page === "payment") {
      try {
        const lookup = await callBackend("billBridge", "queryBill", {
          session_token: state.sessionToken,
          provider_id: String(formData.get("provider") || "").trim(),
          customer_code: String(formData.get("bill-code") || "").trim(),
        });
        const bill = lookup.bill || {};
        state.pendingTransaction = { type: "payment", amount: Number(bill.amount_vnd || 0), data: { ...values, bill_id: bill.bill_id } };
        state.showPinModal = true;
        render();
      } catch (error) { showToast("Không tra cứu được hóa đơn: " + (error?.message || String(error))); }
      return;
    }
    if (["transfer", "savings"].includes(state.page) && amountVal > state.balance) return showToast("Số dư khả dụng không đủ để thực hiện giao dịch");
    if (state.page === "transfer" && amountVal > 50000000 && hasBackend()) return showToast("Giao dịch trên 50 triệu cần OTP từ nhà cung cấp thật; hệ thống chưa cấu hình OTP.");
    state.pendingTransaction = { type: state.page, amount: amountVal, data: values };
    state.showPinModal = true;
    if (state.page === "transfer") maybeSendOtp();
    render();
  }

  // 2b. Xác nhận giải ngân khoản vay -> Modal PIN
  if (event.target.id === "disburse-form") {
    const formData = new FormData(event.target);
    const loan = loanApps.find((a) => a.id === formData.get("loanId"));
    if (loan) {
      state.pendingTransaction = { type: "DISBURSE", loanId: loan.id, targetAccountId: String(formData.get("dest") || ""), amount: loan.amt };
      state.showPinModal = true;
      render();
    }
  }

  // 3. Submit Modal PIN/OTP -> Xác nhận hoàn tất
  if (event.target.id === "pin-modal-form") {
    const tx = state.pendingTransaction;
    const formData = new FormData(event.target);
    const pin = String(formData.get("pin") || "");
    const otp = String(formData.get("otp") || "");
    state.showPinModal = false;
    try {
      if (!tx) throw new Error("Không có giao dịch chờ xác thực.");
      const common = { session_token: state.sessionToken, pin };
      if (tx.type === "transfer") {
        await callBackend("transactionBridge", "transfer", {
          ...common,
          idempotency_key: `TRANSFER-${Date.now()}-${Math.random().toString(36).slice(2)}`,
          receiver_account_id: String(tx.data.receiver || "").trim(),
          amount_vnd: tx.amount,
          description: "Chuyển tiền qua OOPBank",
          otp,
          is_interbank: !/oopbank/i.test(String(tx.data.bank || "")),
        });
      } else if (tx.type === "payment") {
        await callBackend("billBridge", "payBill", {
          ...common,
          idempotency_key: `BILL-${Date.now()}-${Math.random().toString(36).slice(2)}`,
          bill_id: tx.data.bill_id,
        });
      } else if (tx.type === "savings") {
        await callBackend("savingsBridge", "openSavings", {
          ...common,
          idempotency_key: `SAV-${Date.now()}-${Math.random().toString(36).slice(2)}`,
          amount_vnd: tx.amount,
          term_months: Number(tx.data.term || 6),
          maturity_option: String(tx.data.maturity_option || "CLOSE_TO_ACCOUNT"),
        });
      } else if (tx.type === "CLOSE_SAVINGS") {
        await callBackend("savingsBridge", "closeSavings", { ...common, savings_id: tx.savingsId });
      } else if (tx.type === "DISBURSE") {
        await callBackend("loanBridge", "disburseLoan", {
          ...common, loan_id: tx.loanId, target_account_id: tx.targetAccountId,
        });
      } else {
        throw new Error("Loại giao dịch này chưa có backend.");
      }
      state.pendingTransaction = null;
      state.transactionSuccess = true;
      render();
      loadCurrent();
    } catch (error) {
      state.pendingTransaction = null;
      render();
      showToast("Giao dịch chưa được ghi nhận: " + (error?.message || String(error)));
    }
  }

  // 4. Submit các Form cập nhật hồ sơ & đổi pass/PIN
  if (event.target.id === "update-profile-form") {
    const formData = new FormData(event.target);
    try {
      await callBackend("userBridge", "updateProfile", {
        session_token: state.sessionToken,
        fullname: String(formData.get("fullname") || "").trim(),
        email: String(formData.get("email") || "").trim(),
        phone: String(formData.get("phone") || "").trim(),
      });
      showToast("Đã cập nhật hồ sơ cá nhân.");
      loadCurrent();
    } catch (error) { showToast("Không thể cập nhật hồ sơ: " + (error?.message || String(error))); }
  }
  if (event.target.id === "change-pass-form") {
    const formData = new FormData(event.target);
    try {
      await callBackend("userBridge", "changePassword", { session_token: state.sessionToken, old_password: formData.get("old_password"), new_password: formData.get("new_password") });
      showToast("Đã cập nhật mật khẩu mới.");
      event.target.reset();
    } catch (error) { showToast("Không thể đổi mật khẩu: " + (error?.message || String(error))); }
  }
  if (event.target.id === "change-pin-form") {
    const formData = new FormData(event.target);
    try {
      await callBackend("userBridge", "changePin", { session_token: state.sessionToken, old_pin: formData.get("old_pin"), new_pin: formData.get("new_pin") });
      showToast("Đã cập nhật mã PIN giao dịch.");
      event.target.reset();
    } catch (error) { showToast("Không thể đổi mã PIN: " + (error?.message || String(error))); }
  }
});

render();
loadCurrent();
