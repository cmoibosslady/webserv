(function () {
  function getBrowserName() {
    var ua = navigator.userAgent;
    var match = ua.match(/(opera|chrome|safari|firefox|msie|trident(?=\/))\/?\s*(\d+)/i) || [];
    var tem;

    if (/trident/i.test(match[1])) {
      tem = /\brv[ :]+(\d+)/g.exec(ua) || [];
      return "IE " + (tem[1] || "");
    }

    if (match[1] === "Chrome") {
      tem = ua.match(/\b(OPR|Edge)\/(\d+)/);
      if (tem !== null) {
        return tem.slice(1).join(" ").replace("OPR", "Opera");
      }
    }

    match = match[2] ? [match[1], match[2]] : [navigator.appName, navigator.appVersion, "-?"];
    tem = ua.match(/version\/(\d+)/i);
    if (tem !== null) {
      match.splice(1, 1, tem[1]);
    }

    return match.join(" ");
  }

  var box = document.querySelector(".box");
  var version = document.querySelector(".box .version");
  var monitor = document.querySelector(".monitor");
  var scanWindow = document.querySelector(".scan-window");

  if (!box || !version || !monitor || !scanWindow) {
    return;
  }

  version.textContent = getBrowserName();
  monitor.textContent = "";

  function startMonitorCycle() {
    monitor.textContent = "Forbidden";
    monitor.classList.add("text");

    setInterval(function () {
      if (monitor.classList.contains("text")) {
        monitor.textContent = "403";
        monitor.classList.remove("text");
      } else {
        monitor.textContent = "Forbidden";
        monitor.classList.add("text");
      }
    }, 1000);
  }

  box.style.transition = "right 4s linear";
  requestAnimationFrame(function () {
    box.style.right = "0";
  });

  setTimeout(function () {
    box.classList.add("scanned");
    scanWindow.classList.add("scanning");
    startMonitorCycle();
  }, 2500);
})();
