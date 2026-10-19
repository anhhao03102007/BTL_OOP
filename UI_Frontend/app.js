import { createStaff } from "./staff.js";

const app = document.querySelector("#app");

const state = {
  loggedIn: true,
  role: "CUSTOMER", // CUSTOMER | TELLER | ADMIN
  authMode: "login",
  page: "home",
  showBalance: true,
  showPassword: false,
  billType: "electricity",
  selectedMessage: 0,
  transactionSuccess: false,
};

const icons = {
  arrow: '<path d="M5 12h14"/><path d="m13 6 6 6-6 6"/>',
  back: '<path d="M19 12H5"/><path d="m11 18-6-6 6-6"/>',
  bell: '<path d="M18 8a6 6 0 0 0-12 0c0 7-3 7-3 9h18c0-2-3-2-3-9"/><path d="M10 21h4"/>',
  card: '<rect x="2" y="5" width="20" height="14" rx="3"/><path d="M2 10h20M6 15h4"/>',
  check: '<path d="m5 12 4 4L19 6"/>',
  eye: '<path d="M2.5 12s3.5-6 9.5-6 9.5 6 9.5 6-3.5 6-9.5 6-9.5-6-9.5-6Z"/><circle cx="12" cy="12" r="2.5"/>',
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
};

function icon(name) {
  return `<svg class="icon" aria-hidden="true" viewBox="0 0 24 24">${icons[name]}</svg>`;
}

function brand() {
  return `
    <div class="brand">
      <span class="brand-mark">${icon("home")}</span>
      <span><span class="brand-name">OOPBANK</span><span class="brand-sub">Private Banking</span></span>
    </div>`;
}

function field(id, label, iconName, placeholder, type = "text") {
  const passwordAction = type === "password"
    ? `<button class="password-toggle" type="button" data-action="toggle-password" aria-label="Hiện mật khẩu">${icon(state.showPassword ? "eyeOff" : "eye")}</button>`
    : "";
  const actualType = type === "password" && state.showPassword ? "text" : type;
  return `
    <div class="field">
      <label for="${id}">${label}</label>
      <div class="input-wrap">${icon(iconName)}<input id="${id}" type="${actualType}" placeholder="${placeholder}" required />${passwordAction}</div>
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
              ${register ? field("name", "Họ và tên", "user", "Nguyễn Minh Anh") : ""}
              ${field("contact", register ? "Địa chỉ email" : "Tên đăng nhập hoặc số điện thoại", "mail", register ? "tenban@email.com" : "Tên đăng nhập")}
              ${register ? field("phone", "Số điện thoại", "phone", "0901 234 567", "tel") : ""}
              ${register ? field("cccd", "Số CCCD", "shield", "Nhập 12 số căn cước công dân") : ""}
              ${field("password", "Mật khẩu", "lock", register ? "Tối thiểu 8 ký tự" : "Nhập mật khẩu của bạn", "password")}
              ${register ? "" : '<div class="forgot-row"><button class="link-button" type="button">Quên mật khẩu?</button></div>'}
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
  const rows = all ? [...transactions, ...transactions.slice(1), ...transactions] : transactions;
  return rows.map(([name, detail, amount, iconName]) => `
    <div class="transaction">
      <span class="icon-box">${icon(iconName)}</span>
      <div class="info"><p>${name}</p><small>${detail}</small></div>
      <span class="amount ${amount.startsWith("+") ? "positive" : ""}">${amount}</span>
    </div>`).join("");
}

function appHeader() {
  return `
    <header class="app-header">
      <div class="header-inner">
        ${brand()}
        <nav class="desktop-nav">
          <button class="nav-button active" data-page="home">${icon("home")} Trang chủ</button>
          <button class="nav-button" data-page="accounts">Tài khoản</button>
          <button class="nav-button" data-page="services">Dịch vụ</button>
        </nav>
        <div class="header-actions">
          <button class="circle-button" data-page="notifications" aria-label="Thông báo">${icon("bell")}<i class="unread"></i></button>
          <button class="avatar" data-action="logout" title="Đăng xuất"><b>NA</b><span>Minh Anh</span></button>
        </div>
      </div>
    </header>`;
}

function mobileNav() {
  return `
    <nav class="mobile-nav">
      <button class="active" data-page="home">${icon("home")}Trang chủ</button>
      <button data-page="accounts">${icon("wallet")}Tài khoản</button>
      <button data-page="transfer">${icon("send")}Chuyển tiền</button>
      <button data-page="services">${icon("grid")}Tiện ích</button>
    </nav>`;
}

function renderHome() {
  app.innerHTML = `
    ${appHeader()}
    <main class="page-container">
      <div class="welcome">
        <div><p>Chào buổi sáng,</p><h1 class="display">Nguyễn Minh Anh</h1></div>
        <time>Thứ Hai, 02 tháng 02, 2026</time>
      </div>
      <div class="grid-main">
        <section class="balance-card">
          <span class="balance-label">Tổng số dư khả dụng</span>
          <div class="balance-line">
            <span class="balance">${state.showBalance ? "128.650.000 ₫" : "••••••••••"}</span>
            <button class="ghost-button" data-action="toggle-balance">${icon(state.showBalance ? "eye" : "eyeOff")}</button>
          </div>
          <div class="account-meta">
            <div><small>Tài khoản thanh toán</small><b>0123 6868 8888</b></div>
            <div><small>Chi nhánh</small><b>OOPBank Hà Nội</b></div>
          </div>
        </section>
        <section class="surface">
          <div class="surface-title"><h2 class="display">Thao tác nhanh</h2><button class="text-action" data-page="services">Chỉnh sửa</button></div>
          <div class="quick-grid">
            ${quickButton("transfer", "send", "Chuyển tiền")}
            ${quickButton("topup", "plus", "Nạp tiền")}
            ${quickButton("payment", "card", "Thanh toán")}
            ${quickButton("services", "grid", "Tất cả")}
          </div>
        </section>
      </div>
      <div class="product-grid">
        ${productCard("savings", "wallet", "Tích lũy sinh lời", "Sổ tiết kiệm", "80.000.000 ₫", "Lãi suất 5,8%/năm · Đáo hạn 18/08/2026", "Mở sổ tiết kiệm mới")}
        ${productCard("loan", "card", "Vay tiêu dùng", "Khoản vay", "45.600.000 ₫", "Còn 14 kỳ · Kỳ tới 3.250.000 ₫", "Thanh toán khoản vay", true)}
      </div>
      <div class="content-grid">
        <section class="surface">
          <div class="surface-title"><h2 class="display">Giao dịch gần đây</h2><button class="text-action" data-page="transactions">Xem tất cả</button></div>
          <div>${transactionRows()}</div>
        </section>
        <section class="surface promo">
          <span class="eyebrow">Tiết kiệm thông minh</span>
          <h2 class="display">Sinh lời mỗi ngày, vững vàng tương lai.</h2>
          <p>Lãi suất hấp dẫn lên đến 5,8%/năm với Tiết kiệm OOPBank.</p>
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
  savings: ["Tích lũy sinh lời", "Sổ tiết kiệm", "Mở sổ trực tuyến và chủ động kế hoạch tương lai.", "wallet"],
  loan: ["Quản lý tín dụng", "Khoản vay", "Theo dõi dư nợ và thanh toán các kỳ vay đúng hạn.", "card"],
};

function featureShell(content) {
  const detail = pageDetails[state.page];
  return `
    <header class="feature-header">
      <div class="feature-header-inner">
        <button class="back" data-page="home">${icon("back")} Quay lại</button>
        ${brand()}
        <button class="circle-button" data-page="home" aria-label="Trang chủ">${icon("home")}</button>
      </div>
    </header>
    <main class="feature-container">
      <div class="feature-title"><span class="icon-box">${icon(detail[3])}</span><div><span class="eyebrow">${detail[0]}</span><h1 class="display">${detail[1]}</h1></div></div>
      <p class="feature-description">${detail[2]}</p>
      ${content}
    </main>`;
}

function renderServices() {
  const services = [
    ["transfer", "send", "Chuyển tiền"], ["topup", "phone", "Nạp điện thoại"],
    ["payment", "card", "Thanh toán"], ["savings", "wallet", "Tiết kiệm"],
    ["loan", "card", "Khoản vay"], ["accounts", "user", "Tài khoản"],
  ];
  renderFeature(services.map(([page, iconName, label]) => `
    <button class="service-item" data-page="${page}"><span class="icon-box">${icon(iconName)}</span><b>${label}</b>${icon("arrow")}</button>`).join(""), "service-grid");
}

function renderFeature(content, className = "") {
  app.innerHTML = featureShell(`<section class="${className}">${content}</section>`);
}

function renderAccounts() {
  const accounts = [
    ["Tài khoản thanh toán", "0123 6868 8888", "128.650.000 ₫", true],
    ["Tài khoản dự phòng", "0123 6868 9999", "12.500.000 ₫", false],
  ];
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

function renderNotifications() {
  const message = messages[state.selectedMessage];
  const list = messages.map((item, index) => `
    <button class="message-item ${state.selectedMessage === index ? "active" : ""}" data-message="${index}">
      <span class="icon-box">${icon(item.icon)}</span>
      <span class="message-info"><strong>${item.title}</strong><span>${item.preview}</span></span>
      <time class="message-time">${item.time}</time>
    </button>`).join("");
  const otp = state.selectedMessage === 2
    ? `<div class="otp">${"482913".split("").map((number) => `<b>${number}</b>`).join("")}</div>` : "";
  renderFeature(`
    <section class="inbox">
      <div class="message-list"><div class="inbox-title"><h2 class="display">Hộp tin nhắn</h2><span class="badge">3 mới</span></div>${list}</div>
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
  return `<div class="field"><label for="${id}">${label}</label><div class="input-wrap">${icon(iconName)}<input id="${id}" type="${type}" placeholder="${placeholder}" required /></div></div>`;
}

function renderTransactionForm() {
  if (state.transactionSuccess) {
    const content = `
      <div class="success"><span class="success-icon">${icon("check")}</span><h2 class="display">Giao dịch thành công</h2><p>Yêu cầu của bạn đã được OOPBank ghi nhận.</p><button class="primary" data-page="home">Về trang chủ</button></div>`;
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
    heading = "Thông tin hóa đơn";
    fields = billSelector() + providerSelector() + formField("bill-code", "Mã khách hàng / Hóa đơn", "Nhập mã tra cứu", "card");
  } else if (state.page === "savings") {
    heading = "Mở sổ tiết kiệm mới";
    fields = formField("saving-amount", "Số tiền gửi", "Tối thiểu 1.000.000", "wallet", "number") + `
      <div class="field"><label for="term">Kỳ hạn</label><div class="input-wrap"><select id="term"><option>3 tháng · 4,5%/năm</option><option>6 tháng · 5,8%/năm</option><option>12 tháng · 6,1%/năm</option></select></div></div>`;
  } else {
    heading = "Thanh toán kỳ vay";
  }
  const needsAmount = !["payment", "savings"].includes(state.page);
  const amount = needsAmount ? formField("amount", state.page === "loan" ? "Số tiền thanh toán" : "Số tiền", "0 VND", "wallet", "number") : "";
  const content = `
    <div class="feature-layout">
      <section class="form-card">
        <h2 class="display">${heading}</h2>
        <form id="transaction-form">${fields}${amount}<button class="primary" type="submit">Xác nhận ${icon("arrow")}</button></form>
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

function providerSelector() {
  const providers = {
    electricity: ["EVN Hà Nội", "EVN TP. Hồ Chí Minh", "EVN Miền Trung"],
    water: ["Nước sạch Hà Nội", "SAWACO", "Cấp nước Đà Nẵng"],
    internet: ["Viettel Internet", "VNPT", "FPT Telecom"],
  };
  return `
    <div class="field"><label for="provider">Nhà cung cấp</label><div class="input-wrap">
      <select id="provider">${providers[state.billType].map((item) => `<option>${item}</option>`).join("")}</select>
    </div></div>`;
}

function sourceCard() {
  return `<aside class="source-card"><span class="eyebrow">Nguồn tiền</span><p>Tài khoản thanh toán</p><b>0123 6868 8888</b><hr /><p>Số dư khả dụng</p><strong>128.650.000 ₫</strong></aside>`;
}

const staff = createStaff({ state, icons, icon, brand, render: () => render() });

function render() {
  if (!state.loggedIn) return renderAuth();
  if (state.role !== "CUSTOMER") return staff.render();
  if (state.page === "home") return renderHome();
  if (state.page === "services") return renderServices();
  if (state.page === "accounts") return renderAccounts();
  if (state.page === "notifications") return renderNotifications();
  if (state.page === "transactions") return renderTransactions();
  renderTransactionForm();
}

app.addEventListener("click", (event) => {
  if (staff.click(event)) return;
  const pageButton = event.target.closest("[data-page]");
  const authButton = event.target.closest("[data-auth-mode]");
  const actionButton = event.target.closest("[data-action]");
  const billButtonElement = event.target.closest("[data-bill]");
  const messageButton = event.target.closest("[data-message]");

  if (pageButton) {
    state.page = pageButton.dataset.page;
    state.transactionSuccess = false;
    window.scrollTo({ top: 0, behavior: "smooth" });
    render();
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
    state.loggedIn = false;
    state.page = "home";
    render();
  }
});

app.addEventListener("submit", (event) => {
  event.preventDefault();
  if (staff.submit(event)) return;
  if (event.target.id === "auth-form") {
    if (state.authMode === "login") {
      state.loggedIn = true;
      state.page = "home";
      staff.login(document.querySelector("#contact")?.value ?? "");
    } else {
      state.authMode = "login";
    }
    render();
  }
  if (event.target.id === "transaction-form") {
    state.transactionSuccess = true;
    render();
  }
});

render();