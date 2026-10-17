import { getIcon, renderBrand, renderField } from '../icons.js';
import { state } from '../state.js';

export function renderLoginView() {
  const isLogin = state.authMode === 'login';

  return `
    <main class="min-h-screen bg-[#e9dece] p-3 text-[#3d2d23] sm:p-5 lg:p-7">
      <section class="mx-auto grid min-h-[calc(100vh-1.5rem)] max-w-[1440px] overflow-hidden rounded-[28px] bg-[#f8f2e8] shadow-[0_24px_80px_rgba(73,49,32,0.16)] sm:min-h-[calc(100vh-2.5rem)] sm:rounded-[36px] lg:min-h-[calc(100vh-3.5rem)] lg:grid-cols-[0.92fr_1.08fr]">
        
        <!-- Banner bên trái -->
        <aside class="relative hidden min-h-full overflow-hidden lg:block">
          <img
            class="absolute inset-0 size-full object-cover"
            src="https://images.unsplash.com/photo-1758448093806-88b2089068ab?crop=entropy&cs=tinysrgb&fit=crop&fm=jpg&q=85&w=1600"
            alt="An Gia Private Banking"
          />
          <div class="absolute inset-0 bg-[linear-gradient(180deg,rgba(45,30,20,0.08)_25%,rgba(38,25,17,0.88)_100%)]"></div>
          <div class="absolute inset-x-0 top-0 flex items-center justify-between p-10">
            ${renderBrand("OOPBANK")}
            <div class="flex items-center gap-2 rounded-full border border-white/30 bg-white/15 px-3 py-2 text-[10px] font-bold uppercase tracking-[0.16em] text-white backdrop-blur-md">
              ${getIcon("shield", "size-3.5")}
              Bảo mật chuẩn quốc tế
            </div>
          </div>
          <div class="absolute inset-x-0 bottom-0 p-10 xl:p-14">
            <div class="mb-6 h-px w-16 bg-[#d2b18f]"></div>
            <p class="font-display max-w-xl text-4xl leading-[1.15] text-white xl:text-5xl">
              Vững tâm hôm nay,<br />thịnh vượng ngày mai.
            </p>
            <p class="mt-5 max-w-md text-sm leading-6 text-white/75">
              Đồng hành cùng bạn kiến tạo một tương lai tài chính bền vững với dịch vụ ngân hàng riêng tư và tận tâm.
            </p>
            <div class="mt-8 flex gap-2">
              <span class="h-1.5 w-8 rounded-full bg-[#d2b18f]"></span>
              <span class="size-1.5 rounded-full bg-white/40"></span>
              <span class="size-1.5 rounded-full bg-white/40"></span>
            </div>
          </div>
        </aside>

        <!-- Form Đăng nhập / Đăng ký -->
        <div class="relative flex min-h-full flex-col px-5 py-6 sm:px-10 sm:py-8 xl:px-20">
          <div class="flex items-center justify-between lg:justify-end">
            <div class="lg:hidden">
              ${renderBrand()}
            </div>
            <button class="text-xs font-semibold text-[#786457] transition hover:text-[#4c3425]" type="button">
              Trợ giúp
            </button>
          </div>

          <div class="mx-auto flex w-full max-w-[470px] flex-1 flex-col justify-center py-10">
            <div class="mb-8">
              <span class="mb-3 block text-[11px] font-bold uppercase tracking-[0.24em] text-[#a0714e]">
                ${isLogin ? "Chào mừng trở lại" : "Bắt đầu hành trình"}
              </span>
              <h1 class="font-display text-4xl leading-tight tracking-[-0.02em] text-[#38281f] sm:text-[44px]">
                ${isLogin ? "Đăng nhập tài khoản" : "Tạo tài khoản mới"}
              </h1>
              <p class="mt-3 text-sm leading-6 text-[#806e61]">
                ${isLogin ? "Quản lý tài chính của bạn an toàn, mọi lúc mọi nơi." : "Chỉ mất vài phút để gia nhập An Gia Private Banking."}
              </p>
            </div>

            <!-- Tab Switcher -->
            <div class="mb-7 grid grid-cols-2 rounded-2xl bg-[#eee4d7] p-1">
              <button
                id="tab-login"
                class="rounded-xl px-4 py-3 text-sm font-bold transition ${isLogin ? "bg-[#fffdf8] text-[#4a3325] shadow-sm" : "text-[#917c6c] hover:text-[#5a4030]"}"
                type="button"
              >
                Đăng nhập
              </button>
              <button
                id="tab-register"
                class="rounded-xl px-4 py-3 text-sm font-bold transition ${!isLogin ? "bg-[#fffdf8] text-[#4a3325] shadow-sm" : "text-[#917c6c] hover:text-[#5a4030]"}"
                type="button"
              >
                Đăng ký
              </button>
            </div>

            <form id="auth-form" class="space-y-5">
              ${!isLogin ? renderField({ id: "name", label: "Họ và tên", icon: "user", placeholder: "Nguyễn Minh Anh" }) : ""}
              
              ${renderField({
                id: "contact",
                label: isLogin ? "tên đăng nhập hoặc số điện thoại" : "Địa chỉ email",
                icon: "mail",
                type: "text",
                placeholder: isLogin ? "nhapten / 0901234567" : "tenban@email.com"
              })}

              ${!isLogin ? `
                ${renderField({ id: "phone", label: "Số điện thoại", icon: "phone", type: "tel", placeholder: "0901 234 567" })}
                ${renderField({ id: "cccd", label: "Số CCCD", icon: "shield", placeholder: "Nhập 12 số căn cước công dân" })}
              ` : ""}

              ${renderField({
                id: "password",
                label: "Mật khẩu",
                icon: "lock",
                type: state.showPassword ? "text" : "password",
                placeholder: isLogin ? "Nhập mật khẩu của bạn" : "Tối thiểu 8 ký tự",
                actionHtml: `
                  <button
                    id="btn-toggle-password"
                    class="rounded-lg p-1 text-[#907c6d] transition hover:bg-[#eee4d7] hover:text-[#4a3325]"
                    type="button"
                    aria-label="Ẩn hiện mật khẩu"
                  >
                    ${getIcon(state.showPassword ? "eyeOff" : "eye", "size-4.5")}
                  </button>
                `
              })}

              <div class="flex items-center gap-3 text-xs ${isLogin ? "justify-end" : "justify-between"}">
                ${!isLogin ? `
                  <label class="flex cursor-pointer items-center gap-2.5 text-[#756356]">
                    <input id="remember-check" type="checkbox" ${state.remember ? "checked" : ""} class="peer sr-only" />
                    <span class="grid size-4.5 place-items-center rounded-md border border-[#cbbbad] bg-[#fffdf8] text-transparent transition peer-checked:border-[#805638] peer-checked:bg-[#805638] peer-checked:text-white">
                      ${getIcon("check", "size-3")}
                    </span>
                    Tôi đồng ý với điều khoản
                  </label>
                ` : `
                  <button class="font-bold text-[#81593e] hover:underline" type="button">
                    Quên mật khẩu?
                  </button>
                `}
              </div>

              <button
                class="group flex h-14 w-full items-center justify-center gap-3 rounded-2xl bg-[#765038] px-6 text-sm font-bold text-white shadow-[0_12px_25px_rgba(93,61,39,0.18)] transition hover:-translate-y-0.5 hover:bg-[#62412d] focus:outline-none focus:ring-4 focus:ring-[#765038]/20 active:translate-y-0"
                type="submit"
              >
                ${isLogin ? "Đăng nhập an toàn" : "Tạo tài khoản"}
                ${getIcon("arrow", "size-4 transition group-hover:translate-x-1")}
              </button>
            </form>

            ${state.formSuccess ? `
              <div class="mt-4 flex items-center gap-2 rounded-xl border border-[#b7c7ae] bg-[#edf3e9] px-4 py-3 text-xs font-semibold text-[#45603d]">
                ${getIcon("check", "size-4")}
                ${isLogin ? "Thông tin đăng nhập đã được xác nhận." : "Đăng ký thành công. Vui lòng kiểm tra email."}
              </div>
            ` : ""}

            <div class="mt-8 flex items-center justify-center gap-2 text-xs text-[#897669]">
              ${getIcon("shield", "size-4 text-[#9b765a]")}
              Dữ liệu của bạn được mã hóa và bảo vệ 24/7
            </div>
          </div>

          <footer class="flex flex-wrap items-center justify-center gap-x-6 gap-y-2 border-t border-[#e3d8ca] pt-5 text-[10px] font-semibold uppercase tracking-[0.12em] text-[#9a887a] sm:justify-between">
            <span>© 2026 OOPBank</span>
          </footer>
        </div>
      </section>
    </main>
  `;
}

export function bindLoginEvents() {
  document.getElementById('tab-login')?.addEventListener('click', () => state.setAuthMode('login'));
  document.getElementById('tab-register')?.addEventListener('click', () => state.setAuthMode('register'));
  
  document.getElementById('btn-toggle-password')?.addEventListener('click', () => {
    state.togglePasswordVisibility();
  });

  document.getElementById('auth-form')?.addEventListener('submit', (e) => {
    e.preventDefault();
    if (state.authMode === 'login') {
      state.setLoggedIn(true); // Đăng nhập thành công -> Vào Trang chủ
    } else {
      state.setFormSuccess(true);
    }
  });
}