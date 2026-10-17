class AppState {
  constructor() {
    this.isLoggedIn = false; // MẶC ĐỊNH LÀ TRANG ĐĂNG NHẬP
    this.authMode = 'login';  // 'login' | 'register'
    this.showPassword = false;
    this.showBalance = true;
    this.currentPage = 'home';
    this.formSuccess = false;
    this.remember = true;
    this.listeners = [];
  }

  subscribe(listener) {
    this.listeners.push(listener);
  }

  notify() {
    this.listeners.forEach(fn => fn(this));
  }

  setLoggedIn(val) {
    this.isLoggedIn = val;
    if (val) this.currentPage = 'home';
    this.notify();
  }

  setAuthMode(mode) {
    this.authMode = mode;
    this.formSuccess = false;
    this.notify();
  }

  togglePasswordVisibility() {
    this.showPassword = !this.showPassword;
    this.notify();
  }

  toggleBalanceVisibility() {
    this.showBalance = !this.showBalance;
    this.notify();
  }

  setCurrentPage(page) {
    this.currentPage = page;
    this.formSuccess = false;
    this.notify();
  }

  setFormSuccess(val) {
    this.formSuccess = val;
    this.notify();
  }
}

export const state = new AppState();