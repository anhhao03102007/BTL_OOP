(function () {
  // Ngày hiển thị (tiếng Việt)
  var today = new Date();
  var dateText = new Intl.DateTimeFormat('vi-VN', {
    weekday: 'long', day: '2-digit', month: 'long', year: 'numeric'
  }).format(today);
  document.querySelectorAll('[data-date]').forEach(function (el) {
    el.textContent = dateText;
    if (el.tagName === 'TIME') el.setAttribute('datetime', today.toISOString());
  });

  // Mở một đường dẫn trong tab mới (nếu bị chặn popup thì mở ngay trong tab này)
  function openLink(url) {
    var win = window.open(url, '_blank');
    if (win) win.opener = null;
    else window.location.href = url;
  }

  // Bấm vào ô công cụ -> mở đường liên kết
  document.querySelectorAll('.card[href]').forEach(function (card) {
    card.addEventListener('click', function (e) {
      // Giữ hành vi mặc định khi bấm giữa / Ctrl / Cmd / Shift / Alt
      if (e.defaultPrevented || e.button !== 0 || e.ctrlKey || e.metaKey || e.shiftKey || e.altKey) return;
      e.preventDefault();
      openLink(card.href);
    });
  });

  // Nút "Cài đặt Desktop App" -> mở đường dẫn Google Drive chứa app để tải
  // >>> DÁN LINK GOOGLE DRIVE CỦA BẠN VÀO ĐÂY <<<
  // Ví dụ: 'https://drive.google.com/file/d/ABC123xyz/view?usp=sharing'
  var DRIVE_URL = '';

  var help = document.getElementById('install-help');
  document.getElementById('install-btn').addEventListener('click', function () {
    if (!DRIVE_URL) {
      help.innerHTML = 'Chưa có đường dẫn tải về. Hãy dán link Google Drive vào biến <strong>DRIVE_URL</strong> trong file <strong>index.html</strong>.';
      help.hidden = false;
      return;
    }
    help.hidden = true;
    openLink(DRIVE_URL);
  });
})();