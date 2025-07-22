$(document).ready(function () {
  $(document).tooltip();
  $("#button").button();
  $("a").button();

  $("#tabs").tabs();

  $("#select_language").bind("change", onchanged_language);
});

var did_Submit = 0;
function submit_click() {
  if (did_Submit) return;

  did_Submit = 1;
  var frm = document.login_frm;
  frm.submit();
}

function onload_body() {
  var frm = document.login_frm;
  var appVer = navigator.appVersion;

  frm.login_id.focus();
}

function onunload_body() {}
//w4000_iframe
/*
function _login_form() {
 
  {
    $("#user_id").val($("#login_id").val());
    $("#user_pwd").val($("#login_pwd").val());
  }

  submit_click();
}
*/
function _login_form() {
  if (did_Submit) return;
  did_Submit = 1;

  const formData = new FormData();
  formData.append("user_id", $("#login_id").val());
  formData.append("user_pwd", $("#login_pwd").val());

  fetch("/cgi-bin/login2.cgi", {
    method: "POST",
    body: formData,
  })
    .then((res) => res.text())
    .then((html) => {
      // 로그인 성공 조건에 맞게 수정
      if (html.includes("login success")) {
        // iframe 안에서 websetup.cgi 페이지로 이동
        window.location.href = "/cgi-bin/websetup.cgi";
      } else {
        show_login_fail_msg(1);
        did_Submit = 0;
      }
    })
    .catch((err) => {
      console.error("Login error:", err);
      did_Submit = 0;
    });
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
function onchanged_language() {
  var language = $("option:selected", "#select_language").val();

  $.ajax({
    type: "POST",
    url: "/cgi-bin/new_setup.cgi",
    data: {
      category: "lanGuage",
      lang: language,
    },
    success: function (data) {
      document.location.href = "/cgi-bin/login.cgi";
    },
    error: function (err) {
        console.log(err);
    },
  });
}

function show_login_fail_msg(login_failed) {
  let login_msg = document.getElementById("login_msg");
  if (login_failed == 0) {
    login_msg.style.display = "none";
  } else {
    login_msg.style.display = "block";
  }
}
