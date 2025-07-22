function disp_exp_time(usec) {
    if (usec == 33) usec = 33.333;
    var et = 1000000 / usec;
    return "1/" + Math.round(et);
  }
  
  function disp_agc_val(val) {
    return Math.round(val);
  }
  
  function disp_schedule_val(val) {
    var hour = Math.floor(val / 60);
    var min = val % 60;
    var pval = "";
    if (hour < 10) {
      pval += "0" + hour;
    } else {
      pval += hour;
    }
    pval += ":";
    if (min < 10) {
      pval += "0" + min;
    } else {
      pval += min;
    }
    return pval;
  }
  
  function init_slider(obj, label, min, max, val, put) {
    obj.slider({
      orientation: "horizontal",
      range: "min",
      min: min,
      max: max,
      value: val,
      change: function (event, ui) {
        label.val(ui.value);
  
        put();
      },
    });
    label.val(val);
  }
  
  
  
  function init_range_slider(
    obj,
    label1,
    label2,
    min,
    max,
    val1,
    val2,
    step,
    put
  ) {
    obj.slider({
      orientation: "horizontal",
      range: true,
      min: min,
      max: max,
      step: step,
      values: [val1, val2],
      change: function (event, ui) {
        label1.val(ui.values[0]);
        label2.val(ui.values[1]);
        put();
      },
    });
    label1.val(val1);
    label2.val(val2);
  }
  
  function init_agc_slider(obj, label1, label2, min, max, val1, val2, step, put) {
    obj.slider({
      orientation: "horizontal",
      range: true,
      min: min,
      max: max,
      step: step,
      values: [val1, val2],
      change: function (event, ui) {
        label1.val(disp_agc_val(ui.values[0] / step));
        label2.val(disp_agc_val(ui.values[1] / step));
        put();
      },
    });
    label1.val(disp_agc_val(val1 / step));
    label2.val(disp_agc_val(val2 / step));
  }
  
  function init_exptime_slider(
    obj,
    label1,
    label2,
    min,
    max,
    val1,
    val2,
    step,
    put
  ) {
    obj.slider({
      orientation: "horizontal",
      range: true,
      min: min,
      max: max,
      step: step,
      values: [val1, val2],
      change: function (event, ui) {
        label1.val(disp_exp_time(ui.values[0]));
        label2.val(disp_exp_time(ui.values[1]));
        put();
      },
    });
    label1.val(disp_exp_time(val1));
    label2.val(disp_exp_time(val2));
  }
  
  function init_schedule_slider(
    obj,
    label1,
    label2,
    min,
    max,
    val1,
    val2,
    step,
    put
  ) {
    obj.slider({
      orientation: "horizontal",
      range: true,
      min: min,
      max: max,
      step: step,
      values: [val1, val2],
      change: function (event, ui) {
        label1.val(disp_schedule_val(ui.values[0]));
        label2.val(disp_schedule_val(ui.values[1]));
        put();
      },
    });
    label1.val(disp_schedule_val(val1));
    label2.val(disp_schedule_val(val2));
  }
  
  function enable_slider(obj, label, enable) {
    if (enable) {
      obj.slider({ disabled: false });
      //label.removeAttr("disabled");
    } else {
      obj.slider({ disabled: true });
      //label.attr("disabled", true);
    }
  }
  
  function refresh_slider_value(obj, label, val) {
    obj.slider("value", val);
    label.val(val);
  }
  
  function refresh_range_slider_value(obj, label1, label2, val1, val2, step) {
    obj.slider("values", 0, val1);
    obj.slider("values", 1, val2);
    label1.val(val1);
    label2.val(val2);
  }
  
  function refresh_agc_slider_value(obj, label1, label2, val1, val2, step) {
    obj.slider("values", 0, val1);
    obj.slider("values", 1, val2);
    label1.val(disp_agc_val(val1 / step));
    label2.val(disp_agc_val(val2 / step));
  }
  
  function refresh_exptime_slider_value(obj, label1, label2, val1, val2) {
    obj.slider("values", 0, val1);
    obj.slider("values", 1, val2);
    label1.val(disp_exp_time(val1));
    label2.val(disp_exp_time(val2));
  }
  
  function refresh_schedule_slider_value(obj, label1, label2, val1, val2, step) {
    obj.slider("values", 0, val1);
    obj.slider("values", 1, val2);
    label1.val(disp_schedule_val(val1));
    label2.val(disp_schedule_val(val2));
  }
  
  function init_combobox(obj, array, val) {
  
    var length = Object.keys(array).length;
    for (var i = 0; i < length; i++) {
      var valstr = array[i].split("|");
      obj.append("<option value=" + valstr[0] + ">" + valstr[1] + "</option>");
    }
    obj.val(val).prop("selected", "selected");
  }
  
  function enable_combobox(obj, enable) {
    if (enable) {
      obj.removeAttr("disabled");
      obj.css("background", "#ffffff");
    } else {
      obj.attr("disabled", true);
      obj.css("background", "#f3f3f3");
    }
  }
  
  function refresh_combobox_value(obj, val) {
    obj.val(val);
  }
  