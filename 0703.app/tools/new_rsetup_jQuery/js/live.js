


$(document).ready(function () {
  return;

  init_live();

  var icons = {
    header: "ui-icon-circle-plus",
    activeHeader: "ui-icon-circle-arrow-e",
  };
  $(".accordion").accordion({
    heightStyle: "content",
    collapsible: true,
    icons: icons,
  });
  $(".accordion_close").accordion({
    heightStyle: "content",
    collapsible: true,
    active: false,
    icons: icons,
  });




  var leftmenu_status = true;
  $("#mhiden").click(function () {
    $("#td_leftmenu").hide();
    $("#leftmenu").hide("slide", { direction: "left" }, 500);
    $("#mshow").show("slide", { direction: "left" }, 500);

    leftmenu_status = leftmenu_status ? false : true;

    setTimeout(function () {
      win_update = 1;
    }, 600);
    fini_event();
  });




  $(".mainmenu").click(function () {
    var menu = $(this).val();
    //alert( menu );

    switch (menu) {
      case "Live":
        break;

      case "Setup":
        onunload_body();
        $("#radio3").removeAttr("checked");
        $("#radio1").attr("checked", true);
        document.location.href = "../cgi-bin/websetup.cgi"; //"../cgi-bin/system_status.cgi";
        break;

      default:
        break;
    }
  });

  $(".resolution").click(function () {
    var resol = $(this).val();
    //alert(cur_resolution);

    onchanged_resolution(resol);

    return resol;
  });




  $("#select_profile").bind("change", onchanged_profile);
  $("#logout").bind("click", onclick_logout);


});



function onunload_body() {
}



function __logout() {
  secUrl = "/cgi-bin/login.cgi";
  redirUrl = "/cgi-bin/logout_proc.cgi";
  document.execCommand("ClearAuthenticationCache", "false");

  $.ajax({
    async: false,
    url: secUrl,
    type: "GET",
    username: "logout",
  });

  setTimeout(function () {
    window.location.href = redirUrl;
  }, 200);
}

function onclick_logout() {
  onunload_body();

  __logout();
}


var save_last_media = "main";
function onchanged_profile() {
}


function cam_connect(ch) {
}
