// Giao diện Teller (GDV) & Admin — dùng chung palette/ class với giao diện khách hàng.
// Dữ liệu bên dưới là MOCK; khi nối backend, thay bằng gọi Bridge (QWebChannel).
export function createStaff({ state, icons, icon, brand, render }) {
  Object.assign(icons, {
    users: '<circle cx="9" cy="8" r="3.5"/><path d="M2 21a7 7 0 0 1 14 0"/><path d="M16 4.5a3.5 3.5 0 0 1 0 7M22 21a7 7 0 0 0-4-6.3"/>',
    chart: '<path d="M4 20V4M4 20h16"/><path d="M8 16v-5M12 16V8M16 16v-8"/>',
    clipboard: '<rect x="5" y="4" width="14" height="17" rx="2"/><path d="M9 4h6v3H9zM9 12h6M9 16h4"/>',
    list: '<path d="M8 6h13M8 12h13M8 18h13M3 6h.01M3 12h.01M3 18h.01"/>',
    settings: '<circle cx="12" cy="12" r="3"/><path d="M12 2v3M12 19v3M2 12h3M19 12h3M5 5l2 2M17 17l2 2M19 5l-2 2M7 17l-2 2"/>',
    search: '<circle cx="11" cy="11" r="7"/><path d="m21 21-4.3-4.3"/>',
  });
  const app = document.querySelector("#app");
  const s = {
    page: "dashboard",
    q: "",
    fees: { interbank: 9900, atm: 3300, rate: 5.8 },
    people: [
      { id: "U001", name: "Nguyễn Minh Anh", phone: "0901 234 567", acc: "0123 6868 8888", role: "CUSTOMER", locked: false },
      { id: "U002", name: "Trần Hoàng Nam", phone: "0912 345 678", acc: "0123 5555 1111", role: "CUSTOMER", locked: false },
      { id: "U003", name: "Lê Thu Hà", phone: "0933 456 789", acc: "0123 5555 2222", role: "CUSTOMER", locked: true },
      { id: "T001", name: "Vũ Thanh Tâm", phone: "0944 111 222", acc: "—", role: "TELLER", locked: false },
      { id: "A001", name: "Quản trị viên", phone: "0955 000 111", acc: "—", role: "ADMIN", locked: false },
    ],
    loans: [
      { id: "LN20261014001", who: "Nguyễn Minh Anh", type: "Vay tiêu dùng", amt: "50.000.000 ₫", term: "24 tháng", status: "PENDING" },
      { id: "LN20261015002", who: "Phạm Quốc Bảo", type: "Vay thế chấp", amt: "900.000.000 ₫", term: "180 tháng", status: "PENDING" },
      { id: "LN20261012003", who: "Lê Thu Hà", type: "Vay tiêu dùng", amt: "20.000.000 ₫", term: "12 tháng", status: "ACTIVE" },
    ],
    audit: [
      ["09:45", "Giao dịch viên", "Mở khóa tài khoản 0123 5555 2222"],
      ["09:12", "Quản trị viên", "Phê duyệt khoản vay LN20261012003"],
      ["08:40", "Giao dịch viên", "Nạp 5.000.000 ₫ tài khoản 0123 6868 8888"],
      ["08:05", "Hệ thống", "Đăng nhập thất bại 3 lần: U003"],
    ],
  };
  const NAV = {
    TELLER: [["dashboard", "home", "Tổng quan"], ["customers", "users", "Khách hàng"], ["counter", "wallet", "Giao dịch quầy"], ["loans", "clipboard", "Duyệt vay"]],
    ADMIN: [["dashboard", "chart", "Tổng quan"], ["users", "users", "Người dùng"], ["loans", "clipboard", "Duyệt vay"], ["audit", "list", "Nhật ký"], ["settings", "settings", "Cấu hình"]],
  };
  const esc = (v) => String(v).replace(/[&<>"']/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[c]));
  const money = (n) => Number(n).toLocaleString("vi-VN");
  const label = () => (state.role === "ADMIN" ? "Quản trị viên" : "Giao dịch viên");
  const log = (msg) => s.audit.unshift([new Date().toTimeString().slice(0, 5), label(), msg]);
  const toast = (msg) => {
    const el = document.createElement("div");
    el.className = "toast";
    el.textContent = msg;
    document.body.append(el);
    setTimeout(() => el.remove(), 2400);
  };
  const chip = (text, cls) => `<span class="chip ${cls}">${text}</span>`;
  const stat = (name, value, ic) => `<div class="stat"><span class="icon-box">${icon(ic)}</span><div><small>${name}</small><b>${value}</b></div></div>`;
  const fld = (id, name, text, ph, type = "text", val = "") =>
    `<div class="field"><label for="${id}">${text}</label><div class="input-wrap"><input id="${id}" name="${name}" type="${type}" placeholder="${ph}" value="${esc(val)}" required /></div></div>`;
  const head = (eyebrow, title) => `<div class="welcome"><div><p>${eyebrow}</p><h1 class="display">${title}</h1></div></div>`;
  const auditRows = (n) =>
    s.audit.slice(0, n).map(([t, who, msg]) => `<div class="row"><span class="icon-box">${icon("list")}</span><div class="info"><p>${esc(msg)}</p><small>${who}</small></div><small>${t}</small></div>`).join("");

  function dashboard() {
    const admin = state.role === "ADMIN";
    const pend = s.loans.filter((l) => l.status === "PENDING").length;
    const locked = s.people.filter((p) => p.locked).length;
    const stats = admin
      ? [["Tổng người dùng", s.people.length, "users"], ["Tổng số dư hệ thống", "48,2 tỷ ₫", "wallet"], ["Khoản vay chờ duyệt", pend, "clipboard"], ["Tài khoản đang khóa", locked, "lock"]]
      : [["Giao dịch hôm nay", "128", "send"], ["Hồ sơ vay chờ duyệt", pend, "clipboard"], ["Tài khoản đang khóa", locked, "lock"], ["Khách phục vụ hôm nay", "37", "users"]];
    const quick = admin
      ? [["users", "Quản lý người dùng"], ["loans", "Duyệt khoản vay"], ["settings", "Cấu hình phí & lãi suất"]]
      : [["customers", "Tra cứu khách hàng"], ["counter", "Giao dịch tại quầy"], ["loans", "Duyệt khoản vay"]];
    const today = new Date().toLocaleDateString("vi-VN", { weekday: "long", day: "2-digit", month: "long", year: "numeric" });
    return `
      <div class="welcome"><div><p>${admin ? "Bảng điều khiển hệ thống" : "Quầy giao dịch"}</p><h1 class="display">${label()}</h1></div><time>${today}</time></div>
      <div class="stat-grid">${stats.map((x) => stat(...x)).join("")}</div>
      <div class="content-grid">
        <section class="surface"><div class="surface-title"><h2 class="display">Hoạt động gần đây</h2></div>${auditRows(4)}</section>
        <section class="surface"><div class="surface-title"><h2 class="display">Tác vụ nhanh</h2></div>
          <div class="quick-list">${quick.map(([p, t]) => `<button class="service-item" data-spage="${p}"><b>${t}</b>${icon("arrow")}</button>`).join("")}</div>
        </section>
      </div>`;
  }

  function peoplePage(admin) {
    const q = s.q.toLowerCase();
    const roleName = { CUSTOMER: "Khách hàng", TELLER: "Giao dịch viên", ADMIN: "Quản trị" };
    const list = s.people.filter((p) => (admin || p.role === "CUSTOMER") && `${p.name} ${p.phone} ${p.acc} ${p.id}`.toLowerCase().includes(q));
    const rows = list.map((p) => `
      <div class="row"><span class="icon-box">${icon("user")}</span>
        <div class="info"><p>${p.name}</p><small>${p.phone} · ${p.acc}</small></div>
        ${admin ? chip(roleName[p.role], "role") : ""}${chip(p.locked ? "Đã khóa" : "Hoạt động", p.locked ? "warn" : "ok")}
        <div class="row-actions">
          ${!admin ? `<button class="btn-sm" data-sact="open-acc" data-id="${p.id}">Mở tài khoản</button>` : ""}
          <button class="btn-sm ${p.locked ? "" : "danger"}" data-sact="toggle-lock" data-id="${p.id}" ${p.role === "ADMIN" ? "disabled" : ""}>${p.locked ? "Mở khóa" : "Khóa"}</button>
        </div>
      </div>`).join("");
    return `${head(admin ? "Quản trị người dùng" : "Tra cứu khách hàng", admin ? "Người dùng" : "Khách hàng")}
      <form id="staff-search" class="search-row"><div class="input-wrap">${icon("search")}<input name="q" value="${esc(s.q)}" placeholder="Tên, số điện thoại, số tài khoản..." /></div><button class="primary" type="submit">Tìm kiếm</button></form>
      <section class="surface">${rows || '<p class="product-note">Không tìm thấy kết quả.</p>'}</section>`;
  }

  function counter() {
    return `${head("Nghiệp vụ tại quầy", "Giao dịch quầy")}
      <div class="feature-layout">
        <section class="form-card"><h2 class="display">Nạp / Rút tiền mặt</h2>
          <form id="counter-form">
            <div class="form-grid">
              <div class="field"><label for="c-type">Loại giao dịch</label><div class="input-wrap"><select id="c-type" name="type"><option value="DEPOSIT">Nạp tiền</option><option value="WITHDRAW">Rút tiền</option></select></div></div>
              ${fld("c-acc", "acc", "Số tài khoản", "Nhập số tài khoản")}
            </div>
            ${fld("c-amt", "amount", "Số tiền", "0 VND", "number")}
            <button class="primary" type="submit">Xác nhận giao dịch ${icon("arrow")}</button>
          </form>
        </section>
        <aside class="source-card"><span class="eyebrow">Lưu ý quầy</span><p>Đối chiếu CCCD của khách trước khi thực hiện giao dịch.</p><hr /><p>Phí rút ATM ngoại mạng</p><strong>${money(s.fees.atm)} ₫</strong></aside>
      </div>`;
  }

  function loans() {
    const rows = s.loans.map((l) => `
      <div class="row"><span class="icon-box">${icon("card")}</span>
        <div class="info"><p>${l.who} · ${l.amt}</p><small>${l.id} · ${l.type} · ${l.term}</small></div>
        ${l.status === "PENDING"
          ? `<div class="row-actions"><button class="btn-sm" data-sact="loan" data-v="APPROVE" data-id="${l.id}">Phê duyệt</button><button class="btn-sm danger" data-sact="loan" data-v="REJECT" data-id="${l.id}">Từ chối</button></div>`
          : chip(l.status === "ACTIVE" ? "Đã giải ngân" : "Đã từ chối", l.status === "ACTIVE" ? "ok" : "warn")}
      </div>`).join("");
    return `${head("Quản lý tín dụng", "Duyệt khoản vay")}<section class="surface">${rows}</section>`;
  }

  const audit = () => `${head("An ninh hệ thống", "Nhật ký hoạt động")}<section class="surface">${auditRows(50)}</section>`;

  function settings() {
    return `${head("Tham số hệ thống", "Cấu hình")}
      <section class="form-card"><h2 class="display">Phí & lãi suất</h2>
        <form id="settings-form">
          <div class="form-grid">
            ${fld("f-inter", "interbank", "Phí chuyển liên ngân hàng (₫)", "9900", "number", s.fees.interbank)}
            ${fld("f-atm", "atm", "Phí rút ATM ngoại mạng (₫)", "3300", "number", s.fees.atm)}
          </div>
          ${fld("f-rate", "rate", "Lãi suất tiết kiệm tối đa (%/năm)", "5.8", "number", s.fees.rate)}
          <button class="primary" type="submit">Lưu cấu hình ${icon("check")}</button>
        </form>
      </section>`;
  }

  const pages = { dashboard, customers: () => peoplePage(false), users: () => peoplePage(true), counter, loans, audit, settings };

  return {
    login(identifier) {
      const v = identifier.trim().toLowerCase();
      state.role = v.startsWith("admin") ? "ADMIN" : /^(teller|gdv)/.test(v) ? "TELLER" : "CUSTOMER";
      s.page = "dashboard";
      s.q = "";
    },
    render() {
      const admin = state.role === "ADMIN";
      const nav = NAV[state.role];
      if (!nav.some((n) => n[0] === s.page)) s.page = "dashboard";
      const items = nav.map(([p, ic, t]) => `<button class="nav-button ${s.page === p ? "active" : ""}" data-spage="${p}">${icon(ic)} ${t}</button>`).join("");
      const mobile = nav.map(([p, ic, t]) => `<button class="${s.page === p ? "active" : ""}" data-spage="${p}">${icon(ic)}${t}</button>`).join("");
      app.innerHTML = `
        <header class="app-header"><div class="header-inner">${brand()}
          <nav class="desktop-nav">${items}</nav>
          <div class="header-actions"><span class="role-chip ${admin ? "admin" : "teller"}">${admin ? "ADMIN" : "TELLER"}</span>
            <button class="avatar" data-sact="logout" title="Đăng xuất"><b>${admin ? "AD" : "GD"}</b><span>${label()}</span></button></div>
        </div></header>
        <main class="page-container">${pages[s.page]()}</main>
        <nav class="mobile-nav">${mobile}</nav>`;
    },
    click(event) {
      const el = event.target.closest("[data-spage],[data-sact]");
      if (!el || state.role === "CUSTOMER") return false;
      if (el.dataset.spage) {
        s.page = el.dataset.spage;
        s.q = "";
        window.scrollTo({ top: 0, behavior: "smooth" });
        render();
        return true;
      }
      const id = el.dataset.id;
      const act = el.dataset.sact;
      const person = s.people.find((p) => p.id === id);
      if (act === "logout") {
        state.loggedIn = false;
        state.role = "CUSTOMER";
        state.page = "home";
        render();
      } else if (act === "toggle-lock" && person) {
        person.locked = !person.locked;
        log(`${person.locked ? "Khóa" : "Mở khóa"} tài khoản của ${person.name}`);
        toast(person.locked ? "Đã khóa tài khoản" : "Đã mở khóa tài khoản");
        render();
      } else if (act === "open-acc" && person) {
        log(`Mở tài khoản thanh toán mới cho ${person.name}`);
        toast(`Đã mở tài khoản mới cho ${person.name}`);
      } else if (act === "loan") {
        const loan = s.loans.find((l) => l.id === id);
        if (loan) {
          const ok = el.dataset.v === "APPROVE";
          loan.status = ok ? "ACTIVE" : "REJECTED";
          log(`${ok ? "Phê duyệt" : "Từ chối"} khoản vay ${loan.id}`);
          toast(ok ? "Đã phê duyệt & giải ngân" : "Đã từ chối khoản vay");
          render();
        }
      }
      return true;
    },
    submit(event) {
      const f = event.target;
      if (state.role === "CUSTOMER") return false;
      if (f.id === "staff-search") {
        s.q = f.elements.q.value.trim();
        render();
        return true;
      }
      if (f.id === "counter-form") {
        const d = new FormData(f);
        const t = d.get("type") === "DEPOSIT" ? "Nạp" : "Rút";
        log(`${t} ${money(d.get("amount"))} ₫ tài khoản ${d.get("acc")}`);
        toast(`${t} tiền thành công`);
        f.reset();
        return true;
      }
      if (f.id === "settings-form") {
        const d = new FormData(f);
        s.fees = { interbank: +d.get("interbank"), atm: +d.get("atm"), rate: +d.get("rate") };
        log("Cập nhật cấu hình phí & lãi suất");
        toast("Đã lưu cấu hình");
        return true;
      }
      return false;
    },
  };
}