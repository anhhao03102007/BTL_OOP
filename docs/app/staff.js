// Giao diện Teller (GDV) & Admin — dùng chung palette/ class với giao diện khách hàng.
// Dữ liệu bên dưới là MOCK; khi nối backend, thay bằng gọi Bridge (QWebChannel).
export function createStaff({ state, icons, icon, brand, render, loans: L, callBackend, hasBackend = () => false, onLogout = () => {} }) {
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
    detail: null, // mã khoản vay đang xem chi tiết
    mode: null, // null | "reject" | "info"
    stats: { txToday: 128, served: 37 }, // chỉ số trong ngày (nạp từ admin.stats khi có backend)
    live: false, // true khi dữ liệu đã nạp từ database (không dùng số dư mô phỏng của khách)
    docView: null, // mã giấy tờ đang xem chi tiết
    preDoc: null, // giấy tờ chọn sẵn khi yêu cầu nộp lại
    fees: { interbank: 9900, atm: 3300, rate: 5.8 },
    staffMembers: [
      { user_id: "T001", username: "teller01", fullname: "Vũ Thanh Tâm", email: "teller@example.vn", phone: "0944111222", role: "TELLER", status: "ACTIVE" },
      { user_id: "A001", username: "admin01", fullname: "Quản trị viên", email: "admin@example.vn", phone: "0955000111", role: "ADMIN", status: "ACTIVE" },
    ],
    people: [
      { id: "U001", name: "Nguyễn Minh Anh", phone: "0901 234 567", acc: "0123 6868 8888", bal: 0, role: "CUSTOMER", locked: false },
      { id: "U002", name: "Trần Hoàng Nam", phone: "0912 345 678", acc: "0123 5555 1111", bal: 85400000, role: "CUSTOMER", locked: false },
      { id: "U003", name: "Lê Thu Hà", phone: "0933 456 789", acc: "0123 5555 2222", bal: 12300000, role: "CUSTOMER", locked: true },
      { id: "T001", name: "Vũ Thanh Tâm", phone: "0944 111 222", acc: "—", role: "TELLER", locked: false },
      { id: "A001", name: "Quản trị viên", phone: "0955 000 111", acc: "—", role: "ADMIN", locked: false },
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
    ADMIN: [["dashboard", "chart", "Tổng quan"], ["users", "users", "Người dùng"], ["staff", "users", "Nhân viên"], ["loans", "clipboard", "Duyệt vay"], ["audit", "list", "Nhật ký"], ["settings", "settings", "Cấu hình"]],
  };
  const esc = (v) => String(v).replace(/[&<>"']/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[c]));
  const money = (n) => Number(n).toLocaleString("vi-VN");
  // Số dư: U001 (khách hàng đang dùng giao diện khách) đọc trực tiếp từ state.balance
  const balOf = (p) => (p.id === "U001" && !s.live ? state.balance : p.bal || 0);
  const setBal = (p, v) => { if (p.id === "U001" && !s.live) state.balance = v; else p.bal = v; };
  const hasAcc = (p) => p.role === "CUSTOMER" && p.acc !== "—";
  // Tổng số dư hệ thống = tổng số dư tất cả tài khoản khách hàng (gồm cả tài khoản đang khóa)
  const totalBalance = () => s.people.filter(hasAcc).reduce((sum, p) => sum + balOf(p), 0);
  const compact = (n) => (n >= 1e9 ? `${(n / 1e9).toFixed(1).replace(".", ",")} tỷ ₫` : n >= 1e6 ? `${(n / 1e6).toFixed(1).replace(".", ",")} triệu ₫` : `${money(n)} ₫`);
  const label = () => (state.role === "ADMIN" ? "Quản trị viên" : "Giao dịch viên");
  const log = (msg) => s.audit.unshift([new Date().toTimeString().slice(0, 5), label(), msg]);
  const toast = (msg) => {
    const el = document.createElement("div");
    el.className = "toast";
    el.textContent = msg;
    document.body.append(el);
    setTimeout(() => el.remove(), 2400);
  };
  async function loadCustomers(query = s.q) {
    const result = await callBackend("userBridge", "searchCustomers", { session_token: state.sessionToken, query });
    const users = await Promise.all((result.customers || []).map(async (user) => {
      const accountResult = await callBackend("accountBridge", "getAccounts", { session_token: state.sessionToken, target_user_id: user.user_id });
      const account = (accountResult.accounts || []).find((item) => item.account_type === "CHECKING" && item.status === "ACTIVE") || (accountResult.accounts || [])[0];
      return { id: user.user_id, name: user.fullname, phone: user.phone, role: user.role, locked: user.status === "BLOCKED", acc: account?.account_id || "—", bal: Number(account?.balance_vnd || 0) };
    }));
    s.people.splice(0, s.people.length, ...users);
    s.live = true;
    render();
  }

  async function loadAuditLogs() {
    if (state.role !== "ADMIN") return;
    const result = await callBackend("auditBridge", "getAuditLogs", { session_token: state.sessionToken, page: 1, page_size: 50 });
    s.audit = (result.items || []).map((item) => {
      const created = String(item.created_at || "");
      const date = new Date(created.replace(" ", "T"));
      const at = Number.isNaN(date.valueOf()) ? created : date.toLocaleTimeString("vi-VN", { hour: "2-digit", minute: "2-digit" });
      const detail = [item.action, item.target_id, item.details].filter(Boolean).join(" · ");
      return [at, item.operator_id || "Hệ thống", detail];
    });
    render();
  }

  async function loadSystemConfig() {
    const config = await callBackend("staffBridge", "getSystemConfig", { session_token: state.sessionToken });
    s.fees = {
      interbank: Number(config.interbank_fee_vnd),
      atm: Number(config.atm_fee_vnd),
      rate: Number(config.savings_max_rate_percent),
    };
    if (Array.isArray(config.loan_terms) && config.loan_terms.length) {
      L.terms.splice(0, L.terms.length, ...config.loan_terms.map((term) => ({
        months: Number(term.months), enabled: Boolean(term.enabled), rate: Number(term.rate),
      })));
    }
    render();
  }

  async function saveSystemConfig(fees, terms) {
    await callBackend("staffBridge", "updateSystemConfig", {
      session_token: state.sessionToken,
      interbank_fee_vnd: Number(fees.interbank),
      atm_fee_vnd: Number(fees.atm),
      savings_max_rate_percent: Number(fees.rate),
      loan_terms: terms,
    });
    s.fees = { ...fees };
    L.terms.splice(0, L.terms.length, ...terms.map((term) => ({ ...term })));
    await loadAuditLogs();
  }

  async function loadStaffMembers() {
    const result = await callBackend("staffBridge", "getStaffMembers", { session_token: state.sessionToken });
    s.staffMembers.splice(0, s.staffMembers.length, ...(result.staff || []));
    render();
  }

  async function loadLoanApplications() {
    const result = await callBackend("loanBridge", "getApplications", { session_token: state.sessionToken });
    const applications = (result.applications || []).map((row) => {
      const status = String(row.status || "PENDING").toUpperCase();
      const stage = status === "REJECTED" ? -1
        : status === "DISBURSED" ? 4
        : status === "PAID" || status === "CLOSED" ? 5
        : status === "APPROVED" ? 3 : 0;
      return {
        id: row.loan_id,
        userId: row.user_id,
        who: row.customer_name || row.user_id,
        phone: row.customer_phone || "Chưa khai báo",
        type: "Vay cá nhân",
        amt: Number(row.amount_vnd || 0),
        term: Number(row.term_months || 0),
        rate: Number(row.interest_rate || 0),
        method: row.calc_method || "DECREASING",
        purpose: "Chưa lưu trong database",
        collateral: row.collateral_info || "Chưa khai báo",
        income: "Chưa khai báo",
        status,
        createdAt: row.created_at || "",
        stage,
        needInfo: false,
        resubmitted: false,
        result: status === "PENDING" ? null : { type: status, note: `Trạng thái hồ sơ: ${status}`, by: "Database", time: row.created_at || "" },
        docs: [],
      };
    });
    L.apps.splice(0, L.apps.length, ...applications);
    render();
  }
  const chip = (text, cls) => `<span class="chip ${cls}">${text}</span>`;
  const stat = (name, value, ic) => `<div class="stat"><span class="icon-box">${icon(ic)}</span><div><small>${name}</small><b>${value}</b></div></div>`;
  const fld = (id, name, text, ph, type = "text", val = "") =>
    `<div class="field"><label for="${id}">${text}</label><div class="input-wrap"><input id="${id}" name="${name}" type="${type}" placeholder="${ph}" value="${esc(val)}" required /></div></div>`;
  const head = (eyebrow, title) => `<div class="welcome"><div><p>${eyebrow}</p><h1 class="display">${title}</h1></div></div>`;
  const auditRows = (n) =>
    s.audit.length
      ? s.audit.slice(0, n).map(([t, who, msg]) => `<div class="row"><span class="icon-box">${icon("list")}</span><div class="info"><p>${esc(msg)}</p><small>${esc(who)}</small></div><small>${esc(t)}</small></div>`).join("")
      : '<p class="product-note">Chưa có nhật ký hoạt động.</p>';

  function dashboard() {
    const admin = state.role === "ADMIN";
    const pend = L.apps.filter((a) => a.stage >= 0 && a.stage < 3).length;
    const locked = s.people.filter((p) => p.locked).length;
    const stats = admin
      ? [[hasBackend() ? "Khách hàng" : "Tổng người dùng", s.people.length, "users"], [hasBackend() ? "Số dư tài khoản chính" : "Tổng số dư hệ thống", compact(totalBalance()), "wallet"], ["Khoản vay chờ duyệt", pend, "clipboard"], ["Khách hàng bị khóa", locked, "lock"]]
      : [["Giao dịch hôm nay", s.stats.txToday ?? "—", "send"], ["Hồ sơ vay chờ duyệt", pend, "clipboard"], ["Khách hàng bị khóa", locked, "lock"], ["Khách phục vụ hôm nay", s.stats.served ?? "—", "users"]];
    const quick = admin
      ? [["users", hasBackend() ? "Quản lý khách hàng" : "Quản lý người dùng"], ["staff", "Quản lý nhân viên"], ["loans", "Duyệt khoản vay"], ["settings", "Cấu hình phí & lãi suất"]]
      : [["customers", "Tra cứu khách hàng"], ["counter", "Giao dịch tại quầy"], ["loans", "Duyệt khoản vay"]];
    const today = new Date().toLocaleDateString("vi-VN", { weekday: "long", day: "2-digit", month: "long", year: "numeric" });
    return `
      <div class="welcome"><div><p>${admin ? "Bảng điều khiển hệ thống" : "Quầy giao dịch"}</p><h1 class="display">${label()}</h1></div><time>${today}</time></div>
      <div class="stat-grid">${stats.map((x) => stat(...x)).join("")}</div>
      <div class="content-grid">
        <section class="surface"><div class="surface-title"><h2 class="display">Hoạt động gần đây</h2></div>${hasBackend() && !admin ? '<p class="product-note">Nhật ký hoạt động chỉ dành cho Admin.</p>' : auditRows(4)}</section>
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
        <div class="info"><p>${p.name}</p><small>${p.phone} · ${p.acc}${hasAcc(p) ? ` · ${money(balOf(p))} ₫` : ""}</small></div>
        ${admin ? chip(roleName[p.role], "role") : ""}${chip(p.locked ? "Đã khóa" : "Hoạt động", p.locked ? "warn" : "ok")}
        <div class="row-actions">
          ${!admin ? `<button class="btn-sm" data-sact="open-acc" data-id="${p.id}">Mở tài khoản</button>` : ""}
          <button class="btn-sm ${p.locked ? "" : "danger"}" data-sact="toggle-lock" data-id="${p.id}" ${p.role === "ADMIN" ? "disabled" : ""}>${p.locked ? "Mở khóa" : "Khóa"}</button>
        </div>
      </div>`).join("");
    return `${head(admin ? (hasBackend() ? "Quản lý khách hàng" : "Quản trị người dùng") : "Tra cứu khách hàng", admin && !hasBackend() ? "Người dùng" : "Khách hàng")}
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

  const appStatus = (a) =>
    a.stage < 0 ? ["Đã từ chối", "warn"]
    : a.status === "PAID" ? ["Đã tất toán", "ok"]
    : a.status === "CLOSED" ? ["Đã đóng", "ok"]
    : a.stage === 4 ? ["Đã giải ngân", "ok"]
    : a.stage === 3 ? ["Đã duyệt · chờ giải ngân", "ok"]
    : a.needInfo ? ["Chờ khách bổ sung", "warn"]
    : a.resubmitted ? ["Khách đã bổ sung", "role"]
    : ["Chờ duyệt", "role"];
  const rank = (a) => (a.stage < 0 ? 3 : a.stage === 4 ? 2 : a.stage === 3 ? 1 : 0);

  function loansPage() {
    const rows = [...L.apps].sort((a, b) => rank(a) - rank(b)).map((a) => {
      const [t, c] = appStatus(a);
      return `
      <div class="row"><span class="icon-box">${icon("card")}</span>
        <div class="info"><p>${esc(a.who)} · ${money(a.amt)} ₫</p><small>${a.id} · ${a.type} · ${a.term} tháng</small></div>
        ${chip(t, c)}
        <div class="row-actions"><button class="btn-sm" data-sact="loan-view" data-id="${a.id}">Xem chi tiết</button></div>
      </div>`;
    }).join("");
    return `${head("Quản lý tín dụng", "Duyệt khoản vay")}<section class="surface">${rows}</section>`;
  }

  const mimeName = (m) => ({ "image/jpeg": "Ảnh JPG", "image/png": "Ảnh PNG", "application/pdf": "Tài liệu PDF" }[m] || m || "—");

  // Chi tiết một giấy tờ trong hồ sơ: thông tin tệp, xem trước, xác nhận / yêu cầu nộp lại
  function docDetail(a, d, decided) {
    const [t, c] = L.docStatus[d.status];
    const fact = (k, v) => `<div><small>${k}</small><b>${esc(v || "—")}</b></div>`;
    const preview = d.status === "MISSING"
      ? `<div class="doc-preview empty">${icon("file")}<p>Khách hàng chưa nộp giấy tờ này.</p></div>`
      : d.url && (d.mime || "").startsWith("image/")
        ? `<div class="doc-preview"><img src="${d.url}" alt="${esc(d.name)}" /></div>`
        : `<div class="doc-preview empty">${icon(d.icon)}<p>${esc(d.file)}</p><small>${d.url ? `<a href="${d.url}" target="_blank" rel="noopener">Mở tệp trong tab mới</a>` : "Bản xem trước chưa khả dụng (dữ liệu mô phỏng)"}</small></div>`;
    const actions = !decided && d.status === "REVIEW"
      ? `<div class="loan-actions"><button class="primary" data-sact="doc-ok" data-id="${a.id}" data-doc="${d.id}">Xác nhận giấy tờ</button><button class="secondary" data-sact="doc-redo" data-doc="${d.id}">Yêu cầu nộp lại</button></div>`
      : !decided && d.status === "MISSING" ? `<div class="loan-actions"><button class="secondary" data-sact="doc-redo" data-doc="${d.id}">Gửi yêu cầu bổ sung</button></div>` : "";
    return `
      <div class="doc-head"><button class="btn-sm" data-sact="doc-back">← Quay lại hồ sơ vay</button>${chip(t, c)}</div>
      <h4 class="modal-sub">${esc(d.name)}</h4>
      ${preview}
      <div class="loan-facts">${fact("Thuộc hồ sơ", a.id)}${fact("Trạng thái", t)}${fact("Tên tệp", d.file)}${fact("Loại tệp", d.file ? mimeName(d.mime) : "")}${fact("Dung lượng", d.size)}${fact("Thời gian nộp", d.at)}</div>
      ${actions}`;
  }

  // Hộp thông tin khoản vay: xem đủ thông tin & hồ sơ trước khi phê duyệt / từ chối / yêu cầu bổ sung
  function loanModal() {
    const a = s.detail && L.apps.find((x) => x.id === s.detail);
    if (!a) return "";
    const monthly = Math.round(a.amt / a.term + (a.amt * a.rate) / 1200);
    const methodName = { DECREASING: "Dư nợ giảm dần", FIXED: "Niên kim cố định" }[a.method] || a.method;
    const missing = a.docs.filter((d) => d.status === "MISSING").length;
    const unverified = a.docs.filter((d) => d.status !== "DONE").length;
    const decided = a.stage >= 3 || a.stage < 0;
    const [st, sc] = appStatus(a);
    const fact = (k, v) => `<div><small>${k}</small><b>${esc(v)}</b></div>`;
    const facts = [
      fact("Khách hàng", a.who), fact("Số điện thoại", a.phone), fact("Loại vay", a.type), fact("Số tiền vay", `${money(a.amt)} ₫`),
      fact("Kỳ hạn", `${a.term} tháng`), fact("Lãi suất", `${a.rate}%/năm`), fact("Phương thức trả nợ", methodName), fact("Trả hàng tháng (ước tính)", `${money(monthly)} ₫`),
      fact("Tổng lãi (ước tính)", `${money(monthly * a.term - a.amt)} ₫`), fact("Mục đích vay", a.purpose), fact("Tài sản đảm bảo", a.collateral), fact("Thu nhập khai báo", a.income),
    ].join("");
    const docs = a.docs.length ? a.docs.map((d) => {
      const [t, c] = L.docStatus[d.status];
      const verify = !decided && d.status === "REVIEW" ? `<button class="btn-sm" data-sact="doc-ok" data-id="${a.id}" data-doc="${d.id}">Xác nhận</button>` : "";
      return `<div class="row"><button class="doc-open" type="button" data-sact="doc-view" data-id="${a.id}" data-doc="${d.id}" title="Xem chi tiết giấy tờ"><span class="icon-box">${icon(d.icon)}</span><span class="info"><p>${esc(d.name)}</p><small>${d.file ? esc(d.file) : d.hint}</small></span></button>${chip(t, c)}${verify}</div>`;
    }).join("") : `<p class="product-note">Database hiện chưa lưu tệp hồ sơ vay; tên, mục đích vay và thu nhập cũng chưa có cột tương ứng.</p>`;
    const header = `<div class="modal-head"><span class="icon-box">${icon("card")}</span><div><h3>Thông tin khoản vay</h3><small>${a.id}</small></div>${chip(st, sc)}<button class="btn-sm" data-sact="loan-close">Đóng</button></div>`;
    const dv = s.docView && a.docs.find((d) => d.id === s.docView);
    if (dv) return `<div class="modal-backdrop"><div class="modal-card wide">${header}${docDetail(a, dv, decided)}</div></div>`;
    let foot;
    if (s.mode === "reject") {
      foot = `<form id="reject-form" class="modal-form"><span class="eyebrow">Từ chối khoản vay</span>
        <div class="field"><label for="rj-note">${hasBackend() ? "Lý do từ chối (lưu vào nhật ký)" : "Lý do từ chối (gửi cho khách hàng)"}</label><div class="input-wrap area"><textarea id="rj-note" name="note" rows="3" required placeholder="Ví dụ: Thu nhập chưa đáp ứng điều kiện vay"></textarea></div></div>
        <div class="modal-actions"><button class="secondary" type="button" data-sact="loan-back">Quay lại</button><button class="primary" type="submit">Xác nhận từ chối</button></div></form>`;
    } else if (s.mode === "info") {
      const boxes = a.docs.map((d) => `<label class="check"><input type="checkbox" name="docs" value="${d.id}" ${d.status === "MISSING" || d.id === s.preDoc ? "checked" : ""} /> ${esc(d.name)}</label>`).join("");
      foot = `<form id="info-form" class="modal-form"><span class="eyebrow">Giấy tờ cần khách bổ sung</span>
        <div class="check-list">${boxes}</div>
        <div class="field"><label for="ix">Giấy tờ khác (nếu có)</label><div class="input-wrap"><input id="ix" name="extra" placeholder="Ví dụ: Hợp đồng mua bán tài sản" /></div></div>
        <div class="field"><label for="im">Nội dung gửi khách hàng</label><div class="input-wrap area"><textarea id="im" name="note" rows="3" required placeholder="Nêu rõ thông tin / giấy tờ cần bổ sung"></textarea></div></div>
        <div class="modal-actions"><button class="secondary" type="button" data-sact="loan-back">Quay lại</button><button class="primary" type="submit">Gửi yêu cầu bổ sung</button></div></form>`;
    } else if (decided) {
      foot = a.result && a.result.type !== "SUBMITTED" ? `<div class="notice ${a.stage < 0 ? "warn" : "ok"}"><b>${a.stage < 0 ? "Đã từ chối" : "Đã phê duyệt"}</b><p>${esc(a.result.note)}</p><small>${esc(a.result.by || "")} · ${a.result.time}</small></div>` : "";
    } else {
      const docsUnavailable = hasBackend() && a.docs.length === 0;
      const hint = docsUnavailable
        ? `<p class="product-note">Chưa có bảng lưu và tải lên tài liệu vay; hệ thống tạm khóa phê duyệt hồ sơ này.</p>`
        : unverified ? `<p class="product-note">${missing ? `Hồ sơ còn thiếu ${missing} giấy tờ` : "Còn giấy tờ đang chờ kiểm tra"} — cần xác nhận đủ hồ sơ trước khi phê duyệt.</p>` : "";
      const pendingNote = a.needInfo && a.result ? `<div class="notice warn"><b>Đang chờ khách bổ sung</b><p>${esc(a.result.note)}</p><small>${esc(a.result.by || "")} · ${a.result.time}</small></div>` : "";
      foot = `${pendingNote}${hint}<div class="loan-actions"><button class="primary" data-sact="loan-approve" data-id="${a.id}" ${docsUnavailable || unverified || a.needInfo ? "disabled" : ""}>Phê duyệt</button><button class="secondary" data-sact="loan-info-start" ${hasBackend() ? "disabled" : ""}>Yêu cầu bổ sung</button><button class="secondary" data-sact="loan-reject-start">Từ chối</button></div>`;
    }
    return `
      <div class="modal-backdrop"><div class="modal-card wide">
        ${header}
        <div class="loan-facts">${facts}</div>
        <h4 class="modal-sub">Hồ sơ đính kèm <span class="modal-hint">· bấm vào từng giấy tờ để xem chi tiết</span></h4><div>${docs}</div>
        ${foot}
      </div></div>`;
  }

  const audit = () => `${head("An ninh hệ thống", "Nhật ký hoạt động")}<section class="surface">${auditRows(50)}</section>`;

  function staffPage() {
    const rows = s.staffMembers.map((member) => `
      <div class="row">
        <span class="icon-box">${icon("users")}</span>
        <div class="info"><p>${esc(member.fullname)} · ${esc(member.role === "ADMIN" ? "Quản trị viên" : "Giao dịch viên")}</p>
          <small>${esc(member.username)} · ${esc(member.email)} · ${esc(member.phone)}</small></div>
        <span class="chip ${member.status === "ACTIVE" ? "ok" : "warn"}">${esc(member.status === "ACTIVE" ? "Đang hoạt động" : "Đã khóa")}</span>
      </div>`).join("");
    return `${head("Quản trị nhân sự", "Nhân viên")}
      <div class="stack">
        <section class="form-card"><h2 class="display">Tạo tài khoản nhân viên</h2>
          <form id="staff-create-form">
            <div class="form-grid">
              ${fld("staff-username", "username", "Tên đăng nhập", "vd: teller01")}
              ${fld("staff-name", "fullname", "Họ và tên", "Nguyễn Văn A")}
              ${fld("staff-email", "email", "Email", "name@example.vn", "email")}
              ${fld("staff-phone", "phone", "Số điện thoại", "10 chữ số", "tel")}
              ${fld("staff-cccd", "cccd", "Số CCCD", "12 chữ số")}
              ${fld("staff-password", "password", "Mật khẩu khởi tạo", "12+ ký tự, gồm chữ hoa/thường và số", "password")}
              <div class="field"><label for="staff-role">Vai trò</label><div class="input-wrap"><select id="staff-role" name="role" required><option value="TELLER">Giao dịch viên</option><option value="ADMIN">Quản trị viên</option></select></div></div>
            </div>
            <p class="product-note">Mỗi tài khoản nhân viên cần email, số điện thoại và CCCD chưa được dùng trong hệ thống.</p>
            <button class="primary" type="submit">Tạo nhân viên ${icon("check")}</button>
          </form>
        </section>
        <section class="surface"><div class="surface-title"><h2 class="display">Danh sách nhân viên</h2>
          ${hasBackend() ? '<button class="secondary" type="button" data-sact="reload-staff">Tải lại</button>' : ""}</div>
          ${rows || '<p class="product-note">Chưa có nhân viên.</p>'}
        </section>
      </div>`;
  }

  function settings() {
    const terms = L.terms.map((t) => `
      <div class="term-row">
        <label class="check"><input type="checkbox" name="on-${t.months}" ${t.enabled ? "checked" : ""} /> Kỳ hạn ${t.months} tháng</label>
        <div class="field"><label for="tr-${t.months}">Lãi suất vay (%/năm)</label><div class="input-wrap"><input id="tr-${t.months}" name="rate-${t.months}" type="number" step="0.1" min="0.1" max="50" value="${t.rate}" required /></div></div>
      </div>`).join("");
    return `${head("Tham số hệ thống", "Cấu hình")}
      <div class="stack">
      <section class="form-card"><h2 class="display">Kỳ hạn vay</h2>
        <form id="terms-form">
          <div class="term-list">${terms}</div>
          <p class="product-note">Khách hàng chỉ thấy các kỳ hạn đang bật khi đăng ký vay. Hồ sơ đã nộp giữ nguyên lãi suất tại thời điểm đăng ký.</p>
          <button class="primary" type="submit">Lưu kỳ hạn vay ${icon("check")}</button>
        </form>
      </section>
      <section class="form-card"><h2 class="display">Phí & lãi suất</h2>
        <form id="settings-form">
          <div class="form-grid">
            ${fld("f-inter", "interbank", "Phí chuyển liên ngân hàng (₫)", "9900", "number", s.fees.interbank)}
            ${fld("f-atm", "atm", "Phí rút ATM ngoại mạng (₫)", "3300", "number", s.fees.atm)}
          </div>
          ${fld("f-rate", "rate", "Lãi suất tiết kiệm tối đa (%/năm)", "5.8", "number", s.fees.rate)}
          <button class="primary" type="submit">Lưu cấu hình ${icon("check")}</button>
        </form>
      </section>
      </div>`;
  }

  const pages = { dashboard, customers: () => peoplePage(false), users: () => peoplePage(true), staff: staffPage, counter, loans: loansPage, audit, settings };

  return {
    get page() { return s.page; },
    // Nhận dữ liệu đã chuẩn hoá từ data.js (nạp từ database)
    hydrate(key, value) {
      if (key === "people") { s.people = value; s.live = true; }
      else if (key === "audit") s.audit = value;
      else if (key === "config") s.fees = value.fees;
      else if (key === "stats") s.stats = value;
    },
    login(identifier) {
      const v = identifier.trim().toLowerCase();
      state.role = v.startsWith("admin") ? "ADMIN" : /^(teller|gdv)/.test(v) ? "TELLER" : "CUSTOMER";
      s.page = "dashboard";
      s.q = "";
      if (hasBackend()) {
        s.people.splice(0, s.people.length);
        L.apps.splice(0, L.apps.length);
        s.audit.splice(0, s.audit.length);
        s.stats = { txToday: null, served: null };
        s.live = true;
        loadCustomers("").catch((error) => toast("Không tải được khách hàng: " + (error?.message || String(error))));
        loadLoanApplications().catch((error) => toast("Không tải được hồ sơ vay: " + (error?.message || String(error))));
        if (state.role === "ADMIN") loadAuditLogs().catch((error) => toast("Không tải được nhật ký: " + (error?.message || String(error))));
        if (state.role === "ADMIN") loadStaffMembers().catch((error) => toast("Không tải được nhân viên: " + (error?.message || String(error))));
        loadSystemConfig().catch((error) => toast("Không tải được cấu hình: " + (error?.message || String(error))));
      }
      s.detail = null;
      s.mode = null;
      s.docView = null;
    },
    render() {
      const admin = state.role === "ADMIN";
      const nav = NAV[state.role].map(([page, image, title]) => [page, image, page === "users" && hasBackend() ? "Khách hàng" : title]);
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
        <nav class="mobile-nav">${mobile}</nav>${loanModal()}`;
    },
    click(event) {
      const el = event.target.closest("[data-spage],[data-sact]");
      if (!el || state.role === "CUSTOMER") return false;
      if (el.dataset.spage) {
        s.page = el.dataset.spage;
        s.q = "";
        window.scrollTo({ top: 0, behavior: "smooth" });
        render();
        if (s.page === "staff" && hasBackend()) {
          loadStaffMembers().catch((error) => toast("Không tải được nhân viên: " + (error?.message || String(error))));
        }
        if (s.page === "settings" && hasBackend()) {
          loadSystemConfig().catch((error) => toast("Không tải được cấu hình: " + (error?.message || String(error))));
        }
        return true;
      }
      const id = el.dataset.id;
      const act = el.dataset.sact;
      const person = s.people.find((p) => p.id === id);
      if (act === "reload-staff" && hasBackend()) {
        loadStaffMembers().catch((error) => toast("Không tải được nhân viên: " + (error?.message || String(error))));
      } else if (act === "logout") {
        onLogout();
        render();
      } else if (act === "toggle-lock" && person) {
        if (hasBackend()) {
          callBackend("userBridge", "toggleUserLock", { session_token: state.sessionToken, target_user_id: person.id })
            .then(() => { person.locked = !person.locked; toast(person.locked ? "Đã khóa tài khoản" : "Đã mở khóa tài khoản"); return state.role === "ADMIN" ? loadAuditLogs() : render(); })
            .catch((error) => toast("Không đổi được trạng thái: " + (error?.message || String(error))));
        } else {
          person.locked = !person.locked;
          log(`${person.locked ? "Khóa" : "Mở khóa"} tài khoản của ${person.name}`);
          toast(person.locked ? "Đã khóa tài khoản" : "Đã mở khóa tài khoản");
          render();
        }
      } else if (act === "open-acc" && person) {
        if (hasBackend()) {
          callBackend("accountBridge", "openNewAccount", { session_token: state.sessionToken, target_user_id: person.id, account_type: "CHECKING" })
            .then(() => loadCustomers(s.q).then(() => { toast(`Đã mở tài khoản cho ${person.name}`); return state.role === "ADMIN" ? loadAuditLogs() : undefined; }))
            .catch((error) => toast("Không mở được tài khoản: " + (error?.message || String(error))));
        } else {
          log(`Mở tài khoản thanh toán mới cho ${person.name}`);
          toast(`Đã mở tài khoản mới cho ${person.name}`);
        }
      } else if (act === "loan-view") {
        s.detail = id;
        s.mode = null;
        s.docView = null;
        s.preDoc = null;
        render();
      } else if (act === "loan-close") {
        s.detail = null;
        s.mode = null;
        s.docView = null;
        s.preDoc = null;
        render();
      } else if (act === "loan-back") {
        s.mode = null;
        s.preDoc = null;
        render();
      } else if (act === "doc-view") {
        s.docView = el.dataset.doc;
        render();
      } else if (act === "doc-back") {
        s.docView = null;
        render();
      } else if (act === "doc-redo") {
        s.preDoc = el.dataset.doc;
        s.docView = null;
        s.mode = "info";
        render();
      } else if (act === "loan-reject-start" || act === "loan-info-start") {
        s.mode = act === "loan-reject-start" ? "reject" : "info";
        render();
      } else if (act === "doc-ok") {
        const loan = L.apps.find((l) => l.id === id);
        const doc = loan && loan.docs.find((d) => d.id === el.dataset.doc);
        if (doc) {
          doc.status = "DONE";
          doc.hint = "Đã được ngân hàng xác nhận";
          log(`Xác nhận giấy tờ "${doc.name}" của hồ sơ ${loan.id}`);
          render();
        }
      } else if (act === "loan-approve") {
        const loan = L.apps.find((l) => l.id === id);
        if (loan && loan.docs.every((d) => d.status === "DONE") && !loan.needInfo) {
          if (hasBackend() && !loan.docs.length) {
            toast("Chưa thể phê duyệt: tài liệu vay chưa được lưu vào hệ thống.");
            return true;
          }
          if (hasBackend()) {
            callBackend("loanBridge", "approveLoan", { session_token: state.sessionToken, loan_id: id, action: "APPROVE", reason: "" })
              .then(() => loadLoanApplications())
              .then(() => state.role === "ADMIN" ? loadAuditLogs() : undefined)
              .then(() => { s.detail = id; toast("Đã cập nhật phê duyệt trong database. Thông báo khách hàng chưa được tích hợp."); render(); })
              .catch((error) => toast("Phê duyệt thất bại: " + (error?.message || String(error))));
            return true;
          }
          const note = "Khoản vay đã được phê duyệt. Vui lòng vào mục Giải ngân để nhận tiền.";
          loan.stage = 3;
          loan.result = { type: "APPROVED", note, by: label(), time: new Date().toTimeString().slice(0, 5) };
          log(`Phê duyệt khoản vay ${loan.id}`);
          L.notify(loan, "APPROVED", note);
          toast("Đã phê duyệt · kết quả đã gửi tới khách hàng");
          render();
        }
      }
      return true;
    },
    async submit(event) {
      const f = event.target;
      if (state.role === "CUSTOMER") return false;
      if (f.id === "staff-create-form") {
        const d = new FormData(f);
        const staff = {
          session_token: state.sessionToken,
          username: String(d.get("username") || "").trim(),
          fullname: String(d.get("fullname") || "").trim(),
          email: String(d.get("email") || "").trim(),
          phone: String(d.get("phone") || "").trim(),
          cccd: String(d.get("cccd") || "").trim(),
          role: String(d.get("role") || "TELLER"),
          password: String(d.get("password") || ""),
        };
        if (hasBackend()) {
          try {
            const result = await callBackend("staffBridge", "createStaff", staff);
            await loadStaffMembers();
            await loadAuditLogs();
            f.reset();
            toast(`Đã tạo tài khoản nhân viên ${result.created_id}`);
          } catch (error) {
            toast("Không tạo được nhân viên: " + (error?.message || String(error)));
          }
          return true;
        }
        s.staffMembers.unshift({
          user_id: `DEMO_${Date.now()}`,
          username: staff.username,
          fullname: staff.fullname,
          email: staff.email,
          phone: staff.phone,
          role: staff.role,
          status: "ACTIVE",
        });
        log(`Tạo nhân viên mẫu ${staff.username}`);
        f.reset();
        toast("Đã thêm nhân viên vào dữ liệu xem trước");
        render();
        return true;
      }
      if (f.id === "staff-search") {
        s.q = f.elements.q.value.trim();
        if (hasBackend()) {
          try { await loadCustomers(s.q); } catch (error) { toast("Tra cứu thất bại: " + (error?.message || String(error))); }
        } else render();
        return true;
      }
      if (f.id === "counter-form") {
        const d = new FormData(f);
        const deposit = d.get("type") === "DEPOSIT";
        const t = deposit ? "Nạp" : "Rút";
        const amount = Number(d.get("amount"));
        const digits = String(d.get("acc")).replace(/\D/g, "");
        const target = s.people.find((p) => hasAcc(p) && p.acc.replace(/\D/g, "") === digits);
        if (!target) { toast("Không tìm thấy tài khoản"); return true; }
        if (!(amount > 0)) { toast("Số tiền không hợp lệ"); return true; }
        if (target.locked) { toast("Tài khoản đang bị khóa, không thể giao dịch"); return true; }
        if (!deposit && amount > balOf(target)) { toast("Số dư tài khoản không đủ để rút"); return true; }
        if (hasBackend()) {
          if (!deposit) { toast("Rút tiền tại quầy chưa có API an toàn cho tài khoản khách hàng."); return true; }
          try {
            const result = await callBackend("transactionBridge", "depositCounter", { session_token: state.sessionToken, target_account_id: target.acc, amount_vnd: amount, description: "Nạp tiền tại quầy" });
            setBal(target, Number(result.post_balance_vnd || 0));
            toast(`Nạp tiền thành công · số dư mới ${money(target.bal)} ₫`);
            f.reset();
            render();
          } catch (error) { toast("Nạp tiền thất bại: " + (error?.message || String(error))); }
          return true;
        }
        setBal(target, balOf(target) + (deposit ? amount : -amount));
        log(`${t} ${money(amount)} ₫ tài khoản ${target.acc}`);
        if (target.id === "U001") {
          L.message({
            title: deposit ? "Nạp tiền tại quầy thành công" : "Rút tiền tại quầy thành công",
            preview: `${deposit ? "+" : "-"}${money(amount)} ₫ tại quầy giao dịch`,
            icon: deposit ? "wallet" : "send",
            content: `Giao dịch ${deposit ? "nạp" : "rút"} ${money(amount)} ₫ tiền mặt tại quầy đã hoàn tất. Số dư khả dụng: ${money(state.balance)} ₫.`,
            reference: `CT${Date.now().toString().slice(-12)}`,
          });
        }
        toast(`${t} tiền thành công · số dư mới ${money(balOf(target))} ₫`);
        f.reset();
        render();
        return true;
      }
      if (f.id === "reject-form" || f.id === "info-form") {
        const loan = L.apps.find((l) => l.id === s.detail);
        if (!loan) return true;
        const d = new FormData(f);
        const note = String(d.get("note")).trim();
        const time = new Date().toTimeString().slice(0, 5);
        if (f.id === "reject-form") {
          if (hasBackend()) {
            try {
              await callBackend("loanBridge", "approveLoan", { session_token: state.sessionToken, loan_id: loan.id, action: "REJECT", reason: note });
              await loadLoanApplications();
              if (state.role === "ADMIN") await loadAuditLogs();
              s.detail = loan.id;
              s.mode = null;
              toast("Đã cập nhật từ chối và lưu lý do vào nhật ký. Thông báo khách hàng chưa được tích hợp.");
              render();
            } catch (error) { toast("Từ chối thất bại: " + (error?.message || String(error))); }
            return true;
          }
          loan.stage = -1;
          loan.needInfo = false;
          loan.result = { type: "REJECTED", note, by: label(), time };
          log(`Từ chối khoản vay ${loan.id}`);
          L.notify(loan, "REJECTED", note);
          toast("Đã từ chối · lý do đã gửi tới khách hàng");
        } else {
          if (hasBackend()) {
            toast("Yêu cầu bổ sung chưa thể lưu: database chưa có bảng hồ sơ và tài liệu vay.");
            return true;
          }
          const picked = d.getAll("docs");
          const extra = String(d.get("extra")).trim();
          if (!picked.length && !extra) {
            toast("Chọn ít nhất một giấy tờ hoặc nhập giấy tờ khác");
            return true;
          }
          loan.docs.forEach((x) => { if (picked.includes(x.id)) { x.status = "MISSING"; x.file = x.mime = x.size = x.at = x.url = ""; x.hint = "Chưa có tệp"; } });
          if (extra) loan.docs.push({ id: `x${Date.now()}`, icon: "file", name: extra, hint: "Chưa có tệp", status: "MISSING", file: "", mime: "", size: "", at: "", url: "" });
          loan.needInfo = true;
          loan.resubmitted = false;
          loan.result = { type: "INFO", note, by: label(), time };
          log(`Yêu cầu bổ sung hồ sơ ${loan.id}`);
          L.notify(loan, "INFO", note);
          toast("Đã gửi yêu cầu bổ sung tới khách hàng");
        }
        s.mode = null;
        s.preDoc = null;
        render();
        return true;
      }
      if (f.id === "terms-form") {
        const d = new FormData(f);
        const next = L.terms.map((t) => ({ months: t.months, enabled: d.get(`on-${t.months}`) !== null, rate: Number(d.get(`rate-${t.months}`)) }));
        if (!next.some((t) => t.enabled)) { toast("Cần bật ít nhất một kỳ hạn vay"); return true; }
        if (next.some((t) => !(t.rate > 0 && t.rate <= 50))) { toast("Lãi suất phải trong khoảng 0 – 50%/năm"); return true; }
        if (hasBackend()) {
          try {
            await saveSystemConfig(s.fees, next);
            toast("Đã lưu kỳ hạn vay vào database");
            render();
          } catch (error) { toast("Không lưu được kỳ hạn vay: " + (error?.message || String(error))); }
          return true;
        }
        next.forEach((t, i) => Object.assign(L.terms[i], t));
        const on = next.filter((t) => t.enabled).map((t) => `${t.months} tháng (${t.rate}%)`).join(", ");
        log(`Cập nhật kỳ hạn vay: ${on}`);
        toast("Đã lưu kỳ hạn vay");
        render();
        return true;
      }
      if (f.id === "settings-form") {
        const d = new FormData(f);
        const fees = { interbank: +d.get("interbank"), atm: +d.get("atm"), rate: +d.get("rate") };
        if (hasBackend()) {
          try {
            await saveSystemConfig(fees, L.terms);
            toast("Đã lưu cấu hình vào database");
            render();
          } catch (error) { toast("Không lưu được cấu hình: " + (error?.message || String(error))); }
          return true;
        }
        s.fees = fees;
        log("Cập nhật cấu hình phí & lãi suất");
        toast("Đã lưu cấu hình");
        return true;
      }
      return false;
    },
  };
}
