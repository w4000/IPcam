var cur_select_menu = null;

var lastMenuObject = null;
var lastMenuText = null;

var loading_cnt = 0;

$(document).ready(function () {
  // $(document).tooltip();
  // $("#button").button();
  // $("a").button();
  // $("#radioset_mainmenu").buttonset();
  // $("#radioset_submenu_style").buttonset();

  init_setup();

  $(".selectable").selectable({
    selecting: function (event, ui) {
      $(event.target)
        .children(".ui-selecting")
        .not(":first")
        .removeClass("ui-selecting");
    },

    selected: function (event, ui) {
      $(ui.selected).siblings().removeClass("ui-selected");
    },

    stop: function () {
      if ($(".selectable").attr("disabled")) {
        $(".ui-selected").removeClass("ui-selected");
        return;
      }

      $(".ui-selected", this).each(function () {
        var menu = $(this).attr("menu");
        cur_select_menu = menu;

        $(".ui-selected").removeClass("ui-selected");
        $(this).addClass("ui-selected");
        //var index = $(".selectable li").index(this);
        var address = null;

        switch (menu) {
          case "ipc_video_stream":
            address = "/cgi-bin/cgi_ipc_video_stream.cgi";
            break;
          case "av_camera_setup":
            address = "/cgi-bin/cgi_av_camera_setup.cgi";
            break;

          case "netInterFace":
            address = "/cgi-bin/cgi_net_interface.cgi";
            break;
          case "netPort":
            address = "/cgi-bin/cgi_net_port.cgi";
            break;
          case "netProto":
            address = "/cgi-bin/cgi_net_rtsp_onvif.cgi";
            break;

          case "sys_info":
            address = "/cgi-bin/cgi_sys_info.cgi";
            break;
          case "sys_date":
            address = "/cgi-bin/cgi_sys_date.cgi";
            break;
          case "sys_user":
            address = "/cgi-bin/cgi_sys_user.cgi";
            break;
          case "sys_default":
            address = "/cgi-bin/cgi_sys_default.cgi";
            break;
          case "sys_reboot":
            address = "/cgi-bin/cgi_sys_reboot.cgi";
            break;
          case "sys_upgrade":
            address = "/cgi-bin/cgi_sys_upgrade.cgi";
            break;
        }



        if (address) {
          if ($(".selectable").attr("name")) {
            if ($(".selectable").attr("name") == "loading") return;
          }
          $(".selectable").attr("name", "loading");
          var lastSelectedMenuObject = $(this);


          loading_cnt = 0;
          if (timer_loading == null) {
            timer_loading = setInterval(function () {
              $("#_menu_cont").empty();
              $("#_menu_cont").append(
              );

              for (var i = 0; i < loading_cnt; i++)
                $("#_menu_cont").append(".");

              loading_cnt++;
            }, 1000);
          }

          if (timer_refresh_1) clearInterval(timer_refresh_1);
          if (timer_refresh_2) clearInterval(timer_refresh_2);
          if (timer_refresh_3) clearInterval(timer_refresh_3);
          timer_refresh_1 = null;
          timer_refresh_2 = null;
          timer_refresh_3 = null;

          if (upgrade_timer) clearInterval(upgrade_timer);
          upgrade_timer = null;
          if (settime_timer) clearInterval(settime_timer);
          settime_timer = null;

          var menu = $(".ui-selected").attr("menu");
          var title;

          switch (menu) {
            case "av_camera_setup":
              title = menu_camera_setup;
              document.location.href = "../cgi-bin/websetup.cgi?sidemenu=" + menu;
              return;
            case "ipc_video_stream":
              title = menu_video_stream;
              break;
              break;

            case "netInterFace":
              title = menu_interface;
              break;
            case "netPort":
              title = menu_port;
              break;
            case "netProto":
              title = menu_rtsp_onvif;
              break;
            case "net_service":
              title = menu_service;
              break;

            case "sys_info":
              title = menu_info;
              break;
            case "sys_date":
              title = menu_date;
              break;
            case "sys_user":
              title = menu_user;
              break;
            case "sys_default":
              title = menu_default;
              break;
            case "sys_reboot":
              title = menu_reboot;
              break;
            case "sys_upgrade":
              title = menu_upgrade;
              break;
          }


        
          
          $("#menu_name").empty();
          $("#menu_name").append(title);
          $("#ajax_load").show();

          $.ajax({
            type: "POST",
            url: address,
            cache: true,  
            data: {},
            success: function (data) {
              $(".selectable").attr("name", "finish");
              $("#ajax_load").hide();
          
              if (
                data.search("LibCGI Warning") >= 0 ||
                (data.search("refresh") >= 0 && data.search("login.cgi") >= 0)
              ) {
                onunload_body();
                document.location.href = "/cgi-bin/login.cgi";
                return;
              }
          
              clearInterval(timer_loading);
              timer_loading = null;
          
              $("#_menu_cont").empty();
              $("#_menu_cont").append(data);
          
              $(".ui-selected").removeClass("ui-selected");
              lastSelectedMenuObject.addClass("ui-selected");
              lastMenuObject = lastSelectedMenuObject;
              lastMenuText = title;
            },
            error: function (err) {
              $(".selectable").attr("name", "finish");
              $("#ajax_load").hide();
          
              clearInterval(timer_loading);
              timer_loading = null;
          
              if (lastMenuObject) {
                $(".ui-selected").removeClass("ui-selected");
                lastMenuObject.addClass("ui-selected");
          
                $("#menu_name").empty();
                $("#menu_name").append(lastMenuText);
              }
          
              alert(msgAjaxFail);
            },
          });
          



        }
      });
    },
  });

  $(".selectable ul").selectable({
    unselected: function () {
      $(".ui-state-highlight", this).each(function () {
        $(this).removeClass("ui-state-highlight");
      });
    },
    selected: function () {
      $(".ui-selected", this).each(function () {
        $(this).addClass("ui-state-highlight");
      });
    },
  });
  $(".selectable li").hover(
    function () {
      $(this).addClass("ui-state-hover");
    },
    function () {
      $(this).removeClass("ui-state-hover");
    }
  );

  $(".mainmenu").click(function () {
    if ($(".mainmenu").attr("disabled")) {
      $("#radio3").attr("checked", true);
      return;
    }

    var menu = $(this).val();
    //alert( menu );


    switch (menu) {
      case "Live":
        onunload_body();
        $("#radio1").removeAttr("checked");
        $("#radio3").attr("checked", true);
        document.location.href = "../cgi-bin/live_fview.cgi";
        break;

      case "Setup":
        break;

      default:
        break;
    }
  });

  $(".sidemenu_style").click(function () {
    if ($(".sidemenu_style").attr("disabled")) {
      return;
    }

    var menu = $(this).val();

    switch (menu) {
      case "collapse":
        onunload_body();
        document.location.href = "../cgi-bin/websetup.cgi";
        break;

      case "expand":
        onunload_body();
        document.location.href = "../cgi-bin/websetup.cgi?sidemenu_style=1";
        break;
    }
  });

  var leftmenu_status = true;
  $("#mhiden").click(function () {
    $("#td_leftmenu").hide();
    $("#leftmenu").hide("slide", { direction: "left" }, 500);
    $("#mshow").show("slide", { direction: "left" }, 500);

    leftmenu_status = leftmenu_status ? false : true;
  });

  $("#mshow").click(function () {
    $("#td_leftmenu").show();
    $("#leftmenu").show("slide", { direction: "left" }, 500);
    $("#mshow").hide("slide", { direction: "left" }, 500);

    leftmenu_status = leftmenu_status ? false : true;
  });

  {
    $("#_menu_cont").empty();
    $("#_menu_cont").append(
    );
    if (timer_loading == null) {
      timer_loading = setInterval(function () {
        $("#_menu_cont").append(".");
      }, 1000);
    }
  }

if (start_submenu == "ipc_video_stream") {
    if (start_submenu_style == "0") $("#header_av").click();

    $(".ui-selected").removeClass("ui-selected");
    $("li[menu=ipc_video_stream]").addClass("ui-selected");

    var title = menu_video_setup;

    lastMenuObject = $("li[menu=ipc_video_stream]");
    lastMenuText = title;

    $("#menu_title").empty();
    $("#menu_title").append(title);

    $.ajax({
      type: "POST",
      url: "/cgi-bin/cgi_av_ipc_video_stream.cgi",
      data: {},
      success: function (data) {
        clearInterval(timer_loading);
        timer_loading = null;

        $("#menu_content").empty();
        $("#menu_content").append(data);
      },
      error: function (err) {
        clearInterval(timer_loading);
        timer_loading = null;

        alert(msgAjaxFail);
      },
    });
  } 
  else {
    $(".ui-selected").removeClass("ui-selected");
    $("#first_menu").addClass("ui-selected");
    var title = menu_video_stream;

    lastMenuObject = $("#first_menu");
    lastMenuText = title;

    $("#menu_name").empty();
    $("#menu_name").append(title);



    $.ajax({
      type: "POST",
      url: "/cgi-bin/cgi_ipc_video_stream.cgi",
      data: {},
      success: function (data) {
        clearInterval(timer_loading);
        timer_loading = null;

        $("#_menu_cont").empty();
        $("#_menu_cont").append(data);
      },
      error: function (err) {
        clearInterval(timer_loading);
        timer_loading = null;
        
        alert(msgAjaxFail);
      },
    });
  }

  $("#mshow").hide();
  $("#logout").bind("click", onclick_logout);

});




var cfg = ($.hoverintent = {
  sensitivity: 7,
  interval: 100,
});

$.event.special.hoverintent = {
  setup: function () {
    $(this).bind("mouseover", jQuery.event.special.hoverintent.handler);
  },
  teardown: function () {
    $(this).unbind("mouseover", jQuery.event.special.hoverintent.handler);
  },
  handler: function (event) {
    var that = this,
      args = arguments,
      target = $(event.target),
      cX,
      cY,
      pX,
      pY;

    function track(event) {
      cX = event.pageX;
      cY = event.pageY;
    }
    pX = event.pageX;
    pY = event.pageY;
    function clear() {
      target.unbind("mousemove", track).unbind("mouseout", arguments.callee);
      clearTimeout(timeout);
    }
    function handler() {
      if (Math.abs(pX - cX) + Math.abs(pY - cY) < cfg.sensitivity) {
        clear();
        event.type = "hoverintent";
        event.originalEvent = {};
        jQuery.event.handle.apply(that, args);
      } else {
        pX = cX;
        pY = cY;
        timeout = setTimeout(handler, cfg.interval);
      }
    }
    var timeout = setTimeout(handler, cfg.interval);
    target.mousemove(track).mouseout(clear);
    return true;
  },
};

function onchanged_language_setup() {
  var language = $("option:selected", "#lan_setup").val();

  $.ajax({
    type: "POST",
    url: "/cgi-bin/new_setup.cgi",
    data: {
      category: "lanGuage",
      lang: language,
    },
    success: function (data) {
      document.location.href =
        "/cgi-bin/websetup.cgi?sidemenu=sys_info&sidemenu_style=" +
        start_submenu_style;
    },
  });
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

function onload_body() {
  var nAgt = navigator.userAgent;
  var ver_index = nAgt.indexOf("MSIE");
  if (ver_index != -1) {
    var index2 = nAgt.substring(ver_index + 5).indexOf(".");
    if (index2 != -1) {
      var ie_major_version = parseInt(
        nAgt.substring(ver_index + 5).substring(0, index2)
      );
      if (ie_major_version < 9) {
        $("#logo_img").attr("style", "width: 296px;");
      }
    }
  } else {
    $.browser.safari =
      $.browser.webkit && !/chrome/.test(navigator.userAgent.toLowerCase());
    if ($.browser.safari) {
      $("#logo_img").attr("style", "width: 296px;");
    } else {
      $("#logo_img").attr("style", "width: 298px;");
    }

    $("#td_show_menu").hide();
  }
}

function onunload_body() {}
